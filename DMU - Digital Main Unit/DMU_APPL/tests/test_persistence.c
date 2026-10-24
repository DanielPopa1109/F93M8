#include <stdlib.h>
#define assert(x) do { if(!(x)) { printf("FAIL line %d: %s\n",__LINE__,#x); fflush(stdout); exit(1); } } while(0)
#include <stdio.h>
#include <string.h>
#include "NvM_Engine.h"
#include "Fls/Fls.h"
#include "Crc/Crc.h"
#include "MemIf/MemIf.h"
#include "Dem.h"
static unsigned statusCallbacks;
void Dem_ApplicationStatusChanged(Dem_EventIdType event,uint8 before,uint8 after){(void)event;assert(before!=after);++statusCallbacks;}
uint32 DEM_LEGACY_ARRAY[DEM_LEGACY_ARRAY_SIZE];
uint8_t NvM_DemImage[512];
static uint8_t ram[2][40],flash[0x80000];
static int failAfter=-1,programs,erases;
static uint8_t beforeRotation[0x80000];
const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_BLOCK_COUNT]={
 {512,2,NvM_DemImage,0,1,1}, {8,1,ram[0],0,1,1},
};
void NvM_ApplicationCapture(void) { Dem_Serialize(NvM_DemImage); }
void NvM_ApplicationRestore(void) { Dem_Init(&Dem_Config); (void)Dem_Deserialize(NvM_DemImage); }
void NvM_ApplicationMigrate(uint16_t b) { (void)b; }
int Fls_Read(uint32_t a,void *p,uint32_t n) { assert(a>=0x08000000 && a+n<=0x08080000); memcpy(p,flash+(a-0x08000000),n); return 0; }
int Fls_Program(uint32_t a,const void *p,uint32_t n) {
 const uint8_t *src=p; uint32_t i;
 assert((a&3)==0 && (n&3)==0);
 if(failAfter==0) return 1; if(failAfter>0) --failAfter;
 ++programs;
 for(i=0;i<n;++i) { uint8_t *d=&flash[a-0x08000000+i]; assert((*d&src[i])==src[i]); *d=src[i]; }
 return 0;
}
int Fls_EraseBank(uint8_t b) { ++erases; if(failAfter==0) return 1; memset(flash+((b?FLS_BANK1:FLS_BANK0)-0x08000000),255,b?FLS_SIZE1:FLS_SIZE0); return 0; }
static void drain(void) { unsigned i; for(i=0;i<10000 && !NvM_IsIdle();++i) { NvM_EngineMainFunction(); Dem_MainFunction(); } assert(NvM_IsIdle()); }
static void boot(void) { NvM_Init(); Dem_PreInit(); NvM_ReadAll(); drain(); }
static void save(uint8_t value) { memset(ram[0],value,8); assert(!NvM_WriteBlock(2,0)); drain(); }
int main(void) {
 setbuf(stdout,0);
 uint8_t snapshot[6],after[6]; uint16 size; uint8_t status; int before,k;
 assert(!NvM_IsIdle()); /* Uninitialized storage must not appear ready for shutdown. */
 assert(Crc_CalculateCRC32((const uint8_t *)"123456789",9,0,1)==0xCBF43926U);
 memset(flash,255,sizeof(flash)); boot(); save(0x11); boot(); assert(ram[0][0]==0x11);
 before=programs; save(0x11); assert(programs==before);
 /* Every write cut point before commit must leave the previous committed data. */
 for(k=0;k<18;++k) {
  save(0x11); memset(ram[0],0x22,8); assert(!NvM_WriteBlock(2,0));
  for(int t=0;t<k;++t) NvM_EngineMainFunction();
  boot(); assert(ram[0][0]==0x11 || ram[0][0]==0x22);
 }
 save(0x33); save(0x44);
 /* Corrupt latest payload; scanner must recover the previous committed record. */
 { uint32_t a,last=0; for(a=FLS_BANK0;a<FLS_BANK0+FLS_SIZE0;a+=1024) if(flash[a-0x08000000]!=0xFFU) last=a;
   if(last) flash[last-0x08000000+40]^=1; }
 boot(); assert(ram[0][0]==0x33);
 /* Cut every operation in a bank rotation. Previous bank must remain readable. */
 memset(flash,255,sizeof(flash));boot();
 for(k=0;k<(int)(FLS_SIZE0/1024U);++k)save((uint8_t)k);
 memcpy(beforeRotation,flash,sizeof(flash));
 for(k=0;k<20;++k){
  memcpy(flash,beforeRotation,sizeof(flash));boot();memset(ram[0],0xE1,8);assert(!NvM_WriteBlock(2,0));
  for(int t=0;t<k;++t)NvM_EngineMainFunction();
  boot();assert(ram[0][0]==(uint8_t)(FLS_SIZE0/1024U-1U)||ram[0][0]==0xE1);
 }
 /* Force bank rotation, then recover after reset. */
 for(k=0;k<160;++k) save((uint8_t)k);
 assert(erases>0); boot(); assert(ram[0][0]==159);
 assert(Dem_SetEventStatus(Dem_Config.events[0].eventId,DEM_EVENT_STATUS_FAILED)==E_OK);
 Dem_MainFunction(); assert(statusCallbacks>0); size=6; assert(!Dem_GetSnapshotDataByDTC(Dem_Config.events[0].dtc,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY,1,snapshot,&size));
 NvM_WriteAll(); drain(); boot();
 size=6; assert(!Dem_GetSnapshotDataByDTC(Dem_Config.events[0].dtc,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY,1,after,&size)); assert(!memcmp(after,snapshot,6));
 assert(!Dem_GetEventUdsStatus(Dem_Config.events[0].eventId,&status)); assert(status&DEM_UDS_STATUS_CDTC);
 assert(!Dem_SelectDTC(0,DEM_DTC_GROUP_ALL_DTCS,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY));
 assert(Dem_ClearDTC(0)==DEM_PENDING); assert(Dem_GetClearDTCStatus()==DEM_CLEAR_PENDING); drain(); Dem_MainFunction(); assert(Dem_GetClearDTCStatus()==DEM_CLEAR_OK); boot();
 assert(!Dem_GetEventUdsStatus(Dem_Config.events[0].eventId,&status)); assert(!(status&DEM_UDS_STATUS_CDTC));
 assert(!Dem_SetEventStatus(Dem_Config.events[0].eventId,DEM_EVENT_STATUS_FAILED)); NvM_WriteAll(); drain();
 failAfter=0; assert(!Dem_SelectDTC(0,DEM_DTC_GROUP_ALL_DTCS,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY)); assert(Dem_ClearDTC(0)==DEM_PENDING); drain(); Dem_MainFunction(); assert(Dem_GetClearDTCStatus()==DEM_CLEAR_RESULT_FAILED);
 failAfter=-1; boot(); assert(!Dem_GetEventUdsStatus(Dem_Config.events[0].eventId,&status)); assert(status&DEM_UDS_STATUS_CDTC);
 assert(!NvM_InvalidateNvBlock(2)); drain(); boot(); assert(ram[0][0]==0);
 puts("persistence: PASS CRC, cut points, corruption fallback, rotation, DEM reset/clear/failure, invalidate"); return 0;
}
