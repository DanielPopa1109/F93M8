#ifndef DCM_H
#define DCM_H
#include "Std_Types.h"
typedef enum { DEFAULT=0,EXTENDED=1,PROGRAMMING=2,CODING=3,CALIBRATION=4,HARDRESET=5,SOFTRESET=6,JUMPTOAPPL=7 } DiagState;
extern DiagState Dcm_DiagnosticSession;
extern uint32 Dcm_AliveCounter;
void Dcm_Init(void);
uint8 Dcm_IsActive(void);
void Dcm_MainFunction(void);
void Dcm_TpRxIndication(const uint8 *data,uint16 len);
void Dcm_TpTxConfirmation(uint8 success);
#endif
