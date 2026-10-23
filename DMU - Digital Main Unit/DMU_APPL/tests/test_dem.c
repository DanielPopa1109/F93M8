#include <assert.h>
#include <string.h>
#include "../BSW/Diag/Dem/Dem.h"
#include "../APPL/DiagMonitor/DiagMonitor.h"
#include "../APPL/DigitalCluster/DigitalCluster.h"

uint32 CanH_ErrArr[22];
uint32 I2cH_ErrArr[9];
uint32 DcmiH_ErrArr[4];
uint32 DcmiH_DmaErrArr[7];
uint32 RevCam_DcmiStatus, RevCam_I2cStatus;
uint8 RevCam_InitStatus;
uint32 RevCam_I2cInitRetry, RevCam_DcmiInitRetry, RevCam_OV7670InitRetry;
uint32 FsmcH_Status, FsmcH_InitRetry;
uint8 DigitalCluster_RetValInit;
uint32 DigitalCluster_InitRetry;
uint16 DigitalCluster_ReadDisplayStatus_RegisterValue;
uint8 DigitalCluster_ReadDisplayPowerMode_RegisterValue;

extern uint8 Dem_DigitalClusterErrorStatus, Dem_ReverseCameraErrorStatus;

int main(void)
{
    Dem_UdsStatusByteType status;
    memset(Dem_DTCArray, 0, sizeof(Dem_DTCArray));
    Dem_PreInit();
    Dem_Init(&Dem_Config);
    DigitalCluster_ReadDisplayStatus_RegisterValue =
        DIGITALCLUSTER_LCD_DISPLAYSTATUS_REGISTERVALUE;
    DigitalCluster_ReadDisplayPowerMode_RegisterValue =
        DIGITALCLUSTER_LCD_POWERMODE_REGISTERVALUE;
    CanH_ErrArr[21] = 1U;
    I2cH_ErrArr[0] = 1U;
    DiagMonitor_MainFunction();
    assert(Dem_GetEventUdsStatus(DEM_COMMUNICATION_ERROR_ID, &status) == E_OK);
    assert((status & DEM_UDS_STATUS_TF) != 0U);
    assert(Dem_GetEventUdsStatus(DEM_REVERSECAMERA_DCMIINTERFACE_MALFUNCTION_ID,
                                 &status) == E_OK);
    assert((status & DEM_UDS_STATUS_TF) != 0U);
    assert(Dem_ReverseCameraErrorStatus == 1U && Dem_DigitalClusterErrorStatus == 0U);
    DiagMonitor_ClearFaultSources();
    assert(CanH_ErrArr[21] == 0U && I2cH_ErrArr[0] == 0U);
    DiagMonitor_MainFunction();
    assert(Dem_GetEventUdsStatus(DEM_COMMUNICATION_ERROR_ID, &status) == E_OK);
    assert((status & DEM_UDS_STATUS_TF) == 0U);
    return 0;
}
