#ifndef __APP_MOTOR_H_
#define __APP_MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <stdbool.h>

#include "main.h"

#define MOTOR0      0
#define MOTOR1      1

typedef struct
{
    bool enabled;
    float speed;   // 转速，单位rpm
    float angle;   // 角度，单位度
    float current; // 电流，单位A
} motor_data_t;

void motor_init(void);
void motor_get_data(uint8_t id, motor_data_t *data);
void motor_enable(uint8_t id);
void motor_disable(uint8_t id);
void motor_set_speed(uint8_t id, float speed);
void motor_set_low_speed(uint8_t id, float speed);
void motor_set_angle(uint8_t id, float angle_deg);
void motor_irq_handler(CAN_HandleTypeDef *hcanx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_MOTOR_H_ */