#include "Dem_Cfg.h"
extern void Dem_ApplicationStatusChanged(Dem_EventIdType,uint8,uint8);

/* Legacy event IDs are retained. The same numeric values are provisional UDS DTCs
 * until an ECU diagnostic-code assignment is supplied and migrated. */
#define CBM_EVENT(id) { (id), (id), 1U, 2, -2, 1U, 40U }
static const Dem_EventConfigType Dem_Events[DEM_EVENT_COUNT] = {
    CBM_EVENT(0x01U), CBM_EVENT(0x02U), CBM_EVENT(0x03U),
    CBM_EVENT(0x04U), CBM_EVENT(0x05U), CBM_EVENT(0x06U),
    CBM_EVENT(0x07U), CBM_EVENT(0x08U), CBM_EVENT(0x09U),
    CBM_EVENT(0x0AU), CBM_EVENT(0x0BU), CBM_EVENT(0x0CU),
    CBM_EVENT(0x0DU)
};
const Dem_ConfigType Dem_Config = {
    Dem_Events, DEM_EVENT_COUNT, DEM_PRIMARY_MEMORY_SIZE, DEM_DEFAULT_OPERATION_CYCLE,
    DEM_UDS_STATUS_AVAILABILITY_MASK, Dem_ApplicationStatusChanged
};
