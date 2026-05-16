#include "main.h"

void HAL_TIM_Callback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        extern void soft_uart_tim1_callback(void);
        soft_uart_tim1_callback();
    }
    else if (htim->Instance == TIM3)
    {
        extern void soft_uart_tim3_callback(void);
        soft_uart_tim3_callback();
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
        extern void soft_uart_gpio_exti_callback(uint16_t GPIO_Pin);
        soft_uart_gpio_exti_callback(GPIO_Pin);
    }
}
