/**
 * @file soft_uart_example.c
 * @brief 软件串口使用示例
 *
 * 使用方法：
 * 1. 在 main.c 的 USER CODE BEGIN 2 区域调用 soft_uart_example_init()
 * 2. 在 main.c 的 USER CODE BEGIN 3 区域调用 soft_uart_example_loop()
 * 3. 连接 USB-TTL 到 PB14(TX) 和 PB15(RX)，波特率设置为 9600
 */

#include "soft_uart.h"
#include "cmsis_os.h"

void soft_uart_example_init(void)
{
    soft_uart_init();

    soft_uart_send_string("Soft UART initialized!\r\n");
    soft_uart_send_string("Baudrate: 9600 8N1\r\n");
    soft_uart_send_string("TX: PB14, RX: PB15\r\n");
}

void soft_uart_example_loop(void)
{
    uint8_t data;

    if (soft_uart_read_byte(&data) == 0)
    {
        soft_uart_send_string("Received: ");
        soft_uart_send_byte(data);
        soft_uart_send_string("\r\n");
    }

    osDelay(10);
}
