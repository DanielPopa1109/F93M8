#ifndef DMU_CANTP_H
#define DMU_CANTP_H
#include "../CanIf/CanIf.h"

#define CANTP_MAX_PAYLOAD 256U
#define CANTP_MAIN_PERIOD_MS 5U
#define CANTP_TARGET_ADDRESS 0x56U
#define CANTP_GATEWAY_RX_ADDRESS 0x41U
#define CANTP_RESPONSE_ADDRESS 0xF0U

void CanTp_Init(void);
void CanTp_TxConfirmation(uint8_t success);
void CanTp_FcConfirmation(uint8_t success);
uint8_t CanTp_IsBusy(void);
void CanTp_MainFunction(void);
Can_StatusType CanTp_Transmit(const uint8_t *data, uint16_t len);
void CanTp_RxIndication(const uint8_t *frame, uint8_t dlc);

#endif
