#include "Dcm.h"
#include "Dcm_App.h"
#include "Dcm_Cfg.h"
#include "Dem.h"
#include "../../Mem/NvM_Engine.h"
#include "../../Com/PduR/PduR.h"
#include "../../Com/CanTp/CanTp.h"
#include <string.h>
#define CAPACITY 256U
DiagState Dcm_DiagnosticSession __attribute__((section(".ncr")));
uint32 Dcm_AliveCounter __attribute__((section(".ncr")));
static uint8 request[CAPACITY],response[CAPACITY];
static uint16 requestLength,responseLength,s3,pendingTimer;
static uint8 requestReady,responseReady,inFlight,operation,operationSid,operationSub,pendingCount;
static uint8 responsePending,resetAfterTx;
uint16 Dcm_DroppedRequests;
void Dcm_Init(void) {
 requestReady=responseReady=inFlight=operation=pendingCount=resetAfterTx=0;
 requestLength=responseLength=s3=0; Dcm_DiagnosticSession=DEFAULT;
 Dcm_DroppedRequests=0;
}
void Dcm_TpRxIndication(const uint8 *p,uint16 n) {
 if(!p||!n||n>CAPACITY||requestReady||operation||responseReady||inFlight) {
  if(Dcm_DroppedRequests!=65535U) ++Dcm_DroppedRequests; return;
 }
 memcpy(request,p,n); requestLength=n; requestReady=1U; s3=0;
}
static uint16 negative(uint8 sid,uint8 nrc) { response[0]=0x7F; response[1]=sid; response[2]=nrc; return 3; }
static uint16 dtcs(const uint8 *p,uint16 n) {
 uint16 pos=3,count,size; uint8 status,mask=0; Dem_DTCType dtc;
 if(n<2) return negative(0x19,0x13);
 if(p[1]==1 || p[1]==2) { if(n!=3) return negative(0x19,0x13); mask=p[2]; }
 else if(p[1]==4) {
  /* No wire-level snapshot DID assignment exists in either ECU configuration. */
  return negative(0x19,n==6?0x31:0x13);
 }
 else if(p[1]==6) {
  if(n!=6) return negative(0x19,0x13);
  dtc=((uint32)p[2]<<16)|((uint32)p[3]<<8)|p[4];size=CAPACITY-7;
  if(Dem_SelectDTC(0,dtc,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY)!=E_OK ||
     Dem_GetStatusOfDTC(0,&status)!=E_OK ||
     Dem_GetExtendedDataRecordByDTC(dtc,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY,p[5],response+7,&size)!=E_OK)return negative(0x19,0x31);
  response[0]=0x59;response[1]=6;memcpy(response+2,p+2,3);response[5]=status;response[6]=p[5];return (uint16)(size+7);
 }
 else if(p[1]!=0x0A || n!=2) return negative(0x19,0x12);
 if(Dem_SetDTCFilter(0,mask,DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY,FALSE,0,FALSE)!=E_OK ||
    Dem_GetDTCStatusAvailabilityMask(0,&status,DEM_DTC_ORIGIN_PRIMARY_MEMORY)!=E_OK) return negative(0x19,0x22);
 response[0]=0x59; response[1]=p[1]; response[2]=status;
 if(p[1]==1) {
  if(Dem_GetNumberOfFilteredDTC(0,&count)!=E_OK) return negative(0x19,0x22);
  response[3]=1; response[4]=(uint8)(count>>8); response[5]=(uint8)count; return 6;
 }
 while(Dem_GetNextFilteredDTC(0,&dtc,&status)==E_OK) {
  if(pos+4>CAPACITY) return negative(0x19,0x14);
  response[pos++]=(uint8)(dtc>>16); response[pos++]=(uint8)(dtc>>8);
  response[pos++]=(uint8)dtc; response[pos++]=status;
 }
 return pos;
}
static uint16 process(const uint8 *p,uint16 n) {
 uint16 i,pos=1,size; uint8 sub;
 const Dcm_ServiceConfig *cfg=0;
 for(i=0;i<DCM_SERVICE_COUNT;++i) if(Dcm_Services[i].sid==p[0]) { cfg=&Dcm_Services[i]; break; }
 if(!cfg) return negative(p[0],0x11);
 if(n<cfg->minimum || n>cfg->maximum) return negative(p[0],0x13);
 if(!(cfg->sessions&(1U<<Dcm_DiagnosticSession))) return negative(p[0],0x7F);
 switch(p[0]) {
 case 0x3E:
  if(p[1]!=0 && p[1]!=0x80) return negative(p[0],0x12);
  if(p[1]==0x80) return 0;
  response[0]=0x7E; response[1]=0; return 2;
 case 0x10:
  sub=p[1];
  if(sub==1) Dcm_DiagnosticSession=DEFAULT;
  else if(sub==3) Dcm_DiagnosticSession=EXTENDED;
  else if((sub==2 || (1 && (sub==4||sub==5))) && Dcm_DiagnosticSession==EXTENDED) {
   if(!NvM_IsIdle()) return negative(p[0],0x21);
   NvM_WriteAll(); operation=2; operationSid=p[0]; operationSub=sub; pendingTimer=DCM_P2_TICKS; return 0;
  } else return negative(p[0],0x7E);
  response[0]=0x50; response[1]=sub; response[2]=0; response[3]=50; response[4]=1; response[5]=244; return 6;
 case 0x22:
  if(((n-1)&1U)!=0) return negative(p[0],0x13);
  response[0]=0x62;
  for(i=1;i<n;i+=2) {
   uint8 value[64]; uint16 did=(uint16)((uint16)p[i]<<8|p[i+1]);
   if(did==0xF186) { value[0]=Dcm_DiagnosticSession==DEFAULT?1:(Dcm_DiagnosticSession==EXTENDED?3:(Dcm_DiagnosticSession==CODING?4:(Dcm_DiagnosticSession==CALIBRATION?5:2))); size=1; }
   else size=Dcm_AppReadDid(did,value);
   if(!size) return negative(p[0],0x31);
   if(pos+2+size>CAPACITY) return negative(p[0],0x14);
   response[pos++]=p[i]; response[pos++]=p[i+1]; memcpy(response+pos,value,size); pos=(uint16)(pos+size);
  } return pos;
 case 0x19: return dtcs(p,n);
 case 0x14:
  if(!NvM_IsIdle()) return negative(p[0],0x21);
  if(Dem_SelectDTC(0,((uint32)p[1]<<16)|((uint32)p[2]<<8)|p[3],DEM_DTC_FORMAT_UDS,DEM_DTC_ORIGIN_PRIMARY_MEMORY)!=E_OK ||
   Dem_GetDTCSelectionResultForClearDTC(0)!=E_OK) return negative(p[0],0x31);
  {Std_ReturnType result=Dem_ClearDTC(0);if(result!=DEM_PENDING && result!=E_OK)return negative(p[0],0x22);}
  operation=1; operationSid=p[0]; pendingTimer=DCM_P2_TICKS; return 0;
 case 0x11:
  if(p[1]!=1 && p[1]!=3) return negative(p[0],0x12);
  if(!NvM_IsIdle()) return negative(p[0],0x21);
  NvM_WriteAll(); operation=2; operationSid=p[0]; operationSub=p[1]; pendingTimer=DCM_P2_TICKS; return 0;
 case 0x28:
  if(p[1]>1 || (p[2]!=1 && p[2]!=3)) return negative(p[0],0x31);
  Dcm_AppCommunication(p[1]); response[0]=0x68; response[1]=p[1]; return 2;
 case 0x85:
  if(p[1]!=1 && p[1]!=2) return negative(p[0],0x12);
  if((p[1]==1?Dem_EnableDTCSetting(0):Dem_DisableDTCSetting(0))!=E_OK) return negative(p[0],0x22);
  response[0]=0xC5; response[1]=p[1]; return 2;
 case 0x31: size=Dcm_AppRoutine(p,n,response); return size?size:negative(p[0],0x31);
 case 0x2A: size=Dcm_AppPeriodic(p,n,response); return size?size:negative(p[0],0x31);
 default: return negative(p[0],0x11);
 }
}
void Dcm_TpTxConfirmation(uint8 success) {
 if(!inFlight) return;
 inFlight=0U;
 if(!success) { operation=resetAfterTx=0U; return; }
 if(resetAfterTx && !responsePending) { uint8 session=resetAfterTx; resetAfterTx=0; Dcm_AppReset(session); }
}
void Dcm_MainFunction(void) {
 uint8 finished=0,failed=0;
 Dcm_AliveCounter+=5;
 if(!operation && !inFlight && !responseReady && ++s3>=DCM_S3_TICKS) { s3=DCM_S3_TICKS; Dcm_DiagnosticSession=DEFAULT; }
 if(operation && !responseReady && !inFlight) {
  if(operation==1) {
   Dem_ClearDTCStatusType status=Dem_GetClearDTCStatus();
   finished=status==DEM_CLEAR_OK || status==DEM_CLEAR_RESULT_FAILED; failed=status==DEM_CLEAR_RESULT_FAILED;
  } else { finished=NvM_IsIdle(); failed=NvM_LastWriteResult!=0; }
  if(finished) {
   if(failed) responseLength=negative(operationSid,0x72);
   else if(operation==1) { Dcm_AppClearSources(); response[0]=0x54; responseLength=1; }
   else {
    response[0]=(uint8)(operationSid+0x40); response[1]=operationSub; responseLength=2;
    resetAfterTx=operationSid==0x10?operationSub:(operationSub==1?5:6);
    if(operationSid==0x10) { Dcm_DiagnosticSession=operationSub==2?PROGRAMMING:(operationSub==4?CODING:CALIBRATION); response[2]=0; response[3]=50; response[4]=1; response[5]=244; responseLength=6; }
   }
   operation=0; pendingCount=0; responsePending=0; responseReady=1;
  } else if(pendingTimer && --pendingTimer==0) {
   if(++pendingCount>20) { responseLength=negative(operationSid,0x72); operation=0; }
   else responseLength=negative(operationSid,0x78);
   responsePending=1; responseReady=1; pendingTimer=DCM_P2STAR_TICKS;
  }
 }
 if(requestReady && !responseReady && !inFlight && !operation) {
  requestReady=0; responsePending=0; responseLength=process(request,requestLength);
  if(responseLength) responseReady=1;
 }
 if(responseReady && !CanTp_IsBusy() && PduR_DcmTransmit(response,responseLength)==CAN_OK) { responseReady=0; inFlight=1; }
}

uint8 Dcm_IsActive(void) { return operation || inFlight || responseReady || requestReady || Dcm_DiagnosticSession!=DEFAULT; }
