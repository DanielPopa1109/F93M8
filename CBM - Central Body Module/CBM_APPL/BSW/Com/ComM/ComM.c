#include "ComM.h"
#include "../CanNm/CanNm.h"
#include "../CanSM/CanSM.h"
#include "../Com/Com.h"

static ComM_ModeType ComM_RequestedMode;
static ComM_ModeType ComM_CurrentMode;

void ComM_Init(void) { ComM_RequestedMode = COMM_NO_COMMUNICATION; ComM_CurrentMode = COMM_NO_COMMUNICATION; }
uint8_t ComM_RequestComMode(ComM_ModeType mode) { if (mode > COMM_FULL_COMMUNICATION) return 1U; ComM_RequestedMode = mode; return 0U; }
void ComM_MainFunction(void)
{
    ComM_ModeType effectiveMode = ComM_RequestedMode;
    if (Com_IsNm3Active()) effectiveMode = COMM_FULL_COMMUNICATION;
    if (effectiveMode == COMM_FULL_COMMUNICATION) CanNm_NetworkRequest(); else CanNm_NetworkRelease();
    (void)CanSM_RequestComMode((CanSM_ModeType)effectiveMode);
    ComM_CurrentMode = (ComM_ModeType)CanSM_GetCurrentComMode();
}
ComM_ModeType ComM_GetCurrentComMode(void) { return ComM_CurrentMode; }
