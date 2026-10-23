#ifndef DMU_DEM_TYPES_H
#define DMU_DEM_TYPES_H
#include "Std_Types.h"

typedef uint16 Dem_EventIdType;
typedef uint32 Dem_DTCType;
typedef uint8 Dem_UdsStatusByteType;
typedef uint8 Dem_EventStatusType;
typedef uint8 Dem_OperationCycleIdType;
typedef uint8 Dem_OperationCycleStateType;
typedef uint8 Dem_DTCFormatType;
typedef uint16 Dem_DTCOriginType;
typedef uint8 Dem_DTCSeverityType;
typedef uint8 Dem_DebounceResetStatusType;

#define DEM_DEBOUNCE_STATUS_FREEZE 0x00U
#define DEM_DEBOUNCE_STATUS_RESET  0x01U

#define DEM_EVENT_STATUS_PASSED    0x00U
#define DEM_EVENT_STATUS_FAILED    0x01U
#define DEM_EVENT_STATUS_PREPASSED 0x02U
#define DEM_EVENT_STATUS_PREFAILED 0x03U
#define DEM_CYCLE_STATE_END        0x00U
#define DEM_CYCLE_STATE_START      0x01U
#define DEM_DTC_FORMAT_UDS         0x01U
#define DEM_DTC_ORIGIN_PRIMARY_MEMORY 0x0001U
#define DEM_DTC_GROUP_ALL_DTCS     0x00FFFFFFUL

/* R20-11 Dem service errors (ClearDTC/selection interface). */
#define DEM_PENDING                4U
#define DEM_CLEAR_BUSY             5U
#define DEM_CLEAR_MEMORY_ERROR     6U
#define DEM_CLEAR_FAILED           7U
#define DEM_WRONG_DTC              8U
#define DEM_WRONG_DTCORIGIN        9U
#define DEM_E_NO_DTC_AVAILABLE     10U
#define DEM_BUSY                   22U
#define DEM_NO_SUCH_ELEMENT        48U

#define DEM_UDS_STATUS_TF          0x01U
#define DEM_UDS_STATUS_TFTOC       0x02U
#define DEM_UDS_STATUS_PDTC        0x04U
#define DEM_UDS_STATUS_CDTC        0x08U
#define DEM_UDS_STATUS_TNCSLC      0x10U
#define DEM_UDS_STATUS_TFSLC       0x20U
#define DEM_UDS_STATUS_TNCTOC      0x40U
#define DEM_UDS_STATUS_WIR         0x80U
#define DEM_UDS_STATUS_AVAILABILITY_MASK 0x7FU

typedef enum { DEM_UNINIT, DEM_PREINIT, DEM_INITIALIZED, DEM_SHUTDOWN } Dem_InitStateType;
typedef enum { DEM_CLEAR_IDLE, DEM_CLEAR_PENDING, DEM_CLEAR_OK, DEM_CLEAR_RESULT_FAILED } Dem_ClearDTCStatusType;

#endif
