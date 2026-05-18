/**
 * @file app_comm.h
 * @brief 通信模块 - TI 命令解析 + K230 数据接收
 */

#ifndef __APP_COMM_H_
#define __APP_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "k230.h"

#define APP_COMM_CMD_MAX_LENGTH 64U

/**
 * @brief TI 控制指令类型
 */
typedef enum {
    CMD_UNKNOWN = 0,
    CMD_LASER_ON,
    CMD_LASER_OFF,
    CMD_MOTOR0_ENABLE,
    CMD_MOTOR0_DISABLE,
    CMD_MOTOR1_ENABLE,
    CMD_MOTOR1_DISABLE,
    CMD_MOTOR0_SPEED,
    CMD_MOTOR1_SPEED,
    CMD_MOTOR0_ANGLE,
    CMD_MOTOR1_ANGLE,
    CMD_LED_RED_ON,
    CMD_LED_RED_OFF,
    CMD_LED_GREEN_ON,
    CMD_LED_GREEN_OFF,
    CMD_LED_BLUE_ON,
    CMD_LED_BLUE_OFF,
    CMD_LED_RED_BLINK,
    CMD_LED_GREEN_BLINK,
    CMD_LED_BLUE_BLINK,
} ti_cmd_type_t;

/**
 * @brief TI 控制指令数据结构
 */
typedef struct {
    ti_cmd_type_t type;
    float value;
    bool valid;
} ti_cmd_t;

/**
 * @brief TI 指令回调函数类型
 */
typedef void (*ti_cmd_callback_t)(const ti_cmd_t *cmd);

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
