#include "main.h"
#include "gpio.h"
#include "Fbl.h"
#include "../../BSW/Com/Can/Can.h"
#include "../../BSW/Com/CanIf/CanIf.h"
#include "../../BSW/Com/CanTp/CanTp.h"
#include "../../BSW/Com/PduR/PduR.h"
#include <string.h>
#include <stdbool.h>
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
uint32_t *FBL_ResetCounterFBL=(uint32_t *)(FBL_BOOT_MAILBOX+12U);
static uint8_t txHardwarePending;
static uint16_t txHardwareAge;
#include "spi.h"
#include "usart.h"
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef enum
{
	DEFAULT = 0U,
	EXTENDED = 1U,
	PROGRAMMING = 2U,
	CODING = 3U,
	CALIBRATION = 4U,
	HARDRESET = 5U,
	SOFTRESET = 6U,
	JUMPTOAPPL = 7U
}FBL_DSC_t;
typedef union
{
	struct
	{
		uint8 idType;
		uint32 id;
		uint8 dlc;
		uint8 data0;
		uint8 data1;
		uint8 data2;
		uint8 data3;
		uint8 data4;
		uint8 data5;
		uint8 data6;
		uint8 data7;
	}frame;
}FBL_CANSPI_uCAN_MSG;
typedef union
{
	struct
	{
		unsigned RX0IF      : 1;
		unsigned RX1IF      : 1;
		unsigned TXB0REQ    : 1;
		unsigned TX0IF      : 1;
		unsigned TXB1REQ    : 1;
		unsigned TX1IF      : 1;
		unsigned TXB2REQ    : 1;
		unsigned TX2IF      : 1;
	};
	uint8 CtrlStatus;
}FBL_CANSPI_CtrlStatus_t;
typedef union
{
	struct
	{
		unsigned filter     : 3;
		unsigned msgType    : 2;
		unsigned unusedBit  : 1;
		unsigned rxBuffer   : 2;
	};
	uint8 CtrlRxStatus;
}FBL_CANSPI_CtrlRxStatus_t;
typedef union
{
	struct
	{
		unsigned EWARN      :1;
		unsigned RXWAR      :1;
		unsigned TXWAR      :1;
		unsigned RXEP       :1;
		unsigned TXEP       :1;
		unsigned TXBO       :1;
		unsigned RX0OVR     :1;
		unsigned RX1OVR     :1;
	};
	uint8 CtrlErrorStatus;
}FBL_CANSPI_CtrlErrorStatus_t;
typedef union
{
	struct
	{
		uint8_t RXBnSIDH;
		uint8_t RXBnSIDL;
		uint8_t RXBnEID8;
		uint8_t RXBnEID0;
		uint8_t RXBnDLC;
		uint8_t RXBnD0;
		uint8_t RXBnD1;
		uint8_t RXBnD2;
		uint8_t RXBnD3;
		uint8_t RXBnD4;
		uint8_t RXBnD5;
		uint8_t RXBnD6;
		uint8_t RXBnD7;
	};
	uint8 rx_reg_array[13];
}FBL_CANSPI_rx_reg_t;
typedef struct
{
	uint8 RXF0SIDH;
	uint8 RXF0SIDL;
	uint8 RXF0EID8;
	uint8 RXF0EID0;
}FBL_CANSPI_RXF0;
typedef struct
{
	uint8 RXF1SIDH;
	uint8 RXF1SIDL;
	uint8 RXF1EID8;
	uint8 RXF1EID0;
}FBL_CANSPI_RXF1;
typedef struct
{
	uint8 RXF2SIDH;
	uint8 RXF2SIDL;
	uint8 RXF2EID8;
	uint8 RXF2EID0;
}FBL_CANSPI_RXF2;
typedef struct
{
	uint8 RXF3SIDH;
	uint8 RXF3SIDL;
	uint8 RXF3EID8;
	uint8 RXF3EID0;
}FBL_CANSPI_RXF3;
typedef struct
{
	uint8 RXF4SIDH;
	uint8 RXF4SIDL;
	uint8 RXF4EID8;
	uint8 RXF4EID0;
}FBL_CANSPI_RXF4;

