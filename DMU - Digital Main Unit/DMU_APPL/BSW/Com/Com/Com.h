#ifndef DMU_COM_H
#define DMU_COM_H
#include <stdint.h>
#include "Com_Cfg.h"
#define COM_NM3_CAN_ID 0x3FFU
#define COM_NM3_ACTIVE_VALUE 0xFFU
void Com_Init(void);
uint8_t Com_ValidateRxPdu(uint16_t id,uint8_t dlc);
uint8_t Com_SendSignal(uint16_t signal,const void *value);
uint8_t Com_ReceiveSignal(uint16_t signal,void *value);
void Com_IpduGroupControl(uint8_t group,uint8_t enabled);
void Com_SetTxEnabled(uint8_t enabled);
uint8_t Com_IsSignalValid(uint16_t signal);
uint8_t Com_TriggerIPDUSend(uint16_t pdu);
uint16_t Com_GetRxTimeoutCount(void);
void Com_TxConfirmation(uint16_t canId,uint8_t success);
void Com_RxIndication(uint16_t canId, const uint8_t *data, uint8_t dlc);
void Com_MainFunction(void);
uint8_t Com_IsNm3Active(void);
uint16_t Com_GetNm3AgeTicks(void);
#endif
