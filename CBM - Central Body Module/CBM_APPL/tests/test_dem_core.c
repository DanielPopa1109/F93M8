#include <assert.h>
#include <string.h>
#include "../BSW/Diag/Dem/Dem.h"

int main(void)
{
    const Dem_EventIdType first = Dem_Config.events[0].eventId;
    const Dem_DTCType firstDtc = Dem_Config.events[0].dtc;
    Dem_UdsStatusByteType status;
    Dem_DTCType dtc;
    uint8 buffer[8];
    uint16 length;
    uint16 count;
    uint8 i;

    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_FAILED) == E_NOT_OK);
    assert(Dem_GetEventUdsStatus(first, &status) == E_NOT_OK);
    memset(DEM_LEGACY_ARRAY, 0, sizeof(uint32) * DEM_LEGACY_ARRAY_SIZE);
    Dem_PreInit();
    assert(Dem_GetInitState() == DEM_PREINIT);
    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_PREFAILED) == E_OK);
    Dem_Init(&Dem_Config);
    assert(Dem_GetInitState() == DEM_INITIALIZED);
    assert(Dem_GetEventUdsStatus(first, &status) == E_OK);
    assert((status & DEM_UDS_STATUS_TF) == 0U);
    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_PREFAILED) == E_OK);
    assert(Dem_GetEventUdsStatus(first, &status) == E_OK);
    assert((status & (DEM_UDS_STATUS_TF | DEM_UDS_STATUS_CDTC)) ==
           (DEM_UDS_STATUS_TF | DEM_UDS_STATUS_CDTC));
    assert(Dem_GetDTCOfEvent(first, 0U, &dtc) == DEM_E_NO_DTC_AVAILABLE);
    assert(Dem_GetDTCOfEvent(first, DEM_DTC_FORMAT_UDS, &dtc) == E_OK &&
           dtc == firstDtc);
    assert(Dem_SetEventStatus(0xFFFFU, DEM_EVENT_STATUS_FAILED) == E_NOT_OK);
    assert(Dem_SetEventStatus(first, 0xFFU) == E_NOT_OK);
    assert(Dem_GetEventUdsStatus(first, NULL_PTR) == E_NOT_OK);
    assert(Dem_SetDTCFilter(1U, 0U, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, 0U, FALSE) == E_NOT_OK);
    assert(Dem_GetDTCStatusAvailabilityMask(1U, &status,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_NOT_OK);
    assert(Dem_GetDTCStatusAvailabilityMask(0U, &status,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_OK &&
           status == DEM_UDS_STATUS_AVAILABILITY_MASK);
    assert(Dem_SelectDTC(0U, firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_OK);
    assert(Dem_GetDTCSelectionResult(0U) == E_OK);
    assert(Dem_GetStatusOfDTC(0U, &status) == E_OK &&
           (status & DEM_UDS_STATUS_TF) != 0U);
    assert(Dem_DisableDTCSetting(0U) == E_OK);
    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_PASSED) == E_OK);
    assert(Dem_GetEventUdsStatus(first, &status) == E_OK &&
           (status & DEM_UDS_STATUS_TF) != 0U);
    assert(Dem_EnableDTCSetting(0U) == E_OK);
    assert(Dem_SelectDTC(0U, 0x123456U, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_OK);
    assert(Dem_GetDTCSelectionResultForClearDTC(0U) == DEM_WRONG_DTC);
    assert(Dem_ClearDTC(0U) == DEM_WRONG_DTC);
    assert(Dem_SelectDTC(0U, firstDtc, DEM_DTC_FORMAT_UDS,
           0xFFFFU) == E_OK);
    assert(Dem_GetDTCSelectionResult(0U) == DEM_WRONG_DTCORIGIN);
    assert(Dem_SelectDTC(0U, firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_OK);
    length = 1U;
    assert(Dem_GetSnapshotDataByDTC(firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, 1U, buffer, &length) == E_NOT_OK);
    length = sizeof(buffer);
    assert(Dem_GetSnapshotDataByDTC(firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, 1U, buffer, &length) == E_OK);
    assert(length == 6U && buffer[0] == (uint8)first);
    length = sizeof(buffer);
    assert(Dem_GetExtendedDataRecordByDTC(firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, 1U, buffer, &length) == E_OK);
    assert(length == 3U && buffer[0] == 1U);

    assert(Dem_SetDTCFilter(0U, 0U, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY,
           FALSE, 0U, FALSE) == E_OK);
    assert(Dem_GetNumberOfFilteredDTC(0U, &count) == E_OK && count == DEM_EVENT_COUNT);
    for (i = 0U; i < DEM_EVENT_COUNT; ++i)
        assert(Dem_GetNextFilteredDTC(0U, &dtc, &status) == E_OK &&
               dtc == Dem_Config.events[i].dtc);
    assert(Dem_GetNextFilteredDTC(0U, &dtc, &status) == DEM_NO_SUCH_ELEMENT);
    assert(Dem_SetDTCFilter(0U, DEM_UDS_STATUS_TF, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, 0U, FALSE) == E_OK);
    assert(Dem_GetNumberOfFilteredDTC(0U, &count) == E_OK && count == 1U);

    assert(Dem_ResetEventDebounceStatus(Dem_Config.events[1].eventId,
           DEM_DEBOUNCE_STATUS_FREEZE) == E_OK);
    for (i = 0U; i < 3U; ++i)
        assert(Dem_SetEventStatus(Dem_Config.events[1].eventId,
               DEM_EVENT_STATUS_PREFAILED) == E_OK);
    assert(Dem_GetEventUdsStatus(Dem_Config.events[1].eventId, &status) == E_OK &&
           (status & DEM_UDS_STATUS_TF) == 0U);
    assert(Dem_ResetEventDebounceStatus(Dem_Config.events[1].eventId,
           DEM_DEBOUNCE_STATUS_RESET) == E_OK);

    for (i = 1U; i <= DEM_PRIMARY_MEMORY_SIZE; ++i)
        assert(Dem_SetEventStatus(Dem_Config.events[i].eventId,
               DEM_EVENT_STATUS_FAILED) == E_OK);
    length = sizeof(buffer);
    assert(Dem_GetSnapshotDataByDTC(firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, 1U, buffer, &length) == E_NOT_OK);
    {
        uint8 entries;
        assert(Dem_GetNumberOfEventMemoryEntries(0U,
               DEM_DTC_ORIGIN_PRIMARY_MEMORY, &entries) == E_OK &&
               entries == DEM_PRIMARY_MEMORY_SIZE);
    }
    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_PASSED) == E_OK);
    assert(Dem_GetEventUdsStatus(first, &status) == E_OK);
    assert((status & DEM_UDS_STATUS_TF) == 0U);
    assert(Dem_SetOperationCycleState(DEM_DEFAULT_OPERATION_CYCLE,
           DEM_CYCLE_STATE_END) == E_OK);
    assert(Dem_SetOperationCycleState(DEM_DEFAULT_OPERATION_CYCLE,
           DEM_CYCLE_STATE_START) == E_OK);
    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_PASSED) == E_OK);
    assert(Dem_SetOperationCycleState(DEM_DEFAULT_OPERATION_CYCLE,
           DEM_CYCLE_STATE_END) == E_OK);
    length = sizeof(buffer);
    assert(Dem_GetExtendedDataRecordByDTC(firstDtc, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, 1U, buffer, &length) == E_OK);
    assert(buffer[2] == 1U);

    assert(Dem_SelectDTC(0U, DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY) == E_OK);
    assert(Dem_GetDTCSelectionResultForClearDTC(0U) == E_OK);
    assert(Dem_ClearDTC(0U) == E_OK);
    assert(Dem_GetClearDTCStatus() == DEM_CLEAR_OK);
    assert(DEM_LEGACY_ARRAY[0] == 0U);
    assert(Dem_SetDTCFilter(0U, DEM_UDS_STATUS_TF, DEM_DTC_FORMAT_UDS,
           DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, 0U, FALSE) == E_OK);
    assert(Dem_GetNumberOfFilteredDTC(0U, &count) == E_OK && count == 0U);
    Dem_Shutdown();
    assert(Dem_SetEventStatus(first, DEM_EVENT_STATUS_FAILED) == E_NOT_OK);

    /* Restore an existing two-word legacy record without changing its slot. */
    Dem_PreInit();
    DEM_LEGACY_ARRAY[0] = first;
    DEM_LEGACY_ARRAY[1] = 2U;
    DEM_LEGACY_ARRAY[2] = 0xFFFFFFFFU;
    DEM_LEGACY_ARRAY[3] = 0xFFFFFFFFU;
    Dem_Init(&Dem_Config);
    assert(Dem_GetEventUdsStatus(first, &status) == E_OK);
    assert((status & DEM_UDS_STATUS_TF) != 0U);
    assert(DEM_LEGACY_ARRAY[2] == 0U && DEM_LEGACY_ARRAY[3] == 0U);
    return 0;
}
