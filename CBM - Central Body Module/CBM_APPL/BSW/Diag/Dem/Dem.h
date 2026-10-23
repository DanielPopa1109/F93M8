#ifndef CBM_DEM_H
#define CBM_DEM_H
#include "Dem_Cfg.h"

extern uint32 Dem_DTCStoreArray[DEM_LEGACY_ARRAY_SIZE];

void Dem_PreInit(void);
void Dem_Init(const Dem_ConfigType *ConfigPtr);
void Dem_Shutdown(void);
void Dem_MainFunction(void);
Dem_InitStateType Dem_GetInitState(void);
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatus);
Std_ReturnType Dem_ReportErrorStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatus);
Std_ReturnType Dem_GetEventUdsStatus(Dem_EventIdType EventId, Dem_UdsStatusByteType *Status);
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType EventId, sint8 *Counter);
Std_ReturnType Dem_ResetEventDebounceStatus(Dem_EventIdType EventId,
    Dem_DebounceResetStatusType DebounceResetStatus);
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType EventId, Dem_DTCFormatType Format, Dem_DTCType *DTC);
Std_ReturnType Dem_SelectDTC(uint8 ClientId, Dem_DTCType DTC,
    Dem_DTCFormatType Format, Dem_DTCOriginType Origin);
Std_ReturnType Dem_GetDTCSelectionResult(uint8 ClientId);
Std_ReturnType Dem_GetDTCSelectionResultForClearDTC(uint8 ClientId);
Std_ReturnType Dem_GetStatusOfDTC(uint8 ClientId, Dem_UdsStatusByteType *Status);
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType Cycle,
                                           Dem_OperationCycleStateType State);
Std_ReturnType Dem_ClearDTC(uint8 ClientId);
Dem_ClearDTCStatusType Dem_GetClearDTCStatus(void);
Std_ReturnType Dem_SetDTCFilter(uint8 ClientId, uint8 StatusMask, Dem_DTCFormatType Format,
                                Dem_DTCOriginType Origin, boolean FilterWithSeverity,
                                Dem_DTCSeverityType SeverityMask,
                                boolean FilterForFaultDetectionCounter);
Std_ReturnType Dem_GetNumberOfFilteredDTC(uint8 ClientId, uint16 *Count);
Std_ReturnType Dem_GetNumberOfEventMemoryEntries(uint8 ClientId,
    Dem_DTCOriginType Origin, uint8 *Count);
Std_ReturnType Dem_GetNextFilteredDTC(uint8 ClientId, Dem_DTCType *DTC, Dem_UdsStatusByteType *Status);
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 ClientId,
    Dem_UdsStatusByteType *StatusMask, Dem_DTCOriginType Origin);
Std_ReturnType Dem_SetDtcSetting(boolean Enabled);
Std_ReturnType Dem_DisableDTCSetting(uint8 ClientId);
Std_ReturnType Dem_EnableDTCSetting(uint8 ClientId);
Std_ReturnType Dem_GetSnapshotDataByDTC(Dem_DTCType DTC, Dem_DTCFormatType Format,
    Dem_DTCOriginType Origin, uint8 RecordNumber, uint8 *DestBuffer, uint16 *BufSize);
Std_ReturnType Dem_GetExtendedDataRecordByDTC(Dem_DTCType DTC, Dem_DTCFormatType Format,
    Dem_DTCOriginType Origin, uint8 RecordNumber, uint8 *DestBuffer, uint16 *BufSize);
void Dem_ValidateStoredDtc(void);
void Dem_Serialize(uint8 *image);
uint8 Dem_Deserialize(const uint8 *image);
void Dem_StartRestoredCycle(void);

/* Compatibility adapter for existing CBM monitors. */
void Dem_SaveDtc(uint8 EventId, uint8 Failed);

#endif
