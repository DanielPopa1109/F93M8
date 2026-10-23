#include <assert.h>
#include <stdint.h>
#include "../BSW/Com/Com/Com.h"
#include "../BSW/Com/CanNm/CanNm.h"
#include "../BSW/Com/CanSM/CanSM.h"
#include "../BSW/Com/ComM/ComM.h"

int main(void)
{
    uint8_t nm3[1] = { COM_NM3_ACTIVE_VALUE };
    unsigned int i;

    Com_Init(); CanNm_Init(); CanSM_Init(); ComM_Init();
    assert(CanNm_GetState() == CANNM_BUS_SLEEP);
    CanNm_RxIndication(COM_NM3_CAN_ID, nm3, 1U);
    ComM_MainFunction(); CanSM_MainFunction();
    assert(Com_IsNm3Active() == 1U);
    assert(CanSM_GetCurrentComMode() == CANSM_FULL_COMMUNICATION);
    for (i = 0U; i < 200U; ++i) { Com_MainFunction(); CanNm_MainFunction(); ComM_MainFunction(); CanSM_MainFunction(); }
    assert(Com_IsNm3Active() == 0U);
    for (i = 0U; i < 201U; ++i) { CanNm_MainFunction(); ComM_MainFunction(); CanSM_MainFunction(); }
    assert(CanNm_GetState() == CANNM_BUS_SLEEP);
    assert(CanSM_GetCurrentComMode() == CANSM_NO_COMMUNICATION);
    return 0;
}
