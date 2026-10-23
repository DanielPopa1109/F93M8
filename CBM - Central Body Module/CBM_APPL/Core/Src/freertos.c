/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ActCtrl.h"
#include "CanSpi.h"
#include "../../BSW/Com/Can/Can.h"
#include "../../BSW/Com/CanIf/CanIf.h"
#include "../../BSW/Com/CanTp/CanTp.h"
#include "../../BSW/Com/PduR/PduR.h"
#include "../../BSW/Com/Com/Com.h"
#include "../../BSW/Com/CanNm/CanNm.h"
#include "../../BSW/Com/CanSM/CanSM.h"
#include "../../BSW/Com/ComM/ComM.h"
#include "TimH.h"
#include "Ain.h"
#include "Nvm.h"
#include "EcuM.h"
#include "Dem.h"
#include "Dcm.h"
#include "InputCtrl.h"
#include "UartH.h"
#include "stdlib.h"
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8 Alarm5ms_QM_APPL = 0;
uint8 Alarm5ms_QM_BSW = 0;
uint8 Alarm5ms_QM_DIAG = 0;
uint8 Alarm10ms_QM_APPL = 0;
uint8 Alarm10ms_QM_BSW = 0;
uint8 Alarm100ms_QM_NVM = 0;
long long OS_Counter = 0;
long long IDLE_Counter = 0;
long long QM_APPL_Counter = 0;
long long QM_BSW_Counter = 0;
long long QM_NVM_Counter = 0;
long long QM_DIAG_Counter = 0;
/* USER CODE END Variables */
/* Definitions for QM_APPL_OS_TASK */
osThreadId_t QM_APPL_OS_TASKHandle;
const osThreadAttr_t QM_APPL_OS_TASK_attributes = {
  .name = "QM_APPL_OS_TASK",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for QM_BSW_OS_TASK */
osThreadId_t QM_BSW_OS_TASKHandle;
const osThreadAttr_t QM_BSW_OS_TASK_attributes = {
  .name = "QM_BSW_OS_TASK",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime1,
};
/* Definitions for QM_NVM_OS_TASK */
osThreadId_t QM_NVM_OS_TASKHandle;
const osThreadAttr_t QM_NVM_OS_TASK_attributes = {
  .name = "QM_NVM_OS_TASK",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh7,
};
/* Definitions for QM_DIAG_OS_TASK */
osThreadId_t QM_DIAG_OS_TASKHandle;
const osThreadAttr_t QM_DIAG_OS_TASK_attributes = {
  .name = "QM_DIAG_OS_TASK",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime3,
};
/* Definitions for Alarm5ms */
osTimerId_t Alarm5msHandle;
const osTimerAttr_t Alarm5ms_attributes = {
  .name = "Alarm5ms"
};
/* Definitions for Alarm10ms */
osTimerId_t Alarm10msHandle;
const osTimerAttr_t Alarm10ms_attributes = {
  .name = "Alarm10ms"
};
/* Definitions for Alarm100ms */
osTimerId_t Alarm100msHandle;
const osTimerAttr_t Alarm100ms_attributes = {
  .name = "Alarm100ms"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void QM_APPL(void *argument);
void QM_BSW(void *argument);
void QM_NVM(void *argument);
void QM_DIAG(void *argument);
void Alarm5ms_Callback(void *argument);
void Alarm10ms_Callback(void *argument);
void Alarm100ms_Callback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
	IDLE_Counter++;
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
void vApplicationTickHook( void )
{
	OS_Counter++;
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	EcuM_PerformReset(STACKOVERFLOW_RESET);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
	EcuM_PerformReset(MALLOC_RESET);
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  Dem_PreInit();
  Dcm_Init();
  Can_Init();
  CanIf_Init();
  CanTp_Init();
  PduR_Init();
  Com_Init();
  CanNm_Init();
  CanSM_Init();
  ComM_Init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of Alarm5ms */
  Alarm5msHandle = osTimerNew(Alarm5ms_Callback, osTimerPeriodic, NULL, &Alarm5ms_attributes);

  /* creation of Alarm10ms */
  Alarm10msHandle = osTimerNew(Alarm10ms_Callback, osTimerPeriodic, NULL, &Alarm10ms_attributes);

  /* creation of Alarm100ms */
  Alarm100msHandle = osTimerNew(Alarm100ms_Callback, osTimerPeriodic, NULL, &Alarm100ms_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
	osTimerStart(Alarm5msHandle, 5);
	osTimerStart(Alarm10msHandle, 10);
	osTimerStart(Alarm100msHandle, 100);
	NvM_MainFunction();
    for(uint8 initStep=0;initStep<8U && !NvM_IsIdle();++initStep) NvM_MainFunction();
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of QM_APPL_OS_TASK */
  QM_APPL_OS_TASKHandle = osThreadNew(QM_APPL, NULL, &QM_APPL_OS_TASK_attributes);

  /* creation of QM_BSW_OS_TASK */
  QM_BSW_OS_TASKHandle = osThreadNew(QM_BSW, NULL, &QM_BSW_OS_TASK_attributes);

  /* creation of QM_NVM_OS_TASK */
  QM_NVM_OS_TASKHandle = osThreadNew(QM_NVM, NULL, &QM_NVM_OS_TASK_attributes);

  /* creation of QM_DIAG_OS_TASK */
  QM_DIAG_OS_TASKHandle = osThreadNew(QM_DIAG, NULL, &QM_DIAG_OS_TASK_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_QM_APPL */
/* USER CODE END Header_QM_APPL */
void QM_APPL(void *argument)
{
  /* USER CODE BEGIN QM_APPL */
	for(;;)
	{
		if(Alarm10ms_QM_APPL == 1 && EcuM_State == RUN)
		{
			ActCtrl_MainFunction();
			InputCtrl_MainFunction();
			Alarm10ms_QM_APPL = 0;
		}
		else
		{
			/* Do nothing */
		}
		QM_APPL_Counter++;
		vTaskSuspend(NULL);
	}
  /* USER CODE END QM_APPL */
}

/* USER CODE BEGIN Header_QM_BSW */
/* USER CODE END Header_QM_BSW */
void QM_BSW(void *argument)
{
  /* USER CODE BEGIN QM_BSW */
	for(;;)
	{
		if(Alarm5ms_QM_BSW == 1)
		{
			EcuM_MainFunction();
			Com_MainFunction();
			CanNm_MainFunction();
			ComM_MainFunction();
			CanSM_MainFunction();
            /* DEM persistence/callback progression runs in the diagnostic owner. */
			Ain_MainFunction();
			TimH_MainFunction();
			UartH_MainFunction();
			Alarm5ms_QM_BSW = 0;
		}
		else
		{
			/* Do nothing. */
		}
		QM_BSW_Counter++;
		vTaskSuspend(NULL);
	}
  /* USER CODE END QM_BSW */
}

/* USER CODE BEGIN Header_QM_NVM */
/* USER CODE END Header_QM_NVM */
void QM_NVM(void *argument)
{
  /* USER CODE BEGIN QM_NVM */
	for(;;)
	{
		if(Alarm100ms_QM_NVM == 1)
		{
			/* NvM is serviced by the single 5 ms diagnostic owner. */
			Alarm100ms_QM_NVM = 0;
		}
		else
		{
			/* Do nothing. */
		}
		QM_NVM_Counter++;
		vTaskSuspend(NULL);
	}
  /* USER CODE END QM_NVM */
}

/* USER CODE BEGIN Header_QM_DIAG */
/* USER CODE END Header_QM_DIAG */
void QM_DIAG(void *argument)
{
  /* USER CODE BEGIN QM_DIAG */
	for(;;)
	{
		if(Alarm5ms_QM_DIAG == 1)
		{
			CanSpi_MainFunction();
            Can_MainFunction();
            NvM_MainFunction();
            Dem_MainFunction();
			CanTp_MainFunction();
			Dcm_MainFunction();
			Alarm5ms_QM_DIAG = 0;
		}
		else
		{
			/* Do nothing. */
		}
		QM_DIAG_Counter++;
		vTaskSuspend(NULL);
	}
  /* USER CODE END QM_DIAG */
}

/* Alarm5ms_Callback function */
void Alarm5ms_Callback(void *argument)
{
  /* USER CODE BEGIN Alarm5ms_Callback */
	Alarm5ms_QM_APPL = 1;
	Alarm5ms_QM_BSW = 1;
	Alarm5ms_QM_DIAG = 1;
	vTaskResume(QM_BSW_OS_TASKHandle);
	vTaskResume(QM_DIAG_OS_TASKHandle);
  /* USER CODE END Alarm5ms_Callback */
}

/* Alarm10ms_Callback function */
void Alarm10ms_Callback(void *argument)
{
  /* USER CODE BEGIN Alarm10ms_Callback */
	Alarm10ms_QM_APPL = 1;
	Alarm10ms_QM_BSW = 1;
	vTaskResume(QM_APPL_OS_TASKHandle);
  /* USER CODE END Alarm10ms_Callback */
}

/* Alarm100ms_Callback function */
void Alarm100ms_Callback(void *argument)
{
  /* USER CODE BEGIN Alarm100ms_Callback */
	Alarm100ms_QM_NVM = 1;
	vTaskResume(QM_NVM_OS_TASKHandle);
  /* USER CODE END Alarm100ms_Callback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */

