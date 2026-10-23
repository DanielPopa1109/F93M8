#include "Dem.h"

/* The nine legacy (event ID, status) pairs retain their on-flash positions. */
__attribute__((section(".ccmram"))) uint32 Dem_DTCArray[DEM_LEGACY_ARRAY_SIZE];

void Dem_SetDtc(uint8 EventId, uint8 Failed)
{
    (void)Dem_SetEventStatus(EventId, Failed != 0U ? DEM_EVENT_STATUS_FAILED :
                                                  DEM_EVENT_STATUS_PASSED);
}

void Dem_ClearDtc(void)
{
    if (Dem_SelectDTC(0U, DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_FORMAT_UDS,
                      DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_OK)
        (void)Dem_ClearDTC(0U);
}

void Dem_ApplicationStatusChanged(Dem_EventIdType event,uint8 before,uint8 after) {
 /* DMU monitors consume status through DEM; no legacy CAN failure announcement was configured. */
 (void)event;(void)before;(void)after;
}
