#include "Can.h"
#include "../CanIf/CanIf.h"
#include "Fbl.h"
static Can_FrameType queue[16];
static volatile uint8_t head,tail,done;
static uint8_t pending;static uint16_t age;
void Can_Init(void) {head=tail=done=pending=0;age=0;}
Can_StatusType Can_Write(uint16_t id,const uint8_t *p,uint8_t n) {
 if(pending || id!=0x710 || !p || n!=8 || Fbl_BoardCanTransmit(p)) return CAN_NOT_OK;
 pending=1;age=0;return CAN_OK;
}
void Can_TxConfirmation(uint8_t ok) { if(pending)done=ok?1:2; }
void Can_BusOff(void) { Can_TxConfirmation(0); }
void Can_RxIndication(const Can_FrameType *p) {
 uint8_t next=(uint8_t)((tail+1)%16);
 if(!p || p->id!=0x710 || p->dlc>8 || next==head)return;
 queue[tail]=*p;tail=next;
}
void Can_MainFunction(void) {
 uint8_t n=0;
 if(pending && !done && ++age>=200) done=2;
 if(done) {uint8_t ok=done==1;done=pending=0;CanIf_TxConfirmation(ok);}
 while(head!=tail && n++<16) {Can_FrameType frame=queue[head];head=(uint8_t)((head+1)%16);CanIf_RxIndication(&frame);}
}
