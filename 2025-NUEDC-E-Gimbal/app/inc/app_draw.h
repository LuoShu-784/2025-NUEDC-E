/**
 * @file app_draw.h
 * @brief 画图区域映射模块
 */

#ifndef __APP_DRAW_H_
#define __APP_DRAW_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 画图区域关键点
 */
typedef enum {
    DRAW_POINT_LEFT_TOP = 0,
    DRAW_POINT_RIGHT_TOP,
    DRAW_POINT_RIGHT_BOTTOM,
    DRAW_POINT_LEFT_BOTTOM,
    DRAW_POINT_CENTER,
    DRAW_POINT_COUNT,
} draw_point_id_t;

/**
 * @brief 云台角度点，单位为度
 */
typedef struct {
    float pitch_deg; // 电机 0
    float yaw_deg;   // 电机 1
} draw_point_t;

/**
 * @brief 可配置画图区域
 */
typedef struct {
    draw_point_t points[DRAW_POINT_COUNT];
} draw_region_t;

/**
 * @brief 归一化区域内的图形参数
 */
typedef struct {
    float center_x;
    float center_y;
    float width;
    float height;
    uint16_t samples;
    uint32_t step_delay_ms;
} draw_shape_config_t;

/**
 * @brief 归一化区域内的函数曲线参数
 */
typedef struct {
    float x_start;
    float x_end;
    float y_center;
    float amplitude;
    float cycles;
    uint16_t samples;
    uint32_t step_delay_ms;
} draw_curve_config_t;

void draw_init(void);
void draw_reset_region(void);
bool draw_set_region(const draw_region_t *region);
void draw_get_region(draw_region_t *region);
bool draw_set_point(draw_point_id_t id, draw_point_t point);
bool draw_get_point(draw_point_id_t id, draw_point_t *point);
bool draw_map_xy(float x_ratio, float y_ratio, draw_point_t *point);
float draw_get_y_aspect_scale(void);
bool draw_move_to(draw_point_t point);
bool draw_move_to_point(draw_point_id_t id);
bool draw_move_to_xy(float x_ratio, float y_ratio);
bool draw_line(float start_x, float start_y, float end_x, float end_y, uint16_t samples, uint32_t step_delay_ms);
bool draw_rectangle(const draw_shape_config_t *config);
bool draw_circle(const draw_shape_config_t *config);
bool draw_triangle(const draw_shape_config_t *config);
bool draw_heart(const draw_shape_config_t *config);
bool draw_pentagram(const draw_shape_config_t *config);
bool draw_sine(const draw_curve_config_t *config);
bool draw_cosine(const draw_curve_config_t *config);
bool draw_tangent(const draw_curve_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_DRAW_H_ */
