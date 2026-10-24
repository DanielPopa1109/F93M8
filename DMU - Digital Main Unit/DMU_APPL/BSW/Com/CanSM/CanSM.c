#include "CanSM.h"
#include "../CanNm/CanNm.h"
static CanSM_ModeType CanSM_RequestedMode;
static CanSM_ModeType CanSM_CurrentMode;
static uint16_t recoveryTicks;
void CanSM_ControllerBusOff(void) { recoveryTicks=200U; CanSM_CurrentMode=CANSM_SILENT_COMMUNICATION; }
void CanSM_Init(void) { recoveryTicks=0; CanSM_RequestedMode = CANSM_NO_COMMUNICATION; CanSM_CurrentMode = CANSM_NO_COMMUNICATION; }
uint8_t CanSM_RequestComMode(CanSM_ModeType mode) { if (mode > CANSM_FULL_COMMUNICATION) return 1U; CanSM_RequestedMode = mode; return 0U; }
void CanSM_MainFunction(void) {
    if (recoveryTicks) { --recoveryTicks; CanSM_CurrentMode=CANSM_SILENT_COMMUNICATION; return; }
    if (CanSM_RequestedMode == CANSM_FULL_COMMUNICATION) CanSM_CurrentMode = CANSM_FULL_COMMUNICATION;
    else if (CanNm_GetState() != CANNM_BUS_SLEEP) CanSM_CurrentMode = CANSM_SILENT_COMMUNICATION;
    else CanSM_CurrentMode = CANSM_NO_COMMUNICATION;
}
CanSM_ModeType CanSM_GetCurrentComMode(void) { return CanSM_CurrentMode; }
uint8_t CanSM_IsTxAllowed(void) { return (CanSM_CurrentMode == CANSM_FULL_COMMUNICATION) ? 1U : 0U; }
