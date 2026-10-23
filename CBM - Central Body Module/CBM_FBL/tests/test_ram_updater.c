#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Fbl_Ram.h"
#include "Fbl_Updater.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static uint8_t flash[0x80000],candidate[FBL_RESIDENT_SIZE],response[8];
static uint32_t length,erases,programs,resets,pendingConfirmed,firstProgram,lastProgram;
static int failErase,failProgram,failRead,corruptRead,failTx,frames,simulateDelay;
static uint32_t mockMillis;
RAM_CODE int Ram_CanInit(void){return 0;}
RAM_CODE int Ram_CanReceive(uint16_t *i,uint8_t *n,uint8_t *p){(void)i;(void)n;(void)p;return 0;}
RAM_CODE uint32_t Ram_Millis(void){return mockMillis;}
RAM_CODE int Ram_CanSend(const uint8_t *p){
 memcpy(response,p,8);frames++;
 CHECK(p[0]==0xF0);
 if(failTx)return 1;
 if(p[1]==3 && p[2]==0x7F && p[3]==0x37 && p[4]==0x78)pendingConfirmed++;
 return 0;
}
RAM_CODE int Ram_Erase(void){
 CHECK(pendingConfirmed);erases++;if(failErase)return 1;
 if(simulateDelay){mockMillis+=600;CHECK(!Ram_KeepAlive());}
 memset(flash,255,FBL_RESIDENT_SIZE);return 0;
}
RAM_CODE int Ram_Program(uint32_t at,const uint8_t *p,uint32_t n){
 CHECK(at>=RAM_FLASH_BASE && at<RAM_FLASH_BASE+FBL_RESIDENT_SIZE && n<=RAM_FLASH_BASE+FBL_RESIDENT_SIZE-at);
 if(!programs){firstProgram=at;}lastProgram=at;programs++;if(failProgram)return 1;
 for(uint32_t i=0;i<n;i++){uint8_t *d=&flash[at-RAM_FLASH_BASE+i];CHECK((*d&p[i])==p[i]);*d=p[i];}return 0;
}
RAM_CODE int Ram_Read(uint32_t at,uint8_t *p,uint32_t n){
 CHECK(at>=RAM_FLASH_BASE && at<RAM_FLASH_BASE+FBL_RESIDENT_SIZE && n<=RAM_FLASH_BASE+FBL_RESIDENT_SIZE-at);
 if(failRead)return 1;memcpy(p,flash+(at-RAM_FLASH_BASE),n);if(corruptRead)p[0]^=1;return 0;
}
RAM_CODE void Ram_Reset(void){resets++;}
static void put32(uint8_t *p,uint32_t v){p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;}
static uint32_t crc32(const uint8_t *p,uint32_t n){uint32_t crc=~0U;while(n--){crc^=*p++;for(int i=0;i<8;i++)crc=(crc>>1)^((0U-(crc&1))&0xEDB88320U);}return ~crc;}
static void send(const uint8_t *p,uint16_t n){
 uint8_t f[8]={0},sn=1;uint16_t at;frames=0;
 f[0]=RAM_TARGET;
 if(n<=6){f[1]=(uint8_t)n;memcpy(f+2,p,n);Ram_OnCan(0x710,8,f);return;}
 f[1]=0x10|(uint8_t)(n>>8);f[2]=(uint8_t)n;memcpy(f+3,p,5);Ram_OnCan(0x710,8,f);
 CHECK(response[1]==0x30 && response[2]==0 && response[3]==5);
 for(at=5;at<n;){uint16_t count=n-at;if(count>6)count=6;memset(f+2,0,6);f[1]=0x20|sn;sn=(sn+1)&15;memcpy(f+2,p+at,count);Ram_OnCan(0x710,8,f);at+=count;}
}
static void nrc(uint8_t code){CHECK(response[1]==3 && response[2]==0x7F && response[4]==code);}
static void fresh(uint32_t n){
 Ram_Init();length=n;erases=programs=resets=pendingConfirmed=0;firstProgram=lastProgram=0;
 failErase=failProgram=failRead=corruptRead=failTx=simulateDelay=0;mockMillis=0;memset(flash,0xA5,sizeof(flash));memset(candidate,0x59,sizeof(candidate));
 uint32_t sp=0x2001FC00,pc=RAM_FLASH_BASE+0x301;memcpy(candidate,&sp,4);memcpy(candidate+4,&pc,4);
 for(uint32_t i=2;i<RAM_RESIDENT_VECTOR_WORDS;i++)memcpy(candidate+4*i,&pc,4);
 uint32_t tag[8]={RAM_IMAGE_MAGIC,1,RAM_TARGET,RAM_MCU_ID,FBL_RESIDENT_SIZE,FBL_APP_START,FBL_METADATA,RAM_RESIDENT_VECTOR_WORDS};
 memcpy(candidate+RAM_IMAGE_TAG_OFFSET,tag,sizeof(tag));
}
static void begin(void){
 uint8_t r[12]={0x31,1,0,1};put32(r+4,RAM_FLASH_BASE);put32(r+8,length);send(r,12);CHECK(response[2]==0x71 && !erases);
 r[0]=0x34;r[1]=0;r[2]=0x44;put32(r+3,RAM_FLASH_BASE);put32(r+7,length);send(r,11);CHECK(response[2]==0x74 && response[4]==1 && response[5]==0);
}
static void transfer(void){
 uint8_t r[256],bsc=1;for(uint32_t at=0;at<length;){uint32_t n=length-at;if(n>254)n=254;
  r[0]=0x36;r[1]=bsc++;memcpy(r+2,candidate+at,n);send(r,(uint16_t)(n+2));CHECK(response[2]==0x76);
  send(r,(uint16_t)(n+2));CHECK(response[2]==0x76);r[2]^=1;send(r,(uint16_t)(n+2));nrc(0x73);r[2]^=1;at+=n;}
 CHECK(!erases && !programs);
}
static void finish(uint32_t crc){uint8_t r[5]={0x37};put32(r+1,crc);send(r,5);}
static void reset(void){uint8_t r[2]={0x11,1};send(r,2);}
int main(int argc,char **argv){
 uint8_t r[256]={0},f[8]={0};uint32_t crc,keep;
 setbuf(stdout,0);
 CHECK(!FblUpdater_ValidateLayout(0x08001000,0x08002000,RAM_BASE,RAM_BASE+4096,RAM_STACK_TOP,RAM_BASE+513));
 CHECK(FblUpdater_ValidateLayout(0x08001000,0x08002000,RAM_BASE,RAM_BASE+4096,RAM_STACK_TOP,RAM_BASE+512));
 CHECK(FblUpdater_ValidateLayout(0x08001000,0x08002000,RAM_BASE,RAM_BASE+4096,RAM_STACK_TOP-8,RAM_BASE+513));
 CHECK(FblUpdater_ValidateLayout(0x08001000,0x08002000,RAM_BASE,RAM_BASE+4096,RAM_STACK_TOP,0x08000101));
 CHECK(FblUpdater_ValidateLayout(0x08001000,0x08002000,RAM_BASE+4,RAM_BASE+4100,RAM_STACK_TOP,RAM_BASE+513));
 CHECK(FblUpdater_ValidateLayout(0x08007FFC,0x08009000,RAM_BASE,RAM_BASE+4100,RAM_STACK_TOP,RAM_BASE+513));
 CHECK(FblUpdater_ValidateLayout(0x08001000,0x08002000,RAM_BASE,RAM_STACK_BOTTOM+4,RAM_STACK_TOP,RAM_BASE+513));
 fresh(sizeof(candidate));reset();nrc(0x22);CHECK(!resets);begin();
 r[0]=0x36;r[1]=2;r[2]=0;send(r,3);nrc(0x73);
 r[0]=0x37;send(r,1);nrc(0x24);
 transfer();crc=crc32(candidate,length);finish(crc);CHECK(response[2]==0x77 && erases==1);
 CHECK(firstProgram==RAM_FLASH_BASE+8 && lastProgram==RAM_FLASH_BASE);
 CHECK(!memcmp(flash,candidate,length));for(uint32_t i=FBL_RESIDENT_SIZE;i<sizeof(flash);i++)CHECK(flash[i]==0xA5);
 keep=erases;finish(crc);CHECK(response[2]==0x77 && erases==keep);
 failTx=1;reset();CHECK(!resets);failTx=0;reset();CHECK(resets==1);
 /* Power/reset loses updater state and never authorizes another reset from it. */
 Ram_Init();reset();nrc(0x22);CHECK(resets==1);
 fresh(1024);begin();transfer();finish(crc32(candidate,length)^1);nrc(0x72);CHECK(!erases);reset();nrc(0x22);
 for(int invalid=0;invalid<8;invalid++){
  fresh(1024);if(invalid==0)candidate[0]|=1;if(invalid==1)memset(candidate,0,4);
  if(invalid==2)candidate[4]&=0xFE;if(invalid==3)memset(candidate+4,255,4);
  if(invalid==4)memset(candidate+12,255,4);
  if(invalid==5)candidate[RAM_IMAGE_TAG_OFFSET+8]^=1;
  if(invalid==6)candidate[RAM_IMAGE_TAG_OFFSET+12]^=1;
  if(invalid==7){uint32_t lowStack=0x20008000;memcpy(candidate,&lowStack,4);}
  begin();transfer();finish(crc32(candidate,length));nrc(0x72);CHECK(!erases);
 }
 /* A confirmed 0x78 is mandatory before physical erase. */
 fresh(1024);begin();transfer();failTx=1;finish(crc32(candidate,length));CHECK(!erases && !programs);
 fresh(1024);begin();transfer();simulateDelay=1;finish(crc32(candidate,length));CHECK(response[2]==0x77 && pendingConfirmed>=2);
 for(int failure=0;failure<4;failure++){
  fresh(1024);begin();transfer();failErase=failure==0;failProgram=failure==1;failRead=failure==2;corruptRead=failure==3;
  finish(crc32(candidate,length));nrc(0x72);CHECK(erases==1);reset();nrc(0x22);CHECK(!resets);
  /* Unexpected reset after erase needs physical recovery, never a success flag. */
  Ram_Init();reset();nrc(0x22);CHECK(!resets);
 }
 /* Bounds, partial transfer, S3 and N_Cr expiration. */
 fresh(1024);r[0]=0x31;r[1]=1;r[2]=0;r[3]=1;put32(r+4,RAM_FLASH_BASE+16384);put32(r+8,1024);send(r,12);nrc(0x31);
 put32(r+4,RAM_FLASH_BASE);put32(r+8,FBL_RESIDENT_SIZE+1);send(r,12);nrc(0x31);CHECK(!erases);
 begin();r[0]=0x36;r[1]=1;r[2]=0;send(r,3);CHECK(response[2]==0x76);r[0]=0x37;send(r,1);nrc(0x24);
 Ram_Tick(30000);r[0]=0x36;r[1]=2;r[2]=0;send(r,3);nrc(0x24);CHECK(!erases);
 fresh(1024);f[0]=RAM_TARGET;f[1]=0x10;f[2]=11;f[3]=0x34;f[4]=0;f[5]=0x44;Ram_OnCan(0x710,8,f);CHECK(response[1]==0x30);
 Ram_Tick(1000);frames=0;f[1]=0x21;Ram_OnCan(0x710,8,f);CHECK(!frames);
 f[1]=0x10;f[2]=11;Ram_OnCan(0x710,8,f);frames=0;f[1]=0x22;Ram_OnCan(0x710,8,f);CHECK(!frames);
 frames=0;f[0]=RAM_TARGET^1;f[1]=2;f[2]=0x10;f[3]=2;Ram_OnCan(0x710,8,f);CHECK(!frames);
 f[0]=RAM_TARGET;Ram_OnCan(0x711,8,f);CHECK(!frames);f[0]=0x41;f[1]=0x30;Ram_OnCan(0x710,8,f);CHECK(!frames);
 if(argc==2){
  FILE *file=fopen(argv[1],"rb");CHECK(file);fresh(sizeof(candidate));
  length=(uint32_t)fread(candidate,1,sizeof(candidate),file);CHECK(length>=512 && fgetc(file)==EOF);CHECK(!fclose(file));
  begin();transfer();finish(crc32(candidate,length));CHECK(response[2]==0x77 && !memcmp(flash,candidate,length));reset();CHECK(resets==1);
  puts("RAM updater: actual linked FBL binary replacement PASS");
 }
 puts("RAM updater: PASS full-image SF/FF/CF/FC, replacement/CRC/readback, vectors-last, retransmission, invalid vectors/ranges/BSC, timeout/interruption, erase/program/readback/TX failures and confirmed reset");
 return 0;
}
