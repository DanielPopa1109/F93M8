/* Real DCM/PduR/CanTp/CanIf/Can/NvM/DEM; only physical hardware and ECU I/O are mocked. */
#define main persistence_suite
#include "test_persistence.c"
#undef main
#include "Dcm.h"
#include "Com.h"
#include "Can.h"
#include "CanIf.h"
#include "CanTp.h"
#include "PduR.h"
#include "CanNm.h"
#include "ComM.h"
#include "CanSM.h"
static uint8_t wire[8];static uint16_t wireId;static unsigned sent,resetCalls,appRxCalls;
Can_StatusType Can_TestTransmit(uint16_t id,const uint8_t *p,uint8_t n){assert(n<=8);memcpy(wire,p,n);wireId=id;++sent;return CAN_OK;}
void Com_AppRx(uint16_t id){(void)id;++appRxCalls;}void Com_AppTx(void){}
uint16 Dcm_AppReadDid(uint16 did,uint8 *p){if(did==0xF180){p[0]=30;return 1;}return 0;}
uint16 Dcm_AppRoutine(const uint8 *p,uint16 n,uint8 *out){(void)p;(void)n;(void)out;return 0;}
uint16 Dcm_AppPeriodic(const uint8 *p,uint16 n,uint8 *out){(void)p;(void)n;(void)out;return 0;}
void Dcm_AppCommunication(uint8 p){Com_SetTxEnabled((uint8)!p);}
void Dcm_AppReset(uint8 s){(void)s;++resetCalls;}void Dcm_AppClearSources(void){}
static void confirm(void){Can_TxConfirmation(1);Can_MainFunction();}
static void rxframe(const uint8_t *p,uint8_t n){Can_FrameType f={0};f.id=0x710;f.dlc=n;memcpy(f.data,p,n);Can_RxIndication(&f);Can_MainFunction();}
static void uds(const uint8_t *p,uint8_t n){uint8_t f[8]={CANTP_TARGET_ADDRESS};f[1]=n;memcpy(f+2,p,n);rxframe(f,(uint8_t)(n+2));Dcm_MainFunction();}
int main(void){
 uint8_t req[6]={0x10,3},frame[8]={0},data[6]={0};uint32_t value;unsigned before;uint16_t resultSize;
 (void)resultSize;
 persistence_suite();boot();Dcm_Init();Can_Init();CanIf_Init();CanTp_Init();PduR_Init();Com_Init();CanNm_Init();ComM_Init();CanSM_Init();
 uds(req,2);assert(wireId==0x710&&wire[0]==0xF0&&wire[2]==0x50);before=sent;
 /* Mailbox acceptance has not completed transport; duplicate response must be busy. */
 assert(CanTp_IsBusy());assert(PduR_DcmTransmit(req,2)!=CAN_OK);assert(sent==before);confirm();assert(!CanTp_IsBusy());
 /* An empty UDS status mask selects no DTCs; DEM's zero-mask all-DTC
  * filter remains reserved for reportSupportedDTC (19 0A). */
 req[0]=0x19;req[1]=1;req[2]=0;uds(req,3);
 assert(wire[2]==0x59&&wire[3]==1&&wire[6]==0&&wire[7]==0);confirm();
 req[1]=2;uds(req,3);assert(wire[2]==0x59&&wire[3]==2&&wire[1]==3);confirm();
 /* Stored record 1 is encoded with one configured metadata DID. */
 req[1]=4;req[2]=(uint8_t)(Dem_Config.events[0].dtc>>16);
 req[3]=(uint8_t)(Dem_Config.events[0].dtc>>8);
 req[4]=(uint8_t)Dem_Config.events[0].dtc;req[5]=1;uds(req,6);
 assert(wire[1]==0x10&&wire[2]==16&&wire[3]==0x59&&wire[4]==4);confirm();
 frame[0]=0x41;frame[1]=0x30;frame[2]=0;frame[3]=0;rxframe(frame,4);
 CanTp_MainFunction();assert(wire[1]==0x21&&wire[4]==1&&wire[5]==0xF1&&wire[6]==0xA0);confirm();
 while(CanTp_IsBusy()){CanTp_MainFunction();confirm();}
 req[5]=2;uds(req,6);assert(wire[2]==0x7F&&wire[3]==0x19&&wire[4]==0x31);confirm();
 frame[0]=255;CanNm_RxIndication(0x3FF,frame,1);
 for(unsigned i=0;i<3;++i){ComM_MainFunction();CanSM_MainFunction();}
 assert(CanSM_IsTxAllowed());
 /* FCD's exact communication and DTC-control preamble remains supported. */
 req[0]=0x28;req[1]=1;req[2]=3;uds(req,3);assert(wire[2]==0x68);confirm();
 data[4]=42;before=appRxCalls;Com_RxIndication(0x97,data,5);assert(appRxCalls==before+1);
 assert(Com_IsSignalValid(0));
 before=sent;assert(Com_TriggerIPDUSend(3)==1&&sent==before);
 req[0]=0x10;req[1]=1;uds(req,2);assert(wire[2]==0x50);confirm();
 before=sent;assert(Com_TriggerIPDUSend(3)==0&&sent==before+1);
 req[1]=3;uds(req,2);assert(wire[2]==0x50);confirm();
 req[0]=0x28;req[1]=1;req[2]=3;uds(req,3);assert(wire[2]==0x68);confirm();
 req[0]=0x85;req[1]=2;uds(req,2);assert(wire[2]==0xC5);confirm();
 req[0]=0x85;req[1]=1;uds(req,2);assert(wire[2]==0xC5);confirm();
 req[0]=0x28;req[1]=0;req[2]=3;uds(req,3);assert(wire[2]==0x68);confirm();
 before=sent;assert(Com_TriggerIPDUSend(3)==0&&sent==before+1);
 /* 17-byte full request and 25-byte response; CF progress requires real confirmations. */
 {uint8_t request[17];request[0]=0x22;for(unsigned i=1;i<17;i+=2){request[i]=0xF1;request[i+1]=0x86;}
 frame[0]=CANTP_TARGET_ADDRESS;frame[1]=0x10;frame[2]=17;memcpy(frame+3,request,5);rxframe(frame,8);assert(wire[1]==0x30);confirm();
 frame[1]=0x21;memcpy(frame+2,request+5,6);rxframe(frame,8);frame[1]=0x22;memcpy(frame+2,request+11,6);rxframe(frame,8);Dcm_MainFunction();assert(wire[1]==0x10&&wire[2]==25);confirm();
 frame[0]=0x41;frame[1]=0x30;frame[2]=2;frame[3]=5;rxframe(frame,4);
 before=sent;CanTp_MainFunction();assert(sent==before+1);CanTp_MainFunction();assert(sent==before+1);confirm();
 CanTp_MainFunction();confirm();assert(CanTp_IsBusy());frame[2]=0;rxframe(frame,4);
 while(CanTp_IsBusy()){CanTp_MainFunction();confirm();}
 }
 /* Wrong CF sequence and timeout are propagated. */
 before=PduR_GetRxErrorCount();frame[0]=CANTP_TARGET_ADDRESS;frame[1]=0x10;frame[2]=20;rxframe(frame,8);confirm();frame[1]=0x22;rxframe(frame,8);assert(PduR_GetRxErrorCount()==before+1);
 /* Durable ClearDTC emits Pending while NvM has not run, then a final response. */
 assert(!Dem_SetEventStatus(Dem_Config.events[0].eventId,DEM_EVENT_STATUS_FAILED));NvM_WriteAll();drain();
 req[0]=0x14;req[1]=req[2]=req[3]=255;before=sent;uds(req,4);assert(sent==before);
 for(unsigned i=0;i<10;++i)Dcm_MainFunction();assert(wire[2]==0x7F&&wire[3]==0x14&&wire[4]==0x78);confirm();drain();Dem_MainFunction();Dcm_MainFunction();assert(wire[2]==0x54);confirm();
 /* Flash failure becomes a final NRC 0x72. */
 assert(!Dem_SetEventStatus(Dem_Config.events[0].eventId,DEM_EVENT_STATUS_FAILED));NvM_WriteAll();drain();failAfter=0;uds(req,4);drain();Dem_MainFunction();Dcm_MainFunction();assert(wire[2]==0x7F&&wire[4]==0x72);confirm();failAfter=-1;
 req[0]=0x11;req[1]=1;uds(req,2);drain();Dcm_MainFunction();assert(wire[2]==0x51&&resetCalls==0);confirm();assert(resetCalls==1);
 /* Configured COM validates DLC, unpacks signals, times out and applies groups. */
 data[4]=77;before=appRxCalls;Com_RxIndication(0x97,data,4);assert(appRxCalls==before);Com_RxIndication(0x97,data,5);assert(appRxCalls==before+1);assert(!Com_ReceiveSignal(0,&value)&&value==77);
 frame[0]=255;CanNm_RxIndication(0x3FF,frame,1);for(unsigned i=0;i<3;++i){ComM_MainFunction();CanSM_MainFunction();}assert(CanSM_IsTxAllowed());
 CanSM_ControllerBusOff();assert(!CanSM_IsTxAllowed());
 for(unsigned i=0;i<200;++i){CanSM_MainFunction();assert(!CanSM_IsTxAllowed());}
 CanSM_MainFunction();assert(CanSM_IsTxAllowed());
 for(unsigned i=0;i<2000;++i)Com_MainFunction();assert(!Com_IsSignalValid(0));assert(Com_GetRxTimeoutCount()==1);assert(!Com_IsNm3Active());
 req[0]=0x28;req[1]=1;req[2]=3;uds(req,3);assert(wire[2]==0x68);confirm();
 before=sent;assert(Com_TriggerIPDUSend(3)==1&&sent==before);
 for(unsigned i=0;i<1000;++i)Dcm_MainFunction();
 before=sent;assert(Dcm_DiagnosticSession==DEFAULT&&Com_TriggerIPDUSend(3)==0&&sent==before+1);
 Com_IpduGroupControl(0,0);before=appRxCalls;Com_RxIndication(0x97,data,5);assert(appRxCalls==before);
 puts("diagnostics/communication: PASS full UDS, physical confirmations, FC/sequence, durable clear, reset, COM Rx/Tx control, S3 restore, NM/bus-off quiet period");return 0;
}
