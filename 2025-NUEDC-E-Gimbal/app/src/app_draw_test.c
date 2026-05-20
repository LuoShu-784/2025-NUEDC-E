/**
 * @file app_draw_test.c
 * @brief 画图测试流程实现
 */

#include "app_draw_test.h"
#include "app_draw.h"
#include "app_log.h"
#include "app_motor.h"
#include "cmsis_os2.h"
#include "laser.h"

#include <math.h>

#define DRAW_TEST_TAG                    "DRAW_TEST"
#define DRAW_TEST_STEP_DELAY_MS          8U
#define DRAW_TEST_FAST_STEP_DELAY_MS     8U
#define DRAW_TEST_AXIS_DELAY_MS          8U
#define DRAW_TEST_SETTLE_MS              500U
#define DRAW_TEST_GAP_MS                 600U
#define DRAW_TEST_YAW_PROBE_INTERVAL     8U
#define DRAW_TEST_SETTLE_CHECK_MS        20U
#define DRAW_TEST_SETTLE_ERROR_DEG       0.2f
#define DRAW_TEST_Y_OFFSET               0.14f

typedef bool (*draw_shape_func_t)(const draw_shape_config_t *config);
typedef bool (*draw_curve_func_t)(const draw_curve_config_t *config);

static void draw_test_log_motor(void)
{
    motor_data_t motor0_data = {0};
    motor_data_t motor1_data = {0};

    motor_get_data(MOTOR0, &motor0_data);
    motor_get_data(MOTOR1, &motor1_data);

    logq_i(DRAW_TEST_TAG, "Motor 0 - Enabled: %d, Speed: %.2f, Angle: %.1f, Current: %.2f",
        motor0_data.enabled ? 1 : 0,
        motor0_data.speed,
        motor0_data.angle,
        motor0_data.current);
    logq_i(DRAW_TEST_TAG, "Motor 1 - Enabled: %d, Speed: %.2f, Angle: %.1f, Current: %.2f",
        motor1_data.enabled ? 1 : 0,
        motor1_data.speed,
        motor1_data.angle,
        motor1_data.current);
}

static void draw_test_log_point(const char *name, float x_ratio, float y_ratio)
{
    draw_point_t point = {0};

    if (draw_map_xy(x_ratio, y_ratio, &point)) {
        logq_i(DRAW_TEST_TAG, "%s: x=%.2f y=%.2f pitch=%.1f yaw=%.1f",
            name,
            x_ratio,
            y_ratio,
            point.pitch_deg,
            point.yaw_deg);
    }
}

static void draw_test_log_map(void)
{
    draw_test_log_point("left_top", 0.0f, 0.0f);
    draw_test_log_point("right_top", 1.0f, 0.0f);
    draw_test_log_point("right_bottom", 1.0f, 1.0f);
    draw_test_log_point("left_bottom", 0.0f, 1.0f);
    draw_test_log_point("center", 0.5f, 0.5f);
}

static float draw_test_angle_error_deg(float feedback_deg, float target_deg)
{
    float error_deg = feedback_deg - target_deg;

    while (error_deg > 180.0f) {
        error_deg -= 360.0f;
    }

    while (error_deg < -180.0f) {
        error_deg += 360.0f;
    }

    return error_deg;
}

static bool draw_test_wait_target(float x_ratio, float y_ratio)
{
    draw_point_t target = {0};

    if (!draw_map_xy(x_ratio, y_ratio, &target)) {
        return false;
    }

    for (uint32_t elapsed_ms = 0U; elapsed_ms <= DRAW_TEST_SETTLE_MS; elapsed_ms += DRAW_TEST_SETTLE_CHECK_MS) {
        motor_data_t pitch_data = {0};
        motor_data_t yaw_data = {0};

        motor_set_angle(MOTOR0, target.pitch_deg);
        motor_set_angle(MOTOR1, target.yaw_deg);
        osDelay(DRAW_TEST_SETTLE_CHECK_MS);

        motor_get_data(MOTOR0, &pitch_data);
        motor_get_data(MOTOR1, &yaw_data);

        float pitch_error = draw_test_angle_error_deg(pitch_data.angle, target.pitch_deg);
        float yaw_error = draw_test_angle_error_deg(yaw_data.angle, target.yaw_deg);

        if (fabsf(pitch_error) <= DRAW_TEST_SETTLE_ERROR_DEG && fabsf(yaw_error) <= DRAW_TEST_SETTLE_ERROR_DEG) {
            return true;
        }
    }

    motor_data_t pitch_data = {0};
    motor_data_t yaw_data = {0};
    motor_get_data(MOTOR0, &pitch_data);
    motor_get_data(MOTOR1, &yaw_data);
    logq_w(DRAW_TEST_TAG, "settle timeout: target=(%.2f, %.2f), feedback=(%.2f, %.2f)",
        target.pitch_deg,
        target.yaw_deg,
        pitch_data.angle,
        yaw_data.angle);

    return false;
}

