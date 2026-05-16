/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_hal_soft_uart.h
  * @brief   Header file of Software UART HAL module.
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

#ifndef __STM32F4xx_HAL_SOFT_UART_H
#define __STM32F4xx_HAL_SOFT_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup SOFT_UART
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup SOFT_UART_Exported_Types SOFT_UART Exported Types
  * @{
  */

/**
  * @brief SOFT_UART State structures definition
  */
typedef enum
{
  HAL_SOFT_UART_STATE_RESET             = 0x00U,
  HAL_SOFT_UART_STATE_READY             = 0x01U,
  HAL_SOFT_UART_STATE_BUSY              = 0x02U,
  HAL_SOFT_UART_STATE_BUSY_TX           = 0x12U,
  HAL_SOFT_UART_STATE_BUSY_RX           = 0x22U,
  HAL_SOFT_UART_STATE_BUSY_TX_RX        = 0x32U,
  HAL_SOFT_UART_STATE_ERROR             = 0x04U
} HAL_SOFT_UART_StateTypeDef;

/**
  * @brief SOFT_UART RX State structures definition
  */
typedef enum
{
  SOFT_UART_RX_STATE_IDLE               = 0x00U,
  SOFT_UART_RX_STATE_DATA               = 0x01U,
  SOFT_UART_RX_STATE_STOP               = 0x02U
} SOFT_UART_RxStateTypeDef;

/**
  * @brief SOFT_UART Init Structure definition
  */
typedef struct
{
  uint32_t BaudRate;
  uint16_t RxBufferSize;
  uint16_t TxBufferSize;
} SOFT_UART_InitTypeDef;

/**
  * @brief SOFT_UART Hardware Configuration Structure definition
  */
typedef struct
{
  GPIO_TypeDef              *TxPort;
  uint16_t                  TxPin;
  GPIO_TypeDef              *RxPort;
  uint16_t                  RxPin;
  TIM_HandleTypeDef         *TxTim;
  TIM_HandleTypeDef         *RxTim;
  IRQn_Type                 RxExtiIRQn;
} SOFT_UART_HwConfigTypeDef;

/**
  * @brief SOFT_UART handle Structure definition
  */
typedef struct __SOFT_UART_HandleTypeDef
{
  const SOFT_UART_HwConfigTypeDef   *HwConfig;
  SOFT_UART_InitTypeDef             Init;
  uint8_t                           *pTxBuffPtr;
  uint8_t                           *pRxBuffPtr;
  __IO uint16_t                     TxXferSize;
  __IO uint16_t                     TxXferCount;
  __IO uint16_t                     RxXferSize;
  __IO uint16_t                     RxXferCount;
  __IO HAL_SOFT_UART_StateTypeDef   gState;
  __IO HAL_SOFT_UART_StateTypeDef   RxState;
  __IO uint32_t                     ErrorCode;

  /* Private fields */
  __IO SOFT_UART_RxStateTypeDef     RxStateMachine;
  __IO uint8_t                      RxData;
  __IO uint8_t                      RxBitCount;
  __IO uint16_t                     RxBitTicks;
  __IO uint16_t                     RxHead;
  __IO uint16_t                     RxTail;
  __IO uint16_t                     TxHead;
  __IO uint16_t                     TxTail;
  __IO uint8_t                      TxBusy;
  __IO uint16_t                     TxFrame;
  __IO uint8_t                      TxBitCount;
  __IO uint16_t                     TxBitTicks;
} SOFT_UART_HandleTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SOFT_UART_Exported_Constants SOFT_UART Exported Constants
  * @{
  */

/** @defgroup SOFT_UART_Error_Code SOFT_UART Error Code
  * @{
  */
#define HAL_SOFT_UART_ERROR_NONE              0x00000000U
#define HAL_SOFT_UART_ERROR_PE                0x00000001U
#define HAL_SOFT_UART_ERROR_FE                0x00000002U
#define HAL_SOFT_UART_ERROR_ORE               0x00000008U
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup SOFT_UART_Exported_Macros SOFT_UART Exported Macros
  * @{
  */

/** @brief Reset SOFT_UART handle state
  * @param  __HANDLE__ specifies the SOFT_UART Handle.
  * @retval None
  */
#define __HAL_SOFT_UART_RESET_HANDLE_STATE(__HANDLE__)  ((__HANDLE__)->gState = HAL_SOFT_UART_STATE_RESET)

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SOFT_UART_Exported_Functions
  * @{
  */

/** @addtogroup SOFT_UART_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_SOFT_UART_Init(SOFT_UART_HandleTypeDef *hsuart);
HAL_StatusTypeDef HAL_SOFT_UART_DeInit(SOFT_UART_HandleTypeDef *hsuart);
/**
  * @}
  */

/** @addtogroup SOFT_UART_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_SOFT_UART_Transmit(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SOFT_UART_Receive(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
/**
  * @}
  */

/** @addtogroup SOFT_UART_Exported_Functions_Group3
  * @{
  */
HAL_StatusTypeDef HAL_SOFT_UART_Transmit_IT(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SOFT_UART_Receive_IT(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size);
void HAL_SOFT_UART_TxTimCallback(SOFT_UART_HandleTypeDef *hsuart);
void HAL_SOFT_UART_RxTimCallback(SOFT_UART_HandleTypeDef *hsuart);
void HAL_SOFT_UART_RxExtiCallback(SOFT_UART_HandleTypeDef *hsuart, uint16_t GPIO_Pin);
/**
  * @}
  */

/** @addtogroup SOFT_UART_Exported_Functions_Group4
  * @{
  */
HAL_SOFT_UART_StateTypeDef HAL_SOFT_UART_GetState(SOFT_UART_HandleTypeDef *hsuart);
uint32_t HAL_SOFT_UART_GetError(SOFT_UART_HandleTypeDef *hsuart);
uint16_t HAL_SOFT_UART_GetRxAvailable(SOFT_UART_HandleTypeDef *hsuart);
uint16_t HAL_SOFT_UART_GetTxAvailable(SOFT_UART_HandleTypeDef *hsuart);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_SOFT_UART_H */
