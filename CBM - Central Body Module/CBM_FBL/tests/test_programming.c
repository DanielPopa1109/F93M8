#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Fbl.h"
#include "Fbl_Updater.h"
#include "Crc.h"
#include "../BSW/Mem/Fls/Fls.h"
#include "../BSW/Mem/MemIf/MemIf.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static uint8_t flash[0x80000],reply[256],image[1024];
static uint16_t replyLen;static int eraseCount,programCount,resetCount,failFlash;

#ifndef FBL_REAL_TRANSPORT
void Can_MainFunction(void){}void CanTp_MainFunction(void){}uint8_t CanTp_IsBusy(void){return 0;}
uint8_t PduR_DcmTransmit(const uint8_t *p,uint16_t n){memcpy(reply,p,n);replyLen=n;return 0;}
#endif
int Fbl_BoardCanTransmit(const uint8_t *p){
#ifdef FBL_REAL_TRANSPORT
 extern int Test_PhysicalTransmit(const uint8_t *);return Test_PhysicalTransmit(p);
#else
 (void)p;return 1;
#endif
}
int Fbl_BoardRead(uint32_t a,void *p,uint32_t n){if(a<FBL_APP_START||a>=FBL_APP_END||n>FBL_APP_END-a)return 1;memcpy(p,flash+(a-0x08000000U),n);return 0;}
int Fbl_BoardErase(void){++eraseCount;if(failFlash)return 1;memset(flash+(FBL_APP_START-0x08000000U),255,FBL_APP_END-FBL_APP_START);return 0;}
int Fbl_BoardProgram(uint32_t a,const void *p,uint32_t n){const uint8_t *v=p;if(failFlash||a<FBL_APP_START||a>=FBL_APP_END||n>FBL_APP_END-a)return 1;for(uint32_t i=0;i<n;++i){uint8_t *d=&flash[a-0x08000000U+i];if((*d&v[i])!=v[i])return 1;*d=v[i];}++programCount;return 0;}
void Fbl_BoardReset(void){++resetCount;}void Fbl_BoardJump(void){}
int Fls_Read(uint32_t a,void *p,uint32_t n){if(a<0x08000000U||a>=0x08080000U||n>0x08080000U-a)return 1;memcpy(p,flash+(a-0x08000000U),n);return 0;}
int Fls_Program(uint32_t a,const void *p,uint32_t n){const uint8_t *v=p;if(failFlash)return 1;for(uint32_t i=0;i<n;++i){uint8_t *d=&flash[a-0x08000000U+i];if((*d&v[i])!=v[i])return 1;*d=v[i];}return 0;}
int Fls_EraseBank(uint8_t b){if(failFlash)return 1;memset(flash+((b?FLS_BANK1:FLS_BANK0)-0x08000000U),255,b?FLS_SIZE1:FLS_SIZE0);return 0;}
static void put32(uint8_t *p,uint32_t v){p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;}
static void send(const uint8_t *p,uint16_t n){replyLen=0;Fbl_RxIndication(p,n);Fbl_MainFunction();CHECK(replyLen);}
static void ack(void){Fbl_TxConfirmation(1);}
static void start(void){uint8_t r[12]={0x31,1,0,1};put32(r+4,FBL_APP_START);put32(r+8,sizeof(image));send(r,12);CHECK(reply[2]==0x78);CHECK(!eraseCount);ack();Fbl_MainFunction();CHECK(eraseCount==1&&reply[0]==0x71);ack();}
static void download(void){uint8_t r[11]={0x34,0,0x44};put32(r+3,FBL_APP_START);put32(r+7,sizeof(image));send(r,11);CHECK(reply[0]==0x74&&reply[2]==1&&reply[3]==0);ack();}
int main(void){
 uint8_t r[256]={0},bsc=1;uint32_t sp=0x2001F000U,pc=FBL_APP_START+9U,crc,off=0;int count;
 setbuf(stdout,0);memset(flash,255,sizeof(flash));memset(image,0xA5,sizeof(image));memcpy(image,&sp,4);memcpy(image+4,&pc,4);crc=Crc_CalculateCRC32(image,sizeof(image),0,1);
 Fbl_Init();CHECK(!Fbl_ApplicationValid());start();download();
 r[0]=0x36;r[1]=2;r[2]=0;send(r,3);CHECK(reply[2]==0x73);ack();
 while(off<sizeof(image)){
  uint16_t n=(uint16_t)(sizeof(image)-off);if(n>254)n=254;
  r[0]=0x36;r[1]=bsc++;memcpy(r+2,image+off,n);send(r,(uint16_t)(n+2));CHECK(reply[0]==0x76);ack();
  count=programCount;send(r,(uint16_t)(n+2));CHECK(reply[0]==0x76&&count==programCount);ack();
  r[2]^=1;send(r,(uint16_t)(n+2));CHECK(reply[2]==0x73);ack();r[2]^=1;
  CHECK(!Fbl_ApplicationValid());off+=n;
 }
 r[0]=0x37;put32(r+1,crc);send(r,5);CHECK(reply[0]==0x7F&&reply[2]==0x78);ack();for(count=0;count<4;++count)Fbl_MainFunction();CHECK(reply[0]==0x77);ack();CHECK(Fbl_ApplicationValid());
 r[0]=0x11;r[1]=1;send(r,2);CHECK(resetCount==0);ack();CHECK(resetCount==1);
 /* Bad TransferExit CRC must leave the image without a valid commit. */
 Fbl_Init();eraseCount=0;start();download();off=0;bsc=1;
 while(off<sizeof(image)){uint16_t n=(uint16_t)(sizeof(image)-off);if(n>254)n=254;r[0]=0x36;r[1]=bsc++;memcpy(r+2,image+off,n);send(r,(uint16_t)(n+2));CHECK(reply[0]==0x76);ack();off+=n;}
 r[0]=0x37;put32(r+1,crc^1U);send(r,5);CHECK(reply[0]==0x7F&&reply[2]==0x72);ack();CHECK(!Fbl_ApplicationValid());
 /* Integrity failures never create a valid app; boot rechecks flash. */
 flash[FBL_APP_START-0x08000000U+50]^=1;CHECK(!Fbl_ApplicationValid());
 Fbl_Init();eraseCount=0;start();download();
 for(count=0;count<6000;++count)Fbl_MainFunction();r[0]=0x36;r[1]=1;r[2]=0;send(r,3);CHECK(reply[2]==0x24);ack();
 Fbl_Init();r[0]=0x31;r[1]=1;r[2]=0;r[3]=1;put32(r+4,0x08000000);put32(r+8,16);send(r,12);CHECK(reply[2]==0x31);ack();
 Fbl_Init();eraseCount=0;failFlash=1;r[0]=0x31;r[1]=1;r[2]=0;r[3]=1;put32(r+4,FBL_APP_START);put32(r+8,1024);send(r,12);ack();Fbl_MainFunction();CHECK(reply[2]==0x72);ack();failFlash=0;
 /* Candidate staging validates bounds and sequence; activation always fails closed. */
 FblUpdater_Init();CHECK(FblUpdater_Begin(FBL_RESIDENT_SIZE+1));CHECK(!FblUpdater_Begin(16));CHECK(FblUpdater_Write(1,image,8));
 {uint8_t candidate[16]={0};uint32_t csp=0x2001F000,cpc=0x08000009;memcpy(candidate,&csp,4);memcpy(candidate+4,&cpc,4);CHECK(!FblUpdater_Write(0,candidate,16));CHECK(!FblUpdater_Validate(Crc_CalculateCRC32(candidate,16,0,1)));CHECK(FblUpdater_Activate());FblUpdater_Init();CHECK(FblUpdater_Validate(0));}
 /* Legacy CBM coding session now commits a durable journal block, preserving the old raw sector. */
 Fbl_Init();Fbl_SetInitialSession(4);
 r[0]=0x31;r[1]=1;r[2]=0;r[3]=1;put32(r+4,0x0800C000U);put32(r+8,24);send(r,12);CHECK(reply[0]==0x71);ack();
 r[0]=0x34;r[1]=0;r[2]=0x44;put32(r+3,0x0800C000U);put32(r+7,24);send(r,11);CHECK(reply[0]==0x74);ack();
 r[0]=0x36;r[1]=1;memset(r+2,0x55,24);send(r,26);CHECK(reply[0]==0x76);ack();
 r[0]=0x37;send(r,1);CHECK(reply[2]==0x78);ack();for(count=0;count<30;++count)Fbl_MainFunction();CHECK(reply[0]==0x77);ack();
 {uint8_t block[24];CHECK(!MemIf_Read(0,3,0,block,24));CHECK(block[0]==0x55&&flash[0xC000]==0xFF);}
 puts("programming: PASS pending-before-erase, FCD 0x44, BSC/duplicate, CRC/commit/boot, address bounds, timeout, reset-confirmation, updater interlock");return 0;
}
