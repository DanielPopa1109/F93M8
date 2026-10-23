#ifndef CBM_CANSM_H
#define CBM_CANSM_H

#include <stdint.h>

typedef enum { CANSM_NO_COMMUNICATION = 0, CANSM_SILENT_COMMUNICATION, CANSM_FULL_COMMUNICATION } CanSM_ModeType;
void CanSM_Init(void);
void CanSM_ControllerBusOff(void);
void CanSM_MainFunction(void);
uint8_t CanSM_RequestComMode(CanSM_ModeType mode);
CanSM_ModeType CanSM_GetCurrentComMode(void);
uint8_t CanSM_IsTxAllowed(void);

#endif
