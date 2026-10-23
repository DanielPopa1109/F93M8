#include "EcuM.h"
#include "Nvm.h"
#include "Dcm.h"
#include "Dem.h"
#include "rtc.h"
#include "gpio.h"
#include "cmsis_os2.h"
#include "I2cH.h"

__attribute__((section(".ncr"))) uint32 EcuM_ResetCounter[13];
__attribute__((section(".ncr"))) uint32 EcuM_ResetFlag[13];
__attribute__((section(".ncr"))) uint8 EcuM_ResetCounterFBL;
__attribute__((section(".ccmram"))) uint8 EcuM_State = 0;
__attribute__((section(".ccmram"))) uint32 EcuM_MainCounter = 0;
__attribute__((section(".ccmram"))) uint8 EcuM_ResetOccured = 0;
__attribute__((section(".ccmram"))) uint8 EcuM_StopModeActive = 0;
extern __attribute__((section(".ccmram"))) uint8 CanH_RequestBusSleep;
extern __attribute__((section(".ccmram"))) uint8 Nvm_WriteAllPending;
extern __attribute__((section(".ccmram"))) uint8 Nvm_ReadAllPending;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef hi2c2;
extern DCMI_HandleTypeDef hdcmi;
extern CAN_HandleTypeDef hcan1;
extern SRAM_HandleTypeDef hsram1;

void EcuM_MainFunction(void);
void EcuM_GoToSleep(void);
void EcuM_DeInitGpio(void);
void EcuM_PerformReset(uint8 param);
extern void Dem_SetDtc(uint8 IDPrimary, uint8 Status);

void EcuM_PerformReset(uint8 param)
{
	__disable_irq();
	if(0 != param)
	{
		EcuM_ResetCounter[param]++;
		EcuM_ResetFlag[param] = 1;
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
void EcuM_MainFunction(void)
{
	if(0 == EcuM_MainCounter)
	{
		for(uint8 i = 0; i < 13; i++)
		{
			if(0 != EcuM_ResetFlag[i])
			{
				Dem_SetDtc(DEM_CONTROLLER_INTERNALFAULT_ID, 1);
				EcuM_ResetFlag[i] = 0;
				EcuM_ResetOccured = 1u;
			}
			else Dem_SetDtc(DEM_CONTROLLER_INTERNALFAULT_ID, 0);
		}
	}
	else
	{
		/* Do nothing. */
	}
	if(0 == Nvm_ReadAllPending)
	{
		Nvm_ReadAllPending = 1;
	}
	else if(2 == Nvm_ReadAllPending)
	{
		if(2000 > EcuM_MainCounter) EcuM_State = 1;
		else
		{
			if(1 == CanH_RequestBusSleep)
			{
				EcuM_State = 2;
				htim3.Instance->CCR4 = 0;
				if(0 == Nvm_WriteAllPending && 2 != Nvm_WriteAllPending) Nvm_WriteAllPending = 1;
				else
				{
					/* Do nothing. */
				}
			}
			else EcuM_State = 1;
			if(2 == EcuM_State && 2 == Nvm_WriteAllPending && !Dcm_IsActive()) EcuM_State = 3;
			else
			{
				/* Do nothing. */
			}
			if(3 == EcuM_State) EcuM_GoToSleep();
			else
			{
				/* Do nothing. */
			}
		}
	}
	else
	{
		/* Do nothing. */
	}
	EcuM_MainCounter++;
}
void EcuM_DeInitGpio(void)
{
	HAL_GPIO_WritePin(BUZOUT_GPIO_Port, BUZOUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, REVERSECAMERA_RESET_Pin|REVERSECAMERA_POWERDOWN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_6);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_14);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
	HAL_GPIO_DeInit(BUZOUT_GPIO_Port, BUZOUT_Pin);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOD, REVERSECAMERA_RESET_Pin);
	HAL_GPIO_DeInit(GPIOD, REVERSECAMERA_POWERDOWN_Pin);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_1);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_4);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_6);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11);
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
}
void EcuM_GoToSleep(void)
{
	HAL_Delay(1);
	HAL_SuspendTick();
	HAL_DCMI_Stop(&hdcmi);
	HAL_TIM_PWM_DeInit(&htim3);
	HAL_TIM_Base_Stop(&htim1);
	HAL_I2C_DeInit(&hi2c2);
	HAL_DCMI_DeInit(&hdcmi);
	for(uint8 i = 0; i < 82; i++) HAL_NVIC_ClearPendingIRQ(i);
	EcuM_DeInitGpio();
	__HAL_RCC_DMA2_CLK_DISABLE();
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
	EcuM_StopModeActive = 1;
	__enable_irq();
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	EcuM_PerformReset(0);
}
