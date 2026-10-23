#ifndef DMU_CANIF_H
#define DMU_CANIF_H
#include "../Can/Can.h"

#define CANIF_RX_DIAG 0U
#define CANIF_TX_DIAG 0U
#define CANIF_DIAG_CAN_ID 0x710U

void CanIf_Init(void);
void CanIf_ApplicationTxConfirmation(uint16_t id,uint8_t ok);
Can_StatusType CanIf_TransmitApplication(uint16_t id,const uint8_t *data,uint8_t dlc);
void CanIf_TxConfirmation(uint8_t success);
Can_StatusType CanIf_Transmit(uint8_t pduId, const uint8_t *data, uint8_t dlc);
void CanIf_RxIndication(const Can_FrameType *frame);

#endif
