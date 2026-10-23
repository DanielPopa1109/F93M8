#include <assert.h>
#include <string.h>
#include "../BSW/Com/Can/Can.h"
#include "../BSW/Com/CanIf/CanIf.h"
#include "../BSW/Com/CanTp/CanTp.h"
#include "../BSW/Com/PduR/PduR.h"
#include "../BSW/Diag/Dcm/Dcm.h"
#include "../BSW/Com/CanH/CanH.h"
#include "../BSW/Diag/Dem/Dem.h"

static Can_FrameType sent[128];
static unsigned sentCount, resetCount, nvmCount, clearCount, filterIndex;
uint32 DataRecorder_KilometerPerDcy, DataRecorder_KilometerTotal;
uint32 Nvm_LastWriteResult;
uint8 CanH_RequestBusSleep;
uint32 CanH_NoCommCounter;
CanH_ComStat_t CanH_CommunicationState;
uint8 DigitalCluster_IsSelfTestRequested;
uint8 RevCam_RxSig_ReverseCameraRequest, DigitalCluster_RxSig_ReverseCameraRequest;
uint32 RevCam_DcmiStatus, RevCam_I2cStatus;
uint8 RevCam_InitStatus, DigitalCluster_FsmcDmaErrorFlag;
uint8 DigitalCluster_LcdInit, DigitalCluster_ShutOffDisplayFlag;
void EcuM_PerformReset(uint8 param) { (void)param; ++resetCount; }
Std_ReturnType Dem_SelectDTC(uint8 clientId, Dem_DTCType dtc,
    Dem_DTCFormatType format, Dem_DTCOriginType origin)
{ (void)clientId; (void)dtc; (void)format; (void)origin; return E_OK; }
Std_ReturnType Dem_GetDTCSelectionResultForClearDTC(uint8 clientId)
{ (void)clientId; return E_OK; }
Std_ReturnType Dem_ClearDTC(uint8 clientId)
{ (void)clientId; ++clearCount; return E_OK; }
void DiagMonitor_ClearFaultSources(void) {}
Std_ReturnType Dem_EnableDTCSetting(uint8 clientId) { (void)clientId; return E_OK; }
Std_ReturnType Dem_DisableDTCSetting(uint8 clientId) { (void)clientId; return E_OK; }
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 clientId,
    Dem_UdsStatusByteType *mask, Dem_DTCOriginType origin)
{ (void)clientId; (void)origin; *mask = DEM_UDS_STATUS_AVAILABILITY_MASK; return E_OK; }
Std_ReturnType Dem_SetDTCFilter(uint8 clientId, uint8 mask, Dem_DTCFormatType format,
    Dem_DTCOriginType origin, boolean severity, Dem_DTCSeverityType severityMask,
    boolean faultCounter)
{ (void)clientId; (void)mask; (void)format; (void)origin; (void)severity; (void)severityMask;
  (void)faultCounter; filterIndex = 0U; return E_OK; }
Std_ReturnType Dem_GetNextFilteredDTC(uint8 clientId, Dem_DTCType *dtc, Dem_UdsStatusByteType *status)
{ (void)clientId; if (filterIndex >= DEM_EVENT_COUNT) return E_NOT_OK;
  *dtc = Dem_Config.events[filterIndex].dtc; *status = 0U; ++filterIndex; return E_OK; }
static const Dem_EventConfigType testEvents[DEM_EVENT_COUNT] = {
    {0x0EU, 0x0EU, 1U, 2, -2, 1U, 40U},
    {0x0FU, 0x0FU, 1U, 2, -2, 1U, 40U},
    {0x10U, 0x10U, 1U, 2, -2, 1U, 40U},
    {0x11U, 0x11U, 1U, 2, -2, 1U, 40U},
    {0x12U, 0x12U, 1U, 2, -2, 1U, 40U},
    {0x13U, 0x13U, 1U, 2, -2, 1U, 40U},
    {0x14U, 0x14U, 1U, 2, -2, 1U, 40U},
    {0x15U, 0x15U, 1U, 2, -2, 1U, 40U},
    {0x16U, 0x16U, 1U, 2, -2, 1U, 40U}
};
const Dem_ConfigType Dem_Config = {testEvents, DEM_EVENT_COUNT,
    DEM_PRIMARY_MEMORY_SIZE, DEM_DEFAULT_OPERATION_CYCLE,
    DEM_UDS_STATUS_AVAILABILITY_MASK};
