#include "uart.h"
#include "ti_msp_dl_config.h"

volatile unsigned char uart_data = 0;
void uart1_send_char(char ch)
{
    
    while( DL_UART_isBusy(UART1_INST) == true );
    DL_UART_Main_transmitData(UART1_INST, ch);
}

void uart1_send_string(char* str)
{
    
    while(*str!=0&&str!=0)
    {
        uart1_send_char(*str++);
    }
}


void UART1_INST_IRQHandler(void)
{
    switch( DL_UART_getPendingInterrupt(UART1_INST) )
    {
        case DL_UART_IIDX_RX:
            uart_data = DL_UART_Main_receiveData(UART1_INST);
            uart1_send_char(uart_data);
            break;

        default:
            break;
    }
}


void uart3_send_char(char ch) {
    while( DL_UART_isBusy(UART3_INST) == true );
    DL_UART_Main_transmitData(UART3_INST, ch);
}

void uart3_send_string(char* str) {
    while(*str != 0) {
        uart3_send_char(*str++);
    }
}