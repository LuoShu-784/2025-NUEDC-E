#include "app.h"
#include "app_comm.h"
#include "app_led.h"
#include "app_log.h"
#include "app_motor.h"
#include "cmsis_os2.h"
#include "pid.h"

#define MAIN_TAG "MAIN"

#define PITCH MOTOR0
#define YAW   MOTOR1

#define PITCH_POLARITY  1.0f
#define YAW_POLARITY   -1.0f

#define PITCH_OFFSET_PX 0.0f
#define YAW_OFFSET_PX   0.0f

#define PITCH_KP 0.08f
#define PITCH_KI 0.0f
#define PITCH_KD 0.0f
#define PITCH_OUTPUT_LIMIT 8.0f
#define PITCH_INTEGRAL_LIMIT 8.0f

#define YAW_KP 0.10f
#define YAW_KI 0.0f
#define YAW_KD 0.0f
#define YAW_OUTPUT_LIMIT 10.0f
#define YAW_INTEGRAL_LIMIT 10.0f

#define PITCH_DEADZONE_PX 8.0f
#define YAW_DEADZONE_PX   8.0f
#define AIM_LOCK_THRESHOLD_PX 6.0f
#define AIM_LOCK_HOLD_MS 150U

#define PITCH_FILTER_ALPHA 0.35f
#define YAW_FILTER_ALPHA   0.35f
#define PITCH_SLEW_RATE_PER_CYCLE 1.0f
#define YAW_SLEW_RATE_PER_CYCLE   1.2f
#define PITCH_MIN_EFFECTIVE_SPEED 0.8f
#define YAW_MIN_EFFECTIVE_SPEED   0.8f

#define PITCH_MIN_ANGLE  -90.0f
#define PITCH_MAX_ANGLE   90.0f
#define PITCH_START_ANGLE -10.0f
#define PITCH_START_WAIT_MS 800U

#define MAIN_TASK_INTERVAL_MS 10U
#define MAIN_PID_INTERVAL_MS  10U
#define LOG_INTERVAL_MS      100U

typedef enum {
    AIM_STATE_WAIT_TARGET = 0,
    AIM_STATE_TRACKING,
    AIM_STATE_LOCKED,
} aim_state_t;

static k230_frame_t k230_dat;
static motor_data_t pitch_data;
static motor_data_t yaw_data;

static pid_t pid_pitch;
static pid_t pid_yaw;

static uint32_t log_tick = 0U;
static uint32_t pid_tick = 0U;
static uint32_t lock_start_tick = 0U;

static float pitch_output = 0.0f;
static float yaw_output = 0.0f;
static float pitch_err_filtered = 0.0f;
static float yaw_err_filtered = 0.0f;

static aim_state_t aim_state = AIM_STATE_WAIT_TARGET;

static inline float abs_float(float value)
{
    return (value < 0.0f) ? -value : value;
}

static inline bool is_near_zero(float value, float threshold)
{
    return abs_float(value) < threshold;
}

static inline float clamp_symmetric(float value, float limit_abs)
{
    if (value > limit_abs) {
        return limit_abs;
    }

    if (value < -limit_abs) {
        return -limit_abs;
    }

    return value;
}

static inline float low_pass_filter(float input, float state, float alpha)
{
    return state + alpha * (input - state);
}

static inline float apply_min_effective_speed(float value, float min_abs)
{
    if (value > 0.0f && value < min_abs) {
        return min_abs;
    }

    if (value < 0.0f && value > -min_abs) {
        return -min_abs;
    }

    return value;
}

static inline float apply_slew_rate(float target, float current, float max_delta)
{
    float delta = clamp_symmetric(target - current, max_delta);
    return current + delta;
}

static bool frame_has_target(const k230_frame_t *frame)
{
    if (frame == NULL) {
        return false;
    }

    return frame->valid && ((frame->mode_flags & K230_FLAG_TARGET_VALID) != 0U);
}

static void motor_stop_all(void)
{
    motor_set_speed(PITCH, 0.0f);
    motor_set_speed(YAW, 0.0f);
}

static void reset_control_state(void)
{
    pid_reset(&pid_pitch);
    pid_reset(&pid_yaw);

    pitch_output = 0.0f;
    yaw_output = 0.0f;
    pitch_err_filtered = 0.0f;
    yaw_err_filtered = 0.0f;
    lock_start_tick = 0U;

    motor_stop_all();
}

static void set_aim_state(aim_state_t new_state)
{
    if (aim_state == new_state) {
        return;
    }

    aim_state = new_state;

    switch (aim_state) {
        case AIM_STATE_WAIT_TARGET:
            led_ctrl(LED_RED, LED_MOD_BLINK_FAST);
            logq_i(MAIN_TAG, "state -> WAIT_TARGET");
            break;

        case AIM_STATE_TRACKING:
            led_ctrl(LED_RED, LED_MOD_BLINK_NORMAL);
            logq_i(MAIN_TAG, "state -> TRACKING");
            break;

        case AIM_STATE_LOCKED:
            led_ctrl(LED_GREEN, LED_MOD_ON);
            logq_i(MAIN_TAG, "state -> LOCKED");
            break;

        default:
            break;
    }
}

static void pitch_limit_stop(void)
{
    if (pitch_data.angle >= PITCH_MAX_ANGLE || pitch_data.angle <= PITCH_MIN_ANGLE) {
        motor_set_speed(PITCH, 0.0f);
        pitch_output = 0.0f;
    }
}

