#ifndef __SOFT_UART_H_
#define __SOFT_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define SOFT_UART_BAUDRATE      9600U
#define SOFT_UART_RX_BUF_SIZE   128U
#define SOFT_UART_TX_BUF_SIZE   256U

typedef enum
{
    SOFT_UART_RX_IDLE = 0,
    SOFT_UART_RX_DATA,
    SOFT_UART_RX_STOP,
} soft_uart_rx_state_t;

void soft_uart_init(void);
void soft_uart_send_byte(uint8_t data);
void soft_uart_send_buffer(const uint8_t *buf, uint16_t len);
void soft_uart_send_string(const char *str);

int soft_uart_read_byte(uint8_t *data);
uint16_t soft_uart_available(void);


#ifdef __cplusplus
}
#endif

#endif /* __SOFT_UART_H_ */
