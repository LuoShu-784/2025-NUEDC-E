/**
 * @file app_comm.h
 * @brief 通信模块 - TI 命令分发 + K230 数据接收
 */

#ifndef __APP_COMM_H_
#define __APP_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ti.h"
#include "k230.h"

#define APP_COMM_CMD_MAX_LENGTH 64U

/**
 * @brief 初始化通信模块
 */
void comm_init(void);

/**
 * @brief 设置 TI 指令回调
 */
void comm_set_ti_callback(ti_cmd_callback_t callback);

/**
 * @brief 设置 K230 帧回调
 */
void comm_set_k230_callback(k230_frame_callback_t callback);

/**
 * @brief 通信任务入口
 */
void comm_task(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_COMM_H_ */
