/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_hal_soft_uart.c
  * @brief   Software UART HAL module driver.
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
#include "stm32f4xx_hal_soft_uart.h"
#include <stdlib.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define UART_FRAME_BITS         10U
#define UART_DATA_BITS          8U

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SOFT_UART_TxPinSet(SOFT_UART_HandleTypeDef *hsuart);
static void SOFT_UART_TxPinReset(SOFT_UART_HandleTypeDef *hsuart);
static uint8_t SOFT_UART_RxPinRead(SOFT_UART_HandleTypeDef *hsuart);
static uint32_t SOFT_UART_GetTimerClock(TIM_HandleTypeDef *htim);
static uint16_t SOFT_UART_CalculateBitTicks(TIM_HandleTypeDef *htim, uint32_t BaudRate);
static void SOFT_UART_TimerStart(TIM_HandleTypeDef *htim, uint16_t Ticks);
static void SOFT_UART_TimerStop(TIM_HandleTypeDef *htim);
static void SOFT_UART_RxBufferPush(SOFT_UART_HandleTypeDef *hsuart, uint8_t Data);
static HAL_StatusTypeDef SOFT_UART_TxBufferPush(SOFT_UART_HandleTypeDef *hsuart, uint8_t Data);
static HAL_StatusTypeDef SOFT_UART_TxBufferPop(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData);
static void SOFT_UART_TxSendNextBit(SOFT_UART_HandleTypeDef *hsuart);
static void SOFT_UART_TxStartFrame(SOFT_UART_HandleTypeDef *hsuart, uint8_t Data);
static void SOFT_UART_TxStartIfIdle(SOFT_UART_HandleTypeDef *hsuart);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Set TX pin to high level
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval None
  */
static void SOFT_UART_TxPinSet(SOFT_UART_HandleTypeDef *hsuart)
{
  hsuart->HwConfig->TxPort->BSRR = hsuart->HwConfig->TxPin;
}

/**
  * @brief  Set TX pin to low level
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval None
  */
static void SOFT_UART_TxPinReset(SOFT_UART_HandleTypeDef *hsuart)
{
  hsuart->HwConfig->TxPort->BSRR = (uint32_t)hsuart->HwConfig->TxPin << 16U;
}

/**
  * @brief  Read RX pin level
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval Pin level (0 or 1)
  */
static uint8_t SOFT_UART_RxPinRead(SOFT_UART_HandleTypeDef *hsuart)
{
  return (hsuart->HwConfig->RxPort->IDR & hsuart->HwConfig->RxPin) != 0U ? 1U : 0U;
}

/**
  * @brief  Get timer clock frequency
  * @param  htim pointer to a TIM_HandleTypeDef structure
  * @retval Timer clock frequency in Hz
  */
static uint32_t SOFT_UART_GetTimerClock(TIM_HandleTypeDef *htim)
{
  uint32_t clock;

  if (htim->Instance == TIM1)
  {
    clock = HAL_RCC_GetPCLK2Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0U)
    {
      clock *= 2U;
    }
  }
  else
  {
    clock = HAL_RCC_GetPCLK1Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0U)
    {
      clock *= 2U;
    }
  }

  return clock;
}

/**
  * @brief  Calculate timer ticks for one bit period
  * @param  htim pointer to a TIM_HandleTypeDef structure
  * @param  BaudRate UART baud rate
  * @retval Timer ticks per bit
  */
static uint16_t SOFT_UART_CalculateBitTicks(TIM_HandleTypeDef *htim, uint32_t BaudRate)
{
  uint32_t tick_hz = SOFT_UART_GetTimerClock(htim) / (htim->Init.Prescaler + 1U);
  uint32_t ticks = (tick_hz + BaudRate / 2U) / BaudRate;

  if (ticks < 2U)
  {
    ticks = 2U;
  }
  if (ticks > 0xFFFFU)
  {
    ticks = 0xFFFFU;
  }

  return (uint16_t)ticks;
}

