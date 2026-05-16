#include "main.h"
#include "soft_uart.h"

void HAL_TIM_Callback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        HAL_SOFT_UART_TxTimCallback(&hsuart);
    }
    else if (htim->Instance == TIM3)
    {
        HAL_SOFT_UART_RxTimCallback(&hsuart);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY_Pin)
    {
        extern void key_irq_handler(uint16_t GPIO_Pin);
        key_irq_handler(GPIO_Pin);
    }
    else if (GPIO_Pin == SOFTUART_RX_Pin)
    {
        HAL_SOFT_UART_RxExtiCallback(&hsuart, GPIO_Pin);
    }
}

void log_output_handler(const char *str, uint16_t len)
{
    HAL_SOFT_UART_Transmit(&hsuart, (uint8_t *)str, len, HAL_MAX_DELAY);
}