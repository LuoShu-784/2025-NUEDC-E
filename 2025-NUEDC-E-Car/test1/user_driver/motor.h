#ifndef __MOTOR_H
#define __MOTOR_H

#include "ti_msp_dl_config.h"

extern volatile int32_t left_encoder_count;
extern volatile int32_t right_encoder_count;

extern volatile int current_speed_L;
extern volatile int current_speed_R;
extern volatile int target_speed_L;
extern volatile int target_speed_R;

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;     // 误差累加值
    int last_error;     // 上次误差
    float max_integral; // I的最大值
    float max_output;   // PID最大输出值
} PID_TypeDef;

void Motor_Left_Set(int32_t duty, int8_t dir);
void Motor_Right_Set(int32_t duty, int8_t dir);

void Motor_Control_Loop(void); 
void Car_SetSpeed(int speed_L, int speed_R); 

#endif