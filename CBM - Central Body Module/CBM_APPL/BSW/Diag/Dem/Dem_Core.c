#include "Dem.h"
#include "SchM_Dem.h"
#include "../../Mem/NvM_Engine.h"
#include <string.h>

/* Project-local DEM state. The legacy pair array remains the NvM migration image. */
typedef struct {
    Dem_UdsStatusByteType status;
    sint16 debounce;
    uint8 confirmation;
    uint8 occurrence;
    uint8 aging;
    uint8 debounceFrozen;
    uint8 queued; /* 0: none, 1: passed, 2: failed before Dem_Init. */
} Dem_RuntimeEventType;

typedef struct {
    uint8 valid;
    uint8 eventIndex;
    uint32 allocationOrder;
    uint8 snapshot[6];
} Dem_PrimaryEntryType;

typedef struct {
    uint8 valid;
    uint8 statusMask;
    uint8 nextIndex;
} Dem_FilterType;

typedef struct {
    uint8 valid;
    Dem_DTCType dtc;
    Dem_DTCFormatType format;
    Dem_DTCOriginType origin;
} Dem_ClientSelectionType;

extern uint32 DEM_LEGACY_ARRAY[DEM_LEGACY_ARRAY_SIZE];
static const Dem_ConfigType *Dem_ActiveConfig = &Dem_Config;
static Dem_RuntimeEventType Dem_Runtime[DEM_EVENT_COUNT];
static Dem_PrimaryEntryType Dem_Primary[DEM_PRIMARY_MEMORY_SIZE];
static Dem_FilterType Dem_Filter;
static Dem_ClientSelectionType Dem_Selection;
static Dem_InitStateType Dem_State = DEM_UNINIT;
static Dem_ClearDTCStatusType Dem_ClearStatus = DEM_CLEAR_IDLE;
static uint8 Dem_CycleActive;
static uint8 Dem_DtcSettingEnabled = 1U;
static uint32 Dem_ChangeCounter;
static uint8 Dem_ClearBackup[512];
static uint8 Dem_CallbackPending[DEM_EVENT_COUNT],Dem_CallbackOld[DEM_EVENT_COUNT];

static int Dem_FindEvent(Dem_EventIdType id)
{
    uint8 i;
    for (i = 0U; i < Dem_ActiveConfig->eventCount; ++i)
        if (Dem_ActiveConfig->events[i].eventId == id) return (int)i;
    return -1;
}

static int Dem_FindDtc(Dem_DTCType dtc)
{
    uint8 i;
    for (i = 0U; i < Dem_ActiveConfig->eventCount; ++i)
        if (Dem_ActiveConfig->events[i].dtc == dtc) return (int)i;
    return -1;
}

static uint8 Dem_ValidAddressing(Dem_DTCFormatType format, Dem_DTCOriginType origin)
{
    return (format == DEM_DTC_FORMAT_UDS && origin == DEM_DTC_ORIGIN_PRIMARY_MEMORY) ? 1U : 0U;
}

static void Dem_ExportLegacy(uint8 index)
{
    const Dem_RuntimeEventType *event = &Dem_Runtime[index];
    uint32 legacy = (event->status & DEM_UDS_STATUS_TF) != 0U ? 2U : 1U;
    DEM_LEGACY_ARRAY[2U * index] = Dem_ActiveConfig->events[index].eventId;
    DEM_LEGACY_ARRAY[2U * index + 1U] = legacy |
        ((uint32)event->status << 8) | ((uint32)event->occurrence << 16) |
        ((uint32)event->aging << 24);
}

