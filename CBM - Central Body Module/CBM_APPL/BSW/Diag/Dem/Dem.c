#include "Dem.h"

/* The 13 legacy (event ID, status) pairs retain their on-flash positions. */
uint32 Dem_DTCStoreArray[DEM_LEGACY_ARRAY_SIZE];

#ifndef DEM_HOST_TEST
extern void CanSpi_DtcEventFailed(uint8 eventId);
#endif

void Dem_SaveDtc(uint8 EventId,uint8 Failed) {
 (void)Dem_SetEventStatus(EventId,Failed?DEM_EVENT_STATUS_FAILED:DEM_EVENT_STATUS_PASSED);
}
void Dem_ApplicationStatusChanged(Dem_EventIdType event,uint8 before,uint8 after) {
#ifndef DEM_HOST_TEST
 if(!(before&DEM_UDS_STATUS_TF) && (after&DEM_UDS_STATUS_TF))CanSpi_DtcEventFailed((uint8)event);
#else
 (void)event;(void)before;(void)after;
#endif
}
