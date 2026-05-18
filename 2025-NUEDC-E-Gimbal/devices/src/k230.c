/**
 * @file k230.c
 * @brief K230 串口数据接收实现（USART6 + DMA + IDLE）
 *
 * 使用 DMA 循环接收模式 + 串口空闲中断，实现高效的数据接收。
 * HAL 层 RxEventCallback 由 app_callback.c 转发到 k230_irq_handler()。
 */

#include "k230.h"
#include "usart.h"
#include "stm32f4xx_hal.h"
#include <string.h>

typedef enum {
    K230_PARSE_IDLE = 0,
    K230_PARSE_DATA
} k230_parse_state_t;

typedef struct {
    uint8_t buffer[K230_FRAME_LENGTH];
    uint8_t index;
    k230_parse_state_t state;
} k230_parser_t;

static uint8_t rx_buf[K230_RX_BUF_SIZE];
static uint16_t rx_old_pos;
static k230_parser_t parser;
static k230_frame_t latest;
static volatile uint32_t total_rx_count;
static volatile uint32_t total_frame_count;
static volatile uint32_t total_error_count;
static volatile k230_frame_callback_t user_callback;
static bool is_initialized;

static inline void parser_reset(void)
{
    parser.index = 0U;
    parser.state = K230_PARSE_IDLE;
}

static bool verify_checksum(const uint8_t *frame)
{
    uint8_t sum = 0U;
    for (uint8_t i = 0U; i < K230_FRAME_LENGTH - 1U; ++i) {
        sum += frame[i];
    }
    return sum == frame[K230_FRAME_LENGTH - 1U];
}

static void extract_frame(const uint8_t *frame, k230_frame_t *data)
{
    data->header = frame[0];
    data->mode_flags = frame[1];
    data->err_x = (int16_t)(((uint16_t)frame[3] << 8) | frame[2]);
    data->err_y = (int16_t)(((uint16_t)frame[5] << 8) | frame[4]);
    data->checksum = frame[6];
    data->valid = true;
}

static bool parse_byte(uint8_t byte, k230_frame_t *frame)
{
    switch (parser.state)
    {
    case K230_PARSE_IDLE:
        if (byte == K230_FRAME_HEADER) {
            parser.buffer[0] = byte;
            parser.index = 1U;
            parser.state = K230_PARSE_DATA;
        }
        return false;

    case K230_PARSE_DATA:
        parser.buffer[parser.index++] = byte;
        if (parser.index >= K230_FRAME_LENGTH) 
        {
            bool ok = verify_checksum(parser.buffer);
            if (ok) {
                extract_frame(parser.buffer, frame);
                total_frame_count++;
            } else {
                total_error_count++;
            }
            parser_reset();
            return ok;
        }
        return false;

    default:
        parser_reset();
        return false;
    }
}

bool k230_init(void)
{
    memset(&parser, 0, sizeof(parser));
    memset(&latest, 0, sizeof(latest));
    memset(rx_buf, 0, sizeof(rx_buf));
    rx_old_pos = 0U;
    total_rx_count = 0U;
    total_frame_count = 0U;
    total_error_count = 0U;
    user_callback = NULL;
    parser_reset();

    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf, K230_RX_BUF_SIZE) != HAL_OK) {
        return false;
    }

    is_initialized = true;
    return true;
}

void k230_set_callback(k230_frame_callback_t callback)
{
    user_callback = callback;
}

static void feed_range(uint16_t start, uint16_t end, k230_frame_callback_t cb)
{
    for (uint16_t pos = start; pos < end; ++pos) {
        k230_frame_t frame;
        if (parse_byte(rx_buf[pos], &frame)) {
            latest = frame;
            if (cb != NULL) {
                cb(&frame);
            }
        }
    }
}

void k230_irq_handler(uint16_t size)
{
    if (!is_initialized || size > K230_RX_BUF_SIZE) {
        return;
    }

    uint16_t old_pos = rx_old_pos;
    if (size == old_pos) {
        return;
    }

    k230_frame_callback_t cb = user_callback;

    if (size > old_pos) {
        uint16_t data_len = size - old_pos;
        feed_range(old_pos, size, cb);
        total_rx_count += data_len;
    } else {
        uint16_t len1 = K230_RX_BUF_SIZE - old_pos;
        uint16_t len2 = size;
        feed_range(old_pos, K230_RX_BUF_SIZE, cb);
        feed_range(0U, size, cb);
        total_rx_count += (len1 + len2);
    }

    rx_old_pos = (size >= K230_RX_BUF_SIZE) ? 0U : size;
}

bool k230_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0 || !is_initialized) {
        return false;
    }

    return HAL_UART_Transmit(&huart6, (uint8_t *)data, len, 1000) == HAL_OK;
}

bool k230_get_frame(k230_frame_t *frame)
{
    if (frame == NULL) {
        return false;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    bool valid = latest.valid;
    if (valid) {
        *frame = latest;
    }
    __set_PRIMASK(primask);

    return valid;
}

void k230_get_statistics(uint32_t *rx_count, uint32_t *frame_count, uint32_t *error_count)
{
    if (rx_count != NULL) {
        *rx_count = total_rx_count;
    }
    if (frame_count != NULL) {
        *frame_count = total_frame_count;
    }
    if (error_count != NULL) {
        *error_count = total_error_count;
    }
}