static void Dem_AllocatePrimary(uint8 index)
{
    uint8 i;
    int chosen = -1;
    for (i = 0U; i < Dem_ActiveConfig->primaryMemorySize; ++i)
    {
        if (Dem_Primary[i].valid != 0U && Dem_Primary[i].eventIndex == index) return;
        if (Dem_Primary[i].valid == 0U) { chosen = (int)i; break; }
    }
    if (chosen < 0)
    {
        /* Same-priority entries displace the oldest. A less important event
         * cannot displace a higher-priority entry. */
        for (i = 0U; i < Dem_ActiveConfig->primaryMemorySize; ++i)
        {
            uint8 oldIndex = Dem_Primary[i].eventIndex;
            if (Dem_ActiveConfig->events[oldIndex].priority >=
                Dem_ActiveConfig->events[index].priority &&
                (chosen < 0 || Dem_Primary[i].allocationOrder < Dem_Primary[(uint8)chosen].allocationOrder))
                chosen = (int)i;
        }
    }
    if (chosen < 0) return;
    Dem_Primary[(uint8)chosen].valid = 1U;
    Dem_Primary[(uint8)chosen].eventIndex = index;
    Dem_Primary[(uint8)chosen].allocationOrder = ++Dem_ChangeCounter;
    Dem_Primary[(uint8)chosen].snapshot[0] = (uint8)Dem_ActiveConfig->events[index].eventId;
    Dem_Primary[(uint8)chosen].snapshot[1] = Dem_Runtime[index].status;
    Dem_Primary[(uint8)chosen].snapshot[2] = Dem_Runtime[index].occurrence;
    Dem_Primary[(uint8)chosen].snapshot[3] = (uint8)Dem_ChangeCounter;
    Dem_Primary[(uint8)chosen].snapshot[4] = (uint8)(Dem_ChangeCounter >> 8);
    Dem_Primary[(uint8)chosen].snapshot[5] = (uint8)(Dem_ChangeCounter >> 16);
}

static void Dem_ApplyQualified(uint8 index, uint8 failed)
{
    Dem_RuntimeEventType *event = &Dem_Runtime[index];
    uint8 old = event->status;
    event->status &= (uint8)~(DEM_UDS_STATUS_TNCSLC | DEM_UDS_STATUS_TNCTOC);
    if (failed != 0U)
    {
        event->status |= DEM_UDS_STATUS_TF | DEM_UDS_STATUS_TFTOC |
                         DEM_UDS_STATUS_PDTC | DEM_UDS_STATUS_TFSLC;
        if ((old & DEM_UDS_STATUS_TF) == 0U && event->occurrence != 0xFFU)
            ++event->occurrence;
        if ((old & DEM_UDS_STATUS_TFTOC) == 0U && event->confirmation != 0xFFU)
            ++event->confirmation;
        if (event->confirmation >= Dem_ActiveConfig->events[index].confirmationThreshold)
            event->status |= DEM_UDS_STATUS_CDTC;
        event->aging = 0U;
        Dem_AllocatePrimary(index);
    }
    else
    {
        event->status &= (uint8)~DEM_UDS_STATUS_TF;
    }
    if (old != event->status) {
        ++Dem_ChangeCounter;
        if(!Dem_CallbackPending[index])Dem_CallbackOld[index]=old;
        Dem_CallbackPending[index]=1;
    }
    Dem_ExportLegacy(index);
}

static uint8 Dem_Qualify(uint8 index, Dem_EventStatusType reported, uint8 *failed)
{
    Dem_RuntimeEventType *event = &Dem_Runtime[index];
    const Dem_EventConfigType *cfg = &Dem_ActiveConfig->events[index];
    if (reported == DEM_EVENT_STATUS_FAILED)
    {
        event->debounce = cfg->failedThreshold;
        *failed = 1U; return 1U;
    }
    if (reported == DEM_EVENT_STATUS_PASSED)
    {
        event->debounce = cfg->passedThreshold;
        *failed = 0U; return 1U;
    }
    if (reported == DEM_EVENT_STATUS_PREFAILED)
    {
        if (event->debounceFrozen != 0U) return 0U;
        if (event->debounce < cfg->failedThreshold) ++event->debounce;
        if (event->debounce >= cfg->failedThreshold)
        { *failed = 1U; return 1U; }
        return 0U;
    }
    if (reported == DEM_EVENT_STATUS_PREPASSED)
    {
        if (event->debounceFrozen != 0U) return 0U;
        if (event->debounce > cfg->passedThreshold) --event->debounce;
        if (event->debounce <= cfg->passedThreshold)
        { *failed = 0U; return 1U; }
    }
    return 0U;
}

