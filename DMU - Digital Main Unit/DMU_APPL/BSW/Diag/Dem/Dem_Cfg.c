#include "Dem_Cfg.h"
extern void Dem_ApplicationStatusChanged(Dem_EventIdType,uint8,uint8);

/* Preserve existing event IDs. Numeric UDS DTC assignments remain provisional. */
#define DMU_EVENT(id) { (id), (id), 1U, 2, -2, 1U, 40U }
static const Dem_EventConfigType Dem_Events[DEM_EVENT_COUNT] = {
    DMU_EVENT(0x0EU), DMU_EVENT(0x0FU), DMU_EVENT(0x10U),
    DMU_EVENT(0x11U), DMU_EVENT(0x12U), DMU_EVENT(0x13U),
    DMU_EVENT(0x14U), DMU_EVENT(0x15U), DMU_EVENT(0x16U)
};
const Dem_ConfigType Dem_Config = {
    Dem_Events, DEM_EVENT_COUNT, DEM_PRIMARY_MEMORY_SIZE, DEM_DEFAULT_OPERATION_CYCLE,
    DEM_UDS_STATUS_AVAILABILITY_MASK, Dem_ApplicationStatusChanged
};
