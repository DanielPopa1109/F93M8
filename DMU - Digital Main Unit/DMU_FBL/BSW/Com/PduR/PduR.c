#include "PduR.h"
#include "../CanTp/CanTp.h"
#include "Fbl.h"
static uint16_t rxErrors,txErrors;static uint8_t initialized;
void PduR_Init(void) {rxErrors=txErrors=0;initialized=1;}
uint8_t PduR_DcmTransmit(const uint8_t *p,uint16_t n) {return initialized?CanTp_Transmit(p,n):CAN_NOT_OK;}
void PduR_CanTpRxIndication(const uint8_t *p,uint16_t n) {if(initialized)Fbl_RxIndication(p,n);}
void PduR_CanTpRxError(void) {if(rxErrors!=65535U)++rxErrors;}
void PduR_CanTpTxConfirmation(uint8_t ok) {if(!ok&&txErrors!=65535U)++txErrors;Fbl_TxConfirmation(ok);}
uint16_t PduR_GetRxErrorCount(void){return rxErrors;}
uint16_t PduR_GetTxErrorCount(void){return txErrors;}