void Dem_ValidateStoredDtc(void)
{
    uint8 i;
    for (i = 0U; i < Dem_Config.eventCount; ++i)
    {
        uint32 id = DEM_LEGACY_ARRAY[2U * i];
        uint32 state = DEM_LEGACY_ARRAY[2U * i + 1U];
        if (!((id == 0U && state == 0U) ||
              (id == Dem_Config.events[i].eventId && (state & 0xFFU) >= 1U &&
               (state & 0xFFU) <= 2U)))
        { DEM_LEGACY_ARRAY[2U * i] = 0U; DEM_LEGACY_ARRAY[2U * i + 1U] = 0U; }
    }
}

void Dem_PreInit(void)
{
    uint32 irq = SchM_Enter_Dem();
    memset(Dem_Runtime, 0, sizeof(Dem_Runtime));
    memset(Dem_Primary, 0, sizeof(Dem_Primary));
    memset(Dem_CallbackPending,0,sizeof(Dem_CallbackPending));
    memset(&Dem_Filter, 0, sizeof(Dem_Filter));
    memset(&Dem_Selection, 0, sizeof(Dem_Selection));
    Dem_ActiveConfig = &Dem_Config;
    Dem_State = DEM_PREINIT;
    Dem_CycleActive = 0U;
    Dem_DtcSettingEnabled = 1U;
    Dem_ClearStatus = DEM_CLEAR_IDLE;
    Dem_ChangeCounter = 0U;
    SchM_Exit_Dem(irq);
}

void Dem_Init(const Dem_ConfigType *ConfigPtr)
{
    uint8 i;
    uint8 pending[DEM_EVENT_COUNT];
    uint32 irq;
    if (Dem_State != DEM_PREINIT) Dem_PreInit();
    if (ConfigPtr == NULL_PTR) ConfigPtr = &Dem_Config;
    if (ConfigPtr->events == NULL_PTR || ConfigPtr->eventCount != DEM_EVENT_COUNT ||
        ConfigPtr->primaryMemorySize == 0U ||
        ConfigPtr->primaryMemorySize > DEM_PRIMARY_MEMORY_SIZE) return;
    irq = SchM_Enter_Dem();
    Dem_ActiveConfig = ConfigPtr;
    Dem_ValidateStoredDtc();
    for (i = 0U; i < ConfigPtr->eventCount; ++i)
    {
        uint32 word = DEM_LEGACY_ARRAY[2U * i + 1U];
        pending[i] = Dem_Runtime[i].queued;
        Dem_Runtime[i].queued = 0U;
        Dem_Runtime[i].status = DEM_UDS_STATUS_TNCSLC | DEM_UDS_STATUS_TNCTOC;
        if (DEM_LEGACY_ARRAY[2U * i] != 0U)
        {
            if ((word & 0xFFFFFF00UL) != 0U)
            {
                Dem_Runtime[i].status = (uint8)((word >> 8) & 0x3DU);
                Dem_Runtime[i].status |= DEM_UDS_STATUS_TNCTOC;
                Dem_Runtime[i].occurrence = (uint8)(word >> 16);
                Dem_Runtime[i].aging = (uint8)(word >> 24);
            }
            else if ((word & 0xFFU) == 2U)
            {
                Dem_Runtime[i].status = DEM_UDS_STATUS_TF | DEM_UDS_STATUS_PDTC |
                                        DEM_UDS_STATUS_CDTC | DEM_UDS_STATUS_TFSLC |
                                        DEM_UDS_STATUS_TNCTOC;
                Dem_Runtime[i].occurrence = 1U;
            }
            else Dem_Runtime[i].status = DEM_UDS_STATUS_TNCTOC;
        }
    }
    Dem_CycleActive = 1U; /* Configured default cycle starts with the ECU. */
    Dem_State = DEM_INITIALIZED;
    SchM_Exit_Dem(irq);
    for (i = 0U; i < ConfigPtr->eventCount; ++i)
    {
        if (pending[i] != 0U)
            (void)Dem_SetEventStatus(ConfigPtr->events[i].eventId,
                pending[i] == 2U ? DEM_EVENT_STATUS_FAILED : DEM_EVENT_STATUS_PASSED);
    }
}

void Dem_Shutdown(void)
{
    uint32 irq = SchM_Enter_Dem();
    if (Dem_State == DEM_INITIALIZED) Dem_State = DEM_SHUTDOWN;
    SchM_Exit_Dem(irq);
}

Dem_InitStateType Dem_GetInitState(void) { return Dem_State; }

