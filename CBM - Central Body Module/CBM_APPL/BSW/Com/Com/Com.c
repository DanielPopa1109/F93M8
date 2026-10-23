#include "Com.h"
#include "../CanIf/CanIf.h"
#include "../CanSM/CanSM.h"
#include <string.h>
#define COM_NM3_TIMEOUT_TICKS 200U
static uint8_t Com_Nm3Active;static uint16_t Com_Nm3AgeTicks;
static uint8_t bytes[COM_PDU_COUNT][8],groups[8],valid[COM_PDU_COUNT],dirty[COM_PDU_COUNT];
static uint16_t age[COM_PDU_COUNT],period[COM_PDU_COUNT],timeoutCount;
static uint32_t unpack(uint16_t s) {
 const Com_SignalConfig *cfg=&Com_Signals[s];uint8_t i;uint32_t value=0;
 for(i=0;i<cfg->width;++i)if(bytes[cfg->pdu][(cfg->bit+i)/8]&(1U<<((cfg->bit+i)%8)))value|=(uint32_t)1U<<i;
 return value;
}
uint8_t Com_SendSignal(uint16_t s,const void *p) {
 const Com_SignalConfig *cfg;uint32_t value;uint8_t i;
 if(!p||s>=COM_SIGNAL_COUNT)return 1;
 cfg=&Com_Signals[s];value=*(const uint32_t *)p;
 if(!Com_Pdus[cfg->pdu].tx || value>cfg->maximum)return 1;
 for(i=0;i<cfg->width;++i) {uint8_t *b=&bytes[cfg->pdu][(cfg->bit+i)/8],mask=(uint8_t)(1U<<((cfg->bit+i)%8));*b=(uint8_t)((*b&~mask)|((value&((uint32_t)1U<<i))?mask:0));}
 dirty[cfg->pdu]=1;return 0;
}
uint8_t Com_ReceiveSignal(uint16_t s,void *p) {if(!p||s>=COM_SIGNAL_COUNT)return 1;*(uint32_t *)p=unpack(s);return valid[Com_Signals[s].pdu]?0:1;}
uint8_t Com_IsSignalValid(uint16_t s) {return s<COM_SIGNAL_COUNT?valid[Com_Signals[s].pdu]:0;}
void Com_Init(void) {
 memset(bytes,0,sizeof(bytes));memset(valid,0,sizeof(valid));memset(dirty,0,sizeof(dirty));memset(age,0,sizeof(age));memset(period,0,sizeof(period));memset(groups,1,sizeof(groups));
 Com_Nm3Active=0;Com_Nm3AgeTicks=COM_NM3_TIMEOUT_TICKS;timeoutCount=0;
}
void Com_RxIndication(uint16_t id,const uint8_t *p,uint8_t n) {
 uint16_t i;
 if(!p||n>8)return;
 if(id==COM_NM3_CAN_ID) {if(n!=1)return;Com_Nm3Active=p[0]==COM_NM3_ACTIVE_VALUE;Com_Nm3AgeTicks=0;return;}
 for(i=0;i<COM_PDU_COUNT;++i) if(!Com_Pdus[i].tx && Com_Pdus[i].canId==id && groups[Com_Pdus[i].group] && n>=Com_Pdus[i].dlc) {
  memcpy(bytes[i],p,Com_Pdus[i].dlc);valid[i]=1;age[i]=0;Com_AppRx(id);return;
 }
}
void Com_IpduGroupControl(uint8_t group,uint8_t on) {if(group<8)groups[group]=on!=0;}
uint8_t Com_TriggerIPDUSend(uint16_t pdu) {
 if(pdu>=COM_PDU_COUNT||!Com_Pdus[pdu].tx||!groups[Com_Pdus[pdu].group]||!CanSM_IsTxAllowed())return 1;
 if(CanIf_TransmitApplication(Com_Pdus[pdu].canId,bytes[pdu],Com_Pdus[pdu].dlc)!=CAN_OK)return 1;
 dirty[pdu]=0;return 0;
}
void Com_MainFunction(void) {
 uint16_t i,s;
 if(Com_Nm3AgeTicks<COM_NM3_TIMEOUT_TICKS)++Com_Nm3AgeTicks;
 if(Com_Nm3AgeTicks>=COM_NM3_TIMEOUT_TICKS)Com_Nm3Active=0;
 Com_AppTx();
 for(i=0;i<COM_PDU_COUNT;++i)if(groups[Com_Pdus[i].group]) {
  if(Com_Pdus[i].tx) {
   if(Com_Pdus[i].period && ++period[i]>=Com_Pdus[i].period) {if(!Com_TriggerIPDUSend(i))period[i]=0;}
   else if(!Com_Pdus[i].period && dirty[i]) (void)Com_TriggerIPDUSend(i);
  } else if(Com_Pdus[i].timeout && valid[i] && ++age[i]>=Com_Pdus[i].timeout) {
   valid[i]=0;if(timeoutCount!=65535)++timeoutCount;
   for(s=0;s<COM_SIGNAL_COUNT;++s)if(Com_Signals[s].pdu==i) {uint8_t bit;uint32_t v=Com_Signals[s].initial;for(bit=0;bit<Com_Signals[s].width;++bit){uint8_t *b=&bytes[i][(Com_Signals[s].bit+bit)/8],mask=(uint8_t)(1U<<((Com_Signals[s].bit+bit)%8));*b=(uint8_t)((*b&~mask)|((v&((uint32_t)1U<<bit))?mask:0));}}
   Com_AppRx(Com_Pdus[i].canId);
  }
 }
}
void Com_TxConfirmation(uint16_t id,uint8_t ok) {uint16_t i;if(!ok)for(i=0;i<COM_PDU_COUNT;++i)if(Com_Pdus[i].canId==id)dirty[i]=1;}
uint16_t Com_GetRxTimeoutCount(void){return timeoutCount;}
uint8_t Com_IsNm3Active(void){return Com_Nm3Active;}
uint16_t Com_GetNm3AgeTicks(void){return Com_Nm3AgeTicks;}

uint8_t Com_ValidateRxPdu(uint16_t id,uint8_t n) {
 uint16_t i;if(n>8)return 0;
 if(id==0x710)return n>=2;
 if(id==COM_NM3_CAN_ID)return n==1;
 for(i=0;i<COM_PDU_COUNT;++i)if(!Com_Pdus[i].tx&&Com_Pdus[i].canId==id)return n>=Com_Pdus[i].dlc;
 return 0;
}
