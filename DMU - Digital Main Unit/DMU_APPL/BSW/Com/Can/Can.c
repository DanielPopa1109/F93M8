#include "Can.h"
#include "../CanIf/CanIf.h"
#ifndef DMU_HOST_TEST
#include "../../../Core/Inc/can.h"
#endif
#include <string.h>
#include "../CanSM/CanSM.h"

static uint8_t canInitialized;
static volatile uint8_t txPending,txResult;
static uint16_t txAge;
static void Can_ProcessConfirmation(void);
#define CAN_RX_QUEUE_SIZE 16U
static Can_FrameType rxQueue[CAN_RX_QUEUE_SIZE];
static volatile uint8_t rxHead, rxTail;
uint16_t Can_RxDroppedCount;

void Can_Init(void)
{
    rxHead = rxTail = 0U; txPending=txResult=0U; txAge=0U;
    Can_RxDroppedCount = 0U;
    canInitialized = 1U;
}

Can_StatusType Can_Write(uint16_t id, const uint8_t *data, uint8_t dlc)
{
    if (txPending || !canInitialized || data == 0 || dlc > 8U || id > 0x7FFU)
        return CAN_NOT_OK;
#ifdef DMU_HOST_TEST
    extern Can_StatusType Can_TestTransmit(uint16_t, const uint8_t *, uint8_t);
    Can_StatusType result=Can_TestTransmit(id,data,dlc);
    if(result==CAN_OK) { txPending=1U; txAge=0U; }
    return result;
#else
    CAN_TxHeaderTypeDef header = {0};
    uint32_t mailbox;
    uint8_t bytes[8] = {0};
    header.StdId = id;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = dlc;
    memcpy(bytes, data, dlc);
    uint32_t irq=__get_PRIMASK(); __disable_irq();
    if(HAL_CAN_AddTxMessage(&hcan1,&header,bytes,&mailbox)!=HAL_OK) { if(!irq) __enable_irq(); return CAN_NOT_OK; }
    extern volatile uint32_t Can_DiagMailbox; Can_DiagMailbox=mailbox;
    txPending=1U; txAge=0U; if(!irq) __enable_irq(); return CAN_OK;
#endif
}

void Can_RxIndication(const Can_FrameType *frame)
{
    uint8_t next;
    if (!canInitialized || frame == 0 || frame->dlc > 8U) return;
    next = (uint8_t)((rxTail + 1U) % CAN_RX_QUEUE_SIZE);
    if (next == rxHead)
    {
        if (Can_RxDroppedCount != UINT16_MAX) ++Can_RxDroppedCount;
        return;
    }
    rxQueue[rxTail] = *frame;
    rxTail = next;
}

void Can_MainFunction(void)
{
    Can_ProcessConfirmation();
    Can_FrameType frame;
    uint8_t processed = 0U;
    while (rxHead != rxTail && processed < CAN_RX_QUEUE_SIZE)
    {
#ifndef DMU_HOST_TEST
        uint32_t irqMask = __get_PRIMASK();
        __disable_irq();
#endif
        if (rxHead == rxTail)
        {
#ifndef DMU_HOST_TEST
            if (irqMask == 0U) __enable_irq();
#endif
            break;
        }
        frame = rxQueue[rxHead];
        rxHead = (uint8_t)((rxHead + 1U) % CAN_RX_QUEUE_SIZE);
#ifndef DMU_HOST_TEST
        if (irqMask == 0U) __enable_irq();
#endif
        CanIf_RxIndication(&frame);
        ++processed;
    }
}

void Can_TxConfirmation(uint8_t success) { if(txPending) txResult=success?1U:2U; }
void Can_BusOff(void) { Can_TxConfirmation(0U); CanSM_ControllerBusOff(); }
static void Can_ProcessConfirmation(void) {
    uint8_t result;
    if(!txPending) return;
    if(!txResult && ++txAge>=200U) txResult=2U;
    result=txResult;
    if(result) { txResult=0U; txPending=0U; CanIf_TxConfirmation(result==1U); }
}

Can_StatusType Can_WriteApplication(uint16_t id,const uint8_t *p,uint8_t n) {
#ifdef DMU_HOST_TEST
 extern Can_StatusType Can_TestTransmit(uint16_t,const uint8_t *,uint8_t);
 return Can_TestTransmit(id,p,n);
#else
 CAN_TxHeaderTypeDef h={0};uint8_t data[8]={0};uint32_t box;h.StdId=id;h.IDE=CAN_ID_STD;h.RTR=CAN_RTR_DATA;h.DLC=n;memcpy(data,p,n);uint32_t irq=__get_PRIMASK();__disable_irq();
    if(HAL_CAN_AddTxMessage(&hcan1,&h,data,&box)!=HAL_OK){if(!irq)__enable_irq();return CAN_NOT_OK;}
    extern volatile uint16_t Can_AppMailboxIds[3];uint8_t index=box==CAN_TX_MAILBOX0?0:(box==CAN_TX_MAILBOX1?1:2);Can_AppMailboxIds[index]=id;
    if(!irq)__enable_irq();return CAN_OK;
#endif
}
