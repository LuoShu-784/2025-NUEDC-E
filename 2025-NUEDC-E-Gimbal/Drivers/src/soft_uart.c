#include "soft_uart.h"
#include "main.h"
#include "tim.h"
#include <stdbool.h>
#include <stddef.h>

#define SOFT_UART_FRAME_BITS    10U

static volatile soft_uart_rx_state_t rx_state = SOFT_UART_RX_IDLE;
static volatile uint8_t rx_data = 0;
static volatile uint8_t rx_bit_index = 0;
static volatile uint16_t rx_bit_ticks = 0;

static uint8_t rx_buffer[SOFT_UART_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static uint8_t tx_buffer[SOFT_UART_TX_BUF_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;
static volatile bool tx_busy = false;
static volatile uint16_t tx_frame = 0;
static volatile uint8_t tx_bit_index = 0;
static volatile uint16_t tx_bit_ticks = 0;

static inline void tx_high(void)
{
    HAL_GPIO_WritePin(SOFTUART_TX_GPIO_Port, SOFTUART_TX_Pin, GPIO_PIN_SET);
}

static inline void tx_low(void)
{
    HAL_GPIO_WritePin(SOFTUART_TX_GPIO_Port, SOFTUART_TX_Pin, GPIO_PIN_RESET);
}

static inline GPIO_PinState rx_read(void)
{
    return HAL_GPIO_ReadPin(SOFTUART_RX_GPIO_Port, SOFTUART_RX_Pin);
}

static uint32_t timer_get_clock(TIM_HandleTypeDef *htim)
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

static uint16_t timer_get_bit_ticks(TIM_HandleTypeDef *htim)
{
    uint32_t tick_hz = timer_get_clock(htim) / (htim->Init.Prescaler + 1U);
    uint32_t ticks = (tick_hz + SOFT_UART_BAUDRATE / 2U) / SOFT_UART_BAUDRATE;

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

static void timer_start_it(TIM_HandleTypeDef *htim, uint16_t ticks)
{
    __HAL_TIM_DISABLE(htim);
    __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(htim, ticks - 1U);
    __HAL_TIM_SET_COUNTER(htim, 0U);
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE(htim);
}

static void timer_stop_it(TIM_HandleTypeDef *htim)
{
    __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
    __HAL_TIM_DISABLE(htim);
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
}

static void rx_buffer_push(uint8_t data)
{
    uint16_t next_head = (rx_head + 1U) % SOFT_UART_RX_BUF_SIZE;

    if (next_head != rx_tail)
    {
        rx_buffer[rx_head] = data;
        rx_head = next_head;
    }
}

static bool tx_buffer_push(uint8_t data)
{
    uint16_t next_head = (tx_head + 1U) % SOFT_UART_TX_BUF_SIZE;

    if (next_head == tx_tail)
    {
        return false;
    }

    tx_buffer[tx_head] = data;
    tx_head = next_head;

    return true;
}

static bool tx_buffer_pop(uint8_t *data)
{
    if (tx_head == tx_tail)
    {
        return false;
    }

    *data = tx_buffer[tx_tail];
    tx_tail = (tx_tail + 1U) % SOFT_UART_TX_BUF_SIZE;

    return true;
}

static void tx_write_next_bit(void)
{
    if ((tx_frame & (1U << tx_bit_index)) != 0U)
    {
        tx_high();
    }
    else
    {
        tx_low();
    }

    tx_bit_index++;
}

static void tx_start_frame(uint8_t data)
{
    tx_frame = ((uint16_t)1U << 9U) | ((uint16_t)data << 1U);
    tx_bit_index = 0;
    tx_busy = true;

    tx_write_next_bit();
    timer_start_it(&htim1, tx_bit_ticks);
}

static void tx_start_if_idle(void)
{
    uint8_t data;

    if (tx_busy)
    {
        return;
    }

    if (tx_buffer_pop(&data))
    {
        tx_start_frame(data);
    }
}

void soft_uart_init(void)
{
    tx_high();

    rx_state = SOFT_UART_RX_IDLE;
    rx_head = 0;
    rx_tail = 0;
    tx_head = 0;
    tx_tail = 0;
    tx_busy = false;

    tx_bit_ticks = timer_get_bit_ticks(&htim1);
    rx_bit_ticks = timer_get_bit_ticks(&htim3);

    timer_stop_it(&htim1);
    timer_stop_it(&htim3);
}

void soft_uart_send_byte(uint8_t data)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    if (tx_buffer_push(data))
    {
        tx_start_if_idle();
    }

    __set_PRIMASK(primask);
}

void soft_uart_send_buffer(const uint8_t *buf, uint16_t len)
{
    if (buf == NULL || len == 0U)
    {
        return;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        soft_uart_send_byte(buf[i]);
    }
}

void soft_uart_send_string(const char *str)
{
    if (str == NULL)
    {
        return;
    }

    while (*str != '\0')
    {
        soft_uart_send_byte((uint8_t)*str++);
    }
}

int soft_uart_read_byte(uint8_t *data)
{
    if (data == NULL || rx_head == rx_tail)
    {
        return -1;
    }

    *data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1U) % SOFT_UART_RX_BUF_SIZE;

    return 0;
}

uint16_t soft_uart_available(void)
{
    return (rx_head - rx_tail + SOFT_UART_RX_BUF_SIZE) % SOFT_UART_RX_BUF_SIZE;
}

void soft_uart_tim1_callback(void)
{
    uint8_t data;

    if (!tx_busy)
    {
        tx_high();
        timer_stop_it(&htim1);
        return;
    }

    if (tx_bit_index < SOFT_UART_FRAME_BITS)
    {
        tx_write_next_bit();
        return;
    }

    if (tx_buffer_pop(&data))
    {
        tx_start_frame(data);
        return;
    }

    tx_busy = false;
    tx_high();
    timer_stop_it(&htim1);
}

void soft_uart_tim3_callback(void)
{
    if (rx_state == SOFT_UART_RX_DATA)
    {
        if (rx_read() == GPIO_PIN_SET)
        {
            rx_data |= (1U << rx_bit_index);
        }

        rx_bit_index++;
        timer_start_it(&htim3, rx_bit_ticks);

        if (rx_bit_index >= 8U)
        {
            rx_state = SOFT_UART_RX_STOP;
        }
    }
    else if (rx_state == SOFT_UART_RX_STOP)
    {
        if (rx_read() == GPIO_PIN_SET)
        {
            rx_buffer_push(rx_data);
        }

        rx_state = SOFT_UART_RX_IDLE;
        timer_stop_it(&htim3);
        __HAL_GPIO_EXTI_CLEAR_IT(SOFTUART_RX_Pin);
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    }
    else
    {
        timer_stop_it(&htim3);
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    }
}

void soft_uart_gpio_exti_callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == SOFTUART_RX_Pin)
    {
        if (rx_state != SOFT_UART_RX_IDLE || rx_read() != GPIO_PIN_RESET)
        {
            return;
        }

        rx_state = SOFT_UART_RX_DATA;
        rx_data = 0;
        rx_bit_index = 0;

        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        timer_start_it(&htim3, rx_bit_ticks + rx_bit_ticks / 2U);
    }
}
