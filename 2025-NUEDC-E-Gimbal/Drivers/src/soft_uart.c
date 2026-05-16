/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    soft_uart.c
  * @brief   This file provides code for the configuration
  *          of the Software UART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "soft_uart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SOFT_UART_HandleTypeDef hsuart;

/* SOFT_UART init function */

void MX_SOFT_UART_Init(void)
{

  /* USER CODE BEGIN SOFT_UART_Init 0 */

  /* USER CODE END SOFT_UART_Init 0 */

  /* USER CODE BEGIN SOFT_UART_Init 1 */

  /* USER CODE END SOFT_UART_Init 1 */

  /* Hardware configuration */
  static const SOFT_UART_HwConfigTypeDef hw_config = {
    .TxPort = SOFTUART_TX_GPIO_Port,
    .TxPin = SOFTUART_TX_Pin,
    .RxPort = SOFTUART_RX_GPIO_Port,
    .RxPin = SOFTUART_RX_Pin,
    .TxTim = &htim1,
    .RxTim = &htim3,
    .RxExtiIRQn = EXTI15_10_IRQn
  };

  hsuart.HwConfig = &hw_config;
  hsuart.Init.BaudRate = 9600;
  hsuart.Init.RxBufferSize = 128;
  hsuart.Init.TxBufferSize = 256;

  if (HAL_SOFT_UART_Init(&hsuart) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SOFT_UART_Init 2 */

  /* USER CODE END SOFT_UART_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
