#include "Can.h"
#include "../CanIf/CanIf.h"
#ifndef CBM_HOST_TEST
#include "../CanSpi/CanSpi.h"
#endif
#include <string.h>
#include "../CanSM/CanSM.h"

static uint8_t canInitialized;
static volatile uint8_t txPending,txResult;
static uint16_t txAge;
static void Can_ProcessConfirmation(void);

void Can_Init(void) { canInitialized=1U; txPending=txResult=0U; txAge=0U; }

Can_StatusType Can_Write(uint16_t id, const uint8_t *data, uint8_t dlc)
{
    if (txPending || !canInitialized || data == 0 || dlc > 8U || id > 0x7FFU)
        return CAN_NOT_OK;
#ifdef CBM_HOST_TEST
    extern Can_StatusType Can_TestTransmit(uint16_t, const uint8_t *, uint8_t);
    Can_StatusType result=Can_TestTransmit(id,data,dlc);
    if(result==CAN_OK) { txPending=1U; txAge=0U; }
    return result;
#else
    CANSPI_uCAN_MSG frame = {0};
    frame.frame.idType = 1U;
    frame.frame.id = id;
    frame.frame.dlc = dlc;
    memcpy(&frame.frame.data0, data, dlc);
    if(CanSpi_Transmit(&frame)==0U) return CAN_NOT_OK;
    txPending=1U; txAge=0U; return CAN_OK;
#endif
}

void Can_RxIndication(const Can_FrameType *frame)
{
    if (canInitialized && frame != 0 && frame->dlc <= 8U)
        CanIf_RxIndication(frame);
}

void Can_MainFunction(void) { Can_ProcessConfirmation(); }

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
#ifdef CBM_HOST_TEST
 extern Can_StatusType Can_TestTransmit(uint16_t,const uint8_t *,uint8_t);
 return Can_TestTransmit(id,p,n);
#else
 CANSPI_uCAN_MSG f={0};f.frame.idType=1;f.frame.id=id;f.frame.dlc=n;memcpy(&f.frame.data0,p,n);return CanSpi_Transmit(&f)?CAN_OK:CAN_NOT_OK;
#endif
}
