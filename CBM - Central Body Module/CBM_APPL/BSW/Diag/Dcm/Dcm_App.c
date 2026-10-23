#include "Dcm_App.h"
#include "../../Com/Com/Com.h"
#include "CanSpi.h"
#include "InputCtrl.h"
#include "ActCtrl.h"
#include "Ain.h"
#include "Nvm.h"
#include "EcuM.h"
#include <string.h>
uint16 Dcm_AppReadDid(uint16 did,uint8 *out) {
 uint16 i; uint32 value;
 if(did==0xF180) { out[0]=30;out[1]=32;out[2]=30;out[3]=30;return 4; }
 if(did==5) { value=Dcm_AliveCounter; for(i=0;i<4;++i) out[i]=(uint8)(value>>(24U-i*8U)); return 4; }
 if(did==2) { for(i=0;i<6;++i) out[i]=(uint8)NvMBlock_Coding[i]; return 6; }
 if(did==3) { for(i=0;i<10;++i) out[i]=(uint8)NvMBlock_Calibration[i]; return 10; }
 return 0;
}
uint16 Dcm_AppRoutine(const uint8 *p,uint16 n,uint8 *out) { (void)p;(void)n;(void)out; return 0; }
uint16 Dcm_AppPeriodic(const uint8 *p,uint16 n,uint8 *out) {
 uint8 id=p[2],index;
 if(n!=3 || id>6 || (p[1]!=1 && p[1]!=4)) return 0;
 out[0]=0x6A; out[1]=p[1]; out[2]=id;
 if(id==0 || id==1 || id==5 || id==6) {
  index=id==0?0:(id==1?2:(id==5?1:3));
  out[3]=(uint8)StatusList_ComOutValue[index]; out[4]=(uint8)StatusList_RawValue[index];
  out[5]=StatusList_InputStatus[index].errorStatus; out[6]=StatusList_InputStatus[index].DCYStatus; return 7;
 }
 if(id==2 || id==3) { index=id==2?2:0; out[3]=CmdList_RawValue[index];out[4]=CmdList_RawValue[index+1];out[5]=CmdList_ActualValue[index];out[6]=CmdList_ActualValue[index+1];return 7; }
 out[3]=CmdList_RawValue[4];out[4]=CmdList_ActualValue[4];return 5;
}
void Dcm_AppCommunication(uint8 control) { CanSpi_Communication_Status=control?CC_ACTIVE:FULL_COMMUNICATION; Com_IpduGroupControl(0,(uint8)!control); }
void Dcm_AppClearSources(void) { /* Faults still present are reported again by their monitors. */ }
void Dcm_AppReset(uint8 session) {
 volatile uint32 *mailbox=(volatile uint32 *)0x2001FFF0U;
 mailbox[0]=0x46424C31U;mailbox[1]=session;mailbox[2]=~(uint32)session;
 EcuM_PerformReset(0);
}
