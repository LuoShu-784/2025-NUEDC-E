/**
 * @file ti.c
 * @brief TI 软串口命令接收实现
 *
 * 使用软串口中断接收模式，在任务上下文轮询并解析 TI 控制命令。
 */

#include "ti.h"
#include "soft_uart.h"
#include <string.h>

#define TI_FRAME_HEADER 0xCCU
#define TI_FRAME_TAIL   0xDDU
#define TI_BEGIN_SIGNAL 0x55U
#define TI_FRAME_LENGTH 4U

typedef enum {
    TI_PARSE_IDLE = 0,
    TI_PARSE_CMD,
    TI_PARSE_DATA,
    TI_PARSE_TAIL,
    TI_PARSE_BEGIN
} ti_parse_state_t;

typedef struct {
    uint8_t buffer[TI_FRAME_LENGTH];
    uint8_t index;
    ti_parse_state_t state;
    uint8_t begin_count;
} ti_parser_t;

static ti_cmd_callback_t user_callback = NULL;
static ti_parser_t parser;
static bool is_initialized = false;

static void parser_reset(void)
{
    parser.index = 0U;
    parser.state = TI_PARSE_IDLE;
    parser.begin_count = 0U;
}

static bool parse_ti_frame(const uint8_t *frame, ti_cmd_t *cmd)
{
    if (frame == NULL || cmd == NULL) {
        return false;
    }

    memset(cmd, 0, sizeof(ti_cmd_t));
    cmd->type = CMD_UNKNOWN;
    cmd->valid = false;

    if (frame[0] != TI_FRAME_HEADER || frame[3] != TI_FRAME_TAIL) {
        return false;
    }

    uint8_t cmd_byte = frame[1];
    uint8_t data_byte = frame[2];

    switch (cmd_byte) {
        case 0xC2:
            cmd->type = CMD_MODE_CORNER_SELECT;
            cmd->data = data_byte;
            cmd->valid = (data_byte >= 1U && data_byte <= 4U);
            break;

        case 0xC3:
            cmd->type = CMD_MODE_SHOOT_1_LAP;
            cmd->data = 1U;
            cmd->valid = true;
            break;

        case 0xC4:
            cmd->type = CMD_MODE_SHOOT_2_LAPS;
            cmd->data = 2U;
            cmd->valid = true;
            break;

        case 0xC5:
            cmd->type = CMD_MODE_CIRCLE_1_LAP;
            cmd->data = 1U;
            cmd->valid = true;
            break;

        default:
            break;
    }

    return cmd->valid;
}

static bool parse_byte(uint8_t byte, ti_cmd_t *cmd)
{
    if (cmd == NULL) {
        parser_reset();
        return false;
    }

    memset(cmd, 0, sizeof(ti_cmd_t));
    cmd->type = CMD_UNKNOWN;

    switch (parser.state) {
        case TI_PARSE_IDLE:
            if (byte == TI_FRAME_HEADER) {
                parser.buffer[0] = byte;
                parser.index = 1U;
                parser.state = TI_PARSE_CMD;
            } else if (byte == TI_BEGIN_SIGNAL) {
                parser.begin_count = 1U;
                parser.state = TI_PARSE_BEGIN;
            }
            return false;

        case TI_PARSE_BEGIN:
            if (byte == TI_BEGIN_SIGNAL && parser.begin_count == 1U) {
                cmd->type = CMD_BEGIN;
                cmd->data = 0U;
                cmd->valid = true;
                parser_reset();
                return true;
            }
            parser_reset();
            return false;

        case TI_PARSE_CMD:
            parser.buffer[parser.index++] = byte;
            parser.state = TI_PARSE_DATA;
            return false;

        case TI_PARSE_DATA:
            parser.buffer[parser.index++] = byte;
            parser.state = TI_PARSE_TAIL;
            return false;

        case TI_PARSE_TAIL:
            parser.buffer[parser.index++] = byte;
            bool ok = (byte == TI_FRAME_TAIL && parser.index == TI_FRAME_LENGTH);
            if (ok) {
                ok = parse_ti_frame(parser.buffer, cmd);
            }
            parser_reset();
            return ok;

        default:
            parser_reset();
            return false;
    }
}

static uint16_t ti_get_rx_available(void)
{
    return HAL_SOFT_UART_GetRxAvailable(&hsuart);
}

static uint16_t ti_read(uint8_t *buffer, uint16_t size)
{
    if (buffer == NULL || size == 0U || !is_initialized) {
        return 0U;
    }

    uint16_t available = HAL_SOFT_UART_GetRxAvailable(&hsuart);
    if (available == 0U) {
        return 0U;
    }

    uint16_t read_len = (available < size) ? available : size;

    if (HAL_SOFT_UART_Receive_IT(&hsuart, buffer, read_len) == HAL_OK) {
        return read_len;
    }

    return 0U;
}

bool ti_init(void)
{
    user_callback = NULL;
    memset(&parser, 0, sizeof(parser));
    parser_reset();
    is_initialized = true;
    return true;
}

void ti_set_callback(ti_cmd_callback_t callback)
{
    user_callback = callback;
}

void ti_update(void)
{
    if (!is_initialized) {
        return;
    }

    uint8_t rx_buffer[TI_RX_BUF_SIZE];
    uint16_t available = ti_get_rx_available();

    if (available == 0U) {
        return;
    }

    uint16_t read_len = (available > sizeof(rx_buffer)) ? sizeof(rx_buffer) : available;
    uint16_t actual_len = ti_read(rx_buffer, read_len);

    for (uint16_t i = 0U; i < actual_len; i++) {
        ti_cmd_t cmd;
        if (parse_byte(rx_buffer[i], &cmd) && user_callback != NULL) {
            user_callback(&cmd);
        }
    }
}
