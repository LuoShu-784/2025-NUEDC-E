/**
 * @file app_comm.c
 * @brief 通信模块实现 - TI 命令分发 + K230 数据接收
 */

#include "app_comm.h"
#include "app_log.h"
#include "cmsis_os.h"

#define COMM_TAG "COMM"

static ti_cmd_callback_t ti_callback = NULL;

static const char *ti_cmd_name(ti_cmd_type_t type)
{
    switch (type) {
        case CMD_MODE_CORNER_SELECT: return "CORNER_SELECT";
        case CMD_MODE_SHOOT_1_LAP: return "SHOOT_1_LAP";
        case CMD_MODE_SHOOT_2_LAPS: return "SHOOT_2_LAPS";
        case CMD_MODE_CIRCLE_1_LAP: return "CIRCLE_1_LAP";
        case CMD_BEGIN: return "BEGIN";
        default: return "UNKNOWN";
    }
}

static void ti_command_handler(const ti_cmd_t *cmd)
{
    if (cmd == NULL || !cmd->valid) {
        return;
    }

    logq_i(COMM_TAG, "TI CMD: %s data=%d", ti_cmd_name(cmd->type), cmd->data);

    if (ti_callback != NULL) {
        ti_callback(cmd);
    }
}

static void k230_frame_handler(const k230_frame_t *frame)
{
    if (frame == NULL || !frame->valid) {
        return;
    }

    // bool target_valid = frame->mode_flags & K230_FLAG_TARGET_VALID;
    // bool corner_mode = frame->mode_flags & K230_FLAG_CORNER_MODE;

    // logq_i(COMM_TAG, "K230: err_x=%d err_y=%d valid=%d mode=%s",
    //        frame->err_x, frame->err_y,
    //        target_valid ? 1 : 0,
    //        corner_mode ? "corner" : "center");
}

void comm_init(void)
{
    if (ti_init()) {
        ti_set_callback(ti_command_handler);
        logq_i(COMM_TAG, "TI initialized");
    } else {
        logq_e(COMM_TAG, "TI init failed");
    }

    if (k230_init()) {
        logq_i(COMM_TAG, "K230 initialized");
        k230_set_callback(k230_frame_handler);
    } else {
        logq_e(COMM_TAG, "K230 init failed");
    }
}

void comm_set_ti_callback(ti_cmd_callback_t callback)
{
    ti_callback = callback;
}

void comm_set_k230_callback(k230_frame_callback_t callback)
{
    k230_set_callback(callback);
}

void comm_task(void *argument)
{
    (void)argument;
    osDelay(100);

    comm_init();

    while (1) {
        ti_update();
        osDelay(100);
    }
}
