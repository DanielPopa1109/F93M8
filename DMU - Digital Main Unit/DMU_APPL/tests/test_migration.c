#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NvM_Engine.h"
#include "Fls/Fls.h"
#include "Dem.h"
#define assert(x) do{if(!(x)){printf("migration FAIL %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static uint8_t flash[0x80000];static int failAfter=-1,programs,erases;
uint32 DataRecorder_KilometerTotal,DataRecorder_KilometerPerDcy;
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
int main(void){
 uint32 legacy[DEM_LEGACY_ARRAY_SIZE]={0};uint8 status;
 setbuf(stdout,0);memset(flash,255,sizeof(flash));legacy[0]=Dem_Config.events[0].eventId;legacy[1]=2;
 {uint32 mileage[2]={123456,789};memcpy(flash+0xC000,legacy,sizeof(legacy));memcpy(flash+0x8000,mileage,sizeof(mileage));}
 boot();assert(DataRecorder_KilometerTotal==123456&&DataRecorder_KilometerPerDcy==789);
 assert(!Dem_GetEventUdsStatus(Dem_Config.events[0].eventId,&status));assert(status&DEM_UDS_STATUS_CDTC);
 NvM_WriteAll();drain();memset(DEM_LEGACY_ARRAY,0,sizeof(DEM_LEGACY_ARRAY));boot();assert(!Dem_GetEventUdsStatus(Dem_Config.events[0].eventId,&status));assert(status&DEM_UDS_STATUS_CDTC);assert(status&DEM_UDS_STATUS_TNCTOC);assert(!(status&DEM_UDS_STATUS_TFTOC));
 puts("migration: PASS real ECU descriptors, legacy data, journal restore and operation-cycle reset");return 0;}
