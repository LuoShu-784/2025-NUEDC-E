/**
 * @file k230.h
 * @brief K230 串口数据接收（USART6 + DMA + IDLE）
 */

#ifndef __K230_H_
#define __K230_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define K230_RX_BUF_SIZE    256U
#define K230_FRAME_HEADER   0xA5U
#define K230_FRAME_LENGTH   7U

/**
 * @brief K230 模式标志位定义
 */
#define K230_FLAG_TARGET_VALID  (1U << 0)  // 目标有效
#define K230_FLAG_CORNER_MODE   (1U << 1)  // 角点误差模式

/**
 * @brief K230 数据帧结构
 */
typedef struct {
    uint8_t  header;        // 帧头 0xA5
    uint8_t  mode_flags;    // 模式标志
    int16_t  err_x;         // X轴误差（像素）
    int16_t  err_y;         // Y轴误差（像素）
    uint8_t  checksum;      // 校验和
    bool     valid;         // 数据有效标志
} k230_frame_t;

/**
 * @brief K230 帧数据回调函数类型
 * @param frame 解析后的帧数据
 * @note 回调在 ISR 上下文执行，实现侧需使用 ISR 安全 API
 */
typedef void (*k230_frame_callback_t)(const k230_frame_t *frame);

/**
 * @brief 初始化 K230 UART 接收
 * @return true 成功，false 失败
 */
bool k230_init(void);

/**
 * @brief 设置帧数据回调函数
 * @param callback 回调函数指针
 */
void k230_set_callback(k230_frame_callback_t callback);

/**
 * @brief 获取最新的有效帧数据
 * @param frame 输出参数，存储帧数据
 * @return true 有有效数据，false 无有效数据
 */
bool k230_get_frame(k230_frame_t *frame);

/**
 * @brief UART 空闲中断处理函数
 * @param size DMA 当前写入位置
 * @note 由 HAL_UARTEx_RxEventCallback 调用
 */
void k230_irq_handler(uint16_t size);

/**
 * @brief 发送数据到 K230
 * @param data 数据指针
 * @param len 数据长度
 * @return true 成功，false 失败
 */
bool k230_send(const uint8_t *data, uint16_t len);

/**
 * @brief 获取统计信息
 * @param rx_count 接收字节数
 * @param frame_count 有效帧数
 * @param error_count 错误帧数
 */
void k230_get_statistics(uint32_t *rx_count, uint32_t *frame_count, uint32_t *error_count);

#ifdef __cplusplus
}
#endif

#endif /* __K230_H_ */
