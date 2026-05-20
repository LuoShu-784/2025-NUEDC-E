/**
 * @file app_draw.c
 * @brief 画图区域映射模块实现
 */

#include "app_draw.h"
#include "app_motor.h"
#include "cmsis_os2.h"

#include <math.h>
#include <stdint.h>

#define DRAW_PITCH_MOTOR_ID         MOTOR0
#define DRAW_YAW_MOTOR_ID           MOTOR1
#define DRAW_PI                     3.14159265358979323846f
#define DRAW_TWO_PI                 (2.0f * DRAW_PI)
#define DRAW_MAX_INPUT_ABS          1000.0f
#define DRAW_MIN_POLYGON_SAMPLES    1U
#define DRAW_MIN_CURVE_SAMPLES      2U
#define DRAW_TAN_LIMIT              1.0f
#define DRAW_MAX_INTERP_ANGLE_DEG   0.08f
#define DRAW_MAX_INTERP_STEPS       24U
#define DRAW_CMD_DEADBAND_DEG       0.005f
#define DRAW_ASPECT_SCALE_MIN       0.5f
#define DRAW_ASPECT_SCALE_MAX       2.5f
#define DRAW_ASPECT_SCALE_GAIN      0.6f
#define DRAW_TAN_PHASE_MAX          (DRAW_PI * 0.40f)

typedef struct {
    float x;
    float y;
} draw_xy_t;

typedef enum {
    DRAW_CURVE_SINE = 0,
    DRAW_CURVE_COSINE,
    DRAW_CURVE_TANGENT,
} draw_curve_type_t;

static const draw_region_t default_region = {
    .points = {
        [DRAW_POINT_LEFT_TOP] = {.pitch_deg = -16.2f, .yaw_deg = 6.9f},
        [DRAW_POINT_RIGHT_TOP] = {.pitch_deg = -16.2f, .yaw_deg = -7.2f},
        [DRAW_POINT_RIGHT_BOTTOM] = {.pitch_deg = -7.2f, .yaw_deg = -7.3f},
        [DRAW_POINT_LEFT_BOTTOM] = {.pitch_deg = -7.2f, .yaw_deg = 6.9f},
        [DRAW_POINT_CENTER] = {.pitch_deg = -113.2f, .yaw_deg = -174.9f},
    },
};

static draw_region_t current_region = {
    .points = {
        [DRAW_POINT_LEFT_TOP] = {.pitch_deg = -16.2f, .yaw_deg = 6.9f},
        [DRAW_POINT_RIGHT_TOP] = {.pitch_deg = -16.2f, .yaw_deg = -7.2f},
        [DRAW_POINT_RIGHT_BOTTOM] = {.pitch_deg = -7.2f, .yaw_deg = -7.3f},
        [DRAW_POINT_LEFT_BOTTOM] = {.pitch_deg = -7.2f, .yaw_deg = 6.9f},
        [DRAW_POINT_CENTER] = {.pitch_deg = -113.2f, .yaw_deg = -174.9f},
    },
};

static bool last_xy_valid = false;
static draw_xy_t last_xy = {0};
static bool last_cmd_valid = false;
static draw_point_t last_cmd_point = {0};

static bool point_id_valid(draw_point_id_t id)
{
    return (id >= DRAW_POINT_LEFT_TOP) && (id < DRAW_POINT_COUNT);
}

static bool finite_input_valid(float value)
{
    return (value == value) && (value >= -DRAW_MAX_INPUT_ABS) && (value <= DRAW_MAX_INPUT_ABS);
}

static bool angle_valid(float angle_deg)
{
    return (angle_deg == angle_deg) && (angle_deg >= -180.0f) && (angle_deg <= 180.0f);
}

static bool point_valid(draw_point_t point)
{
    return angle_valid(point.pitch_deg) && angle_valid(point.yaw_deg);
}

static bool region_valid(const draw_region_t *region)
{
    if (region == NULL) {
        return false;
    }

    for (uint32_t i = 0; i < (uint32_t)DRAW_POINT_COUNT; i++) {
        if (!point_valid(region->points[i])) {
            return false;
        }
    }

    return true;
}

