/**
 * @file app_callback.c
 * @brief HAL 回调桥接层 - 统一处理硬件中断回调
 */

#include "main.h"
#include "soft_uart.h"
#include "app_motor.h"
#include "jy901s.h"
#include "key.h"


void HAL_TIM_Callback(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return;
    }

    if (htim->Instance == TIM1) {
        HAL_SOFT_UART_TxTimCallback(&hsuart);
    } else if (htim->Instance == TIM3) {
        HAL_SOFT_UART_RxTimCallback(&hsuart);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY_Pin) {
        key_irq_handler(GPIO_Pin);
    } else if (GPIO_Pin == SOFTUART_RX_Pin) {
        HAL_SOFT_UART_RxExtiCallback(&hsuart, GPIO_Pin);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == NULL) {
        return;
    }

    if (huart->Instance == USART1) {
        jy901s_irq_handler(Size);
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcanx)
{
    if (hcanx == NULL) {
        return;
    }

    motor_irq_handler(hcanx);
}