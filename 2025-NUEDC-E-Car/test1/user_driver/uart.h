#ifndef UART_H
#define UART_H

#include "ti_msp_dl_config.h"

void uart1_send_char(char ch);

void uart1_send_string(char* str);

void UART1_INST_IRQHandler(void);

#endif /* UART_H */