typedef struct
{
	uint8 RXF5SIDH;
	uint8 RXF5SIDL;
	uint8 RXF5EID8;
	uint8 RXF5EID0;
}FBL_CANSPI_RXF5;
typedef struct
{
	uint8 RXM0SIDH;
	uint8 RXM0SIDL;
	uint8 RXM0EID8;
	uint8 RXM0EID0;
}FBL_CANSPI_RXM0;
typedef struct
{
	uint8 RXM1SIDH;
	uint8 RXM1SIDL;
	uint8 RXM1EID8;
	uint8 RXM1EID0;
}FBL_CANSPI_RXM1;
typedef struct
{
	uint8 tempSIDH;
	uint8 tempSIDL;
	uint8 tempEID8;
	uint8 tempEID0;
}FBL_CANSPI_id_reg_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SESSIONSTATUS_ADDR 						0x2001fdc0
#define NCR_START_ADDRESS 						0x2001F30C
#define NCR_END_ADDRESS   						0x2001FFFF
#define FBL_BLU_RAM_EXECUTION_ADDRESS 			0x20001000
#define NCR_SIZE          						(NCR_END_ADDRESS - NCR_START_ADDRESS + 1)
#define APPL_START_ADDRESS 						0x08060000U
#define APPL_END_ADDRESS                         0x08080000U
#define FBL_START_ADDRESS 						0x8000000
#define RESET_COUNTER_ADDRESS					0x2001fdd0
#define FBL_CANSPI_MCP2515_RESET           		0xC0
#define FBL_CANSPI_MCP2515_READ            		0x03
#define FBL_CANSPI_MCP2515_READ_RXB0SIDH   		0x90
#define FBL_CANSPI_MCP2515_READ_RXB0D0     		0x92
#define FBL_CANSPI_MCP2515_READ_RXB1SIDH   		0x94
#define FBL_CANSPI_MCP2515_READ_RXB1D0     		0x96
#define FBL_CANSPI_MCP2515_WRITE           		0x02
#define FBL_CANSPI_MCP2515_LOAD_TXB0SIDH   		0x40
#define FBL_CANSPI_MCP2515_LOAD_TXB0D0     		0x41
#define FBL_CANSPI_MCP2515_LOAD_TXB1SIDH   		0x42
#define FBL_CANSPI_MCP2515_LOAD_TXB1D0     		0x43
#define FBL_CANSPI_MCP2515_LOAD_TXB2SIDH   		0x44
#define FBL_CANSPI_MCP2515_LOAD_TXB2D0     		0x45
#define FBL_CANSPI_MCP2515_RTS_TX0         		0x81
#define FBL_CANSPI_MCP2515_RTS_TX1         		0x82
#define FBL_CANSPI_MCP2515_RTS_TX2         		0x84
#define FBL_CANSPI_MCP2515_RTS_ALL        		0x87
#define FBL_CANSPI_MCP2515_READ_STATUS    		0xA0
#define FBL_CANSPI_MCP2515_RX_STATUS       		0xB0
#define FBL_CANSPI_MCP2515_BIT_MOD         		0x05
#define FBL_CANSPI_MCP2515_RXF0SIDH				0x00
#define FBL_CANSPI_MCP2515_RXF0SIDL				0x01
#define FBL_CANSPI_MCP2515_RXF0EID8				0x02
#define FBL_CANSPI_MCP2515_RXF0EID0				0x03
#define FBL_CANSPI_MCP2515_RXF1SIDH				0x04
#define FBL_CANSPI_MCP2515_RXF1SIDL				0x05
#define FBL_CANSPI_MCP2515_RXF1EID8				0x06
#define FBL_CANSPI_MCP2515_RXF1EID0				0x07
#define FBL_CANSPI_MCP2515_RXF2SIDH				0x08
#define FBL_CANSPI_MCP2515_RXF2SIDL				0x09
#define FBL_CANSPI_MCP2515_RXF2EID8				0x0A
#define FBL_CANSPI_MCP2515_RXF2EID0				0x0B
#define FBL_CANSPI_MCP2515_CANSTAT				0x0E
#define FBL_CANSPI_MCP2515_CANCTRL				0x0F
#define FBL_CANSPI_MCP2515_RXF3SIDH				0x10
#define FBL_CANSPI_MCP2515_RXF3SIDL				0x11
#define FBL_CANSPI_MCP2515_RXF3EID8				0x12
#define FBL_CANSPI_MCP2515_RXF3EID0				0x13
#define FBL_CANSPI_MCP2515_RXF4SIDH				0x14
#define FBL_CANSPI_MCP2515_RXF4SIDL				0x15
#define FBL_CANSPI_MCP2515_RXF4EID8				0x16
#define FBL_CANSPI_MCP2515_RXF4EID0				0x17
#define FBL_CANSPI_MCP2515_RXF5SIDH				0x18
#define FBL_CANSPI_MCP2515_RXF5SIDL				0x19
#define FBL_CANSPI_MCP2515_RXF5EID8				0x1A
#define FBL_CANSPI_MCP2515_RXF5EID0				0x1B
#define FBL_CANSPI_MCP2515_TEC					0x1C
#define FBL_CANSPI_MCP2515_REC					0x1D
#define FBL_CANSPI_MCP2515_RXM0SIDH				0x20
#define FBL_CANSPI_MCP2515_RXM0SIDL				0x21
#define FBL_CANSPI_MCP2515_RXM0EID8				0x22
#define FBL_CANSPI_MCP2515_RXM0EID0				0x23
#define FBL_CANSPI_MCP2515_RXM1SIDH				0x24
#define FBL_CANSPI_MCP2515_RXM1SIDL				0x25
#define FBL_CANSPI_MCP2515_RXM1EID8				0x26
#define FBL_CANSPI_MCP2515_RXM1EID0				0x27
#define FBL_CANSPI_MCP2515_CNF3					0x28
#define FBL_CANSPI_MCP2515_CNF2					0x29
#define FBL_CANSPI_MCP2515_CNF1					0x2A
#define FBL_CANSPI_MCP2515_CANINTE				0x2B
#define FBL_CANSPI_MCP2515_CANINTF				0x2C
#define FBL_CANSPI_MCP2515_EFLG					0x2D
#define FBL_CANSPI_MCP2515_TXB0CTRL				0x30
#define FBL_CANSPI_MCP2515_TXB1CTRL				0x40
#define FBL_CANSPI_MCP2515_TXB2CTRL				0x50
#define FBL_CANSPI_MCP2515_RXB0CTRL				0x60
#define FBL_CANSPI_MCP2515_RXB0SIDH				0x61
#define FBL_CANSPI_MCP2515_RXB1CTRL				0x70
#define FBL_CANSPI_MCP2515_RXB1SIDH				0x71
#define FBL_CANSPI_MSG_IN_RXB0             		0x01
#define FBL_CANSPI_MSG_IN_RXB1             		0x02
#define FBL_CANSPI_MSG_IN_BOTH_BUFFERS     		0x03
#define FBL_CANSPI_dSTANDARD_CAN_MSG_ID_2_0B 	1
#define FBL_CANSPI_dEXTENDED_CAN_MSG_ID_2_0B 	2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define FBL_CANSPI_SPI_CAN                 	&hspi1
#define FBL_CANSPI_MCP2515_CS_HIGH()   		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET)
#define FBL_CANSPI_MCP2515_CS_LOW()    		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET)
/* USER CODE END PM */


