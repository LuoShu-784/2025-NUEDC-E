/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    soft_uart.h
  * @brief   This file contains all the function prototypes for
  *          the soft_uart.c file
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
#ifndef __SOFT_UART_H__
#define __SOFT_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "stm32f4xx_hal_soft_uart.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SOFT_UART_HandleTypeDef hsuart;

/* USER CODE BEGIN Private defines */

/* 硬件配置宏定义 - 可在外部覆盖 */
#ifndef SOFTUART_TX_GPIO_Port
#define SOFTUART_TX_GPIO_Port           GPIOB
#endif

#ifndef SOFTUART_TX_Pin
#define SOFTUART_TX_Pin                 GPIO_PIN_14
#endif

#ifndef SOFTUART_RX_GPIO_Port
#define SOFTUART_RX_GPIO_Port           GPIOB
#endif

#ifndef SOFTUART_RX_Pin
#define SOFTUART_RX_Pin                 GPIO_PIN_15
#endif

#ifndef SOFTUART_TX_TIM
#define SOFTUART_TX_TIM                 htim1
#endif

#ifndef SOFTUART_RX_TIM
#define SOFTUART_RX_TIM                 htim3
#endif

#ifndef SOFTUART_RX_EXTI_IRQn
#define SOFTUART_RX_EXTI_IRQn           EXTI15_10_IRQn
#endif

#ifndef SOFTUART_BAUDRATE
#define SOFTUART_BAUDRATE               9600
#endif

#ifndef SOFTUART_RX_BUFFER_SIZE
#define SOFTUART_RX_BUFFER_SIZE         256
#endif

#ifndef SOFTUART_TX_BUFFER_SIZE
#define SOFTUART_TX_BUFFER_SIZE         512
#endif

/* USER CODE END Private defines */

void MX_SOFT_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_UART_H__ */