static void pitch_control_step(float err_y)
{
    if (is_near_zero(err_y, PITCH_DEADZONE_PX)) {
        pid_reset(&pid_pitch);
        pitch_output = 0.0f;
        motor_set_speed(PITCH, 0.0f);
        return;
    }

    pitch_err_filtered = low_pass_filter(err_y, pitch_err_filtered, PITCH_FILTER_ALPHA);
    pitch_output = pid_calc(&pid_pitch, PITCH_OFFSET_PX, pitch_err_filtered);
    pitch_output = apply_min_effective_speed(pitch_output, PITCH_MIN_EFFECTIVE_SPEED);
    pitch_output = apply_slew_rate(pitch_output, pitch_output, PITCH_SLEW_RATE_PER_CYCLE);
    motor_set_speed(PITCH, PITCH_POLARITY * pitch_output);
}

static void yaw_control_step(float err_x)
{
    if (is_near_zero(err_x, YAW_DEADZONE_PX)) {
        pid_reset(&pid_yaw);
        yaw_output = 0.0f;
        motor_set_speed(YAW, 0.0f);
        return;
    }

    yaw_err_filtered = low_pass_filter(err_x, yaw_err_filtered, YAW_FILTER_ALPHA);
    yaw_output = pid_calc(&pid_yaw, YAW_OFFSET_PX, yaw_err_filtered);
    yaw_output = apply_min_effective_speed(yaw_output, YAW_MIN_EFFECTIVE_SPEED);
    yaw_output = apply_slew_rate(yaw_output, yaw_output, YAW_SLEW_RATE_PER_CYCLE);
    motor_set_speed(YAW, YAW_POLARITY * yaw_output);
}

static void tracking_step(uint32_t current_tick)
{
    float err_x = (float)k230_dat.err_x;
    float err_y = (float)k230_dat.err_y;

    pitch_control_step(err_y);
    yaw_control_step(err_x);
    pitch_limit_stop();

    if (is_near_zero(err_x, AIM_LOCK_THRESHOLD_PX) &&
        is_near_zero(err_y, AIM_LOCK_THRESHOLD_PX)) {
        if (lock_start_tick == 0U) {
            lock_start_tick = current_tick;
        } else if ((current_tick - lock_start_tick) >= AIM_LOCK_HOLD_MS) {
            motor_stop_all();
            set_aim_state(AIM_STATE_LOCKED);
        }
    } else {
        lock_start_tick = 0U;
        set_aim_state(AIM_STATE_TRACKING);
    }
}

static void ti_command_handler(const ti_cmd_t *cmd)
{
    if (cmd == NULL || !cmd->valid) {
        return;
    }

    switch (cmd->type) {
        case CMD_BEGIN:
            reset_control_state();
            set_aim_state(AIM_STATE_WAIT_TARGET);
            logq_i(MAIN_TAG, "Begin signal received");
            break;

        case CMD_MODE_CORNER_SELECT:
            logq_i(MAIN_TAG, "Corner select=%u", cmd->data);
            break;

        case CMD_MODE_SHOOT_1_LAP:
            logq_i(MAIN_TAG, "Mode shoot 1 lap");
            break;

        case CMD_MODE_SHOOT_2_LAPS:
            logq_i(MAIN_TAG, "Mode shoot 2 laps");
            break;

        case CMD_MODE_CIRCLE_1_LAP:
            logq_i(MAIN_TAG, "Mode circle 1 lap");
            break;

        default:
            logq_w(MAIN_TAG, "Unknown command type=%d", cmd->type);
            break;
    }
}

void app_init(void)
{
    motor_init();
    comm_set_ti_callback(ti_command_handler);

    osDelay(10);

    motor_enable(PITCH);
    motor_enable(YAW);

    osDelay(10);

    pid_init(&pid_pitch, PITCH_KP, PITCH_KI, PITCH_KD, PITCH_OUTPUT_LIMIT, PITCH_INTEGRAL_LIMIT);
    pid_init(&pid_yaw, YAW_KP, YAW_KI, YAW_KD, YAW_OUTPUT_LIMIT, YAW_INTEGRAL_LIMIT);

    motor_set_angle(PITCH, PITCH_START_ANGLE);
    osDelay(PITCH_START_WAIT_MS);

    reset_control_state();
    set_aim_state(AIM_STATE_WAIT_TARGET);
    laser_on();
}

void app_main_task(void *argument)
{
    (void)argument;
    osDelay(100);

    app_init();

    uint32_t current_tick = 0U;
    uint32_t wake_tick = 0U;

    while (1) {
        current_tick = osKernelGetTickCount();

        motor_get_data(PITCH, &pitch_data);
        motor_get_data(YAW, &yaw_data);
        k230_get_frame(&k230_dat);

        if ((current_tick - log_tick) >= LOG_INTERVAL_MS) {
            log_tick = current_tick;
            logq_i(MAIN_TAG,
                   "state=%d target=%d err_x=%d err_y=%d pitch=%.2f yaw=%.2f",
                   (int)aim_state,
                   frame_has_target(&k230_dat) ? 1 : 0,
                   k230_dat.err_x,
                   k230_dat.err_y,
                   pitch_data.angle,
                   yaw_data.angle);
        }

        if ((current_tick - pid_tick) >= MAIN_PID_INTERVAL_MS) {
            pid_tick = current_tick;

            if (!frame_has_target(&k230_dat)) {
                reset_control_state();
                set_aim_state(AIM_STATE_WAIT_TARGET);
            } else {
                tracking_step(current_tick);
            }
        }

        wake_tick = current_tick + MAIN_TASK_INTERVAL_MS;
        osDelayUntil(wake_tick);
    }
}
