#include "CanH.h"
#include "../CanNm/CanNm.h"
#include "can.h"
#include "Dem.h"
#include "rtc.h"
#include "../Can/Can.h"
#include <string.h>
#include "../Com/Com.h"
#include "../CanSM/CanSM.h"
#include "../CanIf/CanIf.h"

__attribute__((section(".ccmram"))) uint32 CanH_MainCounter = 0;
__attribute__((section(".ccmram"))) uint32 CanH_Status = 0;
__attribute__((section(".ccmram"))) uint32 CanH_ErrArr[22] = {0};
__attribute__((section(".ccmram"))) uint32 CanH_RxFifo0MsgPendingCnt = 0;
__attribute__((section(".ccmram"))) uint32 CanH_RxFifo0FullCnt = 0;
__attribute__((section(".ccmram"))) CAN_RxHeaderTypeDef CanH_RxHeader = {0, 0, 0, 0, 0, 0, 0};
__attribute__((section(".ccmram"))) uint8 CanH_RxData[8] = {0};
__attribute__((section(".ccmram"))) CAN_TxHeaderTypeDef CanH_TxHeader = {0, 0, 0, 0, 0, 0};
__attribute__((section(".ccmram"))) uint8 CanH_TxData[8] = {0};
__attribute__((section(".ccmram"))) uint32 CanH_TxMailbox = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RequestBusSleep = 0;
__attribute__((section(".ccmram"))) uint32 CanH_NoCommCounter = 0;
__attribute__((section(".ccmram"))) CanH_ComStat_t CanH_CommunicationState = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_Networkmanagement3 = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_Ignition = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_Speed = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_Rpm = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_CurrentConsumption = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_Gear = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_TemperatureSensor = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_DisplayMode = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_RotaryLightSwitch = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_TurnSignal = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_HighBeam = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_CcmId = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_IrSensStat = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_DrivecycleStatus = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_FogLights = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_Recirculation = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_FanValue = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_ClimaTemp = 0;
__attribute__((section(".ccmram"))) uint8 CanH_RxSig_AutoClimate = 0;
__attribute__((section(".ccmram"))) uint32 CanH_VehState_MissCnt = 0;
__attribute__((section(".ccmram"))) uint32 CanH_BodyState_MissCnt = 0;
__attribute__((section(".ccmram"))) uint32 CanH_BodyState2_MissCnt = 0;
__attribute__((section(".ccmram"))) uint32 CanH_BusSystemDateAndTime_MissCnt = 0;
__attribute__((section(".ccmram"))) uint32 CanH_DataRecorder_MissCnt = 0;
__attribute__((section(".ccmram"))) uint8 CanH_AliveCounter = 0;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_ReadDisplayPowerMode_RegisterValue;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_LcdInit;
extern __attribute__((section(".ccmram"))) uint16 DigitalCluster_ReadDisplayStatus_RegisterValue;
extern __attribute__((section(".ccmram"))) uint32 RevCam_DcmiStatus;
extern __attribute__((section(".ccmram"))) uint32 RevCam_I2cStatus;
extern __attribute__((section(".ccmram"))) uint8 RevCam_InitStatus;
extern __attribute__((section(".ccmram"))) uint32 DataRecorder_RxSig_VehicleSpeed;
extern __attribute__((section(".ccmram"))) uint8 DataRecorder_RxSig_DriveCycleStatus;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_IgnitionStatus;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_Gear;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_HighBeamStatus;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_VehicleSpeed;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_MotorRpm;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_TurnSignals;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_DisplayMode;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_CollisionWarning;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_ReverseCameraRequest;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_CheckControlMessageId;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_OutsideTemperature;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_FogLightFront;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_FogLightRear;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_Rls;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_Recirculation;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_RequestedTemperature;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_AutoClimate;
extern __attribute__((section(".ccmram"))) uint8 DigitalCluster_RxSig_FanValue;
extern __attribute__((section(".ccmram"))) uint8 EcuM_StopModeActive;
extern __attribute__((section(".ccmram"))) uint32 DataRecorder_KilometerTotal;
extern __attribute__((section(".ccmram"))) uint32 DataRecorder_KilometerPerDcy;
extern CAN_HandleTypeDef hcan1;
extern RTC_HandleTypeDef hrtc;
extern void EcuM_PerformReset(uint8 param);
static uint16_t CanH_DiagnosticHoldTicks;