Std_ReturnType Dem_SetEventStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatus)
{
    int index;
    uint8 failed = 0U;
    uint8 qualified;
    uint32 irq = SchM_Enter_Dem();
    if ((Dem_State != DEM_PREINIT && Dem_State != DEM_INITIALIZED) ||
        EventStatus > DEM_EVENT_STATUS_PREFAILED || Dem_ClearStatus == DEM_CLEAR_PENDING ||
        (index = Dem_FindEvent(EventId)) < 0 ||
        (Dem_State == DEM_INITIALIZED && Dem_CycleActive == 0U))
    { SchM_Exit_Dem(irq); return E_NOT_OK; }
    if (Dem_DtcSettingEnabled == 0U)
    { SchM_Exit_Dem(irq); return E_OK; }
    qualified = Dem_Qualify((uint8)index, EventStatus, &failed);
    if (qualified != 0U)
    {
        if (Dem_State == DEM_PREINIT) Dem_Runtime[index].queued = failed != 0U ? 2U : 1U;
        else Dem_ApplyQualified((uint8)index, failed);
    }
    SchM_Exit_Dem(irq);
    return E_OK;
}

Std_ReturnType Dem_ReportErrorStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatus)
{ return Dem_SetEventStatus(EventId, EventStatus); }

Std_ReturnType Dem_GetEventUdsStatus(Dem_EventIdType EventId, Dem_UdsStatusByteType *Status)
{
    int index;
    if (Dem_State != DEM_INITIALIZED || Status == NULL_PTR ||
        (index = Dem_FindEvent(EventId)) < 0) return E_NOT_OK;
    *Status = Dem_Runtime[index].status & Dem_ActiveConfig->statusAvailabilityMask;
    return E_OK;
}

Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType EventId, sint8 *Counter)
{
    int index;
    sint16 value;
    if (Dem_State != DEM_INITIALIZED || Counter == NULL_PTR ||
        (index = Dem_FindEvent(EventId)) < 0) return E_NOT_OK;
    value = Dem_Runtime[index].debounce;
    if (value >= Dem_ActiveConfig->events[index].failedThreshold) *Counter = 127;
    else if (value <= Dem_ActiveConfig->events[index].passedThreshold) *Counter = -128;
    else if (value >= 0) *Counter = (sint8)(value * 127 / Dem_ActiveConfig->events[index].failedThreshold);
    else *Counter = (sint8)(value * 128 / -Dem_ActiveConfig->events[index].passedThreshold);
    return E_OK;
}

Std_ReturnType Dem_ResetEventDebounceStatus(Dem_EventIdType EventId,
    Dem_DebounceResetStatusType DebounceResetStatus)
{
    int index;
    uint32 irq;
    if ((Dem_State != DEM_PREINIT && Dem_State != DEM_INITIALIZED) ||
        DebounceResetStatus > DEM_DEBOUNCE_STATUS_RESET ||
        (index = Dem_FindEvent(EventId)) < 0) return E_NOT_OK;
    irq = SchM_Enter_Dem();
    if (DebounceResetStatus == DEM_DEBOUNCE_STATUS_RESET)
    {
        Dem_Runtime[index].debounce = 0;
        Dem_Runtime[index].debounceFrozen = 0U;
    }
    else Dem_Runtime[index].debounceFrozen = 1U;
    SchM_Exit_Dem(irq);
    return E_OK;
}

Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType EventId, Dem_DTCFormatType Format, Dem_DTCType *DTC)
{
    int index;
    if (Dem_State != DEM_INITIALIZED || DTC == NULL_PTR ||
        (index = Dem_FindEvent(EventId)) < 0) return E_NOT_OK;
    if (Format != DEM_DTC_FORMAT_UDS) return DEM_E_NO_DTC_AVAILABLE;
    *DTC = Dem_ActiveConfig->events[index].dtc;
    return E_OK;
}

Std_ReturnType Dem_SelectDTC(uint8 ClientId, Dem_DTCType DTC,
    Dem_DTCFormatType Format, Dem_DTCOriginType Origin)
{
    uint32 irq;
    if (ClientId != 0U || Dem_State != DEM_INITIALIZED) return E_NOT_OK;
    if(Dem_ClearStatus==DEM_CLEAR_PENDING)return DEM_BUSY;
    irq = SchM_Enter_Dem();
    Dem_Selection.valid = 1U;
    Dem_Selection.dtc = DTC;
    Dem_Selection.format = Format;
    Dem_Selection.origin = Origin;
    SchM_Exit_Dem(irq);
    return E_OK;
}