/**
  * @brief  Start timer with specified ticks
  * @param  htim pointer to a TIM_HandleTypeDef structure
  * @param  Ticks timer auto-reload value
  * @retval None
  */
static void SOFT_UART_TimerStart(TIM_HandleTypeDef *htim, uint16_t Ticks)
{
  __HAL_TIM_DISABLE(htim);
  __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
  __HAL_TIM_SET_AUTORELOAD(htim, Ticks - 1U);
  __HAL_TIM_SET_COUNTER(htim, 0U);
  __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE(htim);
}

/**
  * @brief  Stop timer
  * @param  htim pointer to a TIM_HandleTypeDef structure
  * @retval None
  */
static void SOFT_UART_TimerStop(TIM_HandleTypeDef *htim)
{
  __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
  __HAL_TIM_DISABLE(htim);
  __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
}

/**
  * @brief  Push data to RX buffer
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  Data data byte to push
  * @retval None
  */
static void SOFT_UART_RxBufferPush(SOFT_UART_HandleTypeDef *hsuart, uint8_t Data)
{
  uint16_t next_head = (hsuart->RxHead + 1U) % hsuart->Init.RxBufferSize;

  if (next_head != hsuart->RxTail)
  {
    hsuart->pRxBuffPtr[hsuart->RxHead] = Data;
    hsuart->RxHead = next_head;
  }
}

/**
  * @brief  Push data to TX buffer
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  Data data byte to push
  * @retval HAL status
  */
static HAL_StatusTypeDef SOFT_UART_TxBufferPush(SOFT_UART_HandleTypeDef *hsuart, uint8_t Data)
{
  uint16_t next_head = (hsuart->TxHead + 1U) % hsuart->Init.TxBufferSize;

  if (next_head == hsuart->TxTail)
  {
    return HAL_ERROR;
  }

  hsuart->pTxBuffPtr[hsuart->TxHead] = Data;
  hsuart->TxHead = next_head;
  return HAL_OK;
}

/**
  * @brief  Pop data from TX buffer
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  pData pointer to data byte
  * @retval HAL status
  */
static HAL_StatusTypeDef SOFT_UART_TxBufferPop(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData)
{
  if (hsuart->TxHead == hsuart->TxTail)
  {
    return HAL_ERROR;
  }

  *pData = hsuart->pTxBuffPtr[hsuart->TxTail];
  hsuart->TxTail = (hsuart->TxTail + 1U) % hsuart->Init.TxBufferSize;
  return HAL_OK;
}

/**
  * @brief  Send next bit in TX frame
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval None
  */
static void SOFT_UART_TxSendNextBit(SOFT_UART_HandleTypeDef *hsuart)
{
  if ((hsuart->TxFrame & (1U << hsuart->TxBitCount)) != 0U)
  {
    SOFT_UART_TxPinSet(hsuart);
  }
  else
  {
    SOFT_UART_TxPinReset(hsuart);
  }
  hsuart->TxBitCount++;
}

/**
  * @brief  Start transmitting a frame
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  Data data byte to transmit
  * @retval None
  */
static void SOFT_UART_TxStartFrame(SOFT_UART_HandleTypeDef *hsuart, uint8_t Data)
{
  hsuart->TxFrame = ((uint16_t)1U << 9U) | ((uint16_t)Data << 1U);
  hsuart->TxBitCount = 0;
  hsuart->TxBusy = 1U;

  SOFT_UART_TxSendNextBit(hsuart);
  SOFT_UART_TimerStart(hsuart->HwConfig->TxTim, hsuart->TxBitTicks);
}

/**
  * @brief  Start transmission if idle
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval None
  */
