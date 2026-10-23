#ifndef DMU_PDUR_H
#define DMU_PDUR_H
#include <stdint.h>

void PduR_Init(void);
uint8_t PduR_DcmTransmit(const uint8_t *data, uint16_t len);
void PduR_CanTpRxIndication(const uint8_t *data, uint16_t len);
void PduR_CanTpRxError(void);
void PduR_CanTpTxConfirmation(uint8_t success);
uint16_t PduR_GetRxErrorCount(void);
uint16_t PduR_GetTxErrorCount(void);

#endif