void FBL_CanSpi_Sleep(void);
uint8 FBL_CanSpi_Initialize(void);
uint8 FBL_CanSpi_Transmit(FBL_CANSPI_uCAN_MSG *tempCanMsg);
uint8 FBL_CanSpi_Receive(FBL_CANSPI_uCAN_MSG *tempCanMsg);
uint32 FBL_CanSpi_MessagesInBuffer(void);
uint8 CanSpi_IsBussOff(void);
uint8 FBL_CanSpi_IsRxErrorPassive(void);
uint8 FBL_CanSpi_IsTxErrorPassive(void);
uint32 FBL_CanSpi_ConvertReg2ExtendedCANid(uint8 tempRXBn_EIDH, uint8 tempRXBn_EIDL, uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL);
uint32 FBL_CanSpi_ConvertReg2StandardCANid(uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL);
void FBL_CanSpi_ConvertCANid2Reg(uint32 tempPassedInID, uint8 canIdType, FBL_CANSPI_id_reg_t *passedIdReg);
bool FBL_CanSpi_MCP2515_Initialize(void);
bool FBL_CanSpi_MCP2515_SetConfigMode(void);
bool FBL_CanSpi_MCP2515_SetNormalMode(void);
bool FBL_CanSpi_MCP2515_SetSleepMode(void);
void FBL_CanSpi_MCP2515_Reset(void);
void FBL_CanSpi_MCP2515_ReadRxSequence(uint8 instruction, uint8 *data, uint8 length);
void FBL_CanSpi_MCP2515_WriteByte(uint8 address, uint8 data);
void FBL_CanSpi_MCP2515_WriteByteSequence(uint8 startAddress, uint8 endAddress, uint8 *data);
void FBL_CanSpi_MCP2515_LoadTxSequence(uint8 instruction, uint8 *idReg, uint8 dlc, uint8 *data);
void FBL_CanSpi_MCP2515_RequestToSend(uint8 instruction);
uint8 FBL_CanSpi_MCP2515_ReadStatus(void);
uint8 FBL_CanSpi_MCP2515_GetRxStatus(void);
void FBL_CanSpi_MCP2515_BitModify(uint8 address, uint8 mask, uint8 data);
void FBL_CanSpi_SPI_Tx(uint8 data);
void FBL_CanSpi_SPI_TxBuffer(uint8 *buffer, uint8 length);
uint8 FBL_CanSpi_SPI_Rx(void);
void FBL_CanSpi_SPI_RxBuffer(uint8 *buffer, uint8 length);
FBL_CANSPI_id_reg_t FBL_idReg;
FBL_CANSPI_CtrlStatus_t FBL_ctrlStatus;
FBL_CANSPI_CtrlErrorStatus_t FBL_errorStatus;
uint8 FBL_CanSpi_MCP2515_ReadByte(uint8 address);
int Fbl_BoardCanTransmit(const uint8_t *p) {
 FBL_CANSPI_id_reg_t id={0};
 if(txHardwarePending || (FBL_CanSpi_MCP2515_ReadByte(0x30)&8U)) return 1;
 FBL_CanSpi_ConvertCANid2Reg(0x710,1,&id);
 FBL_CanSpi_MCP2515_BitModify(0x2C,4,0);
 FBL_CanSpi_MCP2515_LoadTxSequence(FBL_CANSPI_MCP2515_LOAD_TXB0SIDH,&id.tempSIDH,8,(uint8 *)p);
 txHardwarePending=1;txHardwareAge=0;
 FBL_CanSpi_MCP2515_RequestToSend(FBL_CANSPI_MCP2515_RTS_TX0);return 0;
}
static void boardPoll(void) {
 FBL_CANSPI_uCAN_MSG f={0};Can_FrameType frame;
 uint8_t count=0,ctrl,flags;
 if(txHardwarePending) {
  ctrl=FBL_CanSpi_MCP2515_ReadByte(0x30);flags=FBL_CanSpi_MCP2515_ReadByte(0x2C);
  if(!(ctrl&8U) || ++txHardwareAge>=200U) {
   if(ctrl&8U)FBL_CanSpi_MCP2515_BitModify(0x30,8,0);
   txHardwarePending=0;Can_TxConfirmation(!(ctrl&0x58U) && (flags&4U));
   FBL_CanSpi_MCP2515_BitModify(0x2C,4,0);
  }
 }
 while(count++<2U && FBL_CanSpi_Receive(&f)) {
  if(f.frame.idType!=1 || f.frame.id!=0x710 || f.frame.dlc>8) continue;
  frame.id=0x710;frame.dlc=f.frame.dlc;memcpy(frame.data,&f.frame.data0,frame.dlc);Can_RxIndication(&frame);
 }
}
int Fbl_BoardRead(uint32_t a,void *p,uint32_t n) {
 if(!p || a<FBL_APP_START || a>=FBL_APP_END || n>FBL_APP_END-a) return 1;
 memcpy(p,(const void *)(uintptr_t)a,n);return 0;
}
int Fbl_BoardProgram(uint32_t a,const void *p,uint32_t n) {
 const uint8_t *src=p;uint32_t i;int error=0;
 if(!p || !n || a<FBL_APP_START || a>=FBL_APP_END || n>FBL_APP_END-a) return 1;
 if(HAL_FLASH_Unlock()!=HAL_OK){(void)HAL_FLASH_Lock();return 1;}
 for(i=0;i<n;++i) {
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,a+i,src[i])!=HAL_OK || *(volatile uint8_t *)(uintptr_t)(a+i)!=src[i]) {error=1;break;}
 }
 if(HAL_FLASH_Lock()!=HAL_OK)error=1;return error;
}
int Fbl_BoardErase(void) {
 FLASH_EraseInitTypeDef e={0};uint32_t error=0,at;int result=0;
 e.TypeErase=FLASH_TYPEERASE_SECTORS;e.VoltageRange=FLASH_VOLTAGE_RANGE_3;e.Sector=7U;e.NbSectors=1U;
 if(HAL_FLASH_Unlock()!=HAL_OK){(void)HAL_FLASH_Lock();return 1;}
 if(HAL_FLASHEx_Erase(&e,&error)!=HAL_OK || error!=0xFFFFFFFFU)result=1;
 if(HAL_FLASH_Lock()!=HAL_OK)result=1;
 if(!result)for(at=FBL_APP_START;at<FBL_APP_END;at+=4)if(*(volatile uint32_t *)(uintptr_t)at!=0xFFFFFFFFU){result=1;break;}
 return result;
}
void Fbl_BoardReset(void) {
 volatile uint32_t *box=(volatile uint32_t *)FBL_BOOT_MAILBOX;box[0]=0;__DSB();NVIC_SystemReset();
}
void Fbl_BoardJump(void) {
 uint32_t sp,pc,i;
 if(!Fbl_ApplicationValid())return;
 sp=*(volatile uint32_t *)FBL_APP_START;pc=*(volatile uint32_t *)(FBL_APP_START+4U);
 __disable_irq();SysTick->CTRL=0;SysTick->LOAD=0;SysTick->VAL=0;
 for(i=0;i<8;++i){NVIC->ICER[i]=0xFFFFFFFFU;NVIC->ICPR[i]=0xFFFFFFFFU;}
 SCB->ICSR=SCB_ICSR_PENDSTCLR_Msk|SCB_ICSR_PENDSVCLR_Msk;SCB->VTOR=FBL_APP_START;
 __DSB();__ISB();
 /* Naked tail uses no C stack after MSP replacement. APP Reset_Handler owns initialization. */
 __asm volatile("msr msp, %0\n movs r0, #0\n msr control, r0\n isb\n cpsie i\n bx %1"::"r"(sp),"r"(pc):"r0","memory");
 __builtin_unreachable();
}
int main(void) {
 uint32_t lastTick;uint8_t stay=0;
 volatile uint32_t *box=(volatile uint32_t *)FBL_BOOT_MAILBOX;
 HAL_Init();SystemClock_Config();MX_GPIO_Init();
 Can_Init();CanIf_Init();CanTp_Init();PduR_Init();Fbl_Init();
 MX_SPI1_Init(); MX_USART1_UART_Init(); FBL_CanSpi_Initialize();
 MX_NVIC_Init();
 if(!(RCC->CSR&(RCC_CSR_PORRSTF|RCC_CSR_BORRSTF)) && box[0]==0x46424C31U && box[2]==~box[1]){stay=1;Fbl_SetInitialSession((uint8_t)box[1]);}
 box[0]=0;RCC->CSR|=RCC_CSR_RMVF;
 if(!stay && Fbl_ApplicationValid())Fbl_BoardJump();
 lastTick=HAL_GetTick();
 for(;;)if((uint32_t)(HAL_GetTick()-lastTick)>=5U){lastTick=HAL_GetTick();boardPoll();Fbl_MainFunction();}
}
void Error_Handler(void) { __disable_irq();for(;;){} }

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_NVIC_Init(void)
{
  /* PVD_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PVD_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PVD_IRQn);
  /* FLASH_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(FLASH_IRQn);
  /* RCC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(RCC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(RCC_IRQn);
  /* SPI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SPI1_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* FPU_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(FPU_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(FPU_IRQn);
}
void FBL_CanSpi_Sleep(void)
{
	FBL_CanSpi_MCP2515_BitModify(FBL_CANSPI_MCP2515_CANINTF, 0x40, 0x00);
	FBL_CanSpi_MCP2515_BitModify(FBL_CANSPI_MCP2515_CANINTE, 0x40, 0x40);
	FBL_CanSpi_MCP2515_SetSleepMode();
}
uint8 FBL_CanSpi_Initialize(void)
{
	FBL_CanSpi_MCP2515_Reset();
	FBL_CANSPI_RXF0 FBL_RXF0reg;
	FBL_CANSPI_RXF1 FBL_RXF1reg;
	FBL_CANSPI_RXF2 FBL_RXF2reg;
	FBL_CANSPI_RXF3 FBL_RXF3reg;
	FBL_CANSPI_RXF4 FBL_RXF4reg;
	FBL_CANSPI_RXF5 FBL_RXF5reg;
	FBL_CANSPI_RXM0 FBL_RXM0reg;
	FBL_CANSPI_RXM1 FBL_RXM1reg;
	FBL_RXM0reg.RXM0SIDH = 0xFF;
	FBL_RXM0reg.RXM0SIDL = 0xE0;
	FBL_RXM0reg.RXM0EID8 = 0x00;
	FBL_RXM0reg.RXM0EID0 = 0x00;
	FBL_RXM1reg.RXM1SIDH = 0x00;
	FBL_RXM1reg.RXM1SIDL = 0x00;
	FBL_RXM1reg.RXM1EID8 = 0x00;
	FBL_RXM1reg.RXM1EID0 = 0x00;
	FBL_RXF0reg.RXF0SIDH = (0x710 >> 3) & 0xFF;
	FBL_RXF0reg.RXF0SIDL = (0x710 << 5) & 0xE0;
	FBL_RXF0reg.RXF0EID8 = 0x00;
	FBL_RXF0reg.RXF0EID0 = 0x00;
	FBL_RXF1reg.RXF1SIDH = 0x00;
	FBL_RXF1reg.RXF1SIDL = 0x08;
	FBL_RXF1reg.RXF1EID8 = 0x00;
	FBL_RXF1reg.RXF1EID0 = 0x00;
	FBL_RXF2reg.RXF2SIDH = 0x00;
	FBL_RXF2reg.RXF2SIDL = 0x00;
	FBL_RXF2reg.RXF2EID8 = 0x00;
	FBL_RXF2reg.RXF2EID0 = 0x00;
	FBL_RXF3reg.RXF3SIDH = 0x00;
	FBL_RXF3reg.RXF3SIDL = 0x00;
	FBL_RXF3reg.RXF3EID8 = 0x00;
	FBL_RXF3reg.RXF3EID0 = 0x00;
	FBL_RXF4reg.RXF4SIDH = 0x00;
	FBL_RXF4reg.RXF4SIDL = 0x00;
	FBL_RXF4reg.RXF4EID8 = 0x00;
	FBL_RXF4reg.RXF4EID0 = 0x00;
	FBL_RXF5reg.RXF5SIDH = 0x00;
	FBL_RXF5reg.RXF5SIDL = 0x08;
	FBL_RXF5reg.RXF5EID8 = 0x00;
	FBL_RXF5reg.RXF5EID0 = 0x00;
	FBL_CanSpi_MCP2515_Initialize();
	FBL_CanSpi_MCP2515_SetConfigMode();
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXM0SIDH, FBL_CANSPI_MCP2515_RXM0EID0, &(FBL_RXM0reg.RXM0SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXM1SIDH, FBL_CANSPI_MCP2515_RXM1EID0, &(FBL_RXM1reg.RXM1SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXF0SIDH, FBL_CANSPI_MCP2515_RXF0EID0, &(FBL_RXF0reg.RXF0SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXF1SIDH, FBL_CANSPI_MCP2515_RXF1EID0, &(FBL_RXF1reg.RXF1SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXF2SIDH, FBL_CANSPI_MCP2515_RXF2EID0, &(FBL_RXF2reg.RXF2SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXF3SIDH, FBL_CANSPI_MCP2515_RXF3EID0, &(FBL_RXF3reg.RXF3SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXF4SIDH, FBL_CANSPI_MCP2515_RXF4EID0, &(FBL_RXF4reg.RXF4SIDH));
	FBL_CanSpi_MCP2515_WriteByteSequence(FBL_CANSPI_MCP2515_RXF5SIDH, FBL_CANSPI_MCP2515_RXF5EID0, &(FBL_RXF5reg.RXF5SIDH));
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_RXB0CTRL, 0x04);
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_RXB1CTRL, 0x01);
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_CNF1, 0x00);
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_CNF2, 0x90);
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_CNF3, 0x82);
	FBL_CanSpi_MCP2515_SetNormalMode();
	return 1;
}
uint8 FBL_CanSpi_Transmit(FBL_CANSPI_uCAN_MSG *tempCanMsg)
{
	uint8 FBL_returnValue = 0;
	FBL_idReg.tempSIDH = 0;
	FBL_idReg.tempSIDL = 0;
	FBL_idReg.tempEID8 = 0;
	FBL_idReg.tempEID0 = 0;
	FBL_ctrlStatus.CtrlStatus = FBL_CanSpi_MCP2515_ReadStatus();
	if (FBL_ctrlStatus.TXB0REQ != 1)
	{
		FBL_CanSpi_ConvertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &FBL_idReg);
		FBL_CanSpi_MCP2515_LoadTxSequence(FBL_CANSPI_MCP2515_LOAD_TXB0SIDH, &(FBL_idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
		FBL_CanSpi_MCP2515_RequestToSend(FBL_CANSPI_MCP2515_RTS_TX0);
		FBL_returnValue = 1;
	}
	else if (FBL_ctrlStatus.TXB1REQ != 1)
	{
		FBL_CanSpi_ConvertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &FBL_idReg);
		FBL_CanSpi_MCP2515_LoadTxSequence(FBL_CANSPI_MCP2515_LOAD_TXB1SIDH, &(FBL_idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
		FBL_CanSpi_MCP2515_RequestToSend(FBL_CANSPI_MCP2515_RTS_TX1);
		FBL_returnValue = 1;
	}
	else if (FBL_ctrlStatus.TXB2REQ != 1)
	{
		FBL_CanSpi_ConvertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &FBL_idReg);
		FBL_CanSpi_MCP2515_LoadTxSequence(FBL_CANSPI_MCP2515_LOAD_TXB2SIDH, &(FBL_idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
		FBL_CanSpi_MCP2515_RequestToSend(FBL_CANSPI_MCP2515_RTS_TX2);
		FBL_returnValue = 1;
	}
	else
	{
		/* Do nothing. */
	}
	return (FBL_returnValue);
}
uint8 FBL_CanSpi_Receive(FBL_CANSPI_uCAN_MSG *tempCanMsg)
{
	uint8 FBL_returnValue = 0;
	FBL_CANSPI_rx_reg_t FBL_rxReg ={0};
	FBL_CANSPI_CtrlRxStatus_t FBL_rxStatus = {0};
	FBL_rxStatus.CtrlRxStatus = FBL_CanSpi_MCP2515_GetRxStatus();
	if (FBL_rxStatus.rxBuffer != 0)
	{
		if ((FBL_rxStatus.rxBuffer == FBL_CANSPI_MSG_IN_RXB0)|(FBL_rxStatus.rxBuffer == FBL_CANSPI_MSG_IN_BOTH_BUFFERS))
		{
			FBL_CanSpi_MCP2515_ReadRxSequence(FBL_CANSPI_MCP2515_READ_RXB0SIDH, FBL_rxReg.rx_reg_array, sizeof(FBL_rxReg.rx_reg_array));
		}
		else if (FBL_rxStatus.rxBuffer == FBL_CANSPI_MSG_IN_RXB1)
		{
			FBL_CanSpi_MCP2515_ReadRxSequence(FBL_CANSPI_MCP2515_READ_RXB1SIDH, FBL_rxReg.rx_reg_array, sizeof(FBL_rxReg.rx_reg_array));
		}
		if (FBL_rxStatus.msgType == FBL_CANSPI_dEXTENDED_CAN_MSG_ID_2_0B)
		{
			tempCanMsg->frame.idType = (uint8) FBL_CANSPI_dEXTENDED_CAN_MSG_ID_2_0B;
			tempCanMsg->frame.id = FBL_CanSpi_ConvertReg2ExtendedCANid(FBL_rxReg.RXBnEID8, FBL_rxReg.RXBnEID0, FBL_rxReg.RXBnSIDH, FBL_rxReg.RXBnSIDL);
		}
		else
		{
			tempCanMsg->frame.idType = (uint8) FBL_CANSPI_dSTANDARD_CAN_MSG_ID_2_0B;
			tempCanMsg->frame.id = FBL_CanSpi_ConvertReg2StandardCANid(FBL_rxReg.RXBnSIDH, FBL_rxReg.RXBnSIDL);
		}
		tempCanMsg->frame.dlc   = FBL_rxReg.RXBnDLC;
		tempCanMsg->frame.data0 = FBL_rxReg.RXBnD0;
		tempCanMsg->frame.data1 = FBL_rxReg.RXBnD1;
		tempCanMsg->frame.data2 = FBL_rxReg.RXBnD2;
		tempCanMsg->frame.data3 = FBL_rxReg.RXBnD3;
		tempCanMsg->frame.data4 = FBL_rxReg.RXBnD4;
		tempCanMsg->frame.data5 = FBL_rxReg.RXBnD5;
		tempCanMsg->frame.data6 = FBL_rxReg.RXBnD6;
		tempCanMsg->frame.data7 = FBL_rxReg.RXBnD7;
		FBL_returnValue = 1;
	}
	else
	{
		/* Do nothing. */
	}
	return (FBL_returnValue);
}
uint32 FBL_CanSpi_MessagesInBuffer(void)
{
	uint8 FBL_messageCount = 0;
	FBL_ctrlStatus.CtrlStatus = FBL_CanSpi_MCP2515_ReadStatus();
	if(FBL_ctrlStatus.RX0IF != 0) FBL_messageCount++;
	else
	{
		/* Do nothing */
	}
	if(FBL_ctrlStatus.RX1IF != 0) FBL_messageCount++;
	else
	{
		/* Do nothing */
	}
	return (FBL_messageCount);
}
uint8 CanSpi_IsBussOff(void)
{
	uint8 FBL_returnValue = 0;
	FBL_errorStatus.CtrlErrorStatus = FBL_CanSpi_MCP2515_ReadByte(FBL_CANSPI_MCP2515_EFLG);
	if(FBL_errorStatus.TXBO == 1) FBL_returnValue = 1;
	else
	{
		/* Do nothing. */
	}
	return (FBL_returnValue);
}
uint8 FBL_CanSpi_IsRxErrorPassive(void)
{
	uint8 FBL_returnValue = 0;
	FBL_errorStatus.CtrlErrorStatus = FBL_CanSpi_MCP2515_ReadByte(FBL_CANSPI_MCP2515_EFLG);
	if(FBL_errorStatus.RXEP == 1) FBL_returnValue = 1;
	else
	{
		/* Do nothing. */
	}

	return (FBL_returnValue);
}
uint8 FBL_CanSpi_IsTxErrorPassive(void)
{
	uint8 FBL_returnValue = 0;
	FBL_errorStatus.CtrlErrorStatus = FBL_CanSpi_MCP2515_ReadByte(FBL_CANSPI_MCP2515_EFLG);
	if(FBL_errorStatus.TXEP == 1) FBL_returnValue = 1;
	else
	{
		/* Do nothing. */
	}
	return (FBL_returnValue);
}
uint32 FBL_CanSpi_ConvertReg2ExtendedCANid(uint8 tempRXBn_EIDH, uint8 tempRXBn_EIDL, uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL)
{
	static uint32 returnValue = 0;
	static uint32 ConvertedID = 0;
	static uint8 CAN_standardLo_ID_lo2bits;
	static uint8 CAN_standardLo_ID_hi3bits;
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
uint32 FBL_CanSpi_ConvertReg2StandardCANid(uint8 tempRXBn_SIDH, uint8 tempRXBn_SIDL)
{
	static uint32 returnValue = 0;
	static uint32 ConvertedID = 0;
	ConvertedID = (tempRXBn_SIDH << 3);
	ConvertedID = ConvertedID + (tempRXBn_SIDL >> 5);
	returnValue = ConvertedID;
	return (returnValue);
}
void FBL_CanSpi_ConvertCANid2Reg(uint32 tempPassedInID, uint8 canIdType, FBL_CANSPI_id_reg_t *passedIdReg)
{
	static uint8 wipSIDL = 0;
	if (canIdType == FBL_CANSPI_dEXTENDED_CAN_MSG_ID_2_0B)
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
bool FBL_CanSpi_MCP2515_Initialize(void)
{
	uint8 loop = 10;
	FBL_CANSPI_MCP2515_CS_HIGH();
	do
	{
		if(HAL_SPI_GetState(FBL_CANSPI_SPI_CAN) == HAL_SPI_STATE_READY) return true;
		else
		{
			/* Do nothing. */
		}
		loop--;
	}while(loop > 0);
	return false;
}
bool FBL_CanSpi_MCP2515_SetConfigMode(void)
{
	uint8 loop = 10;
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_CANCTRL, 0x80);
	do
	{
		if((FBL_CanSpi_MCP2515_ReadByte(FBL_CANSPI_MCP2515_CANSTAT) & 0xE0) == 0x80) return true;
		else
		{
			/* Do nothing. */
		}
		loop--;
	}while(loop > 0);
	return false;
}
bool FBL_CanSpi_MCP2515_SetNormalMode(void)
{
	uint8 loop = 10;
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_CANCTRL, 0x00);
	do
	{
		if((FBL_CanSpi_MCP2515_ReadByte(FBL_CANSPI_MCP2515_CANSTAT) & 0xE0) == 0x00) return true;
		else
		{
			/* Do nothing. */
		}
		loop--;
	}while(loop > 0);
	return false;
}
bool FBL_CanSpi_MCP2515_SetSleepMode(void)
{
	uint8 loop = 10;
	FBL_CanSpi_MCP2515_WriteByte(FBL_CANSPI_MCP2515_CANCTRL, 0x20);
	do
	{
		if((FBL_CanSpi_MCP2515_ReadByte(FBL_CANSPI_MCP2515_CANSTAT) & 0xE0) == 0x20) return true;
		else
		{
			/* Do nothing. */
		}

		loop--;
	}while(loop > 0);
	return false;
}
void FBL_CanSpi_MCP2515_Reset(void)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_RESET);
	FBL_CANSPI_MCP2515_CS_HIGH();
}
void FBL_CanSpi_MCP2515_ReadRxSequence(uint8 instruction, uint8 *data, uint8 length)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(instruction);
	FBL_CanSpi_SPI_RxBuffer(data, length);
	FBL_CANSPI_MCP2515_CS_HIGH();
}
void FBL_CanSpi_MCP2515_WriteByte(uint8 address, uint8 data)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_WRITE);
	FBL_CanSpi_SPI_Tx(address);
	FBL_CanSpi_SPI_Tx(data);
	FBL_CANSPI_MCP2515_CS_HIGH();
}
void FBL_CanSpi_MCP2515_WriteByteSequence(uint8 startAddress, uint8 endAddress, uint8 *data)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_WRITE);
	FBL_CanSpi_SPI_Tx(startAddress);
	FBL_CanSpi_SPI_TxBuffer(data, (endAddress - startAddress + 1));
	FBL_CANSPI_MCP2515_CS_HIGH();
}
void FBL_CanSpi_MCP2515_LoadTxSequence(uint8 instruction, uint8 *idReg, uint8 dlc, uint8 *data)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(instruction);
	FBL_CanSpi_SPI_TxBuffer(idReg, 4);
	FBL_CanSpi_SPI_Tx(dlc);
	FBL_CanSpi_SPI_TxBuffer(data, dlc);
	FBL_CANSPI_MCP2515_CS_HIGH();
}
void FBL_CanSpi_MCP2515_RequestToSend(uint8 instruction)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(instruction);
	FBL_CANSPI_MCP2515_CS_HIGH();
}
uint8 FBL_CanSpi_MCP2515_ReadStatus(void)
{
	static uint8 FBL_retVal = 0;
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_READ_STATUS);
	FBL_retVal = FBL_CanSpi_SPI_Rx();
	FBL_CANSPI_MCP2515_CS_HIGH();
	return FBL_retVal;
}
uint8 FBL_CanSpi_MCP2515_GetRxStatus(void)
{
	static uint8 FBL_retVal = 0;
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_RX_STATUS);
	FBL_retVal = FBL_CanSpi_SPI_Rx();
	FBL_CANSPI_MCP2515_CS_HIGH();
	return FBL_retVal;
}
void FBL_CanSpi_MCP2515_BitModify(uint8 address, uint8 mask, uint8 data)
{
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_BIT_MOD);
	FBL_CanSpi_SPI_Tx(address);
	FBL_CanSpi_SPI_Tx(mask);
	FBL_CanSpi_SPI_Tx(data);
	FBL_CANSPI_MCP2515_CS_HIGH();
}
void FBL_CanSpi_SPI_Tx(uint8 data)
{
	HAL_SPI_Transmit(FBL_CANSPI_SPI_CAN, &data, 1, 3);
}
void FBL_CanSpi_SPI_TxBuffer(uint8 *buffer, uint8 length)
{
	HAL_SPI_Transmit(FBL_CANSPI_SPI_CAN, buffer, length, 3);
}
uint8 FBL_CanSpi_SPI_Rx(void)
{
	static uint8 FBL_retVal = 0;

	HAL_SPI_Receive(FBL_CANSPI_SPI_CAN, &FBL_retVal, 1, 3);

	return FBL_retVal;
}
void FBL_CanSpi_SPI_RxBuffer(uint8 *buffer, uint8 length)
{
	HAL_SPI_Receive(FBL_CANSPI_SPI_CAN, buffer, length, 3);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM5)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  /* USER CODE END Callback 1 */
}

uint8 FBL_CanSpi_MCP2515_ReadByte (uint8 address)
{
	static uint8 FBL_retVal = 0;
	FBL_CANSPI_MCP2515_CS_LOW();
	FBL_CanSpi_SPI_Tx(FBL_CANSPI_MCP2515_READ);
	FBL_CanSpi_SPI_Tx(address);
	FBL_retVal = FBL_CanSpi_SPI_Rx();
	FBL_CANSPI_MCP2515_CS_HIGH();
	return (FBL_retVal);
}
