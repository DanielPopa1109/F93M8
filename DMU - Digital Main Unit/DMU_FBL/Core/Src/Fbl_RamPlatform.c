/* Independent STM32F407 CAN1. Inherit resident PLL, pin and bit timing setup;
 * CAN1 PA11/12 AF9, APB1 42 MHz, prescaler 4, BS1=16, BS2=4 (500 kbit/s).
 * All IRQ sources are masked; no CCM execution, DMA, HAL or callbacks. */
#include "Fbl_Ram.h"
#include "stm32f4xx.h"
static RAM_DATA uint32_t lastCycles,milliseconds,remainder;
RAM_CODE uint32_t Ram_Millis(void) {
 uint32_t now=DWT->CYCCNT,delta=now-lastCycles;lastCycles=now;
 remainder+=delta;milliseconds+=remainder/168000U;remainder%=168000U;return milliseconds;
}
RAM_CODE int Ram_CanInit(void) {
 CoreDebug->DEMCR|=CoreDebug_DEMCR_TRCENA_Msk;DWT->CTRL|=1;
 lastCycles=DWT->CYCCNT;milliseconds=remainder=0;
 CAN1->IER=0;CAN1->TSR=CAN_TSR_ABRQ0|CAN_TSR_ABRQ1|CAN_TSR_ABRQ2;
 uint32_t begin=Ram_Millis(),budget=10000000;
 while((CAN1->TSR&(CAN_TSR_TME0|CAN_TSR_TME1|CAN_TSR_TME2))!=(CAN_TSR_TME0|CAN_TSR_TME1|CAN_TSR_TME2))
  if(!--budget || Ram_Millis()-begin>100)return 1;
 CAN1->TSR=CAN_TSR_RQCP0|CAN_TSR_RQCP1|CAN_TSR_RQCP2;
 /* Existing CAN1 is normal/started; drain pre-handover requests. */
 for(uint8_t i=0;i<3 && (CAN1->RF0R&3);i++)CAN1->RF0R=CAN_RF0R_RFOM0;
 return (CAN1->MSR&CAN_MSR_INAK)?1:0;
}
RAM_CODE int Ram_CanReceive(uint16_t *id,uint8_t *dlc,uint8_t *p) {
 if(!(CAN1->RF0R&3))return 0;
 uint32_t rir=CAN1->sFIFOMailBox[0].RIR,dtr=CAN1->sFIFOMailBox[0].RDTR;
 uint32_t lo=CAN1->sFIFOMailBox[0].RDLR,hi=CAN1->sFIFOMailBox[0].RDHR;
 CAN1->RF0R=CAN_RF0R_RFOM0;
 if((rir&6) || (dtr&15)>8)return 0;
 *id=(uint16_t)(rir>>21);*dlc=dtr&15;
 for(uint8_t i=0;i<4;i++){p[i]=(uint8_t)(lo>>(8*i));p[i+4]=(uint8_t)(hi>>(8*i));}return 1;
}
RAM_CODE int Ram_CanSend(const uint8_t *p) {
 if(!(CAN1->TSR&CAN_TSR_TME0) || (CAN1->ESR&CAN_ESR_BOFF))return 1;
 CAN1->TSR=CAN_TSR_RQCP0;
 CAN1->sTxMailBox[0].TDTR=8;
 CAN1->sTxMailBox[0].TDLR=(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
 CAN1->sTxMailBox[0].TDHR=(uint32_t)p[4]|((uint32_t)p[5]<<8)|((uint32_t)p[6]<<16)|((uint32_t)p[7]<<24);
 CAN1->sTxMailBox[0].TIR=(0x710U<<21)|CAN_TI0R_TXRQ;
 uint32_t begin=Ram_Millis(),budget=100000000;
 while(!(CAN1->TSR&CAN_TSR_RQCP0))if(!--budget || (CAN1->ESR&CAN_ESR_BOFF) || Ram_Millis()-begin>1000){CAN1->TSR=CAN_TSR_ABRQ0;return 1;}
 uint32_t status=CAN1->TSR;CAN1->TSR=CAN_TSR_RQCP0;
 return ((status&CAN_TSR_TXOK0) && !(status&(CAN_TSR_ALST0|CAN_TSR_TERR0)))?0:1;
}
RAM_CODE void Ram_Reset(void) {
 /* One-shot programming entry lets FCD continue directly with APP download. */
 volatile uint32_t *box=(volatile uint32_t *)FBL_BOOT_MAILBOX;
 box[1]=2;box[2]=~2U;box[0]=0x46424C31U;__asm volatile("dsb":::"memory");
 SCB->AIRCR=0x05FA0004U;__asm volatile("dsb":::"memory");for(;;){}
}
static RAM_CODE void fault(void){for(;;){}}
__attribute__((section(".ram_blob_vectors"),used,aligned(512)))
const uint32_t ram_vectors[112]={RAM_STACK_TOP,(uint32_t)Ram_Start,[2 ... 111]=(uint32_t)fault};
