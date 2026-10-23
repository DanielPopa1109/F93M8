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
#include "can.h"
static uint32_t txMailbox;
int Fbl_BoardCanTransmit(const uint8_t *p) {
 CAN_TxHeaderTypeDef h={0};uint32_t irq=__get_PRIMASK();
 if(txHardwarePending)return 1;
 h.StdId=0x710;h.IDE=CAN_ID_STD;h.RTR=CAN_RTR_DATA;h.DLC=8;
 __disable_irq();
 if(HAL_CAN_AddTxMessage(&hcan1,&h,(uint8_t *)p,&txMailbox)!=HAL_OK) {if(!irq)__enable_irq();return 1;}
 txHardwarePending=1;txHardwareAge=0;if(!irq)__enable_irq();return 0;
}
static void txDone(uint32_t box,uint8_t ok) { if(txHardwarePending && txMailbox==box) {txHardwarePending=0;Can_TxConfirmation(ok);} }
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *h) {if(h==&hcan1)txDone(CAN_TX_MAILBOX0,1);}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *h) {if(h==&hcan1)txDone(CAN_TX_MAILBOX1,1);}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *h) {if(h==&hcan1)txDone(CAN_TX_MAILBOX2,1);}
void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *h) {if(h==&hcan1)txDone(CAN_TX_MAILBOX0,0);}
void HAL_CAN_TxMailbox1AbortCallback(CAN_HandleTypeDef *h) {if(h==&hcan1)txDone(CAN_TX_MAILBOX1,0);}
void HAL_CAN_TxMailbox2AbortCallback(CAN_HandleTypeDef *h) {if(h==&hcan1)txDone(CAN_TX_MAILBOX2,0);}
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *h) {if(h==&hcan1) {txHardwarePending=0;Can_BusOff();}}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *h) {
 CAN_RxHeaderTypeDef header;Can_FrameType f;
 if(h!=&hcan1 || HAL_CAN_GetRxMessage(h,CAN_RX_FIFO0,&header,f.data)!=HAL_OK || header.IDE!=CAN_ID_STD ||
  header.RTR!=CAN_RTR_DATA || header.StdId!=0x710 || header.DLC>8)return;
 f.id=0x710;f.dlc=(uint8_t)header.DLC;Can_RxIndication(&f);
}
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *h) {HAL_CAN_RxFifo0MsgPendingCallback(h);}
static void boardPoll(void) {
 if(txHardwarePending && ++txHardwareAge>=200) {HAL_CAN_AbortTxRequest(&hcan1,txMailbox);txHardwarePending=0;Can_TxConfirmation(0);}
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
 e.TypeErase=FLASH_TYPEERASE_SECTORS;e.VoltageRange=FLASH_VOLTAGE_RANGE_3;e.Sector=5U;e.NbSectors=2U;
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
 MX_CAN1_Init();
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
	RCC_OscInitStruct.PLL.PLLN = 168;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}
static void MX_NVIC_Init(void)
{
	/* PVD_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(PVD_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(PVD_IRQn);
	/* FLASH_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(FLASH_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(FLASH_IRQn);
	/* FPU_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(FPU_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(FPU_IRQn);
	/* CAN1_RX0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	/* CAN1_TX_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	/* RCC_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(RCC_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(RCC_IRQn);
}
