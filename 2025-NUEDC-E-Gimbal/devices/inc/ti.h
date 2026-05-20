/**
 * @file ti.h
 * @brief TI 软串口命令接收（软串口 + 任务轮询解析）
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
 * @brief TI 控制指令类型
 */
typedef enum {
    CMD_UNKNOWN = 0,
    CMD_MODE_CORNER_SELECT,    // 0xC2 - 角点选择模式
    CMD_MODE_SHOOT_1_LAP,      // 0xC3 - 边打靶边循迹 1 圈
    CMD_MODE_SHOOT_2_LAPS,     // 0xC4 - 边打靶边循迹 2 圈
    CMD_MODE_CIRCLE_1_LAP,     // 0xC5 - 边画圆边循迹 1 圈
    CMD_BEGIN,                 // 0x55 0x55 - 启动信号
} ti_cmd_type_t;

/**
 * @brief TI 控制指令数据结构
 */
typedef struct {
    ti_cmd_type_t type;
    uint8_t data;  // 角点编号 (1-4) 或其他参数
    bool valid;
} ti_cmd_t;

/**
 * @brief TI 命令回调函数类型
 * @param cmd 解析后的命令
 * @note 回调在任务上下文执行
 */
typedef void (*ti_cmd_callback_t)(const ti_cmd_t *cmd);

/**
 * @brief 初始化 TI 软串口接收
 * @return true 成功，false 失败
 */
bool ti_init(void);

/**
 * @brief 设置命令回调函数
 * @param callback 回调函数指针
 */
void ti_set_callback(ti_cmd_callback_t callback);

/**
 * @brief 轮询接收并解析 TI 命令
 */
void ti_update(void);

#ifdef __cplusplus
}
#endif

#endif /* __TI_H_ */
