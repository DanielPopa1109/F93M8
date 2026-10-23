#include "DiagMonitor.h"
#include "Dem.h"
#include "DigitalCluster.h"

/* Hardware and application fault evaluation belongs to the monitor, not DEM. */
__attribute__((section(".ccmram"))) uint8 Dem_DigitalClusterErrorStatus = 0U;
__attribute__((section(".ccmram"))) uint8 Dem_ReverseCameraErrorStatus = 0U;

extern uint32 CanH_ErrArr[22];
extern uint32 I2cH_ErrArr[9];
extern uint32 DcmiH_ErrArr[4];
extern uint32 DcmiH_DmaErrArr[7];
extern uint32 RevCam_DcmiStatus, RevCam_I2cStatus;
extern uint8 RevCam_InitStatus;
extern uint32 RevCam_I2cInitRetry, RevCam_DcmiInitRetry, RevCam_OV7670InitRetry;
extern uint32 FsmcH_Status, FsmcH_InitRetry;
extern uint8 DigitalCluster_RetValInit;
extern uint32 DigitalCluster_InitRetry;
extern uint16 DigitalCluster_ReadDisplayStatus_RegisterValue;
extern uint8 DigitalCluster_ReadDisplayPowerMode_RegisterValue;

void DiagMonitor_ClearFaultSources(void)
{
    uint8 i;
    for (i = 0U; i < 22U; ++i) CanH_ErrArr[i] = 0U;
    for (i = 0U; i < 9U; ++i) I2cH_ErrArr[i] = 0U;
    for (i = 0U; i < 4U; ++i) DcmiH_ErrArr[i] = 0U;
    for (i = 0U; i < 7U; ++i) DcmiH_DmaErrArr[i] = 0U;
}

void DiagMonitor_MainFunction(void)
{
    uint8 i;
    uint8 communicationFailed = 0U;
    uint8 cameraFailed = 0U;
    uint8 clusterFailed;

    for (i = 0U; i < 22U; ++i)
        if (CanH_ErrArr[i] != 0U) communicationFailed = 1U;
    for (i = 0U; i < 9U; ++i)
        if (I2cH_ErrArr[i] != 0U) cameraFailed = 1U;
    for (i = 0U; i < 4U; ++i)
        if (DcmiH_ErrArr[i] != 0U) cameraFailed = 1U;
    for (i = 0U; i < 7U; ++i)
        if (DcmiH_DmaErrArr[i] != 0U) cameraFailed = 1U;
    if (RevCam_DcmiInitRetry > 10U || RevCam_I2cInitRetry > 10U ||
        RevCam_OV7670InitRetry > 10U || RevCam_DcmiStatus != 0U ||
        RevCam_I2cStatus != 0U ||
        (RevCam_InitStatus != 0U && RevCam_InitStatus != 0xFFU)) cameraFailed = 1U;
    clusterFailed = (FsmcH_Status == 4U || FsmcH_InitRetry > 10U ||
        (DigitalCluster_RetValInit != 0U && DigitalCluster_RetValInit != 0xFFU) ||
        DigitalCluster_InitRetry > 10U ||
        DIGITALCLUSTER_LCD_DISPLAYSTATUS_REGISTERVALUE != DigitalCluster_ReadDisplayStatus_RegisterValue ||
        DIGITALCLUSTER_LCD_POWERMODE_REGISTERVALUE != DigitalCluster_ReadDisplayPowerMode_RegisterValue) ? 1U : 0U;

    (void)Dem_SetEventStatus(DEM_COMMUNICATION_ERROR_ID,
        communicationFailed != 0U ? DEM_EVENT_STATUS_FAILED : DEM_EVENT_STATUS_PASSED);
    (void)Dem_SetEventStatus(DEM_REVERSECAMERA_DCMIINTERFACE_MALFUNCTION_ID,
        cameraFailed != 0U ? DEM_EVENT_STATUS_FAILED : DEM_EVENT_STATUS_PASSED);
    (void)Dem_SetEventStatus(DEM_DIGITALCLUSTER_CONTROLINTERFACE_MALFUNCTION_ID,
        clusterFailed != 0U ? DEM_EVENT_STATUS_FAILED : DEM_EVENT_STATUS_PASSED);
    Dem_ReverseCameraErrorStatus = cameraFailed;
    Dem_DigitalClusterErrorStatus = clusterFailed;
}
