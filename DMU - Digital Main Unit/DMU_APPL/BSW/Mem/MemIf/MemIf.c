#include "MemIf.h"
#include "../Fls/Fls.h"
#include "../Crc/Crc.h"
#include "../NvM_Engine.h"
#include <string.h>
#define MAGIC 0x314D564EU
#define COMMIT 0x434F4D54U
#define CRC_OFFSET (MEMIF_IMAGE_SIZE-8U)
#define COMMIT_OFFSET (MEMIF_IMAGE_SIZE-4U)
static uint8_t image[MEMIF_IMAGE_SIZE], next[MEMIF_IMAGE_SIZE], check[MEMIF_IMAGE_SIZE];
static uint32_t sequence, latest, target, cursor;
static uint8_t result,state,bank;
static uint32_t get32(const uint8_t *p) { uint32_t v; memcpy(&v,p,4); return v; }
static void set32(uint8_t *p,uint32_t v) { memcpy(p,&v,4); }
static uint16_t blockOffset(uint16_t block) {
    uint16_t i,off=16U;
    for(i=1U;i<block;++i) off=(uint16_t)(off+8U+NvM_BlockDescriptor[i-1U].length);
    return off;
}
static uint8_t valid(const uint8_t *p) {
    return get32(p)==MAGIC && get32(p+COMMIT_OFFSET)==COMMIT &&
        get32(p+CRC_OFFSET)==Crc_CalculateCRC32(p,CRC_OFFSET,0U,1U);
}
void MemIf_Init(void) {
    uint8_t b; uint32_t a,base,size,s;
    memset(image,0xFF,sizeof(image)); sequence=0U; latest=0U; bank=0U;
    for(b=0;b<2U;++b) {
        base=b?FLS_BANK1:FLS_BANK0; size=b?FLS_SIZE1:FLS_SIZE0;
        for(a=base;a+MEMIF_IMAGE_SIZE<=base+size;a+=MEMIF_IMAGE_SIZE) {
            if(Fls_Read(a,check,sizeof(check))) continue;
            s=get32(check+4);
            if(valid(check) && (latest==0U || (int32_t)(s-sequence)>0)) {
                memcpy(image,check,sizeof(image)); sequence=s; latest=a; bank=b;
            }
        }
    }
    result=MEMIF_JOB_OK; state=0U;
}
int MemIf_Read(uint8_t d,uint16_t b,uint16_t off,uint8_t *p,uint16_t n) {
    uint16_t at;
    if(d || !p || b==0U || b>NVM_BLOCK_COUNT || off>NvM_BlockDescriptor[b-1].length ||
        n>NvM_BlockDescriptor[b-1].length-off || state) return 1;
    at=blockOffset(b);
    if(get32(image+at)!=((uint32_t)NvM_BlockDescriptor[b-1].version<<16 | b) ||
        get32(image+at+4)!=NvM_BlockDescriptor[b-1].length) return get32(image+at)==0U?2:1;
    memcpy(p,image+at+8U+off,n); return 0;
}
static int start(uint16_t b,const uint8_t *p,uint8_t invalid) {
    uint16_t at=blockOffset(b); uint32_t a,base,size,i; uint8_t empty;
    memcpy(next,image,sizeof(next));
    set32(next,MAGIC); set32(next+4,sequence+1U);
    set32(next+at,invalid?0U:((uint32_t)NvM_BlockDescriptor[b-1].version<<16 | b));
    set32(next+at+4,NvM_BlockDescriptor[b-1].length);
    if(!invalid) memcpy(next+at+8,p,NvM_BlockDescriptor[b-1].length);
    set32(next+CRC_OFFSET,Crc_CalculateCRC32(next,CRC_OFFSET,0,1));
    set32(next+COMMIT_OFFSET,COMMIT);
    base=bank?FLS_BANK1:FLS_BANK0; size=bank?FLS_SIZE1:FLS_SIZE0;
    target=0U;
    for(a=latest?latest+MEMIF_IMAGE_SIZE:base;a+MEMIF_IMAGE_SIZE<=base+size;a+=MEMIF_IMAGE_SIZE) {
        if(Fls_Read(a,check,sizeof(check))) return 1;
        empty=1U; for(i=0;i<sizeof(check);++i) if(check[i]!=0xFFU) { empty=0U; break; }
        if(empty) { target=a; break; }
    }
    state=2U;
    if(!target) { target=bank?FLS_BANK0:FLS_BANK1; state=1U; }
    cursor=0U; result=MEMIF_JOB_PENDING; return 0;
}
int MemIf_Write(uint8_t d,uint16_t b,const uint8_t *p) {
    if(d || !p || !b || b>NVM_BLOCK_COUNT || state) return 1;
    if(!MemIf_Read(d,b,0,check,NvM_BlockDescriptor[b-1].length) &&
        !memcmp(check,p,NvM_BlockDescriptor[b-1].length)) { result=MEMIF_JOB_OK; return 0; }
    return start(b,p,0);
}
int MemIf_InvalidateBlock(uint8_t d,uint16_t b) {
    if(d || !b || b>NVM_BLOCK_COUNT || state) return 1;
    return start(b,0,1);
}
void MemIf_MainFunction(void) {
    uint32_t count;
    if(!state) return;
    if(state==1U) {
        if(Fls_EraseBank(target==FLS_BANK1?1U:0U)) goto fail;
        state=2U; return;
    }
    if(state==2U) {
        count=COMMIT_OFFSET-cursor; if(count>64U) count=64U;
        if(Fls_Program(target+cursor,next+cursor,count)) goto fail;
        cursor+=count; if(cursor==COMMIT_OFFSET) state=3U; return;
    }
    if(state==3U) {
        if(Fls_Read(target,check,COMMIT_OFFSET) || memcmp(next,check,COMMIT_OFFSET)) goto fail;
        if(Fls_Program(target+COMMIT_OFFSET,next+COMMIT_OFFSET,4U)) goto fail;
        state=4U; return;
    }
    if(Fls_Read(target,check,sizeof(check)) || !valid(check)) goto fail;
    memcpy(image,next,sizeof(image)); sequence=get32(image+4); latest=target;
    bank=target>=FLS_BANK1?1U:0U; state=0U; result=MEMIF_JOB_OK; return;
fail:
    state=0U; result=MEMIF_JOB_FAILED;
}
uint8_t MemIf_GetJobResult(uint8_t d) { return d?MEMIF_JOB_FAILED:result; }
uint8_t MemIf_GetStatus(uint8_t d) { return d?0U:(state?2U:1U); }
