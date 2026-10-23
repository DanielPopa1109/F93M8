/* START OF FILE */
/* INCLUDE START */
#include "CanSpi.h"
#include "../CanNm/CanNm.h"
#include "../Can/Can.h"
#include <string.h>
#include "../Com/Com.h"
#include "../CanSM/CanSM.h"
#include "../CanIf/CanIf.h"
#include "ActCtrl.h"
#include "InputCtrl.h"
#include "Dcm.h"
#include "spi.h"
#include "EcuM.h"
/* DEFINE START */
/* Macro for SPI handler. */
#define CANSPI_SPI_CAN                 &hspi1
/* Macro for GPIO status control. */
#define CANSPI_MCP2515_CS_HIGH()   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET)
/* Macro for GPIO status control. */
#define CANSPI_MCP2515_CS_LOW()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET)
/* DEFINE STOP */
/* VARIABLES START */
uint32 CanSpi_StatusBodyControl2_MissCnt = 0;
/* Message missed counter. */
uint32 CanSpi_StatusBodyControl_MissCnt = 0;
/* Message missed counter. */
uint32 CanSpi_VehicleState_MissCnt = 0;
/* CAN signal for climate fan. */
extern uint8 StatusBodyControl_FanValue;
/* CAN signal for climate fan. */
extern uint8 StatusBodyControl_Auto;
/* CAN signal for climate fan. */
extern uint8 StatusBodyControl_Recirc;
/* CAN signal for climate fan. */
extern uint8 StatusBodyControl_Temperature;
/* CAN signal for climate fan. */
extern uint8 StatusBodyControl_OutsideTemp;
/* SW state. */
extern EcuMStates_t EcuM_State;
/* Error counter for CAN bus. */
uint32 CanSpi_Bus_ErrorArr[3] = {0};
/* Communication status variable. */
CANSPI_ComState_t CanSpi_Communication_Status = {0};
/* Communication status timeout variable. */
uint16 CanSpi_Communication_StatusTimeout = 200;
/* CAN RX buffer 1. */
CANSPI_uCAN_MSG CanSpi_RxFrame_Buffer0 = {0};
/* CAN RX buffer 2. */
CANSPI_uCAN_MSG CanSpi_RxFrame_Buffer1 = {0};
/* DIAG ROUTINE CAN RX buffer. */
CANSPI_uCAN_MSG Dcm_DiagServiceResponse_Frame = {0};
/* DIAG ROUTINE CAN TX buffer. */
CANSPI_uCAN_MSG Dcm_DiagServiceRequest_Frame = {0};
/* NM3 signal value. */
uint8 CanSpi_Networkmanagement3_Signal = 255;
/* VARIABLES STOP */
/* FUNCTIONS START */
/* Transceiver handling function. */
extern void Dem_SaveDtc(uint8 index, uint8 status);
static uint32 CanSpi_ConvertReg2ExtendedCANid(uint8 tempRXBn_EIDH, uint8 tempRXBn_EIDL, uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL);
/* Transceiver handling function. */
static uint32 CanSpi_ConvertReg2StandardCANid(uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL);
/* Transceiver handling function. */
static uint8 CanSpi_SPI_Rx(void);
/* Transceiver handling function. */
static void CanSpi_ConvertCANid2Reg(uint32 tempPassedInID, uint8 canIdType, CANSPI_id_reg_t *passedIdReg);
/* Transceiver handling function. */
static void CanSpi_SPI_Tx(uint8 data);
/* Transceiver handling function. */
static void CanSpi_SPI_TxBuffer(uint8 *buffer, uint8 length);
/* Transceiver handling function. */
static void CanSpi_SPI_RxBuffer(uint8 *buffer, uint8 length);
/* Transceiver handling function. */
static void CanSpi_MCP2515_Reset(void);
/* Transceiver handling function. */
static void CanSpi_MCP2515_BitModify(uint8 address, uint8 mask, uint8 data);
/* Transceiver handling function. */
static void CanSpi_MCP2515_ReadRxSequence(uint8 instruction, uint8 *data, uint8 length);
/* Transceiver handling function. */
static void CanSpi_MCP2515_WriteByte(uint8 address, uint8 data);
/* Transceiver handling function. */
static void CanSpi_MCP2515_WriteByteSequence(uint8 startAddress, uint8 endAddress, uint8 *data);
/* Transceiver handling function. */
static void CanSpi_MCP2515_LoadTxSequence(uint8 instruction, uint8 *idReg, uint8 dlc, uint8 *data);
/* Transceiver handling function. */
static void CanSpi_MCP2515_RequestToSend(uint8 instruction);
/* Retrieve the number of messages in the buffers. */
uint32 CanSpi_MessagesInBuffer(void);
/* Transceiver reading function. */
static uint8 CanSpi_MCP2515_ReadByte (uint8 address);
/* Bus error retrieval. */
static uint8 CanSpi_IsBussOff(void);
/* Bus error retrieval. */
static uint8 CanSpi_IsRxErrorPassive(void);
/* Bus error retrieval. */
static uint8 CanSpi_IsTxErrorPassive(void);
/* Transceiver status retrieval. */
static uint8 CanSpi_MCP2515_ReadStatus(void);
/* Transceiver status retrieval. */
static uint8 CanSpi_MCP2515_GetRxStatus(void);
/* Transceiver mode configuration. */
static bool CanSpi_MCP2515_Initialize(void);
/* Transceiver mode configuration. */
static bool CanSpi_MCP2515_SetConfigMode(void);
/* Transceiver mode configuration. */
static bool CanSpi_MCP2515_SetNormalMode(void);
/* Transceiver mode configuration. */
static bool CanSpi_MCP2515_SetSleepMode(void);
/* Main function. */
void CanSpi_MainFunction(void);
/* Configure CAN transceiver for sleep. */
void CanSpi_Sleep(void);
/* Initialization function. */
uint8 CanSpi_Initialize(void);
/* Transmit function. */
uint8 CanSpi_Transmit(CANSPI_uCAN_MSG *tempCanMsg);
/* Receive function. */
uint8 CanSpi_Receive(CANSPI_uCAN_MSG *tempCanMsg);
/* FUNCTIONS STOP */
/* FUNCTIONS START */
/* Existing CBM event announcement is an application communication callback,
 * separate from DEM event storage and status transitions. */
