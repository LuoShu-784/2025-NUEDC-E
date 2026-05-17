/**
 * @file jy901s.c
 * @brief JY901S IMU 姿态数据解析（USART1 + DMA + IDLE）
 *
 * 模块自管 DMA 接收缓冲与解析状态机，对外仅暴露设备级单例 API。
 * HAL 层 RxEventCallback 由 app_callback.c 转发到 jy901s_rx_event_handler()，
 * 解析在 ISR 上下文完成，回调亦在 ISR 中执行（实现侧需使用 ISR 安全 API）。
 */

#include "jy901s.h"
#include "usart.h"
#include "stm32f4xx_hal.h"
#include <string.h>

typedef enum {
    JY901S_PARSE_IDLE = 0,
    JY901S_PARSE_HEADER,
    JY901S_PARSE_DATA
} jy901s_parse_state_t;

typedef struct {
    uint8_t buffer[JY901S_FRAME_LENGTH];
    uint8_t index;
    jy901s_parse_state_t state;
} jy901s_parser_t;

static uint8_t  rx_buf[JY901S_RX_BUF_SIZE];
static uint16_t rx_old_pos;
static jy901s_parser_t parser;
static jy901s_data_t   latest;
static volatile uint32_t total_frame_count;
static volatile uint32_t total_error_count;
static volatile jy901s_callback_t user_callback;
static bool is_initialized;

static inline void parser_reset(void)
{
    parser.index = 0U;
    parser.state = JY901S_PARSE_IDLE;
}

static bool verify_checksum(const uint8_t *frame)
{
    uint8_t sum = 0U;
    for (uint8_t i = 0U; i < JY901S_FRAME_LENGTH - 1U; ++i) {
        sum += frame[i];
    }
    return sum == frame[JY901S_FRAME_LENGTH - 1U];
}

static void extract_attitude(const uint8_t *frame, jy901s_data_t *attitude)
{
    int16_t roll_raw  = (int16_t)(((uint16_t)frame[3] << 8) | frame[2]);
    int16_t pitch_raw = (int16_t)(((uint16_t)frame[5] << 8) | frame[4]);
    int16_t yaw_raw   = (int16_t)(((uint16_t)frame[7] << 8) | frame[6]);

    attitude->roll    = (float)roll_raw  * JY901S_ANGLE_SCALE;
    attitude->pitch   = (float)pitch_raw * JY901S_ANGLE_SCALE;
    attitude->yaw     = (float)yaw_raw   * JY901S_ANGLE_SCALE;
    attitude->version = (uint16_t)(((uint16_t)frame[9] << 8) | frame[8]);
    attitude->valid   = true;
}

// 单字节状态机，返回 true 表示完成一帧 ANGLE 帧
static bool parse_byte(uint8_t byte, jy901s_data_t *attitude)
{
    switch (parser.state)
    {
    case JY901S_PARSE_IDLE:
        if (byte == JY901S_FRAME_HEADER) {
            parser.buffer[0] = byte;
            parser.index = 1U;
            parser.state = JY901S_PARSE_HEADER;
        }
        return false;

    case JY901S_PARSE_HEADER:
        parser.buffer[parser.index++] = byte;
        if (byte == JY901S_FRAME_TYPE_ANGLE) {
            parser.state = JY901S_PARSE_DATA;
        } else {
            // JY901S 会交错输出多种帧类型，非角度帧直接丢弃，不计错
            parser_reset();
        }
        return false;

    case JY901S_PARSE_DATA:
        parser.buffer[parser.index++] = byte;
        if (parser.index >= JY901S_FRAME_LENGTH) {
            bool ok = verify_checksum(parser.buffer);
            if (ok) {
                extract_attitude(parser.buffer, attitude);
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

bool jy901s_init(void)
{
    memset(&parser, 0, sizeof(parser));
    memset(&latest, 0, sizeof(latest));
    rx_old_pos = 0U;
    total_frame_count = 0U;
    total_error_count = 0U;
    user_callback = NULL;
    parser_reset();

    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, JY901S_RX_BUF_SIZE) != HAL_OK) {
        return false;
    }

    is_initialized = true;
    return true;
}

void jy901s_set_callback(jy901s_callback_t callback)
{
    user_callback = callback;
}

// 将单段 [start, end) 区间字节喂给状态机
static void feed_range(uint16_t start, uint16_t end, jy901s_callback_t cb)
{
    for (uint16_t pos = start; pos < end; ++pos) {
        jy901s_data_t attitude;
        if (parse_byte(rx_buf[pos], &attitude)) {
            latest = attitude;
            if (cb != NULL) {
                cb(&attitude);
            }
        }
    }
}

void jy901s_rx_event_handler(uint16_t size)
{
    if (!is_initialized || size > JY901S_RX_BUF_SIZE) {
        return;
    }

    // size 是 DMA 当前物理写入位置 [0, BUF_SIZE]：
    // - IDLE 事件给出真实位置（绕回瞬间可能为 0）
    // - TC 事件给出 BUF_SIZE
    // 等于 old_pos 表示无新字节
    uint16_t old_pos = rx_old_pos;
    if (size == old_pos) {
        return;
    }

    jy901s_callback_t cb = user_callback;

    if (size > old_pos) {
        // 线性区间
        feed_range(old_pos, size, cb);
    } else {
        // 跨越缓冲末尾：先处理 [old_pos, BUF_SIZE)，再处理 [0, size)
        feed_range(old_pos, JY901S_RX_BUF_SIZE, cb);
        feed_range(0U, size, cb);
    }

    rx_old_pos = (size >= JY901S_RX_BUF_SIZE) ? 0U : size;
}

bool jy901s_get_attitude(jy901s_data_t *attitude)
{
    if (attitude == NULL) {
        return false;
    }

    // 写者在 ISR，读者短暂屏蔽中断完成结构体原子快照
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    bool valid = latest.valid;
    if (valid) {
        *attitude = latest;
    }
    __set_PRIMASK(primask);

    return valid;
}

void jy901s_get_statistics(uint32_t *frame_count, uint32_t *error_count)
{
    if (frame_count != NULL) {
        *frame_count = total_frame_count;
    }
    if (error_count != NULL) {
        *error_count = total_error_count;
    }
}
