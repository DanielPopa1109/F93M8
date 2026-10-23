/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */
  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */
  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_16TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = ENABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = ENABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = ENABLE;
  hcan1.Init.TransmitFifoPriority = ENABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
	CAN_FilterTypeDef filterConfig;
	filterConfig.SlaveStartFilterBank = 14;
	filterConfig.FilterBank = 6;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x99 << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 7;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x97 << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 8;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x98 << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 9;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x10B << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 10;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x10F << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 11;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x3FF << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 12;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x710 << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0006; /* standard data frames only */
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
	filterConfig.FilterBank = 13;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterIdHigh = (0x201 << 5) & 0xFFFF;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = (0x7FF << 5);
	filterConfig.FilterMaskIdLow = 0x0000;
	HAL_CAN_ConfigFilter(&hcan1, &filterConfig);
  /* USER CODE END CAN1_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */
  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */
  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */
  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */
  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* USER CODE END 1 */
