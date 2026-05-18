/**
 * @file ti.h
 * @brief TI 软串口数据接收（软串口 + 中断）
 */

#ifndef __TI_H_
#define __TI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define TI_RX_BUF_SIZE      128U
#define TI_CMD_MAX_LENGTH   64U

/**
 * @brief TI 数据回调函数类型
 * @param data 接收到的数据指针
 * @param len 数据长度
 * @note 回调在任务上下文执行
 */
typedef void (*ti_rx_callback_t)(const uint8_t *data, uint16_t len);

/**
 * @brief 初始化 TI 软串口接收
 * @return true 成功，false 失败
 */
bool ti_init(void);

/**
 * @brief 设置接收回调函数
 * @param callback 回调函数指针
 */
void ti_set_callback(ti_rx_callback_t callback);

/**
 * @brief 获取可用接收数据字节数
 * @return 可用字节数
 */
uint16_t ti_get_rx_available(void);

/**
 * @brief 非阻塞读取接收数据
 * @param buffer 接收缓冲区
 * @param size 期望读取的字节数
 * @return 实际读取的字节数
 */
uint16_t ti_read(uint8_t *buffer, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __TI_H_ */
