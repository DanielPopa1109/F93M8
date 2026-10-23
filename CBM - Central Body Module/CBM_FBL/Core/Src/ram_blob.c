/* ELC separation: this translation unit owns the updater protocol and state.
 * No resident HAL, libc, interrupt, timer or diagnostic callback is used. */
#include "Fbl_Ram.h"
typedef struct {
 uint8_t image[FBL_RESIDENT_SIZE],rx[256],last[254];
 uint32_t size,written,crc,rxAge,idle,lastPending;
 uint16_t have,expected,lastLen;
 uint8_t session,authorized,download,verified,committed,sn,bsc,lastBsc,lastValid,failed;
} Ram_State;
static RAM_BSS Ram_State state;
static RAM_CODE void copy(uint8_t *d,const uint8_t *s,uint32_t n) { while(n--)*d++=*s++; }
static RAM_CODE uint32_t be32(const uint8_t *p) {return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static RAM_CODE uint32_t le32(const uint8_t *p) {return ((uint32_t)p[3]<<24)|((uint32_t)p[2]<<16)|((uint32_t)p[1]<<8)|p[0];}
static RAM_CODE uint32_t crc32(const uint8_t *p,uint32_t n,uint32_t crc) {
 crc=~crc;while(n--){crc^=*p++;for(uint8_t i=0;i<8;i++)crc=(crc>>1)^((0U-(crc&1U))&0xEDB88320U);}return ~crc;
}
static RAM_CODE int vectors(void) {
 uint32_t sp=le32(state.image),pc=le32(state.image+4);
 if(state.size<RAM_IMAGE_TAG_OFFSET+RAM_IMAGE_TAG_BYTES+2U || sp!=RAM_RESIDENT_STACK_TOP || (sp&7U) || !(pc&1U) ||
  (pc&~1U)<RAM_FLASH_BASE+RAM_IMAGE_TAG_OFFSET+RAM_IMAGE_TAG_BYTES || (pc&~1U)>=RAM_FLASH_BASE+state.size)return 0;
 const uint8_t *tag=state.image+RAM_IMAGE_TAG_OFFSET;
 if(le32(tag)!=RAM_IMAGE_MAGIC || le32(tag+4)!=1 || le32(tag+8)!=RAM_TARGET || le32(tag+12)!=RAM_MCU_ID ||
    le32(tag+16)!=FBL_RESIDENT_SIZE || le32(tag+20)!=FBL_APP_START || le32(tag+24)!=FBL_METADATA ||
    le32(tag+28)!=RAM_RESIDENT_VECTOR_WORDS)return 0;
 for(uint32_t i=2;i<RAM_RESIDENT_VECTOR_WORDS;i++){
  uint32_t handler=le32(state.image+4*i);if(!handler)continue;
  if(!(handler&1U) || (handler&~1U)<RAM_FLASH_BASE+RAM_IMAGE_TAG_OFFSET+RAM_IMAGE_TAG_BYTES || (handler&~1U)>=RAM_FLASH_BASE+state.size)return 0;
 }return 1;
}
static RAM_CODE int reply(const uint8_t *p,uint8_t n) {
 uint8_t f[8];for(uint8_t i=0;i<8;i++)f[i]=0;
 f[0]=0xF0;f[1]=n;copy(f+2,p,n);return Ram_CanSend(f);
}
static RAM_CODE void negative(uint8_t sid,uint8_t code) {uint8_t p[3];p[0]=0x7F;p[1]=sid;p[2]=code;(void)reply(p,3);}
static RAM_CODE int pending(uint8_t sid) {uint8_t p[3];p[0]=0x7F;p[1]=sid;p[2]=0x78;return reply(p,3);}
RAM_CODE int Ram_KeepAlive(void) {
 uint32_t now=Ram_Millis();
 if(now-state.lastPending<500)return 0;
 state.lastPending=now;return pending(0x37);
}
static RAM_CODE void abortTransfer(void) {
 state.download=state.authorized=state.verified=state.lastValid=0;state.expected=state.have=0;
}
RAM_CODE void Ram_Init(void) {
 /* Explicit initialization also works after an abandoned updater invocation. */
 uint8_t *p=(uint8_t *)&state;for(uint32_t i=0;i<sizeof(state);i++)p[i]=0;
 state.session=2;state.bsc=1;
}
static RAM_CODE int commit(void) {
 uint8_t check[128];uint32_t at=0,crc=0;
 state.committed=0;
 if(Ram_Erase())return 1;
 /* Reset vectors are programmed last. A partial image is never reset deliberately. */
 while(at<state.size-8U){
  uint32_t n=state.size-8U-at;if(n>128)n=128;
  if(Ram_Program(RAM_FLASH_BASE+8+at,state.image+8+at,n) || Ram_KeepAlive())return 1;at+=n;
 }
 if(Ram_Program(RAM_FLASH_BASE,state.image,8))return 1;
 for(at=0;at<state.size;){
  uint32_t n=state.size-at;if(n>sizeof(check))n=sizeof(check);
  if(Ram_Read(RAM_FLASH_BASE+at,check,n))return 1;
  for(uint32_t i=0;i<n;i++)if(check[i]!=state.image[at+i])return 1;
  crc=crc32(check,n,crc);at+=n;
 }
 if(crc!=state.crc)return 1;
 state.committed=1;return 0;
}
static RAM_CODE void process(const uint8_t *p,uint16_t n) {
 uint8_t sid=p[0],r[6];uint32_t a,z;
 state.idle=0;
 switch(sid) {
 case 0x10:
  if(n!=2){negative(sid,0x13);return;}
  if(p[1]!=1 && p[1]!=2 && p[1]!=3){negative(sid,0x12);return;}
  if(state.download){negative(sid,0x22);return;}
  state.session=p[1];r[0]=0x50;r[1]=p[1];r[2]=0;r[3]=50;r[4]=1;r[5]=244;(void)reply(r,6);return;
 case 0x3E:
  if(n!=2){negative(sid,0x13);return;}if(p[1]==0x80)return;
  if(p[1]){negative(sid,0x12);return;}r[0]=0x7E;r[1]=0;(void)reply(r,2);return;
 case 0x22:
  if(n!=3){negative(sid,0x13);return;}
  if(p[1]!=0xF1 || (p[2]!=0x86 && p[2]!=0)){negative(sid,0x31);return;}
  r[0]=0x62;r[1]=p[1];r[2]=p[2];r[3]=p[2]==0x86?state.session:2;(void)reply(r,4);return;
 case 0x31:
  if(state.session!=2){negative(sid,0x7F);return;}
  if(n<4 || p[1]!=1){negative(sid,0x13);return;}
  if(p[2]==2 && p[3]==0 && n==5 && p[4]==2){r[0]=0x71;copy(r+1,p+1,4);(void)reply(r,5);return;}
  if(p[2]==1 && p[3]==0x55 && n==4){r[0]=0x71;copy(r+1,p+1,3);r[4]=0;(void)reply(r,5);return;}
  if(p[2]!=0){negative(sid,0x31);return;}
  if(p[3]==1 && n==12){
   a=be32(p+4);z=be32(p+8);
   if(a!=RAM_FLASH_BASE || z<8 || z>FBL_RESIDENT_SIZE){negative(sid,0x31);return;}
   if(state.download){negative(sid,0x22);return;}
   abortTransfer();state.size=z;state.written=0;state.authorized=1;state.committed=0;state.failed=0;
   /* Full image is buffered/validated before physical erase on TransferExit. */
   r[0]=0x71;copy(r+1,p+1,3);r[4]=0;(void)reply(r,5);return;
  }
  if(p[3]==2 && n==16){
   if(!state.committed || be32(p+4)!=RAM_FLASH_BASE || be32(p+8)!=state.size || be32(p+12)!=state.crc){negative(sid,0x22);return;}
   r[0]=0x71;copy(r+1,p+1,3);r[4]=0;(void)reply(r,5);return;
  }negative(sid,0x31);return;
 case 0x34:
  if(state.session!=2){negative(sid,0x7F);return;}
  if(n!=11 || p[1] || p[2]!=0x44){negative(sid,0x13);return;}
  if(!state.authorized || state.download){negative(sid,0x22);return;}
  if(be32(p+3)!=RAM_FLASH_BASE || be32(p+7)!=state.size){negative(sid,0x31);return;}
  state.written=0;state.bsc=1;state.lastValid=0;state.download=1;state.crc=0;
  r[0]=0x74;r[1]=0x20;r[2]=1;r[3]=0;(void)reply(r,4);return;
 case 0x36:
  if(!state.download){negative(sid,0x24);return;}
  if(n<3 || n>256){negative(sid,0x13);return;}
  if(state.lastValid && p[1]==state.lastBsc){
   if(n-2!=state.lastLen){negative(sid,0x73);return;}
   for(uint16_t i=0;i<n-2;i++)if(p[i+2]!=state.last[i]){negative(sid,0x73);return;}
  }else{
   if(p[1]!=state.bsc){negative(sid,0x73);return;}
   if((uint32_t)(n-2)>state.size-state.written){negative(sid,0x31);return;}
   copy(state.image+state.written,p+2,n-2);copy(state.last,p+2,n-2);
   state.lastLen=n-2;state.lastBsc=p[1];state.lastValid=1;state.bsc++;state.written+=n-2;
  }r[0]=0x76;r[1]=p[1];(void)reply(r,2);return;
 case 0x37:
  if(state.committed && n==5 && be32(p+1)==state.crc){r[0]=0x77;(void)reply(r,1);return;}
  if(!state.download || state.written!=state.size){negative(sid,0x24);return;}
  /* FCD must enable TransferExit CRC for FBL replacement. */
  if(n!=5){negative(sid,0x13);return;}
  state.crc=crc32(state.image,state.size,0);
  if(state.crc!=be32(p+1) || !vectors()){abortTransfer();negative(sid,0x72);return;}
  state.verified=1;
  if(pending(sid)){abortTransfer();return;}
  state.lastPending=Ram_Millis();
  state.download=state.authorized=0;
  if(commit()){state.failed=1;negative(sid,0x72);return;}
  state.verified=0;r[0]=0x77;(void)reply(r,1);return;
 case 0x11:
  if(n!=2){negative(sid,0x13);return;}if(p[1]!=1 && p[1]!=3){negative(sid,0x12);return;}
  if(!state.committed || state.download || state.failed){negative(sid,0x22);return;}
  r[0]=0x51;r[1]=p[1];if(!reply(r,2))Ram_Reset();return;
 default:negative(sid,0x11);return;
 }
}
RAM_CODE void Ram_OnCan(uint16_t id,uint8_t dlc,const uint8_t *p) {
 uint8_t type,count;uint16_t n;
 if(!p || id!=0x710 || dlc<2 || dlc>8 || p[0]!=RAM_TARGET)return;
 type=p[1]>>4;count=p[1]&15;
 if(type==0){
  state.expected=state.have=0;if(!count || count>6 || count>dlc-2)return;
  copy(state.rx,p+2,count);process(state.rx,count);return;
 }
 if(type==1){
  state.expected=state.have=0;if(dlc!=8)return;n=((uint16_t)count<<8)|p[2];
  if(n<=6 || n>256)return;
  state.expected=n;state.have=5;state.sn=1;state.rxAge=0;copy(state.rx,p+3,5);
  {uint8_t fc[8];for(uint8_t i=0;i<8;i++)fc[i]=0;fc[0]=0xF0;fc[1]=0x30;fc[3]=5;
   if(Ram_CanSend(fc))state.expected=state.have=0;}
  return;
 }
 if(type==2){
  if(!state.expected || count!=state.sn || dlc<3){state.expected=state.have=0;return;}
  n=state.expected-state.have;if(n>6)n=6;
  if(dlc<n+2){state.expected=state.have=0;return;}
  copy(state.rx+state.have,p+2,n);state.have+=n;state.sn=(state.sn+1)&15;state.rxAge=0;
  if(state.have==state.expected){n=state.expected;state.expected=state.have=0;process(state.rx,n);}return;
 }
 /* All responses fit an EA single frame: no segmented TX needs incoming FC.
  * Gateway FC EA 0x41 is therefore intentionally ignored in this bounded profile. */
}
RAM_CODE void Ram_Tick(uint32_t ms) {
 if(ms>30000)ms=30000;
 state.idle+=ms;if(state.expected){state.rxAge+=ms;if(state.rxAge>=1000){state.expected=state.have=0;}}
 if(state.idle>=30000){abortTransfer();state.session=1;state.idle=30000;}
}
RAM_CODE void Ram_Start(void) {
 Ram_Init();if(Ram_CanInit())for(;;){}
 /* Resident acknowledged 0155 with NRC78 before handover. Final response is
  * sent here, after SRAM exclusively owns the controller and RX buffers. */
 uint8_t ready[5];ready[0]=0x71;ready[1]=1;ready[2]=1;ready[3]=0x55;ready[4]=0;
 (void)reply(ready,5);
 uint32_t before=Ram_Millis();
 for(;;){uint16_t id;uint8_t dlc,p[8];uint32_t now=Ram_Millis();
  Ram_Tick(now-before);before=now;if(Ram_CanReceive(&id,&dlc,p)>0)Ram_OnCan(id,dlc,p);}
}
