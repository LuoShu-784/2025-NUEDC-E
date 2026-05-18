/**
 * @file ti.c
 * @brief TI 软串口数据接收实现
 *
 * 使用软串口中断接收模式，实现数据接收。
 */

#include "ti.h"
#include "soft_uart.h"
#include <string.h>

static ti_rx_callback_t user_callback = NULL;
static bool is_initialized = false;

bool ti_init(void)
{
    user_callback = NULL;
    is_initialized = true;
    return true;
}

void ti_set_callback(ti_rx_callback_t callback)
{
    user_callback = callback;
}

uint16_t ti_get_rx_available(void)
{
    return HAL_SOFT_UART_GetRxAvailable(&hsuart);
}

uint16_t ti_read(uint8_t *buffer, uint16_t size)
{
    if (buffer == NULL || size == 0 || !is_initialized) {
        return 0;
    }

    uint16_t available = HAL_SOFT_UART_GetRxAvailable(&hsuart);
    if (available == 0) {
        return 0;
    }

    uint16_t read_len = (available < size) ? available : size;

    if (HAL_SOFT_UART_Receive_IT(&hsuart, buffer, read_len) == HAL_OK) {
        return read_len;
    }

    return 0;
}