static void SOFT_UART_TxStartIfIdle(SOFT_UART_HandleTypeDef *hsuart)
{
  uint8_t data;

  if (hsuart->TxBusy == 0U && SOFT_UART_TxBufferPop(hsuart, &data) == HAL_OK)
  {
    SOFT_UART_TxStartFrame(hsuart, data);
  }
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initializes the SOFT_UART according to the specified parameters
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SOFT_UART_Init(SOFT_UART_HandleTypeDef *hsuart)
{
  if (hsuart == NULL)
  {
    return HAL_ERROR;
  }

  if (hsuart->gState == HAL_SOFT_UART_STATE_RESET)
  {
    hsuart->pRxBuffPtr = (uint8_t *)malloc(hsuart->Init.RxBufferSize);
    if (hsuart->pRxBuffPtr == NULL)
    {
      return HAL_ERROR;
    }

    hsuart->pTxBuffPtr = (uint8_t *)malloc(hsuart->Init.TxBufferSize);
    if (hsuart->pTxBuffPtr == NULL)
    {
      free(hsuart->pRxBuffPtr);
      return HAL_ERROR;
    }
  }

  hsuart->gState = HAL_SOFT_UART_STATE_BUSY;

  SOFT_UART_TxPinSet(hsuart);

  hsuart->RxStateMachine = SOFT_UART_RX_STATE_IDLE;
  hsuart->RxHead = 0;
  hsuart->RxTail = 0;
  hsuart->TxHead = 0;
  hsuart->TxTail = 0;
  hsuart->TxBusy = 0;
  hsuart->ErrorCode = HAL_SOFT_UART_ERROR_NONE;

  hsuart->TxBitTicks = SOFT_UART_CalculateBitTicks(hsuart->HwConfig->TxTim, hsuart->Init.BaudRate);
  hsuart->RxBitTicks = SOFT_UART_CalculateBitTicks(hsuart->HwConfig->RxTim, hsuart->Init.BaudRate);

  SOFT_UART_TimerStop(hsuart->HwConfig->TxTim);
  SOFT_UART_TimerStop(hsuart->HwConfig->RxTim);

  hsuart->gState = HAL_SOFT_UART_STATE_READY;
  hsuart->RxState = HAL_SOFT_UART_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  DeInitializes the SOFT_UART peripheral
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SOFT_UART_DeInit(SOFT_UART_HandleTypeDef *hsuart)
{
  if (hsuart == NULL)
  {
    return HAL_ERROR;
  }

  hsuart->gState = HAL_SOFT_UART_STATE_BUSY;

  if (hsuart->HwConfig != NULL)
  {
    SOFT_UART_TimerStop(hsuart->HwConfig->TxTim);
    SOFT_UART_TimerStop(hsuart->HwConfig->RxTim);
  }

  if (hsuart->pRxBuffPtr != NULL)
  {
    free(hsuart->pRxBuffPtr);
    hsuart->pRxBuffPtr = NULL;
  }

  if (hsuart->pTxBuffPtr != NULL)
  {
    free(hsuart->pTxBuffPtr);
    hsuart->pTxBuffPtr = NULL;
  }

  hsuart->gState = HAL_SOFT_UART_STATE_RESET;
  hsuart->RxState = HAL_SOFT_UART_STATE_RESET;

  return HAL_OK;
}

/**
  * @brief  Sends an amount of data in blocking mode
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SOFT_UART_Transmit(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  if (hsuart->gState != HAL_SOFT_UART_STATE_READY)
  {
    return HAL_BUSY;
  }

  if ((pData == NULL) || (Size == 0U))
  {
    return HAL_ERROR;
  }

  hsuart->gState = HAL_SOFT_UART_STATE_BUSY_TX;

  for (uint16_t i = 0; i < Size; i++)
  {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (SOFT_UART_TxBufferPush(hsuart, pData[i]) == HAL_OK)
    {
      SOFT_UART_TxStartIfIdle(hsuart);
    }

    __set_PRIMASK(primask);

    if ((Timeout != HAL_MAX_DELAY) && ((HAL_GetTick() - tickstart) > Timeout))
    {
      hsuart->gState = HAL_SOFT_UART_STATE_READY;
      return HAL_TIMEOUT;
    }
  }

  hsuart->gState = HAL_SOFT_UART_STATE_READY;
  return HAL_OK;
}

/**
  * @brief  Receives an amount of data in blocking mode
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be received
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SOFT_UART_Receive(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  uint16_t count = 0;

  if (hsuart->RxState != HAL_SOFT_UART_STATE_READY)
  {
    return HAL_BUSY;
  }

  if ((pData == NULL) || (Size == 0U))
  {
    return HAL_ERROR;
  }

  hsuart->RxState = HAL_SOFT_UART_STATE_BUSY_RX;

  while (count < Size)
  {
    if (hsuart->RxHead != hsuart->RxTail)
    {
      pData[count++] = hsuart->pRxBuffPtr[hsuart->RxTail];
      hsuart->RxTail = (hsuart->RxTail + 1U) % hsuart->Init.RxBufferSize;
    }

    if ((Timeout != HAL_MAX_DELAY) && ((HAL_GetTick() - tickstart) > Timeout))
    {
      hsuart->RxState = HAL_SOFT_UART_STATE_READY;
      return HAL_TIMEOUT;
    }
  }

  hsuart->RxState = HAL_SOFT_UART_STATE_READY;
  return HAL_OK;
}

/**
  * @brief  Sends an amount of data in non blocking mode
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SOFT_UART_Transmit_IT(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size)
{
  if (hsuart->gState != HAL_SOFT_UART_STATE_READY)
  {
    return HAL_BUSY;
  }

  if ((pData == NULL) || (Size == 0U))
  {
    return HAL_ERROR;
  }

  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  for (uint16_t i = 0; i < Size; i++)
  {
    if (SOFT_UART_TxBufferPush(hsuart, pData[i]) != HAL_OK)
    {
      __set_PRIMASK(primask);
      return HAL_ERROR;
    }
  }

  SOFT_UART_TxStartIfIdle(hsuart);

  __set_PRIMASK(primask);
  return HAL_OK;
}

/**
  * @brief  Receives an amount of data in non blocking mode
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be received
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SOFT_UART_Receive_IT(SOFT_UART_HandleTypeDef *hsuart, uint8_t *pData, uint16_t Size)
{
  uint16_t count = 0;

  if (hsuart->RxState != HAL_SOFT_UART_STATE_READY)
  {
    return HAL_BUSY;
  }

  if ((pData == NULL) || (Size == 0U))
  {
    return HAL_ERROR;
  }

  while (count < Size && hsuart->RxHead != hsuart->RxTail)
  {
    pData[count++] = hsuart->pRxBuffPtr[hsuart->RxTail];
    hsuart->RxTail = (hsuart->RxTail + 1U) % hsuart->Init.RxBufferSize;
  }

  return (count == Size) ? HAL_OK : HAL_ERROR;
}

/**
  * @brief  This function handles SOFT_UART TX timer interrupt
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval None
  */
void HAL_SOFT_UART_TxTimCallback(SOFT_UART_HandleTypeDef *hsuart)
{
  uint8_t data;

  if (hsuart->TxBusy == 0U)
  {
    SOFT_UART_TxPinSet(hsuart);
    SOFT_UART_TimerStop(hsuart->HwConfig->TxTim);
    return;
  }

  if (hsuart->TxBitCount < UART_FRAME_BITS)
  {
    SOFT_UART_TxSendNextBit(hsuart);
    return;
  }

  if (SOFT_UART_TxBufferPop(hsuart, &data) == HAL_OK)
  {
    SOFT_UART_TxStartFrame(hsuart, data);
    return;
  }

  hsuart->TxBusy = 0;
  SOFT_UART_TxPinSet(hsuart);
  SOFT_UART_TimerStop(hsuart->HwConfig->TxTim);
}

/**
  * @brief  This function handles SOFT_UART RX timer interrupt
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval None
  */
void HAL_SOFT_UART_RxTimCallback(SOFT_UART_HandleTypeDef *hsuart)
{
  if (hsuart->RxStateMachine == SOFT_UART_RX_STATE_DATA)
  {
    if (SOFT_UART_RxPinRead(hsuart))
    {
      hsuart->RxData |= (1U << hsuart->RxBitCount);
    }

    hsuart->RxBitCount++;
    SOFT_UART_TimerStart(hsuart->HwConfig->RxTim, hsuart->RxBitTicks);

    if (hsuart->RxBitCount >= UART_DATA_BITS)
    {
      hsuart->RxStateMachine = SOFT_UART_RX_STATE_STOP;
    }
  }
  else if (hsuart->RxStateMachine == SOFT_UART_RX_STATE_STOP)
  {
    if (SOFT_UART_RxPinRead(hsuart))
    {
      SOFT_UART_RxBufferPush(hsuart, hsuart->RxData);
    }

    hsuart->RxStateMachine = SOFT_UART_RX_STATE_IDLE;
    SOFT_UART_TimerStop(hsuart->HwConfig->RxTim);
    __HAL_GPIO_EXTI_CLEAR_IT(hsuart->HwConfig->RxPin);
    HAL_NVIC_EnableIRQ(hsuart->HwConfig->RxExtiIRQn);
  }
  else
  {
    SOFT_UART_TimerStop(hsuart->HwConfig->RxTim);
    HAL_NVIC_EnableIRQ(hsuart->HwConfig->RxExtiIRQn);
  }
}

/**
  * @brief  This function handles SOFT_UART RX EXTI interrupt
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_SOFT_UART_RxExtiCallback(SOFT_UART_HandleTypeDef *hsuart, uint16_t GPIO_Pin)
{
  if (GPIO_Pin != hsuart->HwConfig->RxPin)
  {
    return;
  }

  if (hsuart->RxStateMachine != SOFT_UART_RX_STATE_IDLE || SOFT_UART_RxPinRead(hsuart))
  {
    return;
  }

  hsuart->RxStateMachine = SOFT_UART_RX_STATE_DATA;
  hsuart->RxData = 0;
  hsuart->RxBitCount = 0;

  HAL_NVIC_DisableIRQ(hsuart->HwConfig->RxExtiIRQn);
  SOFT_UART_TimerStart(hsuart->HwConfig->RxTim, hsuart->RxBitTicks + hsuart->RxBitTicks / 2U);
}

/**
  * @brief  Returns the SOFT_UART state
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval HAL state
  */
HAL_SOFT_UART_StateTypeDef HAL_SOFT_UART_GetState(SOFT_UART_HandleTypeDef *hsuart)
{
  return hsuart->gState;
}

/**
  * @brief  Return the SOFT_UART error code
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval SOFT_UART Error Code
  */
uint32_t HAL_SOFT_UART_GetError(SOFT_UART_HandleTypeDef *hsuart)
{
  return hsuart->ErrorCode;
}

/**
  * @brief  Get number of bytes available in RX buffer
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval Number of bytes available
  */
uint16_t HAL_SOFT_UART_GetRxAvailable(SOFT_UART_HandleTypeDef *hsuart)
{
  return (hsuart->RxHead - hsuart->RxTail + hsuart->Init.RxBufferSize) % hsuart->Init.RxBufferSize;
}

/**
  * @brief  Get number of bytes available in TX buffer
  * @param  hsuart pointer to a SOFT_UART_HandleTypeDef structure
  * @retval Number of bytes available
  */
uint16_t HAL_SOFT_UART_GetTxAvailable(SOFT_UART_HandleTypeDef *hsuart)
{
  return (hsuart->Init.TxBufferSize - 1U -
          (hsuart->TxHead - hsuart->TxTail + hsuart->Init.TxBufferSize) % hsuart->Init.TxBufferSize);
}