Std_ReturnType Dem_GetDTCSelectionResult(uint8 ClientId)
{
    if (ClientId != 0U || Dem_State != DEM_INITIALIZED || Dem_Selection.valid == 0U)
        return E_NOT_OK;
    if (Dem_Selection.origin != DEM_DTC_ORIGIN_PRIMARY_MEMORY)
        return DEM_WRONG_DTCORIGIN;
    if (Dem_Selection.format != DEM_DTC_FORMAT_UDS ||
        (Dem_Selection.dtc != DEM_DTC_GROUP_ALL_DTCS &&
         Dem_FindDtc(Dem_Selection.dtc) < 0)) return DEM_WRONG_DTC;
    return E_OK;
}

Std_ReturnType Dem_GetDTCSelectionResultForClearDTC(uint8 ClientId)
{ return Dem_GetDTCSelectionResult(ClientId); }

Std_ReturnType Dem_GetStatusOfDTC(uint8 ClientId, Dem_UdsStatusByteType *Status)
{
    int index;
    if (ClientId != 0U || Dem_State != DEM_INITIALIZED || Status == NULL_PTR ||
        Dem_Selection.valid == 0U) return E_NOT_OK;
    if (Dem_Selection.origin != DEM_DTC_ORIGIN_PRIMARY_MEMORY)
        return DEM_WRONG_DTCORIGIN;
    if (Dem_Selection.format != DEM_DTC_FORMAT_UDS ||
        (index = Dem_FindDtc(Dem_Selection.dtc)) < 0) return DEM_WRONG_DTC;
    *Status = Dem_Runtime[index].status & Dem_ActiveConfig->statusAvailabilityMask;
    return E_OK;
}

Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType Cycle,
                                           Dem_OperationCycleStateType State)
{
    uint8 i;
    uint32 irq;
    if (Dem_State != DEM_INITIALIZED || Cycle != Dem_ActiveConfig->operationCycle ||
        State > DEM_CYCLE_STATE_START) return E_NOT_OK;
    irq = SchM_Enter_Dem();
    if ((State == DEM_CYCLE_STATE_START && Dem_CycleActive != 0U) ||
        (State == DEM_CYCLE_STATE_END && Dem_CycleActive == 0U))
    { SchM_Exit_Dem(irq); return E_NOT_OK; }
    Dem_CycleActive = State == DEM_CYCLE_STATE_START ? 1U : 0U;
    for (i = 0U; i < Dem_ActiveConfig->eventCount; ++i)
    {
        Dem_RuntimeEventType *event = &Dem_Runtime[i];
        if (State == DEM_CYCLE_STATE_END)
        {
            if ((event->status & (DEM_UDS_STATUS_TFTOC | DEM_UDS_STATUS_TNCTOC)) == 0U)
            {
                event->status &= (uint8)~DEM_UDS_STATUS_PDTC;
                if ((event->status & DEM_UDS_STATUS_CDTC) != 0U && event->aging != 0xFFU)
                {
                    ++event->aging;
                    if (event->aging >= Dem_ActiveConfig->events[i].agingThreshold)
                        event->status &= (uint8)~DEM_UDS_STATUS_CDTC;
                }
            }
        }
        else
        {
            event->status &= (uint8)~DEM_UDS_STATUS_TFTOC;
            event->status |= DEM_UDS_STATUS_TNCTOC;
        }
        if (DEM_LEGACY_ARRAY[2U * i] != 0U) Dem_ExportLegacy(i);
    }
    SchM_Exit_Dem(irq);
    return E_OK;
}

