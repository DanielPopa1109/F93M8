#include "Dcm_App.h"
#include "../../Com/Com/Com.h"
#include "CanH.h"
extern CanH_ComStat_t CanH_CommunicationState;
#include "../../../APPL/DiagMonitor/DiagMonitor.h"
#include <string.h>
extern uint32 DataRecorder_KilometerTotal,DataRecorder_KilometerPerDcy;
extern uint8 DigitalCluster_IsSelfTestRequested,RevCam_RxSig_ReverseCameraRequest,DigitalCluster_RxSig_ReverseCameraRequest;
extern uint32 RevCam_DcmiStatus,RevCam_I2cStatus;
extern uint8 RevCam_InitStatus,DigitalCluster_FsmcDmaErrorFlag,DigitalCluster_LcdInit,DigitalCluster_ShutOffDisplayFlag;
extern void EcuM_PerformReset(uint8);
uint16 Dcm_AppReadDid(uint16 did,uint8 *out) {
 uint16 i;
 if(did==0xF180) {out[0]=30;out[1]=34;out[2]=255;out[3]=255;return 4;}
 if(did==5) { for(i=0;i<4;++i) out[i]=(uint8)(Dcm_AliveCounter>>(24-i*8)); return 4; }
 return 0;
}
uint16 Dcm_AppRoutine(const uint8 *p,uint16 n,uint8 *out) {
 uint16 i;
 if(p[1]==1 && p[2]==0x3E && p[3]==0x3E && n==4) DigitalCluster_IsSelfTestRequested=1;
 else if(p[1]==1 && p[2]==0x3F && p[3]==0x3F && n==5) { RevCam_RxSig_ReverseCameraRequest=p[4];DigitalCluster_RxSig_ReverseCameraRequest=p[4]; }
 else if(p[1]==0 && p[2]==0 && p[3]==0x31 && n==7) DataRecorder_KilometerTotal=((uint32)p[6]<<16)|((uint32)p[5]<<8)|p[4];
 else if(p[1]==0 && p[2]==0 && p[3]==0x30 && n==4) {
  out[0]=0x71;memcpy(out+1,p+1,3);
  for(i=0;i<4;++i) {out[4+i]=(uint8)(DataRecorder_KilometerTotal>>(24-i*8));out[8+i]=(uint8)(DataRecorder_KilometerPerDcy>>(24-i*8));}return 12;
 } else return 0;
 out[0]=0x71;memcpy(out+1,p+1,3);return 4;
}
uint16 Dcm_AppPeriodic(const uint8 *p,uint16 n,uint8 *out) {
 if(n!=3 || (p[2]!=12 && p[2]!=13)) return 0;
 out[0]=0x6A;out[1]=p[1];out[2]=p[2];
 if(p[2]==12) {out[3]=(uint8)RevCam_DcmiStatus;out[4]=(uint8)RevCam_I2cStatus;out[5]=RevCam_InitStatus;out[6]=RevCam_RxSig_ReverseCameraRequest;}
 else {out[3]=DigitalCluster_FsmcDmaErrorFlag;out[4]=DigitalCluster_IsSelfTestRequested;out[5]=DigitalCluster_LcdInit;out[6]=DigitalCluster_ShutOffDisplayFlag;}
 return 7;
}
void Dcm_AppCommunication(uint8 control) { CanH_CommunicationState=control?CC_ACTIVE:FULL_COMMUNICATION; Com_IpduGroupControl(0,(uint8)!control); }
void Dcm_AppClearSources(void) { DiagMonitor_ClearFaultSources(); }
void Dcm_AppReset(uint8 session) {
 volatile uint32 *mailbox=(volatile uint32 *)0x1000FFF0U;
 mailbox[0]=0x46424C31U;mailbox[1]=session;mailbox[2]=~(uint32)session;
 EcuM_PerformReset(0);
}
