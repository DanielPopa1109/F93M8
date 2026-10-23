#ifndef CBM_SCHM_DEM_H
#define CBM_SCHM_DEM_H
#include "Std_Types.h"
#ifdef DEM_HOST_TEST
static inline uint32 SchM_Enter_Dem(void) { return 0U; }
static inline void SchM_Exit_Dem(uint32 state) { (void)state; }
#else
#include "main.h"
static inline uint32 SchM_Enter_Dem(void)
{
    uint32 state = __get_PRIMASK();
    __disable_irq();
    return state;
}
static inline void SchM_Exit_Dem(uint32 state)
{
    if (state == 0U) __enable_irq();
}
#endif
#endif