Std_ReturnType Dem_ClearDTC(uint8 ClientId)
{
    uint8 i, found = 0U;
    uint32 irq;
    Dem_DTCType DTC;
    {
        Std_ReturnType result = Dem_GetDTCSelectionResultForClearDTC(ClientId);
        if (result != E_OK)
        { Dem_ClearStatus = DEM_CLEAR_RESULT_FAILED; return result; }
    }
    DTC = Dem_Selection.dtc;
    irq = SchM_Enter_Dem();
    if (Dem_ClearStatus == DEM_CLEAR_PENDING) { SchM_Exit_Dem(irq); return DEM_PENDING; }
    Dem_Serialize(Dem_ClearBackup);
    Dem_ClearStatus = DEM_CLEAR_PENDING;
    for (i = 0U; i < Dem_ActiveConfig->eventCount; ++i)
    {
        uint8 slot;
        if (DTC != DEM_DTC_GROUP_ALL_DTCS && Dem_ActiveConfig->events[i].dtc != DTC) continue;
        found = 1U;
        memset(&Dem_Runtime[i], 0, sizeof(Dem_Runtime[i]));
        Dem_Runtime[i].status = DEM_UDS_STATUS_TNCSLC | DEM_UDS_STATUS_TNCTOC;
        DEM_LEGACY_ARRAY[2U * i] = 0U;
        DEM_LEGACY_ARRAY[2U * i + 1U] = 0U;
        for (slot = 0U; slot < Dem_ActiveConfig->primaryMemorySize; ++slot)
            if (Dem_Primary[slot].valid != 0U && Dem_Primary[slot].eventIndex == i)
                Dem_Primary[slot].valid = 0U;
    }
    Dem_ClearStatus = found != 0U ? DEM_CLEAR_PENDING : DEM_CLEAR_RESULT_FAILED;
    if (found != 0U) ++Dem_ChangeCounter;
    SchM_Exit_Dem(irq);
    if(found != 0U) {
        Dem_Serialize(NvM_DemImage);
        if(NvM_WriteBlock(NVM_BLOCK_DEM,NvM_DemImage)!=0U) {
            (void)Dem_Deserialize(Dem_ClearBackup); Dem_ClearStatus=DEM_CLEAR_RESULT_FAILED; return DEM_CLEAR_FAILED;
        }
    }
    return found != 0U ? DEM_PENDING : DEM_CLEAR_FAILED;
}

Dem_ClearDTCStatusType Dem_GetClearDTCStatus(void) { return Dem_ClearStatus; }

Std_ReturnType Dem_SetDTCFilter(uint8 ClientId, uint8 StatusMask, Dem_DTCFormatType Format,
                                Dem_DTCOriginType Origin, boolean FilterWithSeverity,
                                Dem_DTCSeverityType SeverityMask,
                                boolean FilterForFaultDetectionCounter)
{
    if (ClientId != 0U || Dem_State != DEM_INITIALIZED || Dem_ValidAddressing(Format, Origin) == 0U ||
        FilterWithSeverity != FALSE ||
        FilterForFaultDetectionCounter != FALSE) return E_NOT_OK;
    (void)SeverityMask;
    Dem_Filter.valid = 1U;
    Dem_Filter.statusMask = StatusMask & Dem_ActiveConfig->statusAvailabilityMask;
    Dem_Filter.nextIndex = 0U;
    return E_OK;
}

Std_ReturnType Dem_GetNumberOfFilteredDTC(uint8 ClientId, uint16 *Count)
{
    uint8 i;
    if (ClientId != 0U || Count == NULL_PTR || Dem_State != DEM_INITIALIZED || Dem_Filter.valid == 0U) return E_NOT_OK;
    *Count = 0U;
    for (i = 0U; i < Dem_ActiveConfig->eventCount; ++i)
        if (Dem_Filter.statusMask == 0U ||
            (Dem_Runtime[i].status & Dem_Filter.statusMask) != 0U) ++*Count;
    return E_OK;
}

Std_ReturnType Dem_GetNumberOfEventMemoryEntries(uint8 ClientId,
    Dem_DTCOriginType Origin, uint8 *Count)
{
    uint8 i;
    if (ClientId != 0U || Origin != DEM_DTC_ORIGIN_PRIMARY_MEMORY ||
        Count == NULL_PTR || Dem_State != DEM_INITIALIZED) return E_NOT_OK;
    *Count = 0U;
    for (i = 0U; i < Dem_ActiveConfig->primaryMemorySize; ++i)
        if (Dem_Primary[i].valid != 0U) ++*Count;
    return E_OK;
}