static void CanH_DiagnosticRx(void)
{
    Can_FrameType frame = {0};
    if (CanH_RxHeader.IDE != CAN_ID_STD || CanH_RxHeader.RTR != CAN_RTR_DATA ||
        CanH_RxHeader.StdId != 0x710U || CanH_RxHeader.DLC < 2U ||
        CanH_RxHeader.DLC > 8U ||
        (CanH_RxData[0] != 0x56U && CanH_RxData[0] != 0x41U)) return;
    frame.id = (uint16_t)CanH_RxHeader.StdId;
    frame.dlc = (uint8_t)CanH_RxHeader.DLC;
    memcpy(frame.data, CanH_RxData, frame.dlc);
    Can_RxIndication(&frame);
    CanH_RequestBusSleep = 0U;
    if (CanH_CommunicationState != CC_ACTIVE) CanH_CommunicationState = FULL_COMMUNICATION;
    CanH_DiagnosticHoldTicks = 1000U; /* Five seconds after the last addressed frame. */
    if (EcuM_StopModeActive != 0U) EcuM_PerformReset(0U);
}
void CanH_MainFunction(void);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);

void CanH_MainFunction(void)
{
	static uint32 notificationLocal = 0;
	CanH_Status = HAL_CAN_GetState(&hcan1);
	if(0x00 == CanH_MainCounter)
	{
		HAL_CAN_Start(&hcan1);
		notificationLocal = CAN_IT_RX_FIFO0_MSG_PENDING |
				CAN_IT_RX_FIFO1_MSG_PENDING |
				CAN_IT_ERROR_WARNING |
				CAN_IT_ERROR_PASSIVE |
				CAN_IT_BUSOFF |
				CAN_IT_LAST_ERROR_CODE |
				CAN_IT_ERROR;
		HAL_CAN_ActivateNotification(&hcan1, notificationLocal | CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_BUSOFF);
		CanH_CommunicationState = PARTIAL_COMMUNICATION;
		CanH_TxHeader.DLC = 1;
		CanH_TxHeader.IDE = CAN_ID_STD;
		CanH_TxHeader.StdId = 0x200;
		CanH_TxData[0] = CanH_AliveCounter;
		HAL_CAN_AddTxMessage(&hcan1, &CanH_TxHeader, CanH_TxData, &CanH_TxMailbox);

		CanH_TxHeader.DLC = 8;
		CanH_TxHeader.IDE = CAN_ID_STD;
		CanH_TxHeader.StdId = 0x299;
		CanH_AliveCounter++;
		CanH_TxData[0] = (uint8)(DataRecorder_KilometerTotal >> 24);
		CanH_TxData[1] = (uint8)(DataRecorder_KilometerTotal >> 16);
		CanH_TxData[2] = (uint8)(DataRecorder_KilometerTotal >> 8);
		CanH_TxData[3] = (uint8)(DataRecorder_KilometerTotal);
		CanH_TxData[4] = (uint8)(DataRecorder_KilometerPerDcy >> 24);
		CanH_TxData[5] = (uint8)(DataRecorder_KilometerPerDcy >> 16);
		CanH_TxData[6] = (uint8)(DataRecorder_KilometerPerDcy >> 8);
		CanH_TxData[7] = (uint8)(DataRecorder_KilometerPerDcy);
		HAL_CAN_AddTxMessage(&hcan1, &CanH_TxHeader, CanH_TxData, &CanH_TxMailbox);
	}
	else
	{
		/* Do nothing. */
	}

	if((FULL_COMMUNICATION == CanH_CommunicationState || PARTIAL_COMMUNICATION == CanH_CommunicationState) && CC_ACTIVE != CanH_CommunicationState)
	{
		for(uint8 i = 0; i < 22; i++) CanH_ErrArr[i] = 0;
		CanH_BodyState_MissCnt++;
		CanH_VehState_MissCnt++;
		CanH_DataRecorder_MissCnt++;
		CanH_BodyState2_MissCnt++;
		CanH_BusSystemDateAndTime_MissCnt++;
		if(2000 < CanH_BodyState_MissCnt)
		{
			Dem_SetDtc(DEM_BODYSTATE_MESSAGEMISSING_ID, 1);
			CanH_RxSig_Recirculation = 0;
			CanH_RxSig_FogLights = 0;
			CanH_RxSig_HighBeam = 0;
			CanH_RxSig_TemperatureSensor = 0;
			CanH_RxSig_TurnSignal = 0;
			CanH_RxSig_AutoClimate = 0;
		}
		else Dem_SetDtc(DEM_BODYSTATE_MESSAGEMISSING_ID, 0);
		if(2000 < CanH_VehState_MissCnt)
		{
			Dem_SetDtc(DEM_VEHSTATE_MESSAGEMISSING_ID, 1);
			CanH_RxSig_Ignition = 0;
			CanH_RxSig_Speed = 0;
			CanH_RxSig_Rpm = 0;
			CanH_RxSig_Gear = 0;
			CanH_RxSig_IrSensStat = 0;
		}
		else Dem_SetDtc(DEM_VEHSTATE_MESSAGEMISSING_ID, 0);
		if(2000 < CanH_DataRecorder_MissCnt)
		{
			Dem_SetDtc(DEM_DATARECORDER_MESSAGEMISSING_ID, 1);
			CanH_RxSig_DrivecycleStatus = 0;
		}
		else Dem_SetDtc(DEM_DATARECORDER_MESSAGEMISSING_ID, 0);
		if(2000 < CanH_BodyState2_MissCnt)
		{
			Dem_SetDtc(DEM_BODYSTATE2_MESSAGEMISSING_ID, 1);
			CanH_RxSig_FanValue = 0;
			CanH_RxSig_DisplayMode = 0;
			CanH_RxSig_ClimaTemp = 0;
			CanH_RxSig_RotaryLightSwitch = 0;
		}
		else Dem_SetDtc(DEM_BODYSTATE2_MESSAGEMISSING_ID, 0);
		if(2000 < CanH_BodyState2_MissCnt)
		{
			Dem_SetDtc(DEM_BODYSTATE2_MESSAGEMISSING_ID, 1);
			CanH_RxSig_FanValue = 0;
			CanH_RxSig_DisplayMode = 0;
			CanH_RxSig_ClimaTemp = 0;
			CanH_RxSig_RotaryLightSwitch = 0;
		}
		else Dem_SetDtc(DEM_BODYSTATE2_MESSAGEMISSING_ID, 0);

		if(0 /* COM owns configured 207-tick PDU. */)
		{
			CanH_TxHeader.DLC = 1;
			CanH_TxHeader.IDE = CAN_ID_STD;
			CanH_TxHeader.StdId = 0x200;
			CanH_AliveCounter++;
			CanH_TxData[0] = CanH_AliveCounter;
			HAL_CAN_AddTxMessage(&hcan1, &CanH_TxHeader, CanH_TxData, &CanH_TxMailbox);
		}
		else
		{
			/* Do nothing. */
		}

		if(0 /* COM owns configured 205-tick PDU. */)
		{

			CanH_TxHeader.DLC = 8;
			CanH_TxHeader.IDE = CAN_ID_STD;
			CanH_TxHeader.StdId = 0x299;
			CanH_TxData[0] = (uint8)(DataRecorder_KilometerTotal >> 24);
			CanH_TxData[1] = (uint8)(DataRecorder_KilometerTotal >> 16);
			CanH_TxData[2] = (uint8)(DataRecorder_KilometerTotal >> 8);
			CanH_TxData[3] = (uint8)(DataRecorder_KilometerTotal);
			CanH_TxData[4] = (uint8)(DataRecorder_KilometerPerDcy >> 24);
			CanH_TxData[5] = (uint8)(DataRecorder_KilometerPerDcy >> 16);
			CanH_TxData[6] = (uint8)(DataRecorder_KilometerPerDcy >> 8);
			CanH_TxData[7] = (uint8)(DataRecorder_KilometerPerDcy);
			HAL_CAN_AddTxMessage(&hcan1, &CanH_TxHeader, CanH_TxData, &CanH_TxMailbox);
		}
		else
		{
			/* Do nothing. */
		}
	}
	else
	{
		/* Do nothing. */
	}
	if (CanH_DiagnosticHoldTicks > 0U) --CanH_DiagnosticHoldTicks;
	if(200 <= CanH_NoCommCounter && CanH_DiagnosticHoldTicks == 0U)
		CanH_CommunicationState = NO_COMMUNICATION;
	else
	{
		/* Do nothing. */
	}
	if(NO_COMMUNICATION == CanH_CommunicationState) CanH_RequestBusSleep = 1;
	else
	{
		/* Do nothing.  */
	}
	if(1 == CanH_RequestBusSleep && 0 == HAL_CAN_IsSleepActive(&hcan1)) HAL_CAN_RequestSleep(&hcan1);
	else
	{
		/* Do nothing. */
	}
	CanH_NoCommCounter++;
	CanH_MainCounter++;
	for(uint8 i = 0; i < 8; i++) CanH_TxData[i] = 0;
}
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    if(hcan==&hcan1) { if(HAL_CAN_GetError(hcan)&HAL_CAN_ERROR_BOF) Can_BusOff(); }
	switch(hcan->ErrorCode)
	{
	case HAL_CAN_ERROR_EWG:
		CanH_ErrArr[0] = 1;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_EPV:
		CanH_ErrArr[1] = 2;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_BOF:
		CanH_ErrArr[2] = 3;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_STF:
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_FOR:
		CanH_ErrArr[4] = 5;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_ACK:
		CanH_ErrArr[5] = 6;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_BR:
		CanH_ErrArr[6] = 7;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_BD:
		CanH_ErrArr[7] = 8;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_CRC:
		CanH_ErrArr[8] = 9;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_RX_FOV0:
		CanH_ErrArr[9] = 10;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_RX_FOV1:
		CanH_ErrArr[10] = 11;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_TX_ALST0:
		CanH_ErrArr[11] = 12;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_TX_TERR0:
		CanH_ErrArr[12] = 13;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_TX_ALST1:
		CanH_ErrArr[13] = 14;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_TX_TERR1:
		CanH_ErrArr[14] = 15;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_TX_ALST2:
		CanH_ErrArr[16] = 17;
		break;
	case HAL_CAN_ERROR_TX_TERR2:
		CanH_ErrArr[17] = 18;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_NOT_INITIALIZED:
		CanH_ErrArr[18] = 19;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_NOT_READY:
		CanH_ErrArr[19] = 20;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_NOT_STARTED:
		CanH_ErrArr[20] = 21;
		HAL_CAN_ResetError(hcan);
		break;
	case HAL_CAN_ERROR_PARAM:
		CanH_ErrArr[21] = 22;
		HAL_CAN_ResetError(hcan);
		break;
	default:
		break;
	}
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CanH_RxFifo0MsgPendingCnt++;
	if(1u == EcuM_StopModeActive)
	{
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CanH_RxHeader, CanH_RxData);
        if(CanH_RxHeader.IDE!=CAN_ID_STD || CanH_RxHeader.RTR!=CAN_RTR_DATA || !Com_ValidateRxPdu((uint16_t)CanH_RxHeader.StdId,(uint8_t)CanH_RxHeader.DLC)) return;
        CanH_DiagnosticRx();
        if(CanH_RxHeader.IDE==CAN_ID_STD && CanH_RxHeader.RTR==CAN_RTR_DATA) Com_RxIndication((uint16_t)CanH_RxHeader.StdId,CanH_RxData,(uint8_t)CanH_RxHeader.DLC);
		/* NM3 */
		if(0x3FF == CanH_RxHeader.StdId && CanH_RxHeader.IDE == CAN_ID_STD && CanH_RxHeader.RTR == CAN_RTR_DATA && CanH_RxHeader.DLC == 1U)
		{
			CanNm_RxIndication(0x3FFU, CanH_RxData, CanH_RxHeader.DLC);
			CanH_RxSig_Networkmanagement3 = CanH_RxData[0];
			if(0xFF == CanH_RxData[0])
			{
				EcuM_PerformReset(0);
			}
			else
			{
				/* Do nothing. */
			}
		}
		else
		{
			/* Do nothing. */
		}
		HAL_PWR_EnableSleepOnExit();
	}
	else
	{
		CanH_RxFifo0MsgPendingCnt++;
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CanH_RxHeader, CanH_RxData);
        if(CanH_RxHeader.IDE!=CAN_ID_STD || CanH_RxHeader.RTR!=CAN_RTR_DATA || !Com_ValidateRxPdu((uint16_t)CanH_RxHeader.StdId,(uint8_t)CanH_RxHeader.DLC)) return;
        CanH_DiagnosticRx();
        if(CanH_RxHeader.IDE==CAN_ID_STD && CanH_RxHeader.RTR==CAN_RTR_DATA) Com_RxIndication((uint16_t)CanH_RxHeader.StdId,CanH_RxData,(uint8_t)CanH_RxHeader.DLC);
		/* NM3 */
		if(0x3FF == CanH_RxHeader.StdId && CanH_RxHeader.IDE == CAN_ID_STD && CanH_RxHeader.RTR == CAN_RTR_DATA && CanH_RxHeader.DLC == 1U)
		{
			CanNm_RxIndication(0x3FFU, CanH_RxData, CanH_RxHeader.DLC);
			CanH_RxSig_Networkmanagement3 = CanH_RxData[0];
			if(0xFF == CanH_RxData[0])
			{
				CanH_RequestBusSleep = 0;
				if(CC_ACTIVE != CanH_CommunicationState) CanH_CommunicationState = FULL_COMMUNICATION;
				else
				{
					/* Do nothing. */
				}
				CanH_NoCommCounter = 0;
			}
			else
			{
				/* Do nothing. */
			}
		}
		else
		{
			/* Do nothing. */
		}
		/* VehicleState */
		if(0x97 == CanH_RxHeader.StdId)
		{





			CanH_VehState_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		/* StatusBodyControl */
		if(0x98 == CanH_RxHeader.StdId)
		{






			CanH_BodyState_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		/* StatusBodyControl2 */
		if(0x99 == CanH_RxHeader.StdId)
		{




			CanH_BodyState2_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		if(0x10b == CanH_RxHeader.StdId)CanH_RxSig_CcmId = CanH_RxData[0];
		else
		{
			/* Do nothing. */
		}
		/* DataRecorder */
		if(0x10f == CanH_RxHeader.StdId)
		{

			CanH_DataRecorder_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		if(0x201 == CanH_RxHeader.StdId)
		{
			CanH_BusSystemDateAndTime_MissCnt = 0;
			RTC_DateTypeDef sDate;
			RTC_TimeTypeDef sTime;
			sDate.Year = CanH_RxData[0];
			sDate.Month = CanH_RxData[1];
			sDate.Date = CanH_RxData[2];
			HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
			sTime.Hours = CanH_RxData[3];
			sTime.Minutes = CanH_RxData[4];
			sTime.Seconds = CanH_RxData[5];
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		}
		else
		{
			/* Do nothing. */
		}
		/* Routed DMU diagnostics were dispatched immediately after FIFO read. */
		CanH_RxHeader.DLC = 0;
		CanH_RxHeader.ExtId = 0;
		CanH_RxHeader.FilterMatchIndex = 0;
		CanH_RxHeader.IDE = 0;
		CanH_RxHeader.RTR = 0;
		CanH_RxHeader.StdId = 0;
		CanH_RxHeader.Timestamp = 0;
		for(uint8 i = 0; i < 8; i++) CanH_RxData[i] = 0;
		DataRecorder_RxSig_VehicleSpeed = CanH_RxSig_Speed;
		DataRecorder_RxSig_DriveCycleStatus = CanH_RxSig_DrivecycleStatus;
		DigitalCluster_RxSig_IgnitionStatus = CanH_RxSig_Ignition;
		DigitalCluster_RxSig_Gear = CanH_RxSig_Gear;
		DigitalCluster_RxSig_HighBeamStatus = CanH_RxSig_HighBeam;
		DigitalCluster_RxSig_VehicleSpeed = CanH_RxSig_Speed;
		DigitalCluster_RxSig_MotorRpm = CanH_RxSig_Rpm;
		DigitalCluster_RxSig_TurnSignals = CanH_RxSig_TurnSignal;
		DigitalCluster_RxSig_DisplayMode = CanH_RxSig_DisplayMode;
		DigitalCluster_RxSig_CollisionWarning = CanH_RxSig_IrSensStat;
		if(0 != CanH_RxSig_CcmId && 0 == DigitalCluster_RxSig_CheckControlMessageId) DigitalCluster_RxSig_CheckControlMessageId = CanH_RxSig_CcmId;
		else
		{
			/* Do nothing. */
		}
		DigitalCluster_RxSig_OutsideTemperature = CanH_RxSig_TemperatureSensor;
		DigitalCluster_RxSig_FanValue = CanH_RxSig_FanValue;
		DigitalCluster_RxSig_RequestedTemperature = CanH_RxSig_ClimaTemp;
		DigitalCluster_RxSig_Recirculation = CanH_RxSig_Recirculation;
		DigitalCluster_RxSig_AutoClimate = CanH_RxSig_AutoClimate;
		DigitalCluster_RxSig_Rls = CanH_RxSig_RotaryLightSwitch;
		switch(CanH_RxSig_FogLights)
		{
		case 1:
			DigitalCluster_RxSig_FogLightFront = 1;
			DigitalCluster_RxSig_FogLightRear = 0;
			break;
		case 2:
			DigitalCluster_RxSig_FogLightFront = 0;
			DigitalCluster_RxSig_FogLightRear = 1;
			break;
		case 3:
			DigitalCluster_RxSig_FogLightFront = 1;
			DigitalCluster_RxSig_FogLightRear = 1;
			break;
		default:
			DigitalCluster_RxSig_FogLightFront = 0;
			DigitalCluster_RxSig_FogLightRear = 0;
			break;
		}
	}
}
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan)
{
	CanH_RxFifo0FullCnt++;
	if(1u == EcuM_StopModeActive)
	{
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CanH_RxHeader, CanH_RxData);
        if(CanH_RxHeader.IDE!=CAN_ID_STD || CanH_RxHeader.RTR!=CAN_RTR_DATA || !Com_ValidateRxPdu((uint16_t)CanH_RxHeader.StdId,(uint8_t)CanH_RxHeader.DLC)) return;
        CanH_DiagnosticRx();
        if(CanH_RxHeader.IDE==CAN_ID_STD && CanH_RxHeader.RTR==CAN_RTR_DATA) Com_RxIndication((uint16_t)CanH_RxHeader.StdId,CanH_RxData,(uint8_t)CanH_RxHeader.DLC);
		/* NM3 */
		if(0x3FF == CanH_RxHeader.StdId && CanH_RxHeader.IDE == CAN_ID_STD && CanH_RxHeader.RTR == CAN_RTR_DATA && CanH_RxHeader.DLC == 1U)
		{
			CanNm_RxIndication(0x3FFU, CanH_RxData, CanH_RxHeader.DLC);
			CanH_RxSig_Networkmanagement3 = CanH_RxData[0];
			if(0xFF == CanH_RxData[0])
			{
				EcuM_PerformReset(0);
			}
			else
			{
				/* Do nothing. */
			}
		}
		else
		{
			/* Do nothing. */
		}
		HAL_PWR_EnableSleepOnExit();
	}
	else
	{
		CanH_RxFifo0MsgPendingCnt++;
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CanH_RxHeader, CanH_RxData);
        if(CanH_RxHeader.IDE!=CAN_ID_STD || CanH_RxHeader.RTR!=CAN_RTR_DATA || !Com_ValidateRxPdu((uint16_t)CanH_RxHeader.StdId,(uint8_t)CanH_RxHeader.DLC)) return;
        CanH_DiagnosticRx();
        if(CanH_RxHeader.IDE==CAN_ID_STD && CanH_RxHeader.RTR==CAN_RTR_DATA) Com_RxIndication((uint16_t)CanH_RxHeader.StdId,CanH_RxData,(uint8_t)CanH_RxHeader.DLC);
		/* NM3 */
		if(0x3FF == CanH_RxHeader.StdId && CanH_RxHeader.IDE == CAN_ID_STD && CanH_RxHeader.RTR == CAN_RTR_DATA && CanH_RxHeader.DLC == 1U)
		{
			CanNm_RxIndication(0x3FFU, CanH_RxData, CanH_RxHeader.DLC);
			CanH_RxSig_Networkmanagement3 = CanH_RxData[0];
			if(0xFF == CanH_RxData[0])
			{
				CanH_RequestBusSleep = 0;
				if(CC_ACTIVE != CanH_CommunicationState) CanH_CommunicationState = FULL_COMMUNICATION;
				else
				{
					/* Do nothing. */
				}
				CanH_NoCommCounter = 0;
			}
			else
			{
				/* Do nothing. */
			}
		}
		else
		{
			/* Do nothing. */
		}
		/* VehicleState */
		if(0x97 == CanH_RxHeader.StdId)
		{





			CanH_VehState_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		/* StatusBodyControl */
		if(0x98 == CanH_RxHeader.StdId)
		{






			CanH_BodyState_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		/* StatusBodyControl2 */
		if(0x99 == CanH_RxHeader.StdId)
		{




			CanH_BodyState2_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		if(0x10b == CanH_RxHeader.StdId)CanH_RxSig_CcmId = CanH_RxData[0];
		else
		{
			/* Do nothing. */
		}
		/* DataRecorder */
		if(0x10f == CanH_RxHeader.StdId)
		{

			CanH_DataRecorder_MissCnt = 0;
		}
		else
		{
			/* Do nothing. */
		}
		if(0x201 == CanH_RxHeader.StdId)
		{
			CanH_BusSystemDateAndTime_MissCnt = 0;
			RTC_DateTypeDef sDate;
			RTC_TimeTypeDef sTime;
			sDate.Year = CanH_RxData[0] + 2000;
			sDate.Month = CanH_RxData[1];
			sDate.Date = CanH_RxData[2];
			HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
			sTime.Hours = CanH_RxData[3];
			sTime.Minutes = CanH_RxData[4];
			sTime.Seconds = CanH_RxData[5];
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		}
		else
		{
			/* Do nothing. */
		}
		/* Routed DMU diagnostics were dispatched immediately after FIFO read. */
		CanH_RxHeader.DLC = 0;
		CanH_RxHeader.ExtId = 0;
		CanH_RxHeader.FilterMatchIndex = 0;
		CanH_RxHeader.IDE = 0;
		CanH_RxHeader.RTR = 0;
		CanH_RxHeader.StdId = 0;
		CanH_RxHeader.Timestamp = 0;
		for(uint8 i = 0; i < 8; i++) CanH_RxData[i] = 0;
		DataRecorder_RxSig_VehicleSpeed = CanH_RxSig_Speed;
		DataRecorder_RxSig_DriveCycleStatus = CanH_RxSig_DrivecycleStatus;
		DigitalCluster_RxSig_IgnitionStatus = CanH_RxSig_Ignition;
		DigitalCluster_RxSig_Gear = CanH_RxSig_Gear;
		DigitalCluster_RxSig_HighBeamStatus = CanH_RxSig_HighBeam;
		DigitalCluster_RxSig_VehicleSpeed = CanH_RxSig_Speed;
		DigitalCluster_RxSig_MotorRpm = CanH_RxSig_Rpm;
		DigitalCluster_RxSig_TurnSignals = CanH_RxSig_TurnSignal;
		DigitalCluster_RxSig_DisplayMode = CanH_RxSig_DisplayMode;
		DigitalCluster_RxSig_CollisionWarning = CanH_RxSig_IrSensStat;
		if(0 != CanH_RxSig_CcmId && 0 != DigitalCluster_RxSig_CheckControlMessageId) DigitalCluster_RxSig_CheckControlMessageId = CanH_RxSig_CcmId;
		else
		{
			/* Do nothing. */
		}
		DigitalCluster_RxSig_OutsideTemperature = CanH_RxSig_TemperatureSensor;
		DigitalCluster_RxSig_FanValue = CanH_RxSig_FanValue;
		DigitalCluster_RxSig_RequestedTemperature = CanH_RxSig_ClimaTemp;
		DigitalCluster_RxSig_Recirculation = CanH_RxSig_Recirculation;
		DigitalCluster_RxSig_AutoClimate = CanH_RxSig_AutoClimate;
		DigitalCluster_RxSig_Rls = CanH_RxSig_RotaryLightSwitch;
		switch(CanH_RxSig_FogLights)
		{
		case 1:
			DigitalCluster_RxSig_FogLightFront = 1;
			DigitalCluster_RxSig_FogLightRear = 0;
			break;
		case 2:
			DigitalCluster_RxSig_FogLightFront = 0;
			DigitalCluster_RxSig_FogLightRear = 1;
			break;
		case 3:
			DigitalCluster_RxSig_FogLightFront = 1;
			DigitalCluster_RxSig_FogLightRear = 1;
			break;
		default:
			DigitalCluster_RxSig_FogLightFront = 0;
			DigitalCluster_RxSig_FogLightRear = 0;
			break;
		}
	}
}

volatile uint32_t Can_DiagMailbox;
volatile uint16_t Can_AppMailboxIds[3];
static void CanH_Complete(uint32_t mailbox,uint8_t ok) {
 if(Can_DiagMailbox==mailbox) { Can_DiagMailbox=0U; Can_TxConfirmation(ok); }
 else {uint8_t i=mailbox==CAN_TX_MAILBOX0?0:(mailbox==CAN_TX_MAILBOX1?1:2);uint16_t id=Can_AppMailboxIds[i];Can_AppMailboxIds[i]=0;if(id)CanIf_ApplicationTxConfirmation(id,ok);}
}
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *h) { if(h==&hcan1) CanH_Complete(CAN_TX_MAILBOX0,1); }
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *h) { if(h==&hcan1) CanH_Complete(CAN_TX_MAILBOX1,1); }
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *h) { if(h==&hcan1) CanH_Complete(CAN_TX_MAILBOX2,1); }
void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *h) { if(h==&hcan1) CanH_Complete(CAN_TX_MAILBOX0,0); }
void HAL_CAN_TxMailbox1AbortCallback(CAN_HandleTypeDef *h) { if(h==&hcan1) CanH_Complete(CAN_TX_MAILBOX1,0); }
void HAL_CAN_TxMailbox2AbortCallback(CAN_HandleTypeDef *h) { if(h==&hcan1) CanH_Complete(CAN_TX_MAILBOX2,0); }