void CanSpi_DtcEventFailed(uint8 eventId)
{
	CANSPI_uCAN_MSG message = {0};
	message.frame.idType = 1U;
	message.frame.id = 0x10CU;
	message.frame.dlc = 1U;
	message.frame.data0 = eventId;
	(void)CanSpi_Transmit(&message);
}

static uint16 CanSpi_DiagOwner[3];
static void CanSpi_PollTx(void) {
 uint32 irq=__get_PRIMASK();__disable_irq();
 uint8 b,ctrl,flags=CanSpi_MCP2515_ReadByte(0x2CU);
 for(b=0;b<3U;++b) if(CanSpi_DiagOwner[b]) {
  ctrl=CanSpi_MCP2515_ReadByte((uint8)(0x30U+b*0x10U));
  if(!(ctrl&0x08U)) {
   uint16 owner=CanSpi_DiagOwner[b];uint8 ok=(uint8)((flags&(uint8)(0x04U<<b))!=0U && !(ctrl&0x50U));
   CanSpi_DiagOwner[b]=0U;
   if(owner==0x710U)Can_TxConfirmation(ok);else CanIf_ApplicationTxConfirmation(owner,ok);
   CanSpi_MCP2515_BitModify(0x2CU,(uint8)(0x04U<<b),0U);
  }
 }
 if(!irq)__enable_irq();
}
void CanSpi_MainFunction(void)
{
    CanSpi_PollTx();
	/* TX buffer. */
	static CANSPI_uCAN_MSG CanSpi_TxFrame = {0};
	/* Message received in buffers value. */
	static uint32 CanSpi_MessagesInBuffer_var = 0;
	/* Main counter. */
	static uint32 CanSpi_MainCounter = 0;
	/* Restore error state. */
	/* Perform transceiver initialization. */
	if(CanSpi_MainCounter == 0) CanSpi_MCP2515_Reset();
	else if(1 == CanSpi_MainCounter)
	{
		CanSpi_Initialize();
		CanSpi_Communication_Status = PARTIAL_COMMUNICATION;
		CanSpi_TxFrame.frame.idType = 1;
		CanSpi_TxFrame.frame.id = 0x100;
		CanSpi_TxFrame.frame.dlc = 2;
		CanSpi_TxFrame.frame.data0 = 0;
		CanSpi_TxFrame.frame.data1 = 0;
		CanSpi_Transmit(&CanSpi_TxFrame);
		CanSpi_TxFrame.frame.idType = 1;
		CanSpi_TxFrame.frame.id = 0x101;
		CanSpi_TxFrame.frame.dlc = 5;
		CanSpi_TxFrame.frame.data0 = 0;
		CanSpi_TxFrame.frame.data1 = 0;
		CanSpi_TxFrame.frame.data2 = 0;
		CanSpi_TxFrame.frame.data3 = 0;
		CanSpi_TxFrame.frame.data4 = 0;
		CanSpi_Transmit(&CanSpi_TxFrame);
	}
	else
	{
		/* Do nothing. */
	}
	/* Get the number of messages in the buffer now. */
	CanSpi_MessagesInBuffer_var = CanSpi_MessagesInBuffer();
	/* As long as there are messages in the buffers. */
	if(CanSpi_MessagesInBuffer_var != 0)
	{
		/* Store them in the buffer 1 if it is empty. */
		if(CanSpi_RxFrame_Buffer0.frame.id == 0)
		{
			/* If RX is performed successfully. */
			if(CanSpi_Receive(&CanSpi_RxFrame_Buffer0) != 0)
			{
                if(CanSpi_RxFrame_Buffer0.frame.idType!=1U || !Com_ValidateRxPdu((uint16)CanSpi_RxFrame_Buffer0.frame.id,CanSpi_RxFrame_Buffer0.frame.dlc)) CanSpi_RxFrame_Buffer0.frame.id=0;
                Com_RxIndication((uint16)CanSpi_RxFrame_Buffer0.frame.id,&CanSpi_RxFrame_Buffer0.frame.data0,CanSpi_RxFrame_Buffer0.frame.dlc);
				/* StatusBodyControl2 */
				if(0x99 == CanSpi_RxFrame_Buffer0.frame.id)
				{



					CanSpi_StatusBodyControl2_MissCnt = 0;
				}
				else
				{
					/* Do nothing. */
				}
				/* StatusBodyControl */
				if(0x98 == CanSpi_RxFrame_Buffer0.frame.id)
				{



					CanSpi_StatusBodyControl_MissCnt = 0;
				}
				else
				{
					/* Do nothing. */
				}
				/* VehicleState Frame */
				if(CanSpi_RxFrame_Buffer0.frame.id == 0x097)
				{

					CanSpi_VehicleState_MissCnt = 0;
				}
				else
				{
					/* Do nothing. */
				}
				/* Networkmanagement3 Frame */
				if(CanSpi_RxFrame_Buffer0.frame.id == 0x3FF && CanSpi_RxFrame_Buffer0.frame.idType == 1U && CanSpi_RxFrame_Buffer0.frame.dlc == 1U)
				{
					CanNm_RxIndication(0x3FFU, &CanSpi_RxFrame_Buffer0.frame.data0, CanSpi_RxFrame_Buffer0.frame.dlc);
					if(CanSpi_RxFrame_Buffer0.frame.data0 == 0xFF)
					{
						/* Update the status of the communication. */
						CanSpi_Networkmanagement3_Signal = CanSpi_RxFrame_Buffer0.frame.data0;
						if(CC_ACTIVE != CanSpi_Communication_Status) CanSpi_Communication_Status = FULL_COMMUNICATION;
						else
						{
							/* Do nothing. */
						}
						CanSpi_Communication_StatusTimeout = 200;
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
				/* Diagnostic request Frame */
				if(CanSpi_RxFrame_Buffer0.frame.id == 0x710 && CanSpi_RxFrame_Buffer0.frame.dlc >= 2U && CanSpi_RxFrame_Buffer0.frame.dlc <= 8U && (CanSpi_RxFrame_Buffer0.frame.data0 == 0x54U || CanSpi_RxFrame_Buffer0.frame.data0 == 0x41U)) { Can_FrameType diag = {0}; diag.id = 0x710U; diag.dlc = CanSpi_RxFrame_Buffer0.frame.dlc; memcpy(diag.data, &CanSpi_RxFrame_Buffer0.frame.data0, diag.dlc); Can_RxIndication(&diag); }
				else
				{
					/* Do nothing. */
				}
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
		/* Store them in the buffer 2 if it is empty. */
		if(CanSpi_RxFrame_Buffer1.frame.id == 0)
		{
			if(CanSpi_Receive(&CanSpi_RxFrame_Buffer1) != 0)
			{
                if(CanSpi_RxFrame_Buffer1.frame.idType!=1U || !Com_ValidateRxPdu((uint16)CanSpi_RxFrame_Buffer1.frame.id,CanSpi_RxFrame_Buffer1.frame.dlc)) CanSpi_RxFrame_Buffer1.frame.id=0;
                Com_RxIndication((uint16)CanSpi_RxFrame_Buffer1.frame.id,&CanSpi_RxFrame_Buffer1.frame.data0,CanSpi_RxFrame_Buffer1.frame.dlc);
				/* StatusBodyControl2 */
				if(0x99 == CanSpi_RxFrame_Buffer1.frame.id)
				{



					CanSpi_StatusBodyControl2_MissCnt = 0;
				}
				else
				{
					/* Do nothing. */
				}
				/* StatusBodyControl */
				if(0x98 == CanSpi_RxFrame_Buffer1.frame.id)
				{



					CanSpi_StatusBodyControl_MissCnt = 0;
				}
				else
				{
					/* Do nothing. */
				}
				/* VehicleState Frame */
				if(CanSpi_RxFrame_Buffer1.frame.id == 0x097)
				{

					CanSpi_VehicleState_MissCnt = 0;
				}
				else
				{
					/* Do nothing. */
				}
				/* Networkmanagement3 Frame */
				if(CanSpi_RxFrame_Buffer1.frame.id == 0x3FF && CanSpi_RxFrame_Buffer1.frame.idType == 1U && CanSpi_RxFrame_Buffer1.frame.dlc == 1U)
				{
					CanNm_RxIndication(0x3FFU, &CanSpi_RxFrame_Buffer1.frame.data0, CanSpi_RxFrame_Buffer1.frame.dlc);
					if(CanSpi_RxFrame_Buffer1.frame.data0 == 0xFF)
					{
						/* Update the status of the communication. */
						CanSpi_Networkmanagement3_Signal = CanSpi_RxFrame_Buffer1.frame.data0;
						if(CC_ACTIVE != CanSpi_Communication_Status) CanSpi_Communication_Status = FULL_COMMUNICATION;
						else
						{
							/* Do nothing. */
						}
						CanSpi_Communication_StatusTimeout = 200;
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
				/* Diagnostic request Frame */
				if(CanSpi_RxFrame_Buffer1.frame.id == 0x710 && CanSpi_RxFrame_Buffer1.frame.dlc >= 2U && CanSpi_RxFrame_Buffer1.frame.dlc <= 8U && (CanSpi_RxFrame_Buffer1.frame.data0 == 0x54U || CanSpi_RxFrame_Buffer1.frame.data0 == 0x41U)) { Can_FrameType diag = {0}; diag.id = 0x710U; diag.dlc = CanSpi_RxFrame_Buffer1.frame.dlc; memcpy(diag.data, &CanSpi_RxFrame_Buffer1.frame.data0, diag.dlc); Can_RxIndication(&diag); }
				else
				{
					/* Do nothing. */
				}
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
		/* Reset the buffers. */
		CanSpi_RxFrame_Buffer0.frame.idType = 0;
		CanSpi_RxFrame_Buffer0.frame.id = 0;
		CanSpi_RxFrame_Buffer0.frame.dlc = 0;
		CanSpi_RxFrame_Buffer0.frame.data0 = 0;
		CanSpi_RxFrame_Buffer0.frame.data1 = 0;
		CanSpi_RxFrame_Buffer0.frame.data2 = 0;
		CanSpi_RxFrame_Buffer0.frame.data3 = 0;
		CanSpi_RxFrame_Buffer0.frame.data4 = 0;
		CanSpi_RxFrame_Buffer0.frame.data5 = 0;
		CanSpi_RxFrame_Buffer0.frame.data6 = 0;
		CanSpi_RxFrame_Buffer0.frame.data7 = 0;
		CanSpi_RxFrame_Buffer1.frame.idType = 0;
		CanSpi_RxFrame_Buffer1.frame.id = 0;
		CanSpi_RxFrame_Buffer1.frame.dlc = 0;
		CanSpi_RxFrame_Buffer1.frame.data0 = 0;
		CanSpi_RxFrame_Buffer1.frame.data1 = 0;
		CanSpi_RxFrame_Buffer1.frame.data2 = 0;
		CanSpi_RxFrame_Buffer1.frame.data3 = 0;
		CanSpi_RxFrame_Buffer1.frame.data4 = 0;
		CanSpi_RxFrame_Buffer1.frame.data5 = 0;
		CanSpi_RxFrame_Buffer1.frame.data6 = 0;
		CanSpi_RxFrame_Buffer1.frame.data7 = 0;
	}
	else
	{
		/* Do nothing. */
	}
	/* Check for RX error. */
	if(CanSpi_IsRxErrorPassive() != 0 && 0 == CanSpi_MainCounter % 20 && 0 != CanSpi_MainCounter) CanSpi_Bus_ErrorArr[0]++;
	else CanSpi_Bus_ErrorArr[0] = 0;
	/* Check for TX error. */
	if(CanSpi_IsTxErrorPassive() != 0 && 0 == CanSpi_MainCounter % 20 && 0 != CanSpi_MainCounter) CanSpi_Bus_ErrorArr[1]++;
	else CanSpi_Bus_ErrorArr[1] = 0;
	/* Check for bus off error. */
	if(CanSpi_IsBussOff() != 0 && 0 == CanSpi_MainCounter % 20 && 0 != CanSpi_MainCounter)
	{
		CanSpi_Bus_ErrorArr[2]++;
		Dem_SaveDtc(0x09, 1);
	}
	else
	{
		CanSpi_Bus_ErrorArr[2] = 0;
		Dem_SaveDtc(0x09, 0);
	}
	/* If ignition processed value is greater than zero. */
	if((StatusList_ComOutValue[0] != 0))
	{
		/* Set the communication status. */
		if(CanSpi_Communication_Status == NO_COMMUNICATION) CanSpi_Communication_Status = PARTIAL_COMMUNICATION;
		else
		{
			/* Do nothing. */
		}
	}
	else
	{
		/* Set the communication status. */
		if(CanSpi_Communication_Status == PARTIAL_COMMUNICATION) CanSpi_Communication_Status = NO_COMMUNICATION;
		else
		{
			/* Do nothing. */
		}
	}
	/* Decrease the timeout counter. */
	if(CanSpi_Communication_StatusTimeout != 0) CanSpi_Communication_StatusTimeout--;
	/* Set the communication status. */
	else if((CanSpi_Communication_StatusTimeout == 0) && (CanSpi_Communication_Status == FULL_COMMUNICATION)) CanSpi_Communication_Status = PARTIAL_COMMUNICATION;
	else
	{
		/* Do nothing. */
	}
	/* If communication status is full or partial. */
	if(CanSpi_Communication_Status != CC_ACTIVE
			&& EcuM_State == RUN)
	{
		CanSpi_StatusBodyControl2_MissCnt++;
		CanSpi_StatusBodyControl_MissCnt++;
		CanSpi_VehicleState_MissCnt++;

		if(2000 < CanSpi_StatusBodyControl2_MissCnt)
		{
			Dem_SaveDtc(0xB, 1);
			StatusBodyControl_FanValue = 0;
			StatusBodyControl_Temperature = 0;
			WiperStock_VehicleState = 255;
		}
		else Dem_SaveDtc(0xB, 0);
		if(2000 < CanSpi_StatusBodyControl_MissCnt)
		{
			Dem_SaveDtc(0x06, 1);

			StatusBodyControl_OutsideTemp = 0;
			StatusBodyControl_Auto = 0;
			StatusBodyControl_Recirc = 0;
		}
		else Dem_SaveDtc(0x06, 0);
		if(2000 < CanSpi_VehicleState_MissCnt)
		{
			Dem_SaveDtc(0x07, 1);
			VehicleSpeed_VehicleState = 0;

		}
		else Dem_SaveDtc(0x07, 0);
		/* Status Drive Control */
		if(0 /* Owned by Com configured 14-tick PDU. */)
		{
			CanSpi_TxFrame.frame.idType = 1;
			CanSpi_TxFrame.frame.id = 0x100;
			CanSpi_TxFrame.frame.dlc = 2;
			CanSpi_TxFrame.frame.data0 = StatusList_ComOutValue[0];
			CanSpi_TxFrame.frame.data1 = StatusList_ComOutValue[2];
			CanSpi_Transmit(&CanSpi_TxFrame);
			CanSpi_TxFrame.frame.idType = 0;
			CanSpi_TxFrame.frame.id = 0;
			CanSpi_TxFrame.frame.dlc = 0;
			CanSpi_TxFrame.frame.data0 = 0;
			CanSpi_TxFrame.frame.data1 = 0;
			CanSpi_TxFrame.frame.data2 = 0;
			CanSpi_TxFrame.frame.data3 = 0;
			CanSpi_TxFrame.frame.data4 = 0;
			CanSpi_TxFrame.frame.data5 = 0;
			CanSpi_TxFrame.frame.data6 = 0;
			CanSpi_TxFrame.frame.data7 = 0;
		}
		else
		{
			/* Do nothing. */
		}
		/* Status Command Actuator */
		if(0 /* Owned by Com configured 26-tick PDU. */)
		{
			CanSpi_TxFrame.frame.idType = 1;
			CanSpi_TxFrame.frame.id = 0x101;
			CanSpi_TxFrame.frame.dlc = 5;
			CanSpi_TxFrame.frame.data0 = CmdList_ActualValue[2];
			CanSpi_TxFrame.frame.data1 = CmdList_ActualValue[3];
			CanSpi_TxFrame.frame.data2 = StatusList_ComOutValue[4];
			CanSpi_TxFrame.frame.data3 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
			CanSpi_TxFrame.frame.data4 = StatusList_ComOutValue[1];
			CanSpi_Transmit(&CanSpi_TxFrame);
			CanSpi_TxFrame.frame.idType = 0;
			CanSpi_TxFrame.frame.id = 0;
			CanSpi_TxFrame.frame.dlc = 0;
			CanSpi_TxFrame.frame.data0 = 0;
			CanSpi_TxFrame.frame.data1 = 0;
			CanSpi_TxFrame.frame.data2 = 0;
			CanSpi_TxFrame.frame.data3 = 0;
			CanSpi_TxFrame.frame.data4 = 0;
			CanSpi_TxFrame.frame.data5 = 0;
			CanSpi_TxFrame.frame.data6 = 0;
			CanSpi_TxFrame.frame.data7 = 0;
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
	/* Refresh TX buffer. */
	CanSpi_TxFrame.frame.idType = 0;
	CanSpi_TxFrame.frame.id = 0;
	CanSpi_TxFrame.frame.dlc = 0;
	CanSpi_TxFrame.frame.data0 = 0;
	CanSpi_TxFrame.frame.data1 = 0;
	CanSpi_TxFrame.frame.data2 = 0;
	CanSpi_TxFrame.frame.data3 = 0;
	CanSpi_TxFrame.frame.data4 = 0;
	CanSpi_TxFrame.frame.data5 = 0;
	CanSpi_TxFrame.frame.data6 = 0;
	CanSpi_TxFrame.frame.data7 = 0;
	/* Increase the main counter. */
	CanSpi_MainCounter++;
}
void CanSpi_Sleep(void)
{
	CanSpi_MCP2515_BitModify(CANSPI_MCP2515_CANINTF, 0x40, 0x00);
	CanSpi_MCP2515_BitModify(CANSPI_MCP2515_CANINTE, 0x40, 0x40);
	CanSpi_MCP2515_SetSleepMode();
}
uint8 CanSpi_Initialize(void)
{
	CanSpi_MCP2515_Reset();
	CANSPI_RXF0 RXF0reg;
	CANSPI_RXF1 RXF1reg;
	CANSPI_RXF2 RXF2reg;
	CANSPI_RXF3 RXF3reg;
	CANSPI_RXF4 RXF4reg;
	CANSPI_RXF5 RXF5reg;
	CANSPI_RXM0 RXM0reg;
	CANSPI_RXM1 RXM1reg;
	RXM0reg.RXM0SIDH = 0xFF;
	RXM0reg.RXM0SIDL = 0xE0;
	RXM0reg.RXM0EID8 = 0x00;
	RXM0reg.RXM0EID0 = 0x00;
	RXM1reg.RXM1SIDH = 0xFF;
	RXM1reg.RXM1SIDL = 0xE0;
	RXM1reg.RXM1EID8 = 0x00;
	RXM1reg.RXM1EID0 = 0x00;
	RXF0reg.RXF0SIDH = 0x12;
	RXF0reg.RXF0SIDL = 0xE0;
	RXF0reg.RXF0EID8 = 0x00;
	RXF0reg.RXF0EID0 = 0x00;
	RXF1reg.RXF1SIDH = 0x13;
	RXF1reg.RXF1SIDL = 0x00;
	RXF1reg.RXF1EID8 = 0x00;
	RXF1reg.RXF1EID0 = 0x00;
	RXF2reg.RXF2SIDH = 0x13;
	RXF2reg.RXF2SIDL = 0x20;
	RXF2reg.RXF2EID8 = 0x00;
	RXF2reg.RXF2EID0 = 0x00;
	RXF3reg.RXF3SIDH = 0xA2;
	RXF3reg.RXF3SIDL = 0x00;
	RXF3reg.RXF3EID8 = 0x00;
	RXF3reg.RXF3EID0 = 0x00;
	RXF4reg.RXF4SIDH = 0xE2;
	RXF4reg.RXF4SIDL = 0x00;
	RXF4reg.RXF4EID8 = 0x00;
	RXF4reg.RXF4EID0 = 0x00;
	RXF5reg.RXF5SIDH = 0xE2;
	RXF5reg.RXF5SIDL = 0x00;
	RXF5reg.RXF5EID8 = 0x00;
	RXF5reg.RXF5EID0 = 0x00;
	CanSpi_MCP2515_Initialize();
	CanSpi_MCP2515_SetConfigMode();
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXM0SIDH, CANSPI_MCP2515_RXM0EID0, &(RXM0reg.RXM0SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXM1SIDH, CANSPI_MCP2515_RXM1EID0, &(RXM1reg.RXM1SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXF0SIDH, CANSPI_MCP2515_RXF0EID0, &(RXF0reg.RXF0SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXF1SIDH, CANSPI_MCP2515_RXF1EID0, &(RXF1reg.RXF1SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXF2SIDH, CANSPI_MCP2515_RXF2EID0, &(RXF2reg.RXF2SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXF3SIDH, CANSPI_MCP2515_RXF3EID0, &(RXF3reg.RXF3SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXF4SIDH, CANSPI_MCP2515_RXF4EID0, &(RXF4reg.RXF4SIDH));
	CanSpi_MCP2515_WriteByteSequence(CANSPI_MCP2515_RXF5SIDH, CANSPI_MCP2515_RXF5EID0, &(RXF5reg.RXF5SIDH));
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_RXB0CTRL, 0x04);
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_RXB1CTRL, 0x01);
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_CNF1, 0x00);
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_CNF2, 0x90);
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_CNF3, 0x82);
	CanSpi_MCP2515_SetNormalMode();
	return 1;
}
uint8 CanSpi_Transmit(CANSPI_uCAN_MSG *tempCanMsg)
{
    uint32 irq=__get_PRIMASK();
    if(!tempCanMsg || tempCanMsg->frame.dlc>8U) return 0U;
    __disable_irq();
    CanSpi_PollTx();
	uint8 returnValue = 0;
	CANSPI_id_reg_t idReg = {0};
	CANSPI_CtrlStatus_t ctrlStatus = {0};
	idReg.tempSIDH = 0;
	idReg.tempSIDL = 0;
	idReg.tempEID8 = 0;
	idReg.tempEID0 = 0;
	ctrlStatus.CtrlStatus = CanSpi_MCP2515_ReadStatus();
	if (ctrlStatus.TXB0REQ != 1)
	{
		CanSpi_ConvertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &idReg);
		CanSpi_MCP2515_LoadTxSequence(CANSPI_MCP2515_LOAD_TXB0SIDH, &(idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
		CanSpi_MCP2515_BitModify(0x2CU,0x04U,0U);
        CanSpi_DiagOwner[0]=(uint16)tempCanMsg->frame.id;
		CanSpi_MCP2515_RequestToSend(CANSPI_MCP2515_RTS_TX0);
		returnValue = 1;
	}
	else if (ctrlStatus.TXB1REQ != 1)
	{
		CanSpi_ConvertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &idReg);
		CanSpi_MCP2515_LoadTxSequence(CANSPI_MCP2515_LOAD_TXB1SIDH, &(idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
		CanSpi_MCP2515_BitModify(0x2CU,0x08U,0U);
        CanSpi_DiagOwner[1]=(uint16)tempCanMsg->frame.id;
		CanSpi_MCP2515_RequestToSend(CANSPI_MCP2515_RTS_TX1);
		returnValue = 1;
	}
	else if (ctrlStatus.TXB2REQ != 1)
	{
		CanSpi_ConvertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &idReg);
		CanSpi_MCP2515_LoadTxSequence(CANSPI_MCP2515_LOAD_TXB2SIDH, &(idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
		CanSpi_MCP2515_BitModify(0x2CU,0x10U,0U);
        CanSpi_DiagOwner[2]=(uint16)tempCanMsg->frame.id;
		CanSpi_MCP2515_RequestToSend(CANSPI_MCP2515_RTS_TX2);
		returnValue = 1;
	}
	else
	{
		/* Do nothing. */
	}
	if(irq==0U) __enable_irq();
	return (returnValue);
}
uint8 CanSpi_Receive(CANSPI_uCAN_MSG *tempCanMsg)
{
	__disable_irq();
	uint8 returnValue = 0;
	CANSPI_rx_reg_t rxReg = {0};
	CANSPI_CtrlRxStatus_t rxStatus = {0};
	rxStatus.CtrlRxStatus = CanSpi_MCP2515_GetRxStatus();

	if (rxStatus.rxBuffer != 0)
	{
		if ((rxStatus.rxBuffer == CANSPI_MSG_IN_RXB0)|(rxStatus.rxBuffer == CANSPI_MSG_IN_BOTH_BUFFERS))
		{
			CanSpi_MCP2515_ReadRxSequence(CANSPI_MCP2515_READ_RXB0SIDH, rxReg.rx_reg_array, sizeof(rxReg.rx_reg_array));
		}
		else if (rxStatus.rxBuffer == CANSPI_MSG_IN_RXB1)
		{
			CanSpi_MCP2515_ReadRxSequence(CANSPI_MCP2515_READ_RXB1SIDH, rxReg.rx_reg_array, sizeof(rxReg.rx_reg_array));
		}
		else
		{
			/* Do nothing. */
		}

		if (rxStatus.msgType == CANSPI_dEXTENDED_CAN_MSG_ID_2_0B)
		{
			tempCanMsg->frame.idType = (uint8) CANSPI_dEXTENDED_CAN_MSG_ID_2_0B;
			tempCanMsg->frame.id = CanSpi_ConvertReg2ExtendedCANid(rxReg.RXBnEID8, rxReg.RXBnEID0, rxReg.RXBnSIDH, rxReg.RXBnSIDL);
		}
		else
		{
			tempCanMsg->frame.idType = (uint8) CANSPI_dSTANDARD_CAN_MSG_ID_2_0B;
			tempCanMsg->frame.id = CanSpi_ConvertReg2StandardCANid(rxReg.RXBnSIDH, rxReg.RXBnSIDL);
		}
		tempCanMsg->frame.dlc   = rxReg.RXBnDLC;
		tempCanMsg->frame.data0 = rxReg.RXBnD0;
		tempCanMsg->frame.data1 = rxReg.RXBnD1;
		tempCanMsg->frame.data2 = rxReg.RXBnD2;
		tempCanMsg->frame.data3 = rxReg.RXBnD3;
		tempCanMsg->frame.data4 = rxReg.RXBnD4;
		tempCanMsg->frame.data5 = rxReg.RXBnD5;
		tempCanMsg->frame.data6 = rxReg.RXBnD6;
		tempCanMsg->frame.data7 = rxReg.RXBnD7;
		returnValue = 1;
	}
	else
	{
		/* Do nothing. */
	}
	__enable_irq();
	return (returnValue);
}
uint32 CanSpi_MessagesInBuffer(void)
{
	CANSPI_CtrlStatus_t ctrlStatus = {0};
	uint8 messageCount = 0;
	ctrlStatus.CtrlStatus = CanSpi_MCP2515_ReadStatus();
	if(ctrlStatus.RX0IF != 0) messageCount++;
	else
	{
		/* Do nothing */
	}
	if(ctrlStatus.RX1IF != 0) messageCount++;
	else
	{
		/* Do nothing */
	}
	return (messageCount);
}
static uint8 CanSpi_IsBussOff(void)
{
	CANSPI_CtrlErrorStatus_t errorStatus = {0};
	uint8 returnValue = 0;
	errorStatus.CtrlErrorStatus = CanSpi_MCP2515_ReadByte(CANSPI_MCP2515_EFLG);
	if(errorStatus.TXBO == 1) { returnValue = 1; Can_BusOff(); }
	else
	{
		/* Do nothing. */
	}
	return (returnValue);
}
static uint8 CanSpi_IsRxErrorPassive(void)
{
	CANSPI_CtrlErrorStatus_t errorStatus = {0};
	uint8 returnValue = 0;
	errorStatus.CtrlErrorStatus = CanSpi_MCP2515_ReadByte(CANSPI_MCP2515_EFLG);
	if(errorStatus.RXEP == 1) returnValue = 1;
	else
	{
		/* Do nothing. */
	}
	return (returnValue);
}
static uint8 CanSpi_IsTxErrorPassive(void)
{
	CANSPI_CtrlErrorStatus_t errorStatus = {0};
	uint8 returnValue = 0;
	errorStatus.CtrlErrorStatus = CanSpi_MCP2515_ReadByte(CANSPI_MCP2515_EFLG);
	if(errorStatus.TXEP == 1) returnValue = 1;
	else
	{
		/* Do nothing. */
	}
	return (returnValue);
}
static uint32 CanSpi_ConvertReg2ExtendedCANid(uint8 tempRXBn_EIDH, uint8 tempRXBn_EIDL, uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL)
{
	uint32 returnValue = 0;
	uint32 ConvertedID = 0;
	uint8 CAN_standardLo_ID_lo2bits = 0;
	uint8 CAN_standardLo_ID_hi3bits = 0;
	CAN_standardLo_ID_lo2bits = (tempRXBn_SIDL & 0x03);
	CAN_standardLo_ID_hi3bits = (tempRXBn_SIDL >> 5);
	ConvertedID = (tempRXBn_SIDH << 3);
	ConvertedID = ConvertedID + CAN_standardLo_ID_hi3bits;
	ConvertedID = (ConvertedID << 2);
	ConvertedID = ConvertedID + CAN_standardLo_ID_lo2bits;
	ConvertedID = (ConvertedID << 8);
	ConvertedID = ConvertedID + tempRXBn_EIDH;
	ConvertedID = (ConvertedID << 8);
	ConvertedID = ConvertedID + tempRXBn_EIDL;
	returnValue = ConvertedID;
	return (returnValue);
}
static uint32 CanSpi_ConvertReg2StandardCANid(uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL)
{
	uint32 returnValue = 0;
	uint32 ConvertedID = 0;
	ConvertedID = (tempRXBn_SIDH << 3);
	ConvertedID = ConvertedID + (tempRXBn_SIDL >> 5);
	returnValue = ConvertedID;
	return (returnValue);
}
static void CanSpi_ConvertCANid2Reg(uint32 tempPassedInID, uint8 canIdType, CANSPI_id_reg_t *passedIdReg)
{
	uint8 wipSIDL = 0;
	if (canIdType == CANSPI_dEXTENDED_CAN_MSG_ID_2_0B)
	{
		passedIdReg->tempEID0 = 0xFF & tempPassedInID;
		tempPassedInID = tempPassedInID >> 8;
		passedIdReg->tempEID8 = 0xFF & tempPassedInID;
		tempPassedInID = tempPassedInID >> 8;
		wipSIDL = 0x03 & tempPassedInID;
		tempPassedInID = tempPassedInID << 3;
		wipSIDL = (0xE0 & tempPassedInID) + wipSIDL;
		wipSIDL = wipSIDL + 0x08;
		passedIdReg->tempSIDL = 0xEB & wipSIDL;
		tempPassedInID = tempPassedInID >> 8;
		passedIdReg->tempSIDH = 0xFF & tempPassedInID;
	}
	else
	{
		passedIdReg->tempEID8 = 0;
		passedIdReg->tempEID0 = 0;
		tempPassedInID = tempPassedInID << 5;
		passedIdReg->tempSIDL = 0xFF & tempPassedInID;
		tempPassedInID = tempPassedInID >> 8;
		passedIdReg->tempSIDH = 0xFF & tempPassedInID;
	}
}
static bool CanSpi_MCP2515_Initialize(void)
{
	uint8 loop = 10;
	CANSPI_MCP2515_CS_HIGH();
	do
	{
		if(HAL_SPI_GetState(CANSPI_SPI_CAN) == HAL_SPI_STATE_READY) return true;
		else
		{
			/* Do nothing. */
		}
		loop--;
	}while(loop > 0);
	return false;
}
static bool CanSpi_MCP2515_SetConfigMode(void)
{
	uint8 loop = 10;
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_CANCTRL, 0x80);
	do
	{
		if((CanSpi_MCP2515_ReadByte(CANSPI_MCP2515_CANSTAT) & 0xE0) == 0x80) return true;
		else
		{
			/* Do nothing. */
		}
		loop--;
	}while(loop > 0);
	return false;
}
static bool CanSpi_MCP2515_SetNormalMode(void)
{
	uint8 loop = 10;
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_CANCTRL, 0x00);
	do
	{
		if((CanSpi_MCP2515_ReadByte(CANSPI_MCP2515_CANSTAT) & 0xE0) == 0x00) return true;
		else
		{
			/* Do nothing. */
		}
		loop--;
	}while(loop > 0);
	return false;
}
static bool CanSpi_MCP2515_SetSleepMode(void)
{
	uint8 loop = 10;
	CanSpi_MCP2515_WriteByte(CANSPI_MCP2515_CANCTRL, 0x20);
	do
	{
		if((CanSpi_MCP2515_ReadByte(CANSPI_MCP2515_CANSTAT) & 0xE0) == 0x20) return true;
		else
		{
			/* Do nothing. */
		}

		loop--;
	}while(loop > 0);
	return false;
}
static void CanSpi_MCP2515_Reset(void)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_RESET);
	CANSPI_MCP2515_CS_HIGH();
}
static uint8 CanSpi_MCP2515_ReadByte (uint8 address)
{
	uint8 retVal = 0;
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_READ);
	CanSpi_SPI_Tx(address);
	retVal = CanSpi_SPI_Rx();
	CANSPI_MCP2515_CS_HIGH();
	return retVal;
}
static void CanSpi_MCP2515_ReadRxSequence(uint8 instruction, uint8 *data, uint8 length)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(instruction);
	CanSpi_SPI_RxBuffer(data, length);
	CANSPI_MCP2515_CS_HIGH();
}
static void CanSpi_MCP2515_WriteByte(uint8 address, uint8 data)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_WRITE);
	CanSpi_SPI_Tx(address);
	CanSpi_SPI_Tx(data);
	CANSPI_MCP2515_CS_HIGH();
}
static void CanSpi_MCP2515_WriteByteSequence(uint8 startAddress, uint8 endAddress, uint8 *data)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_WRITE);
	CanSpi_SPI_Tx(startAddress);
	CanSpi_SPI_TxBuffer(data, (endAddress - startAddress + 1));
	CANSPI_MCP2515_CS_HIGH();
}
static void CanSpi_MCP2515_LoadTxSequence(uint8 instruction, uint8 *idReg, uint8 dlc, uint8 *data)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(instruction);
	CanSpi_SPI_TxBuffer(idReg, 4);
	CanSpi_SPI_Tx(dlc);
	CanSpi_SPI_TxBuffer(data, dlc);
	CANSPI_MCP2515_CS_HIGH();
}
static void CanSpi_MCP2515_RequestToSend(uint8 instruction)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(instruction);
	CANSPI_MCP2515_CS_HIGH();
}
static uint8 CanSpi_MCP2515_ReadStatus(void)
{
	uint8 retVal = 0;
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_READ_STATUS);
	retVal = CanSpi_SPI_Rx();
	CANSPI_MCP2515_CS_HIGH();
	return retVal;
}
static uint8 CanSpi_MCP2515_GetRxStatus(void)
{
	uint8 retVal = 0;
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_RX_STATUS);
	retVal = CanSpi_SPI_Rx();
	CANSPI_MCP2515_CS_HIGH();
	return retVal;
}
static void CanSpi_MCP2515_BitModify(uint8 address, uint8 mask, uint8 data)
{
	CANSPI_MCP2515_CS_LOW();
	CanSpi_SPI_Tx(CANSPI_MCP2515_BIT_MOD);
	CanSpi_SPI_Tx(address);
	CanSpi_SPI_Tx(mask);
	CanSpi_SPI_Tx(data);
	CANSPI_MCP2515_CS_HIGH();
}
static void CanSpi_SPI_Tx(uint8 data)
{
	HAL_SPI_Transmit(CANSPI_SPI_CAN, &data, 1, 2);
}
static void CanSpi_SPI_TxBuffer(uint8 *buffer, uint8 length)
{
	HAL_SPI_Transmit(CANSPI_SPI_CAN, buffer, length, 2);
}
static uint8 CanSpi_SPI_Rx(void)
{
	uint8 retVal = 0;
	HAL_SPI_Receive(CANSPI_SPI_CAN, &retVal, 1, 2);
	return retVal;
}
static void CanSpi_SPI_RxBuffer(uint8 *buffer, uint8 length)
{
	HAL_SPI_Receive(CANSPI_SPI_CAN, buffer, length, 2);
}
/* FUNCTIONS STOP */
/* STOP OF FILE */