Std_ReturnType Dem_GetNextFilteredDTC(uint8 ClientId, Dem_DTCType *DTC, Dem_UdsStatusByteType *Status)
{
    uint8 i;
    if (ClientId != 0U || DTC == NULL_PTR || Status == NULL_PTR || Dem_State != DEM_INITIALIZED ||
        Dem_Filter.valid == 0U) return E_NOT_OK;
    for (i = Dem_Filter.nextIndex; i < Dem_ActiveConfig->eventCount; ++i)
    {
        if (Dem_Filter.statusMask == 0U ||
            (Dem_Runtime[i].status & Dem_Filter.statusMask) != 0U)
        {
            *DTC = Dem_ActiveConfig->events[i].dtc;
            *Status = Dem_Runtime[i].status & Dem_ActiveConfig->statusAvailabilityMask;
            Dem_Filter.nextIndex = (uint8)(i + 1U);
            return E_OK;
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 ClientId,
    Dem_UdsStatusByteType *StatusMask, Dem_DTCOriginType Origin)
{
    if (ClientId != 0U || StatusMask == NULL_PTR ||
        Origin != DEM_DTC_ORIGIN_PRIMARY_MEMORY || Dem_State != DEM_INITIALIZED)
        return E_NOT_OK;
    *StatusMask = Dem_ActiveConfig->statusAvailabilityMask;
    return E_OK;
}

Std_ReturnType Dem_SetDtcSetting(boolean Enabled)
{
    if (Dem_State != DEM_INITIALIZED) return E_NOT_OK;
    Dem_DtcSettingEnabled = Enabled != FALSE ? 1U : 0U;
    return E_OK;
}

Std_ReturnType Dem_DisableDTCSetting(uint8 ClientId)
{ return ClientId == 0U ? Dem_SetDtcSetting(FALSE) : E_NOT_OK; }

Std_ReturnType Dem_EnableDTCSetting(uint8 ClientId)
{ return ClientId == 0U ? Dem_SetDtcSetting(TRUE) : E_NOT_OK; }

Std_ReturnType Dem_GetSnapshotDataByDTC(Dem_DTCType DTC, Dem_DTCFormatType Format,
    Dem_DTCOriginType Origin, uint8 RecordNumber, uint8 *DestBuffer, uint16 *BufSize)
{
    uint8 i;
    int index;
    if (Dem_State != DEM_INITIALIZED || Dem_ValidAddressing(Format, Origin) == 0U ||
        RecordNumber != 1U || DestBuffer == NULL_PTR || BufSize == NULL_PTR ||
        (index = Dem_FindDtc(DTC)) < 0 || *BufSize < sizeof(Dem_Primary[0].snapshot))
        return E_NOT_OK;
    for (i = 0U; i < Dem_ActiveConfig->primaryMemorySize; ++i)
    {
        if (Dem_Primary[i].valid != 0U && Dem_Primary[i].eventIndex == (uint8)index)
        {
            memcpy(DestBuffer, Dem_Primary[i].snapshot, sizeof(Dem_Primary[i].snapshot));
            *BufSize = sizeof(Dem_Primary[i].snapshot);
            return E_OK;
        }
    }
    return E_NOT_OK;
}

/* Configured record 1 stores occurrence, confirmation and aging counters. */
Std_ReturnType Dem_GetExtendedDataRecordByDTC(Dem_DTCType DTC, Dem_DTCFormatType Format,
    Dem_DTCOriginType Origin, uint8 RecordNumber, uint8 *DestBuffer, uint16 *BufSize)
{
    int index;
    if (Dem_State != DEM_INITIALIZED || Dem_ValidAddressing(Format, Origin) == 0U ||
        RecordNumber != 1U || DestBuffer == NULL_PTR || BufSize == NULL_PTR ||
        *BufSize < 3U || (index = Dem_FindDtc(DTC)) < 0) return E_NOT_OK;
    DestBuffer[0] = Dem_Runtime[index].occurrence;
    DestBuffer[1] = Dem_Runtime[index].confirmation;
    DestBuffer[2] = Dem_Runtime[index].aging;
    *BufSize = 3U;
    return E_OK;
}

void Dem_MainFunction(void)
{
    uint8 i;
    NvM_RequestResultType result;
    for(i=0;i<DEM_EVENT_COUNT;++i) {
        uint8 old,now,notify;uint32 irq=SchM_Enter_Dem();
        notify=Dem_CallbackPending[i];old=Dem_CallbackOld[i];now=Dem_Runtime[i].status;Dem_CallbackPending[i]=0;
        SchM_Exit_Dem(irq);
        if(notify && Dem_ActiveConfig->statusChanged)Dem_ActiveConfig->statusChanged(Dem_ActiveConfig->events[i].eventId,old,now);
    }
    if(Dem_ClearStatus==DEM_CLEAR_PENDING && NvM_GetErrorStatus(NVM_BLOCK_DEM,&result)==0U && result!=NVM_REQ_PENDING) {
        if(result==NVM_REQ_OK) Dem_ClearStatus=DEM_CLEAR_OK;
        else { (void)Dem_Deserialize(Dem_ClearBackup); Dem_ClearStatus=DEM_CLEAR_RESULT_FAILED; }
    }
}

/* Stable byte schema; compiler padding and native pointers are never persisted. */
void Dem_Serialize(uint8 *p) {
    uint8 i; uint16 at=16U; uint32 irq=SchM_Enter_Dem();
    memset(p,0,512U); p[0]='D'; p[1]='E'; p[2]='M'; p[3]=2U;
    p[4]=DEM_EVENT_COUNT; p[5]=DEM_PRIMARY_MEMORY_SIZE;
    memcpy(p+8,&Dem_ChangeCounter,4U);
    for(i=0;i<DEM_EVENT_COUNT;++i) {
        p[at]=Dem_Runtime[i].status; p[at+1]=Dem_Runtime[i].confirmation;
        p[at+2]=Dem_Runtime[i].occurrence; p[at+3]=Dem_Runtime[i].aging;
        memcpy(p+at+4,&Dem_Runtime[i].debounce,2U); at+=8U;
    }
    for(i=0;i<DEM_PRIMARY_MEMORY_SIZE;++i) {
        p[at]=Dem_Primary[i].valid; p[at+1]=Dem_Primary[i].eventIndex;
        memcpy(p+at+2,&Dem_Primary[i].allocationOrder,4U);
        memcpy(p+at+6,Dem_Primary[i].snapshot,6U); at+=12U;
    }
    SchM_Exit_Dem(irq);
}
uint8 Dem_Deserialize(const uint8 *p) {
    uint8 i; uint16 at=16U; uint32 irq;
    if(p[0]!='D'||p[1]!='E'||p[2]!='M'||p[3]!=2U||p[4]!=DEM_EVENT_COUNT||p[5]!=DEM_PRIMARY_MEMORY_SIZE) return 1U;
    for(i=0;i<DEM_PRIMARY_MEMORY_SIZE;++i) {
        uint16 off=(uint16)(16U+DEM_EVENT_COUNT*8U+i*12U);
        if(p[off]>1U || (p[off] && p[off+1]>=DEM_EVENT_COUNT)) return 1U;
    }
    irq=SchM_Enter_Dem(); memcpy(&Dem_ChangeCounter,p+8,4U);
    for(i=0;i<DEM_EVENT_COUNT;++i) {
        Dem_Runtime[i].status=p[at]; Dem_Runtime[i].confirmation=p[at+1];
        Dem_Runtime[i].occurrence=p[at+2]; Dem_Runtime[i].aging=p[at+3];
        memcpy(&Dem_Runtime[i].debounce,p+at+4,2U); Dem_ExportLegacy(i); at+=8U;
    }
    for(i=0;i<DEM_PRIMARY_MEMORY_SIZE;++i) {
        Dem_Primary[i].valid=p[at]; Dem_Primary[i].eventIndex=p[at+1];
        memcpy(&Dem_Primary[i].allocationOrder,p+at+2,4U);
        memcpy(Dem_Primary[i].snapshot,p+at+6,6U); at+=12U;
    }
    SchM_Exit_Dem(irq); return 0U;
}

void Dem_StartRestoredCycle(void) {
 uint8 i;uint32 irq=SchM_Enter_Dem();
 for(i=0;i<DEM_EVENT_COUNT;++i){Dem_Runtime[i].status&=(uint8)~DEM_UDS_STATUS_TFTOC;Dem_Runtime[i].status|=DEM_UDS_STATUS_TNCTOC;Dem_ExportLegacy(i);}
 Dem_CycleActive=1U;SchM_Exit_Dem(irq);
}
