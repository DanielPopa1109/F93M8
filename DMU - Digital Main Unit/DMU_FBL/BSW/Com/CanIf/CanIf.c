#include "CanIf.h"
#include "../CanTp/CanTp.h"

typedef struct { uint8_t pdu; uint16_t id; uint8_t minDlc; uint8_t maxDlc; } CanIf_PduConfig;
static const CanIf_PduConfig rxPdus[] = {{CANIF_RX_DIAG, CANIF_DIAG_CAN_ID, 2U, 8U}};
static const CanIf_PduConfig txPdus[] = {{CANIF_TX_DIAG, CANIF_DIAG_CAN_ID, 8U, 8U}};
static uint8_t initialized,flowControl;

void CanIf_Init(void) { initialized = 1U; }

Can_StatusType CanIf_Transmit(uint8_t pduId, const uint8_t *data, uint8_t dlc)
{
    if (!initialized || data == 0 || pduId != txPdus[0].pdu || dlc != txPdus[0].maxDlc)
        return CAN_NOT_OK;
    Can_StatusType result=Can_Write(txPdus[0].id,data,dlc);
    if(result==CAN_OK) flowControl=(data[1]&0xF0U)==0x30U;
    return result;
}

void CanIf_RxIndication(const Can_FrameType *frame)
{
    if (!initialized || frame == 0 || frame->id != rxPdus[0].id ||
        frame->dlc < rxPdus[0].minDlc || frame->dlc > rxPdus[0].maxDlc)
        return;
    CanTp_RxIndication(frame->data, frame->dlc);
}

void CanIf_TxConfirmation(uint8_t success) { if(flowControl) CanTp_FcConfirmation(success); else CanTp_TxConfirmation(success); }
