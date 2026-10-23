#include "NvM_Engine.h"
#include "MemIf/MemIf.h"
#include <string.h>
#ifdef DEM_HOST_TEST
static uint32_t enter(void){return 0;}static void leave(uint32_t irq){(void)irq;}
#else
#include "main.h"
static uint32_t enter(void){uint32_t irq=__get_PRIMASK();__disable_irq();return irq;}
static void leave(uint32_t irq){if(!irq)__enable_irq();}
#endif
#define QUEUE_SIZE 8U
typedef struct { uint16_t block; uint8_t op; void *dst; uint8_t data[NVM_MAX_BLOCK_LENGTH]; } Job;
static Job jobs[QUEUE_SIZE];
static uint8_t head,tail,count,active,retry,delay,dirty[NVM_BLOCK_COUNT],multi,multiIndex;
static NvM_RequestResultType results[NVM_BLOCK_COUNT];
uint32_t NvM_LastWriteResult;
uint8_t NvM_Ready;
void NvM_Init(void) {
    uint16_t i,total=16U;
    head=tail=count=active=retry=delay=multi=0U; NvM_Ready=0U; NvM_LastWriteResult=0U;
    for(i=0;i<NVM_BLOCK_COUNT;++i) {
        results[i]=NVM_REQ_OK; dirty[i]=0U;
        total=(uint16_t)(total+8U+NvM_BlockDescriptor[i].length);
        if(!NvM_BlockDescriptor[i].ram || NvM_BlockDescriptor[i].length>NVM_MAX_BLOCK_LENGTH) return;
    }
    if(total>MEMIF_IMAGE_SIZE-8U) return;
    MemIf_Init(); NvM_Ready=1U;
}
static uint8_t enqueue(uint16_t b,uint8_t op,const void *src,void *dst) {
    Job *j; uint32_t irq=enter();
    if(!NvM_Ready || !b || b>NVM_BLOCK_COUNT || count==QUEUE_SIZE || results[b-1]==NVM_REQ_PENDING) {leave(irq);return 1U;}
    j=&jobs[tail]; j->block=b; j->op=op; j->dst=dst;
    if(op==2U) memcpy(j->data,src?src:NvM_BlockDescriptor[b-1].ram,NvM_BlockDescriptor[b-1].length);
    tail=(uint8_t)((tail+1U)%QUEUE_SIZE); ++count; results[b-1]=NVM_REQ_PENDING;leave(irq);return 0U;
}
uint8_t NvM_ReadBlock(uint16_t b,void *p) { return enqueue(b,1,0,p); }
uint8_t NvM_WriteBlock(uint16_t b,const void *p) { return enqueue(b,2,p,0); }
uint8_t NvM_InvalidateNvBlock(uint16_t b) { return enqueue(b,3,0,0); }
uint8_t NvM_SetRamBlockStatus(uint16_t b,uint8_t changed) {
    if(!b || b>NVM_BLOCK_COUNT) return 1; dirty[b-1]=changed!=0; return 0;
}
uint8_t NvM_GetErrorStatus(uint16_t b,NvM_RequestResultType *p) {
    if(!p || !b || b>NVM_BLOCK_COUNT) return 1; *p=results[b-1]; return 0;
}
uint8_t NvM_RestoreBlockDefaults(uint16_t b,void *p) {
    if(!b || b>NVM_BLOCK_COUNT || results[b-1]==NVM_REQ_PENDING) return 1;
    if(!p) p=NvM_BlockDescriptor[b-1].ram;
    if(NvM_BlockDescriptor[b-1].defaults) memcpy(p,NvM_BlockDescriptor[b-1].defaults,NvM_BlockDescriptor[b-1].length);
    else memset(p,0,NvM_BlockDescriptor[b-1].length);
    dirty[b-1]=1; return 0;
}
void NvM_ReadAll(void) { if(NvM_Ready && !multi) { multi=1U; multiIndex=0U; } }
void NvM_WriteAll(void) {
    if(NvM_Ready && !multi) { NvM_ApplicationCapture(); multi=2U; multiIndex=0U; NvM_LastWriteResult=2U; }
}
uint8_t NvM_IsIdle(void) { return count==0U && multi==0U; }
void NvM_EngineMainFunction(void) {
    Job *j; const NvM_BlockDescriptorType *cfg; uint8_t r; uint16_t b;
    if(!NvM_Ready) return;
    if(multi && multiIndex<NVM_BLOCK_COUNT) {
        b=(uint16_t)(multiIndex+1U); cfg=&NvM_BlockDescriptor[multiIndex];
        if(!(multi==1U?cfg->readAll:cfg->writeAll)) ++multiIndex;
        else if(results[b-1]!=NVM_REQ_PENDING && !enqueue(b,multi,0,0)) ++multiIndex;
    }
    if(!count) {
        if(multi && multiIndex==NVM_BLOCK_COUNT) {
            r=multi; multi=0;
            if(r==1U) NvM_ApplicationRestore();
            else if(NvM_LastWriteResult==2U) NvM_LastWriteResult=0U;
        }
        return;
    }
    j=&jobs[head]; b=j->block; cfg=&NvM_BlockDescriptor[b-1];
    if(j->op==1U) {
        void *dst=j->dst?j->dst:cfg->ram;
        r=(uint8_t)MemIf_Read(0,b,0,dst,cfg->length);
        if(r) {
            if(cfg->defaults) memcpy(dst,cfg->defaults,cfg->length); else memset(dst,0,cfg->length);
            if(!j->dst && r!=2U) NvM_ApplicationMigrate(b);
            results[b-1]=NVM_REQ_RESTORED_FROM_ROM; dirty[b-1]=1U;
        } else results[b-1]=NVM_REQ_OK;
        goto done;
    }
    if(delay) { --delay; return; }
    if(!active) {
        r=j->op==3U?MemIf_InvalidateBlock(0,b):MemIf_Write(0,b,j->data);
        if(r) goto failed;
        active=1U;
    }
    MemIf_MainFunction(); r=MemIf_GetJobResult(0);
    if(r==MEMIF_JOB_PENDING) return;
    if(r!=MEMIF_JOB_OK) goto failed;
    results[b-1]=j->op==3U?NVM_REQ_NV_INVALIDATED:NVM_REQ_OK;
    dirty[b-1]=(uint8_t)(j->op==2U && memcmp(cfg->ram,j->data,cfg->length)!=0);
    goto done;
failed:
    active=0U;
    if(++retry<3U) { delay=(uint8_t)(retry*10U); return; }
    results[b-1]=NVM_REQ_NOT_OK; NvM_LastWriteResult=1U; dirty[b-1]=1U;
done:
    {uint32_t irq=enter();head=(uint8_t)((head+1U)%QUEUE_SIZE);--count;active=retry=delay=0U;leave(irq);}
}
