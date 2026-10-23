#include "Fbl.h"
#include "Fbl_Updater.h"
#include "../../BSW/Crc/Crc.h"
#include "../../BSW/Com/CanTp/CanTp.h"
#include "../../BSW/Com/Can/Can.h"
#include "../../BSW/Com/PduR/PduR.h"
#include <string.h>
#if FBL_HAS_PARAMETER_BLOCKS
#include "../../BSW/Mem/MemIf/MemIf.h"
static uint8_t parameterData[40],parameterBlock,parameterCommit;
#endif
#define VALID_MARK 0x41505031U
static uint8_t req[256],out[256],last[254];
static uint16_t reqLen,outLen,lastLen;
static uint8_t ready,sendReady,flight,pendingResponse,eraseWaiting,eraseAuthorized,resetWaiting,updaterWaiting;
static uint8_t session,selected,download,bsc,lastBsc,lastValid;
static uint32_t address,size,offset,lastOffset,crc;
static uint16_t transferTimer,s3;
static uint8_t verifyWaiting,verifySid,verifyCommit;
static uint32_t verifyAddress,verifyLength,verifyOffset,verifyCrc,verifyExpected;
static uint32_t be32(const uint8_t *p) { return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }
static uint16_t nrc(uint8_t sid,uint8_t code) { out[0]=0x7F;out[1]=sid;out[2]=code;return 3; }
static uint8_t range(uint32_t a,uint32_t n) { return n && a>=FBL_APP_START && a<FBL_METADATA && n<=FBL_METADATA-a; }
void Fbl_Init(void) {
 ready=sendReady=flight=eraseWaiting=eraseAuthorized=resetWaiting=download=lastValid=0;
 updaterWaiting=0;verifyWaiting=0;session=2;selected=1;bsc=1;offset=0;transferTimer=s3=0;FblUpdater_Init();
#if FBL_HAS_PARAMETER_BLOCKS
 parameterBlock=parameterCommit=0;MemIf_Init();
#endif
}
void Fbl_RxIndication(const uint8_t *p,uint16_t n) {
 if(!p||!n||n>sizeof(req)||ready||sendReady||flight||eraseWaiting||verifyWaiting || parameterCommit) return;
 memcpy(req,p,n);reqLen=n;ready=1;s3=0;
}
void Fbl_SetInitialSession(uint8_t value) {
 if(value==2 || (FBL_HAS_PARAMETER_BLOCKS && (value==4 || value==5)))session=value;
}
uint8_t Fbl_ApplicationValid(void) {
 uint32_t m[4],vectors[2],count,check=0;uint8_t chunk[256];
 if(Fbl_BoardRead(FBL_METADATA,m,sizeof(m)) || m[3]!=VALID_MARK || !range(FBL_APP_START,m[0]) ||
    Fbl_BoardRead(FBL_APP_START,vectors,8)) return 0;
 if(vectors[0]<0x20000000U || vectors[0]>0x20020000U || (vectors[0]&7U) || !(vectors[1]&1U) ||
  (vectors[1]&~1U)<FBL_APP_START || (vectors[1]&~1U)>=FBL_APP_START+m[0]) return 0;
 for(count=0;count<m[0];) {
  uint32_t n=m[0]-count;if(n>sizeof(chunk)) n=sizeof(chunk);
  if(Fbl_BoardRead(FBL_APP_START+count,chunk,n)) return 0;
  check=Crc_CalculateCRC32(chunk,n,check,count==0);count+=n;
 }
 return check==m[1] && m[2]==~m[0];
}
static uint16_t process(void) {
 uint8_t sid=req[0];uint16_t n=reqLen;uint32_t a,z,expected;
 switch(sid) {
 case 0x10:
  if(n!=2) return nrc(sid,0x13);
  if(req[1]!=1 && req[1]!=2 && req[1]!=3 && !(FBL_HAS_PARAMETER_BLOCKS && (req[1]==4 || req[1]==5))) return nrc(sid,0x12);
  if(download) return nrc(sid,0x22);
  session=req[1];out[0]=0x50;out[1]=session;out[2]=0;out[3]=50;out[4]=1;out[5]=244;return 6;
 case 0x28:
  if(n!=3)return nrc(sid,0x13);
  if(req[1]>1 || (req[2]!=1 && req[2]!=3))return nrc(sid,0x31);
  out[0]=0x68;out[1]=req[1];return 2; /* FBL has diagnostic-only traffic. */
 case 0x85:
  if(n!=2)return nrc(sid,0x13);
  if(req[1]!=1 && req[1]!=2)return nrc(sid,0x12);
  out[0]=0xC5;out[1]=req[1];return 2; /* FBL owns no DEM event monitoring. */
 case 0x3E:
  if(n!=2) return nrc(sid,0x13);
  if(req[1]==0x80) return 0;
  if(req[1]!=0) return nrc(sid,0x12);
  out[0]=0x7E;out[1]=0;return 2;
 case 0x22:
  if(n!=3) return nrc(sid,0x13);
  if(req[1]!=0xF1 || req[2]!=0x86) return nrc(sid,0x31);
  out[0]=0x62;out[1]=0xF1;out[2]=0x86;out[3]=session;return 4;
 case 0x31:
  if(session!=2 && !(FBL_HAS_PARAMETER_BLOCKS && (session==4 || session==5))) return nrc(sid,0x7F);
  if(n<4 || req[1]!=1) return nrc(sid,0x13);
  if(req[2]==2 && req[3]==0 && n==5) {
   if(req[4]!=1 && req[4]!=2) return nrc(sid,0x31);
   if(download) return nrc(sid,0x22);
   selected=req[4];out[0]=0x71;memcpy(out+1,req+1,4);return 5;
  }
  if(req[2]==1 && req[3]==0x55) {
   if(n!=4)return nrc(sid,0x13);
   if(session!=2 || selected!=2 || download || !FBL_UPDATER_ACTIVATION_ALLOWED)return nrc(sid,0x22);
   if(FblUpdater_Prepare())return nrc(sid,0x72);
   /* RAM emits final 0155 only after it owns CAN; the client cannot race handover. */
   updaterWaiting=1;return nrc(sid,0x78);
  }
  if(req[2]!=0) return nrc(sid,0x31);
  if(req[3]==1 && n==12) {
   a=be32(req+4);z=be32(req+8);
#if FBL_HAS_PARAMETER_BLOCKS
   if(session==4 || session==5) {
    if(a!=(session==4?0x0800C000U:0x08008000U) || z!=(session==4?24U:40U))return nrc(sid,0x31);
    eraseAuthorized=1;out[0]=0x71;memcpy(out+1,req+1,3);out[4]=0;return 5;
   }
#endif
   if(selected==2) {
    return nrc(sid,0x22); /* Enter the independent updater through RID 0155 first. */
   }
   if(!range(a,z) || a!=FBL_APP_START) return nrc(sid,0x31);
   download=eraseAuthorized=lastValid=0;eraseWaiting=1;pendingResponse=1;return nrc(sid,0x78);
  }
  if(req[3]==2 && n==16) {
   uint8_t bytes[256];uint32_t at,check=0;
   a=be32(req+4);z=be32(req+8);expected=be32(req+12);
#if FBL_HAS_PARAMETER_BLOCKS
   if(session==4 || session==5) {
    uint8_t block=session==4?3:2;
    if(a!=(session==4?0x0800C000U:0x08008000U) || z!=(session==4?24U:40U))return nrc(sid,0x31);
    if(MemIf_Read(0,block,0,bytes,(uint16_t)z) || Crc_CalculateCRC32(bytes,z,0,1)!=expected)return nrc(sid,0x72);
    out[0]=0x71;memcpy(out+1,req+1,3);out[4]=0;return 5;
   }
#endif
   if(!range(a,z)) return nrc(sid,0x31);
   (void)at;(void)check;(void)bytes;
   verifyAddress=a;verifyLength=z;verifyOffset=verifyCrc=0;verifyExpected=expected;
   verifySid=0x31;verifyCommit=0;verifyWaiting=1;pendingResponse=1;return nrc(sid,0x78);
  } return nrc(sid,0x31);
 case 0x34:
  if(session!=2 && !(FBL_HAS_PARAMETER_BLOCKS && (session==4 || session==5))) return nrc(sid,0x7F);
  if(n!=11 || req[1]!=0 || req[2]!=0x44) return nrc(sid,0x13);
  if(download || !eraseAuthorized) return nrc(sid,0x22);
  a=be32(req+3);z=be32(req+7);
#if FBL_HAS_PARAMETER_BLOCKS
  parameterBlock=0;
  if(session==4 || session==5) {
   parameterBlock=session==4?3:2;
   if(a!=(session==4?0x0800C000U:0x08008000U) || z!=(session==4?24U:40U))return nrc(sid,0x31);
  } else
#endif
  if(selected==1) { if(a!=FBL_APP_START || !range(a,z)) return nrc(sid,0x31); }
  else return nrc(sid,0x22);
  address=a;size=z;offset=crc=0;bsc=1;lastValid=0;download=1;transferTimer=0;
  out[0]=0x74;out[1]=0x20;out[2]=1;out[3]=0;return 4; /* maxBlockLength=256 incl SID/BSC */
 case 0x36:
  if(!download) return nrc(sid,0x24);
  if(n<3 || n>256) return nrc(sid,0x13);
  if(lastValid && req[1]==lastBsc) {
   if(n-2!=lastLen || memcmp(last,req+2,lastLen)) return nrc(sid,0x73);
   out[0]=0x76;out[1]=lastBsc;transferTimer=0;return 2;
  }
  if(req[1]!=bsc) return nrc(sid,0x73);
  if((uint32_t)(n-2)>size-offset) return nrc(sid,0x31);
#if FBL_HAS_PARAMETER_BLOCKS
  if(parameterBlock)memcpy(parameterData+offset,req+2,n-2);
  else
#endif
  if(selected==1) {
   if(Fbl_BoardProgram(address+offset,req+2,n-2)) {download=0;return nrc(sid,0x72);}
  } else if(FblUpdater_Write(offset,req+2,n-2)) return nrc(sid,0x72);
  lastOffset=offset;(void)lastOffset;lastLen=(uint16_t)(n-2);memcpy(last,req+2,lastLen);lastBsc=bsc;lastValid=1;
  crc=Crc_CalculateCRC32(req+2,n-2,crc,offset==0);offset+=n-2;++bsc;transferTimer=0;
  out[0]=0x76;out[1]=req[1];return 2;
 case 0x37:
  if(!download || offset!=size) return nrc(sid,0x24);
  if(n!=1 && n!=5) return nrc(sid,0x13);
  if(n==5 && be32(req+1)!=crc) { download=0;return nrc(sid,0x72); }
#if FBL_HAS_PARAMETER_BLOCKS
  if(parameterBlock) {
   if(MemIf_Write(0,parameterBlock,parameterData))return nrc(sid,0x72);
   parameterCommit=1;download=0;pendingResponse=1;return nrc(sid,0x78);
  }
#endif
  if(selected==2) {
   if(n!=5 || FblUpdater_Validate(crc)) return nrc(sid,0x72);
   download=0;return nrc(sid,0x22); /* Validated RAM candidate, resident activation disabled. */
  }
  verifyAddress=address;verifyLength=size;verifyOffset=verifyCrc=0;verifyExpected=crc;
  verifySid=0x37;verifyCommit=1;verifyWaiting=1;download=0;pendingResponse=1;return nrc(sid,0x78);
 case 0x11:
  if(n!=2) return nrc(sid,0x13);
  if(req[1]!=1 && req[1]!=3) return nrc(sid,0x12);
  if(download || !Fbl_ApplicationValid()) return nrc(sid,0x22);
  out[0]=0x51;out[1]=req[1];resetWaiting=1;return 2;
 default:return nrc(sid,0x11);
 }
}
void Fbl_TxConfirmation(uint8_t success) {
 flight=0;
 if(!success)updaterWaiting=0;
 if(!success) {eraseWaiting=resetWaiting=verifyWaiting=0;
#if FBL_HAS_PARAMETER_BLOCKS
 parameterCommit=0;
#endif
 return;}
#if FBL_HAS_PARAMETER_BLOCKS
 if(parameterCommit && pendingResponse){parameterCommit=2;pendingResponse=0;return;}
#endif
 if(updaterWaiting==1){updaterWaiting=2;return;}
 if(pendingResponse && verifyWaiting){verifyWaiting=2;pendingResponse=0;return;}
 if(pendingResponse && eraseWaiting) { eraseWaiting=2;pendingResponse=0; }
 else if(resetWaiting) {resetWaiting=0;Fbl_BoardReset();}
}
void Fbl_MainFunction(void) {
 if(updaterWaiting==2){updaterWaiting=0;if(FblUpdater_Activate()){outLen=nrc(0x31,0x72);sendReady=1;}}
 Can_MainFunction();CanTp_MainFunction();
#if FBL_HAS_PARAMETER_BLOCKS
 if(parameterCommit==2 && !flight && !sendReady) {
  MemIf_MainFunction();
  if(MemIf_GetJobResult(0)!=MEMIF_JOB_PENDING) {
   outLen=MemIf_GetJobResult(0)==MEMIF_JOB_OK?1:nrc(0x37,0x72);
   if(outLen==1)out[0]=0x77;
   parameterCommit=eraseAuthorized=0;sendReady=1;
  }
 }
#endif
 if(verifyWaiting==2 && !flight && !sendReady) {
  uint8_t bytes[256];uint16_t budget=1024;uint8_t error=0;
  while(verifyOffset<verifyLength && budget) {
   uint32_t n=verifyLength-verifyOffset;if(n>sizeof(bytes))n=sizeof(bytes);
   if(Fbl_BoardRead(verifyAddress+verifyOffset,bytes,n)){error=1;break;}
   verifyCrc=Crc_CalculateCRC32(bytes,n,verifyCrc,verifyOffset==0);verifyOffset+=n;budget=(uint16_t)(budget-n);
  }
  if(error || verifyOffset==verifyLength) {
   if(!error && verifyCrc!=verifyExpected)error=1;
   if(!error && verifyCommit) {
    uint32_t vectors[2],metadata[4]={verifyLength,verifyCrc,~verifyLength,VALID_MARK};
    if(Fbl_BoardRead(FBL_APP_START,vectors,8) || vectors[0]<0x20000000U || vectors[0]>0x20020000U || (vectors[0]&7U) ||
       !(vectors[1]&1U) || (vectors[1]&~1U)<FBL_APP_START || (vectors[1]&~1U)>=FBL_APP_START+verifyLength ||
       Fbl_BoardProgram(FBL_METADATA,metadata,12) || Fbl_BoardProgram(FBL_METADATA+12,metadata+3,4))error=1;
   }
   if(error)outLen=nrc(verifySid,0x72);
   else if(verifySid==0x37){out[0]=0x77;outLen=1;}
   else {out[0]=0x71;out[1]=1;out[2]=0;out[3]=2;out[4]=0;outLen=5;}
   verifyWaiting=eraseAuthorized=0;sendReady=1;
  }
 }
 if(download && ++transferTimer>=6000) {download=0;eraseAuthorized=0;lastValid=0;}
 if(!download && !eraseWaiting && ++s3>=1000) {s3=1000;session=1;}
 if(eraseWaiting==2 && !flight && !sendReady) {
  eraseWaiting=0;eraseAuthorized=Fbl_BoardErase()==0;
  if(!eraseAuthorized) outLen=nrc(0x31,0x72);
  else {out[0]=0x71;out[1]=1;out[2]=0;out[3]=1;out[4]=0;outLen=5;}
  sendReady=1;
 }
 if(ready && !sendReady && !flight) { ready=0;outLen=process();if(outLen)sendReady=1; }
 if(sendReady && !CanTp_IsBusy() && PduR_DcmTransmit(out,outLen)==CAN_OK) {sendReady=0;flight=1;}
}