static bool shape_config_valid(const draw_shape_config_t *config)
{
    if (config == NULL) {
        return false;
    }

    return finite_input_valid(config->center_x)
        && finite_input_valid(config->center_y)
        && finite_input_valid(config->width)
        && finite_input_valid(config->height)
        && config->width > 0.0f
        && config->height > 0.0f
        && config->samples > 0U;
}

static bool curve_config_valid(const draw_curve_config_t *config)
{
    if (config == NULL) {
        return false;
    }

    return finite_input_valid(config->x_start)
        && finite_input_valid(config->x_end)
        && finite_input_valid(config->y_center)
        && finite_input_valid(config->amplitude)
        && finite_input_valid(config->cycles)
        && config->x_start != config->x_end
        && config->amplitude >= 0.0f
        && config->cycles > 0.0f
        && config->samples > 0U;
}

static float clamp_ratio(float ratio)
{
    if (ratio < 0.0f) {
        return 0.0f;
    }

    if (ratio > 1.0f) {
        return 1.0f;
    }

    return ratio;
}

static float clamp_float(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static uint16_t sample_count(uint16_t samples, uint16_t min_samples)
{
    if (samples < min_samples) {
        return min_samples;
    }

    return samples;
}

static float normalize_angle_deg(float angle_deg)
{
    while (angle_deg > 180.0f) {
        angle_deg -= 360.0f;
    }

    while (angle_deg < -180.0f) {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

static float lerp_float(float start, float end, float ratio)
{
    return start + (end - start) * ratio;
}

static float lerp_angle_deg(float start_deg, float end_deg, float ratio)
{
    float delta_deg = normalize_angle_deg(end_deg - start_deg);
    return normalize_angle_deg(start_deg + delta_deg * ratio);
}

static float angle_abs_delta_deg(float start_deg, float end_deg)
{
    return fabsf(normalize_angle_deg(end_deg - start_deg));
}

static bool angle_changed(float previous_deg, float next_deg)
{
    return angle_abs_delta_deg(previous_deg, next_deg) >= DRAW_CMD_DEADBAND_DEG;
}

static draw_point_t draw_map_raw_xy(float x_ratio, float y_ratio)
{
    float top_pitch = lerp_angle_deg(current_region.points[DRAW_POINT_LEFT_TOP].pitch_deg,
                                     current_region.points[DRAW_POINT_RIGHT_TOP].pitch_deg,
                                     0.5f);
    float bottom_pitch = lerp_angle_deg(current_region.points[DRAW_POINT_LEFT_BOTTOM].pitch_deg,
                                        current_region.points[DRAW_POINT_RIGHT_BOTTOM].pitch_deg,
                                        0.5f);
    float left_yaw = lerp_angle_deg(current_region.points[DRAW_POINT_LEFT_TOP].yaw_deg,
                                    current_region.points[DRAW_POINT_LEFT_BOTTOM].yaw_deg,
                                    0.5f);
    float right_yaw = lerp_angle_deg(current_region.points[DRAW_POINT_RIGHT_TOP].yaw_deg,
                                     current_region.points[DRAW_POINT_RIGHT_BOTTOM].yaw_deg,
                                     0.5f);
    draw_point_t point = {
        .pitch_deg = lerp_angle_deg(top_pitch, bottom_pitch, y_ratio),
        .yaw_deg = lerp_angle_deg(left_yaw, right_yaw, x_ratio),
    };

    return point;
}

static float draw_region_y_aspect_scale(void)
{
    draw_point_t left = draw_map_raw_xy(0.0f, 0.5f);
    draw_point_t right = draw_map_raw_xy(1.0f, 0.5f);
    draw_point_t top = draw_map_raw_xy(0.5f, 0.0f);
    draw_point_t bottom = draw_map_raw_xy(0.5f, 1.0f);
    float yaw_span = angle_abs_delta_deg(left.yaw_deg, right.yaw_deg);
    float pitch_span = angle_abs_delta_deg(top.pitch_deg, bottom.pitch_deg);

    if (pitch_span <= 0.001f) {
        return 1.0f;
    }

    float raw_scale = clamp_float(yaw_span / pitch_span, DRAW_ASPECT_SCALE_MIN, DRAW_ASPECT_SCALE_MAX);
    return 1.0f + (raw_scale - 1.0f) * DRAW_ASPECT_SCALE_GAIN;
}

static float draw_aspect_y(float center_y, float y)
{
    return center_y + (y - center_y) * draw_region_y_aspect_scale();
}

static uint16_t interp_step_count(draw_xy_t start, draw_xy_t end)
{
    draw_point_t start_point = draw_map_raw_xy(start.x, start.y);
    draw_point_t end_point = draw_map_raw_xy(end.x, end.y);
    float pitch_delta = angle_abs_delta_deg(start_point.pitch_deg, end_point.pitch_deg);
    float yaw_delta = angle_abs_delta_deg(start_point.yaw_deg, end_point.yaw_deg);
    float max_delta = (pitch_delta > yaw_delta) ? pitch_delta : yaw_delta;
    uint16_t steps = (uint16_t)(max_delta / DRAW_MAX_INTERP_ANGLE_DEG) + 1U;

    if (steps > DRAW_MAX_INTERP_STEPS) {
        return DRAW_MAX_INTERP_STEPS;
    }

    return steps;
}

static bool draw_output_xy(float x_ratio, float y_ratio)
{
    draw_point_t point = {0};

    x_ratio = clamp_ratio(x_ratio);
    y_ratio = clamp_ratio(y_ratio);

    if (!draw_map_xy(x_ratio, y_ratio, &point) || !draw_move_to(point)) {
        return false;
    }

    last_xy = (draw_xy_t){.x = x_ratio, .y = y_ratio};
    last_xy_valid = true;
    return true;
}

static bool draw_step_xy(float x_ratio, float y_ratio, uint32_t step_delay_ms)
{
    draw_xy_t target = {
        .x = clamp_ratio(x_ratio),
        .y = clamp_ratio(y_ratio),
    };

    if (!last_xy_valid) {
        if (!draw_output_xy(target.x, target.y)) {
            return false;
        }

        if (step_delay_ms > 0U) {
            osDelay(step_delay_ms);
        }

        return true;
    }

    draw_xy_t start = last_xy;
    uint16_t steps = interp_step_count(start, target);

    for (uint16_t i = 1; i <= steps; i++) {
        float ratio = (float)i / (float)steps;
        float x = lerp_float(start.x, target.x, ratio);
        float y = lerp_float(start.y, target.y, ratio);

        if (!draw_output_xy(x, y)) {
            return false;
        }

        if (step_delay_ms > 0U) {
            osDelay(step_delay_ms);
        }
    }

    return true;
}

static bool draw_segment(draw_xy_t start, draw_xy_t end, uint16_t samples, uint32_t step_delay_ms)
{
    uint16_t steps = sample_count(samples, DRAW_MIN_POLYGON_SAMPLES);
    uint16_t angle_steps = interp_step_count(start, end);

    if (steps < angle_steps) {
        steps = angle_steps;
    }

    for (uint16_t i = 0; i <= steps; i++) {
        float ratio = (float)i / (float)steps;
        float x = lerp_float(start.x, end.x, ratio);
        float y = lerp_float(start.y, end.y, ratio);

        if (!draw_output_xy(x, y)) {
            return false;
        }

        if (step_delay_ms > 0U) {
            osDelay(step_delay_ms);
        }
    }

    return true;
}

static bool draw_closed_polyline(const draw_xy_t *points, uint16_t count, uint16_t samples_per_edge, uint32_t step_delay_ms)
{
    if (points == NULL || count < 2U) {
        return false;
    }

    for (uint16_t i = 0; i < count; i++) {
        draw_xy_t start = points[i];
        draw_xy_t end = points[(uint16_t)((i + 1U) % count)];

        if (!draw_segment(start, end, samples_per_edge, step_delay_ms)) {
            return false;
        }
    }

    return draw_step_xy(points[0].x, points[0].y, step_delay_ms);
}

static bool draw_trig_curve(const draw_curve_config_t *config, draw_curve_type_t type)
{
    if (!curve_config_valid(config)) {
        return false;
    }

    uint16_t samples = sample_count(config->samples, DRAW_MIN_CURVE_SAMPLES);

    for (uint16_t i = 0; i <= samples; i++) {
        float ratio = (float)i / (float)samples;
        float phase = DRAW_TWO_PI * config->cycles * ratio;
        float value = 0.0f;

        if (type == DRAW_CURVE_SINE) {
            value = sinf(phase);
        } else if (type == DRAW_CURVE_COSINE) {
            value = cosf(phase);
        } else {
            float steepness = clamp_float(config->cycles, 0.1f, 1.0f);
            float phase_limit = DRAW_TAN_PHASE_MAX * steepness;
            phase = phase_limit * (2.0f * ratio - 1.0f);
            value = tanf(phase) / tanf(phase_limit);
        }

        float x = lerp_float(config->x_start, config->x_end, ratio);
        float y = config->y_center - config->amplitude * value * draw_region_y_aspect_scale();

        if (!draw_step_xy(x, y, config->step_delay_ms)) {
            return false;
        }
    }

    return true;
}

void draw_init(void)
{
    draw_reset_region();
}

void draw_reset_region(void)
{
    current_region = default_region;
    last_xy_valid = false;
    last_cmd_valid = false;
}

bool draw_set_region(const draw_region_t *region)
{
    if (!region_valid(region)) {
        return false;
    }

    current_region = *region;
    last_xy_valid = false;
    last_cmd_valid = false;
    return true;
}

void draw_get_region(draw_region_t *region)
{
    if (region == NULL) {
        return;
    }

    *region = current_region;
}

bool draw_set_point(draw_point_id_t id, draw_point_t point)
{
    if (!point_id_valid(id) || !point_valid(point)) {
        return false;
    }

    current_region.points[id] = point;
    last_xy_valid = false;
    last_cmd_valid = false;
    return true;
}

bool draw_get_point(draw_point_id_t id, draw_point_t *point)
{
    if (!point_id_valid(id) || point == NULL) {
        return false;
    }

    *point = current_region.points[id];
    return true;
}

bool draw_map_xy(float x_ratio, float y_ratio, draw_point_t *point)
{
    if (point == NULL || x_ratio != x_ratio || y_ratio != y_ratio) {
        return false;
    }

    x_ratio = clamp_ratio(x_ratio);
    y_ratio = clamp_ratio(y_ratio);

    *point = draw_map_raw_xy(x_ratio, y_ratio);
    return true;
}

float draw_get_y_aspect_scale(void)
{
    return draw_region_y_aspect_scale();
}

bool draw_move_to(draw_point_t point)
{
    if (!point_valid(point)) {
        return false;
    }

    if (!last_cmd_valid || angle_changed(last_cmd_point.pitch_deg, point.pitch_deg)) {
        motor_set_angle(DRAW_PITCH_MOTOR_ID, point.pitch_deg);
        last_cmd_point.pitch_deg = point.pitch_deg;
    }

    if (!last_cmd_valid || angle_changed(last_cmd_point.yaw_deg, point.yaw_deg)) {
        motor_set_angle(DRAW_YAW_MOTOR_ID, point.yaw_deg);
        last_cmd_point.yaw_deg = point.yaw_deg;
    }

    last_cmd_valid = true;
    last_xy_valid = false;
    return true;
}

bool draw_move_to_point(draw_point_id_t id)
{
    if (!point_id_valid(id)) {
        return false;
    }

    if (id == DRAW_POINT_CENTER) {
        return draw_move_to_xy(0.5f, 0.5f);
    }

    if (id == DRAW_POINT_LEFT_TOP) {
        return draw_move_to_xy(0.0f, 0.0f);
    }

    if (id == DRAW_POINT_RIGHT_TOP) {
        return draw_move_to_xy(1.0f, 0.0f);
    }

    if (id == DRAW_POINT_RIGHT_BOTTOM) {
        return draw_move_to_xy(1.0f, 1.0f);
    }

    return draw_move_to_xy(0.0f, 1.0f);
}

bool draw_move_to_xy(float x_ratio, float y_ratio)
{
    return draw_output_xy(x_ratio, y_ratio);
}

bool draw_line(float start_x, float start_y, float end_x, float end_y, uint16_t samples, uint32_t step_delay_ms)
{
    if (!finite_input_valid(start_x) || !finite_input_valid(start_y)
        || !finite_input_valid(end_x) || !finite_input_valid(end_y)
        || samples == 0U) {
        return false;
    }

    draw_xy_t start = {.x = start_x, .y = start_y};
    draw_xy_t end = {.x = end_x, .y = end_y};

    if (!draw_segment(start, end, samples, step_delay_ms)) {
        return false;
    }

    return draw_step_xy(end.x, end.y, step_delay_ms);
}

bool draw_rectangle(const draw_shape_config_t *config)
{
    if (!shape_config_valid(config)) {
        return false;
    }

    float half_width = config->width * 0.5f;
    float half_height = config->height * 0.5f * draw_region_y_aspect_scale();
    draw_xy_t points[4] = {
        {.x = config->center_x - half_width, .y = config->center_y - half_height},
        {.x = config->center_x + half_width, .y = config->center_y - half_height},
        {.x = config->center_x + half_width, .y = config->center_y + half_height},
        {.x = config->center_x - half_width, .y = config->center_y + half_height},
    };

    return draw_closed_polyline(points, 4U, config->samples, config->step_delay_ms);
}

bool draw_circle(const draw_shape_config_t *config)
{
    if (!shape_config_valid(config)) {
        return false;
    }

    uint16_t samples = sample_count(config->samples, 3U);
    float radius_x = config->width * 0.5f;
    float radius_y = config->height * 0.5f * draw_region_y_aspect_scale();

    for (uint16_t i = 0; i <= samples; i++) {
        float ratio = (float)i / (float)samples;
        float angle = DRAW_TWO_PI * ratio;
        float x = config->center_x + cosf(angle) * radius_x;
        float y = config->center_y - sinf(angle) * radius_y;

        if (!draw_step_xy(x, y, config->step_delay_ms)) {
            return false;
        }
    }

    return true;
}

bool draw_triangle(const draw_shape_config_t *config)
{
    if (!shape_config_valid(config)) {
        return false;
    }

    float half_width = config->width * 0.5f;
    float half_height = config->height * 0.5f * draw_region_y_aspect_scale();
    draw_xy_t points[3] = {
        {.x = config->center_x, .y = config->center_y - half_height},
        {.x = config->center_x + half_width, .y = config->center_y + half_height},
        {.x = config->center_x - half_width, .y = config->center_y + half_height},
    };

    return draw_closed_polyline(points, 3U, config->samples, config->step_delay_ms);
}

bool draw_heart(const draw_shape_config_t *config)
{
    if (!shape_config_valid(config)) {
        return false;
    }

    uint16_t samples = sample_count(config->samples, DRAW_MIN_CURVE_SAMPLES);

    for (uint16_t i = 0; i <= samples; i++) {
        float t = DRAW_TWO_PI * (float)i / (float)samples;
        float sin_t = sinf(t);
        float heart_x = 16.0f * sin_t * sin_t * sin_t;
        float heart_y = 13.0f * cosf(t) - 5.0f * cosf(2.0f * t) - 2.0f * cosf(3.0f * t) - cosf(4.0f * t);
        float x = config->center_x + (heart_x / 32.0f) * config->width;
        float y = config->center_y + (0.5f - ((heart_y + 17.0f) / 30.0f)) * config->height;
        y = draw_aspect_y(config->center_y, y);

        if (!draw_step_xy(x, y, config->step_delay_ms)) {
            return false;
        }
    }

    return true;
}

bool draw_pentagram(const draw_shape_config_t *config)
{
    if (!shape_config_valid(config)) {
        return false;
    }

    static const uint8_t order[5] = {0U, 2U, 4U, 1U, 3U};
    float outer_x = config->width * 0.5f;
    float outer_y = config->height * 0.5f * draw_region_y_aspect_scale();
    draw_xy_t outer[5] = {0};
    draw_xy_t star[5] = {0};

    for (uint16_t i = 0; i < 5U; i++) {
        float angle = DRAW_TWO_PI * (float)i / 5.0f;
        outer[i].x = config->center_x + sinf(angle) * outer_x;
        outer[i].y = config->center_y - cosf(angle) * outer_y;
    }

    for (uint16_t i = 0; i < 5U; i++) {
        star[i] = outer[order[i]];
    }

    return draw_closed_polyline(star, 5U, config->samples, config->step_delay_ms);
}

bool draw_sine(const draw_curve_config_t *config)
{
    return draw_trig_curve(config, DRAW_CURVE_SINE);
}

bool draw_cosine(const draw_curve_config_t *config)
{
    return draw_trig_curve(config, DRAW_CURVE_COSINE);
}

bool draw_tangent(const draw_curve_config_t *config)
{
    return draw_trig_curve(config, DRAW_CURVE_TANGENT);
}
