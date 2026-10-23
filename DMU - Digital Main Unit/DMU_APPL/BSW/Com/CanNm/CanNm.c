#include "CanNm.h"
#include "../Com/Com.h"
#define CANNM_REPEAT_TICKS 40U
#define CANNM_READY_SLEEP_TICKS 200U
static CanNm_StateType CanNm_State;
static uint16_t CanNm_StateTimer;
static uint8_t CanNm_LocalRequest;
void CanNm_Init(void) { CanNm_State = CANNM_BUS_SLEEP; CanNm_StateTimer = 0U; CanNm_LocalRequest = 0U; }
void CanNm_RxIndication(uint16_t canId, const uint8_t *data, uint8_t dlc)
{
    Com_RxIndication(canId, data, dlc);
    if ((canId == COM_NM3_CAN_ID) && (data != 0) && (dlc == 1U) && (data[0] == COM_NM3_ACTIVE_VALUE)) { if (CanNm_State == CANNM_BUS_SLEEP) { CanNm_State = CANNM_REPEAT_MESSAGE; CanNm_StateTimer = CANNM_REPEAT_TICKS; } else { CanNm_State = CANNM_NORMAL_OPERATION; CanNm_StateTimer = 0U; } }
}
void CanNm_NetworkRequest(void) { CanNm_LocalRequest = 1U; if (CanNm_State == CANNM_BUS_SLEEP) { CanNm_State = CANNM_REPEAT_MESSAGE; CanNm_StateTimer = CANNM_REPEAT_TICKS; } }
void CanNm_NetworkRelease(void) { CanNm_LocalRequest = 0U; }
void CanNm_MainFunction(void)
{
    if (CanNm_State == CANNM_REPEAT_MESSAGE) { if (CanNm_StateTimer > 0U) CanNm_StateTimer--; else { CanNm_State = (CanNm_LocalRequest || Com_IsNm3Active()) ? CANNM_NORMAL_OPERATION : CANNM_READY_SLEEP; CanNm_StateTimer = CANNM_READY_SLEEP_TICKS; } }
    else if (CanNm_State == CANNM_NORMAL_OPERATION) { if ((!CanNm_LocalRequest) && (!Com_IsNm3Active())) { CanNm_State = CANNM_READY_SLEEP; CanNm_StateTimer = CANNM_READY_SLEEP_TICKS; } }
    else if (CanNm_State == CANNM_READY_SLEEP) { if (CanNm_LocalRequest || Com_IsNm3Active()) { CanNm_State = CANNM_NORMAL_OPERATION; CanNm_StateTimer = 0U; } else if (CanNm_StateTimer > 0U) CanNm_StateTimer--; else CanNm_State = CANNM_BUS_SLEEP; }
}
CanNm_StateType CanNm_GetState(void) { return CanNm_State; }