static bool draw_test_prepare(float x_ratio, float y_ratio)
{
    laser_off();
    (void)draw_move_to_xy(x_ratio, y_ratio);

    if (!draw_test_wait_target(x_ratio, y_ratio)) {
        return false;
    }

    return true;
}

static void draw_test_finish(void)
{
    laser_off();
    osDelay(DRAW_TEST_GAP_MS);
}

static void draw_test_shape(const char *name,
                            draw_shape_func_t draw_func,
                            const draw_shape_config_t *config,
                            float start_x,
                            float start_y)
{
    logq_i(DRAW_TEST_TAG, "Start shape: %s", name);
    if (!draw_test_prepare(start_x, start_y)) {
        logq_e(DRAW_TEST_TAG, "Shape prepare failed: %s", name);
        draw_test_finish();
        return;
    }

    laser_on();
    if (draw_func(config)) {
        logq_i(DRAW_TEST_TAG, "Shape done: %s", name);
    } else {
        logq_e(DRAW_TEST_TAG, "Shape failed: %s", name);
    }

    draw_test_finish();
}

static void draw_test_curve(const char *name,
                            draw_curve_func_t draw_func,
                            const draw_curve_config_t *config,
                            float start_y)
{
    logq_i(DRAW_TEST_TAG, "Start curve: %s", name);
    if (!draw_test_prepare(config->x_start, start_y)) {
        logq_e(DRAW_TEST_TAG, "Curve prepare failed: %s", name);
        draw_test_finish();
        return;
    }

    laser_on();
    if (draw_func(config)) {
        logq_i(DRAW_TEST_TAG, "Curve done: %s", name);
    } else {
        logq_e(DRAW_TEST_TAG, "Curve failed: %s", name);
    }

    draw_test_finish();
}

static bool draw_test_line_xy(float start_x, float start_y, float end_x, float end_y, uint16_t samples)
{
    return draw_line(start_x, start_y, end_x, end_y, samples, DRAW_TEST_AXIS_DELAY_MS);
}

static bool draw_test_vertical_yaw_probe(float x_ratio, float start_y, float end_y, uint16_t samples)
{
    for (uint16_t i = 0; i <= samples; i++) {
        float ratio = (float)i / (float)samples;
        float y_ratio = start_y + (end_y - start_y) * ratio;
        draw_point_t target = {0};

        if (!draw_map_xy(x_ratio, y_ratio, &target) || !draw_move_to_xy(x_ratio, y_ratio)) {
            return false;
        }

        osDelay(DRAW_TEST_AXIS_DELAY_MS);

        if ((i % DRAW_TEST_YAW_PROBE_INTERVAL) == 0U || i == samples) {
            motor_data_t yaw_data = {0};
            motor_get_data(MOTOR1, &yaw_data);
            logq_i(DRAW_TEST_TAG, "vertical yaw: i=%u y=%.2f target=%.2f feedback=%.2f error=%.2f",
                (unsigned int)i,
                y_ratio,
                target.yaw_deg,
                yaw_data.angle,
                draw_test_angle_error_deg(yaw_data.angle, target.yaw_deg));
        }
    }

    return true;
}

static void draw_test_axis(void)
{
    logq_i(DRAW_TEST_TAG, "Start axis: horizontal");
    if (draw_test_prepare(0.15f, 0.5f + DRAW_TEST_Y_OFFSET)) {
        laser_on();
        (void)draw_test_line_xy(0.15f, 0.5f + DRAW_TEST_Y_OFFSET, 0.85f, 0.5f + DRAW_TEST_Y_OFFSET, 80U);
    }
    draw_test_finish();

    logq_i(DRAW_TEST_TAG, "Start axis: vertical yaw probe");
    if (draw_test_prepare(0.5f, 0.15f + DRAW_TEST_Y_OFFSET)) {
        laser_on();
        (void)draw_test_vertical_yaw_probe(0.5f, 0.15f + DRAW_TEST_Y_OFFSET, 0.85f + DRAW_TEST_Y_OFFSET, 80U);
    }
    draw_test_finish();
}

