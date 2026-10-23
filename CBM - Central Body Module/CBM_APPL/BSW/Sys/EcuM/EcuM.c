/* START OF FILE */
/* INCLUDE START */
#include "EcuM.h"
#include "tim.h"
#include "adc.h"
#include "spi.h"
#include "Ain.h"
#include "TimH.h"
#include "CanSpi.h"
#include "UartH.h"
#include "Nvm.h"
#include "Dem.h"
#include "Dcm.h"
#include "usart.h"
#include "gpio.h"
#include "cmsis_os2.h"
/* INCLUDE END */
/* VARIABLES START */
uint8 EcuM_WakeupReason = 0;
EcuMStates_t EcuM_State = STARTUP;
static uint32 EcuM_MainCounter = 0;
static uint32 EcuM_PostrunTimer = 0;
uint8 EcuM_LastResetReason __attribute__((section(".ncr")));
uint32 EcuM_ResetCounter __attribute__((section(".ncr")));
uint32 EcuM_ResetCounterFBL __attribute__((section(".ncr")));
extern uint32 Dcm_AliveCounter __attribute__((section(".ncr")));
extern TIM_HandleTypeDef        htim5;
extern uint32 StatusList_OutputValue[5];
extern CANSPI_uCAN_MSG CanSpi_RxFrame_Buffer0;
extern CANSPI_uCAN_MSG CanSpi_RxFrame_Buffer1;
/* VARIABLES END */
/* FUNCTIONS START */
void EcuM_MainFunction(void);
void EcuM_PerformReset(EcuMReset_t param);
void EcuM_ProcessTimerInterrupt(void);
static void EcuM_ProcessEcuState(void);
static void EcuM_GoSleep(void);
static void EcuM_ProcessFaultState(void);
extern uint32 CanSpi_MessagesInBuffer(void);
/* FUNCTIONS END */
/* FUNCTIONS START */
void EcuM_MainFunction(void)
{
	EcuM_ProcessEcuState();
	EcuM_ProcessFaultState();
	EcuM_MainCounter++;
}
void EcuM_PerformReset(EcuMReset_t param)
{
	__disable_irq();
	if(param)
	{
		Dcm_AliveCounter = 0;
		EcuM_LastResetReason = param;
		EcuM_ResetCounter++;
		EcuM_ResetCounterFBL++;
		RCC->CSR |= RCC_CSR_RMVF;
	}
	else
	{
		/* Do nothing. */
	}
	RCC->CSR |= RCC_CSR_RMVF;
	__NVIC_SystemReset();
}
static void EcuM_ProcessFaultState(void)
{
	static uint8 firstRun = 0;
	RCC->CSR |= RCC_CSR_RMVF;
	if(firstRun == 0)
	{
		firstRun = 1;

		if(EcuM_LastResetReason)
		{
			Dem_SaveDtc(0x08, 1);
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
}
void EcuM_ProcessEcuState(void)
{
	if(EcuM_State == PRESLEEP) EcuM_GoSleep();
	else
	{
		/* Do nothing. */
	}
	if(EcuM_State == STARTUP)
	{
		EcuM_State = RUN;
		EcuM_PostrunTimer = 0;
	}
	else
	{
		/* Do nothing. */
	}
	if(((CanSpi_Communication_Status == FULL_COMMUNICATION)
			|| (Ain_Mux[0] >= 1200)
			|| (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1))
			|| (StatusList_OutputValue[4] == 3))
	{
		EcuM_State = RUN;
		EcuM_PostrunTimer = 0;
	}
	else
	{
		/* Do nothing. */
	}
	if((CanSpi_Communication_Status == NO_COMMUNICATION)
			&& (Ain_Mux[0] < 1200) && (EcuM_State == RUN)
			&& (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0)
			&& StatusList_OutputValue[4] != 3) EcuM_State = POSTRUN;
	else
	{
		/* Do nothing. */
	}
	if(EcuM_State == POSTRUN)
	{
		if(EcuM_PostrunTimer == 0) EcuM_PostrunTimer = EcuM_MainCounter;
		else
		{
			/* Do nothing. */
		}
		if(((EcuM_MainCounter - EcuM_PostrunTimer) > 2000) && (EcuM_PostrunTimer != 0) && NvM_IsIdle() && NvM_LastWriteResult==0U && !Dcm_IsActive()) EcuM_State = PRESLEEP;
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

void EcuM_ProcessTimerInterrupt(void)
{
	static uint8 debouncePin = 0;
	static uint32 messagesInBuffer = 0;

	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1)
	{
		debouncePin++;
		if(debouncePin == 3)
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
		debouncePin = 0;
	}
	messagesInBuffer = CanSpi_MessagesInBuffer();
	if(messagesInBuffer != 0)
	{
		if(CanSpi_RxFrame_Buffer0.frame.id == 0)
		{
			if(CanSpi_Receive(&CanSpi_RxFrame_Buffer0) != 0)
			{
				if(0x99 == CanSpi_RxFrame_Buffer0.frame.id)
				{
					EcuM_PerformReset(0);
				}
				else
				{
					/* Do nothing. */
				}
				/* StatusBodyControl */
				if(0x98 == CanSpi_RxFrame_Buffer0.frame.id)
				{
					EcuM_PerformReset(0);
				}
				else
				{
					/* Do nothing. */
				}
				/* VehicleState Frame */
				if(CanSpi_RxFrame_Buffer0.frame.id == 0x097)
				{
					EcuM_PerformReset(0);
				}
				else
				{
					/* Do nothing. */
				}
				/* Networkmanagement3 Frame */
				if(CanSpi_RxFrame_Buffer0.frame.id == 0x3FF && CanSpi_RxFrame_Buffer0.frame.idType == 1U && CanSpi_RxFrame_Buffer0.frame.dlc >= 1U)
				{
					if(CanSpi_RxFrame_Buffer0.frame.data0 == 0xFF)
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
				/* Diagnostic request Frame */
				if(CanSpi_RxFrame_Buffer0.frame.id == 0x710 &&
				   CanSpi_RxFrame_Buffer0.frame.dlc >= 2U &&
				   CanSpi_RxFrame_Buffer0.frame.data0 == 0x54U) EcuM_PerformReset(0);
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
				/* StatusBodyControl2 */
				if(0x99 == CanSpi_RxFrame_Buffer1.frame.id)
				{
					EcuM_PerformReset(0);
				}
				else
				{
					/* Do nothing. */
				}
				/* StatusBodyControl */
				if(0x98 == CanSpi_RxFrame_Buffer1.frame.id)
				{
					EcuM_PerformReset(0);
				}
				else
				{
					/* Do nothing. */
				}
				/* VehicleState Frame */
				if(CanSpi_RxFrame_Buffer1.frame.id == 0x097)
				{
					EcuM_PerformReset(0);
				}
				else
				{
					/* Do nothing. */
				}
				/* Networkmanagement3 Frame */
				if(CanSpi_RxFrame_Buffer1.frame.id == 0x3FF && CanSpi_RxFrame_Buffer1.frame.idType == 1U && CanSpi_RxFrame_Buffer1.frame.dlc >= 1U)
				{
					if(CanSpi_RxFrame_Buffer1.frame.data0 == 0xFF)
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
				/* Diagnostic request Frame */
				if(CanSpi_RxFrame_Buffer1.frame.id == 0x710 &&
				   CanSpi_RxFrame_Buffer1.frame.dlc >= 2U &&
				   CanSpi_RxFrame_Buffer1.frame.data0 == 0x54U) EcuM_PerformReset(0);
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

	HAL_PWR_EnableSleepOnExit();
}

static void EcuM_GoSleep(void)
{
	__disable_irq();
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
	//	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
	//	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
	//	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3);
	//	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_14);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOH, GPIO_PIN_0);
	HAL_GPIO_DeInit(GPIOH, GPIO_PIN_1);
	//	__HAL_RCC_GPIOB_CLK_DISABLE();
	//	__HAL_RCC_GPIOC_CLK_DISABLE();
	//	__HAL_RCC_GPIOH_CLK_DISABLE();
	HAL_TIM_PWM_DeInit(&htim2);
	HAL_TIM_PWM_DeInit(&htim3);
	HAL_TIM_Base_DeInit(&htim5);
	HAL_UART_DeInit(&huart1);
	//	CanSpi_Sleep();
	//	HAL_SPI_DeInit(&hspi1);
	HAL_SuspendTick();
	for(uint8 i = 0; i < 82; i++) HAL_NVIC_ClearPendingIRQ(i);
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
	EcuM_LastResetReason = 0;
	EcuM_ResetCounter = 0;
	HAL_TIM_Base_Start_IT(&htim4);
	__enable_irq();
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	EcuM_PerformReset(0);
}

/* FUNCTIONS END */
/* STOP OF FILE */
