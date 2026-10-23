#include "PduR.h"
#include "../CanTp/CanTp.h"
#ifndef DMU_HOST_TEST
#include "../../Diag/Dcm/Dcm.h"
#else
extern void Dcm_TpRxIndication(const uint8_t *, uint16_t);
#endif

static uint8_t initialized;
static uint16_t rxErrors, txErrors;
void PduR_Init(void) { rxErrors = txErrors = 0U; initialized = 1U; }

uint8_t PduR_DcmTransmit(const uint8_t *data, uint16_t len)
{
    return initialized ? CanTp_Transmit(data, len) : CAN_NOT_OK;
}

void PduR_CanTpRxIndication(const uint8_t *data, uint16_t len)
{
    if (initialized) Dcm_TpRxIndication(data, len);
}

void PduR_CanTpRxError(void) { if (rxErrors != UINT16_MAX) ++rxErrors; }
extern void Dcm_TpTxConfirmation(uint8_t success);
void PduR_CanTpTxConfirmation(uint8_t success) { if (!success && txErrors != UINT16_MAX) ++txErrors; Dcm_TpTxConfirmation(success); }
uint16_t PduR_GetRxErrorCount(void) { return rxErrors; }
uint16_t PduR_GetTxErrorCount(void) { return txErrors; }