static void draw_test_run_shapes(void)
{
    const draw_shape_config_t rectangle = {
        .center_x = 0.5f,
        .center_y = 0.5f + DRAW_TEST_Y_OFFSET,
        .width = 0.55f,
        .height = 0.45f,
        .samples = 100U,
        .step_delay_ms = DRAW_TEST_STEP_DELAY_MS,
    };
    const draw_shape_config_t circle = {
        .center_x = 0.5f,
        .center_y = 0.5f + DRAW_TEST_Y_OFFSET,
        .width = 0.50f,
        .height = 0.50f,
        .samples = 180U,
        .step_delay_ms = DRAW_TEST_FAST_STEP_DELAY_MS,
    };
    const draw_shape_config_t triangle = {
        .center_x = 0.5f,
        .center_y = 0.5f + DRAW_TEST_Y_OFFSET,
        .width = 0.55f,
        .height = 0.50f,
        .samples = 100U,
        .step_delay_ms = DRAW_TEST_STEP_DELAY_MS,
    };
    const draw_shape_config_t heart = {
        .center_x = 0.5f,
        .center_y = 0.52f + DRAW_TEST_Y_OFFSET,
        .width = 0.55f,
        .height = 0.55f,
        .samples = 180U,
        .step_delay_ms = DRAW_TEST_FAST_STEP_DELAY_MS,
    };
    const draw_shape_config_t pentagram = {
        .center_x = 0.5f,
        .center_y = 0.5f + DRAW_TEST_Y_OFFSET,
        .width = 0.55f,
        .height = 0.55f,
        .samples = 50U,
        .step_delay_ms = DRAW_TEST_STEP_DELAY_MS,
    };
    const draw_curve_config_t curve = {
        .x_start = 0.12f,
        .x_end = 0.88f,
        .y_center = 0.5f + DRAW_TEST_Y_OFFSET,
        .amplitude = 0.22f,
        .cycles = 1.0f,
        .samples = 180U,
        .step_delay_ms = DRAW_TEST_FAST_STEP_DELAY_MS,
    };
    const draw_curve_config_t tangent = {
        .x_start = 0.12f,
        .x_end = 0.88f,
        .y_center = 0.5f + DRAW_TEST_Y_OFFSET,
        .amplitude = 0.26f,
        .cycles = 1.0f,
        .samples = 180U,
        .step_delay_ms = DRAW_TEST_FAST_STEP_DELAY_MS,
    };
    float y_scale = draw_get_y_aspect_scale();

    // draw_test_axis();
    // draw_test_shape("rectangle", draw_rectangle, &rectangle,
    //     rectangle.center_x - rectangle.width * 0.5f,
    //     rectangle.center_y - rectangle.height * 0.5f * y_scale);
    // draw_test_shape("circle", draw_circle, &circle,
    //     circle.center_x + circle.width * 0.5f,
    //     circle.center_y);
    // draw_test_shape("triangle", draw_triangle, &triangle,
    //     triangle.center_x,
    //     triangle.center_y - triangle.height * 0.5f * y_scale);
    // draw_test_shape("heart", draw_heart, &heart,
    //     heart.center_x,
    //     heart.center_y - 0.233333f * heart.height * y_scale);
    // draw_test_shape("pentagram", draw_pentagram, &pentagram,
    //     pentagram.center_x,
    //     pentagram.center_y - pentagram.height * 0.5f * y_scale);
    // draw_test_curve("sine", draw_sine, &curve, curve.y_center);
    // draw_test_curve("cosine", draw_cosine, &curve, curve.y_center - curve.amplitude * y_scale);
    draw_test_curve("tangent", draw_tangent, &tangent, tangent.y_center + tangent.amplitude * y_scale);

    (void)rectangle;
    (void)circle;
    (void)triangle;
    (void)heart;
    (void)pentagram;
    (void)curve;
    (void)y_scale;
    (void)draw_test_axis;
    (void)draw_test_shape;
}

void app_draw_test_init(void)
{
    draw_init();
    laser_off();
    motor_enable(MOTOR0);
    motor_enable(MOTOR1);
    osDelay(DRAW_TEST_SETTLE_MS);
    draw_test_log_map();
    osDelay(DRAW_TEST_SETTLE_MS);
}

void app_draw_test_run_once(void)
{
    logq_i(DRAW_TEST_TAG, "Draw test begin");
    draw_test_run_shapes();
    (void)draw_move_to_xy(0.5f, 0.5f);
    draw_test_log_motor();
    logq_i(DRAW_TEST_TAG, "Draw test end");
}