void Nvm_WriteAll(void) { ++nvmCount; }
Can_StatusType Can_TestTransmit(uint16_t id, const uint8_t *data, uint8_t dlc)
{
    assert(sentCount < sizeof(sent) / sizeof(sent[0]));
    sent[sentCount].id = id; sent[sentCount].dlc = dlc;
    memcpy(sent[sentCount].data, data, dlc); ++sentCount;
    return CAN_OK;
}
static void reset(void)
{
    sentCount = resetCount = nvmCount = clearCount = filterIndex = 0U;
    Can_Init(); CanIf_Init(); CanTp_Init(); PduR_Init(); Dcm_Init();
}
static void receive(uint16_t id, const uint8_t data[8], uint8_t dlc)
{
    Can_FrameType frame = {0};
    frame.id = id; frame.dlc = dlc;
    memcpy(frame.data, data, dlc);
    Can_RxIndication(&frame);
    Can_MainFunction();
}
int main(void)
{
    static const uint8_t tester[8] = {0x56, 0x02, 0x3E, 0x00};
    static const uint8_t f186[8] = {0x56, 0x03, 0x22, 0xF1, 0x86};
    static const uint8_t multiFf[8] = {0x56, 0x10, 0x09, 0x22, 0xF1, 0x86, 0xF1, 0x86};
    static const uint8_t multiCf[8] = {0x56, 0x21, 0xF1, 0x86, 0xF1, 0x86};
    static const uint8_t badCf[8] = {0x56, 0x22, 0xF1, 0x86, 0xF1, 0x86};
    static const uint8_t fc[8] = {0x41, 0x30, 0, 0};
    static const uint8_t large[8] = {0x56, 0x11, 0x01, 1, 2, 3, 4, 5};
    static const uint8_t dtc[8] = {0x56, 0x02, 0x19, 0x0A};
    static const uint8_t limitedFc[8] = {0x41, 0x30, 1, 10};
    static const uint8_t pacedFc[8] = {0x41, 0x30, 0, 10};
    unsigned i;
    reset();
    receive(0x710, tester, 8); assert(sentCount == 0);
    Dcm_MainFunction();
    assert(sentCount == 1 && sent[0].id == 0x710 && sent[0].dlc == 8);
    assert(sent[0].data[0] == 0xF0 && sent[0].data[1] == 2 && sent[0].data[2] == 0x7E);
    reset();
    receive(0x710, f186, 8); Dcm_MainFunction();
    assert(sentCount == 1 && sent[0].data[1] == 4 && sent[0].data[2] == 0x62);
    assert(sent[0].data[5] == 1);
    reset();
    receive(0x710, multiFf, 8);
    assert(sentCount == 1 && sent[0].data[0] == 0xF0 && sent[0].data[1] == 0x30);
    receive(0x710, multiCf, 8); Dcm_MainFunction();
    assert(sentCount == 2 && sent[1].data[1] == 0x10 && sent[1].data[2] == 13);
    receive(0x710, fc, 8); CanTp_MainFunction();
    assert(sentCount == 3 && sent[2].data[1] == 0x21 && sent[2].data[2] == 0x86);
    reset();
    receive(0x710, multiFf, 8); receive(0x710, badCf, 8);
    assert(PduR_GetRxErrorCount() == 1 && sentCount == 1);
    reset();
    receive(0x710, multiFf, 8);
    for (i = 0; i < 201; ++i) CanTp_MainFunction();
    receive(0x710, multiCf, 8);
    assert(PduR_GetRxErrorCount() == 1);
    reset();
    receive(0x711, tester, 8);
    { uint8_t wrong[8] = {0x54, 2, 0x3E, 0}; receive(0x710, wrong, 8); }
    Dcm_MainFunction(); assert(sentCount == 0);
    reset();
    receive(0x710, large, 8);
    assert(sentCount == 1 && sent[0].data[1] == 0x32);
    reset();
    receive(0x710, multiFf, 8); receive(0x710, multiCf, 8); Dcm_MainFunction();
    for (i = 0; i < 201; ++i) CanTp_MainFunction();
    assert(PduR_GetTxErrorCount() == 1);
    reset();
    receive(0x710, dtc, 8); Dcm_MainFunction();
    assert(sentCount == 1 && sent[0].data[1] == 0x10 && sent[0].data[2] == 39);
    receive(0x710, limitedFc, 8);
    CanTp_MainFunction();
    assert(sentCount == 2 && sent[1].data[1] == 0x21);
    for (i = 0; i < 5; ++i) CanTp_MainFunction();
    assert(sentCount == 2); /* block size one requires another FC */
    receive(0x710, fc, 8);
    CanTp_MainFunction(); CanTp_MainFunction(); CanTp_MainFunction();
    assert(sentCount == 4 && sent[3].data[1] == 0x23);
    reset();
    receive(0x710, dtc, 8); Dcm_MainFunction();
    receive(0x710, pacedFc, 4);
    CanTp_MainFunction(); assert(sentCount == 2);
    CanTp_MainFunction(); assert(sentCount == 2);
    CanTp_MainFunction(); assert(sentCount == 3);
    reset();
    {
        uint8_t ff256[8] = {0x56, 0x11, 0x00, 0x99, 0, 0, 0, 0};
        uint8_t cf256[8] = {0x56, 0x20, 0, 0, 0, 0, 0, 0};
        unsigned remaining = 251;
        receive(0x710, ff256, 8);
        for (i = 0; remaining != 0; ++i)
        {
            unsigned chunk = remaining > 6 ? 6 : remaining;
            cf256[1] = (uint8_t)(0x20U | ((i + 1U) & 0x0FU));
            receive(0x710, cf256, (uint8_t)(chunk + 2U));
            remaining -= chunk;
        }
        Dcm_MainFunction();
        assert(PduR_GetRxErrorCount() == 0 && sent[sentCount - 1].data[2] == 0x7F);
        assert(sent[sentCount - 1].data[3] == 0x99);
    }
    reset();
    receive(0x710, tester, 8); receive(0x710, tester, 8);
    assert(Dcm_DroppedRequests == 1);
    Dcm_MainFunction();
    reset();
    {
        Can_FrameType frame = {0};
        frame.id = 0x710; frame.dlc = 8;
        memcpy(frame.data, tester, 8);
        for (i = 0; i < 16; ++i) Can_RxIndication(&frame);
        assert(Can_RxDroppedCount == 1);
        Can_MainFunction();
        assert(Dcm_DroppedRequests == 14);
    }
    return 0;
}
