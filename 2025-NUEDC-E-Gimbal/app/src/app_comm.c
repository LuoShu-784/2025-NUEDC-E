/**
 * @file app_comm.c
 * @brief 通信模块实现 - TI 命令解析 + K230 数据接收
 */

#include "app_comm.h"
#include "ti.h"
#include "k230.h"
#include "app_log.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define COMM_TAG "COMM"

static ti_cmd_callback_t ti_callback = NULL;
static char cmd_buffer[APP_COMM_CMD_MAX_LENGTH];
static uint16_t cmd_index = 0;


static bool parse_ti_command(const char *cmd_str, ti_cmd_t *cmd)
{
    if (cmd_str == NULL || cmd == NULL) {
        return false;
    }

    memset(cmd, 0, sizeof(ti_cmd_t));
    cmd->type = CMD_UNKNOWN;
    cmd->valid = false;

    if (strcmp(cmd_str, "laser on") == 0) {
        cmd->type = CMD_LASER_ON;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "laser off") == 0) {
        cmd->type = CMD_LASER_OFF;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "motor0 enable") == 0) {
        cmd->type = CMD_MOTOR0_ENABLE;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "motor0 disable") == 0) {
        cmd->type = CMD_MOTOR0_DISABLE;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "motor1 enable") == 0) {
        cmd->type = CMD_MOTOR1_ENABLE;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "motor1 disable") == 0) {
        cmd->type = CMD_MOTOR1_DISABLE;
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "motor0 speed ", 13) == 0) {
        cmd->type = CMD_MOTOR0_SPEED;
        cmd->value = atof(cmd_str + 13);
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "motor1 speed ", 13) == 0) {
        cmd->type = CMD_MOTOR1_SPEED;
        cmd->value = atof(cmd_str + 13);
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "motor0 angle ", 13) == 0) {
        cmd->type = CMD_MOTOR0_ANGLE;
        cmd->value = atof(cmd_str + 13);
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "motor1 angle ", 13) == 0) {
        cmd->type = CMD_MOTOR1_ANGLE;
        cmd->value = atof(cmd_str + 13);
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "led red on") == 0) {
        cmd->type = CMD_LED_RED_ON;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "led red off") == 0) {
        cmd->type = CMD_LED_RED_OFF;
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "led red blink", 13) == 0) {
        cmd->type = CMD_LED_RED_BLINK;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "led green on") == 0) {
        cmd->type = CMD_LED_GREEN_ON;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "led green off") == 0) {
        cmd->type = CMD_LED_GREEN_OFF;
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "led green blink", 15) == 0) {
        cmd->type = CMD_LED_GREEN_BLINK;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "led blue on") == 0) {
        cmd->type = CMD_LED_BLUE_ON;
        cmd->valid = true;
    }
    else if (strcmp(cmd_str, "led blue off") == 0) {
        cmd->type = CMD_LED_BLUE_OFF;
        cmd->valid = true;
    }
    else if (strncmp(cmd_str, "led blue blink", 14) == 0) {
        cmd->type = CMD_LED_BLUE_BLINK;
        cmd->valid = true;
    }

    return cmd->valid;
}

static void process_received_byte(uint8_t byte)
{
    if (byte == '\r' || byte == '\n') {
        if (cmd_index > 0) {
            cmd_buffer[cmd_index] = '\0';

            ti_cmd_t cmd;
            if (parse_ti_command(cmd_buffer, &cmd)) {
                logq_i(COMM_TAG, "TI CMD: %s", cmd_buffer);

                if (ti_callback != NULL) {
                    ti_callback(&cmd);
                }
            } else {
                logq_w(COMM_TAG, "Invalid TI CMD: %s", cmd_buffer);
            }

            cmd_index = 0;
            memset(cmd_buffer, 0, sizeof(cmd_buffer));
        }
    }
    else if (cmd_index < APP_COMM_CMD_MAX_LENGTH - 1) {
        if (isprint(byte)) {
            cmd_buffer[cmd_index++] = (char)byte;
        }
    }
    else {
        logq_w(COMM_TAG, "TI CMD buffer overflow");
        cmd_index = 0;
        memset(cmd_buffer, 0, sizeof(cmd_buffer));
    }
}

static void k230_frame_handler(const k230_frame_t *frame)
{
    if (frame == NULL || !frame->valid) {
        return;
    }

    bool target_valid = frame->mode_flags & K230_FLAG_TARGET_VALID;
    bool corner_mode = frame->mode_flags & K230_FLAG_CORNER_MODE;

    logq_i(COMM_TAG, "K230: err_x=%d err_y=%d valid=%d mode=%s",
           frame->err_x, frame->err_y,
           target_valid ? 1 : 0,
           corner_mode ? "corner" : "center");
}

void comm_init(void)
{
    cmd_index = 0;
    memset(cmd_buffer, 0, sizeof(cmd_buffer));

    if (ti_init()) {
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

    uint8_t rx_buffer[128];

    while (1)
    {
        uint16_t available = ti_get_rx_available();
        if (available > 0)
        {
            uint16_t read_len = (available > sizeof(rx_buffer)) ? sizeof(rx_buffer) : available;
            uint16_t actual_len = ti_read(rx_buffer, read_len);

            if (actual_len > 0)
            {
                for (uint16_t i = 0; i < actual_len; i++) {
                    process_received_byte(rx_buffer[i]);
                }
            }
        }

        osDelay(10);
    }
}
