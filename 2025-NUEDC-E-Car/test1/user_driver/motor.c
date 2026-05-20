#include "motor.h"
#include <stdbool.h> 

volatile int32_t left_encoder_count = 0;
volatile int32_t right_encoder_count = 0;

volatile int current_speed_L = 0;
volatile int current_speed_R = 0;
volatile int target_speed_L = 0;
volatile int target_speed_R = 0;

#define DEADZONE_PWM 800
#define K_FF 0.0f

PID_TypeDef pid_left  = {17.0f, 1.2f, 0.8f, 0.0f, 0, 2100.0f, 3000.0f}; 
PID_TypeDef pid_right = {14.0f, 1.0f, 0.8f, 0.0f, 0, 1800.0f, 3000.0f};

void GROUP1_IRQHandler(void) {
    uint32_t iidx = DL_GPIO_getPendingInterrupt(EECODER_PORT);
    uint32_t pin_states;
    bool a_state, b_state;

    switch (iidx) {
        case EECODER_left_A_IIDX:  
            pin_states = DL_GPIO_readPins(EECODER_PORT, EECODER_left_A_PIN | EECODER_left_B_PIN);
            a_state = (pin_states & EECODER_left_A_PIN) ? true : false;
            b_state = (pin_states & EECODER_left_B_PIN) ? true : false;
            if (a_state == b_state) left_encoder_count++; else left_encoder_count--;
            break;

        case EECODER_right_A_IIDX:
            pin_states = DL_GPIO_readPins(EECODER_PORT, EECODER_right_A_PIN | EECODER_right_B_PIN);
            a_state = (pin_states & EECODER_right_A_PIN) ? true : false;
            b_state = (pin_states & EECODER_right_B_PIN) ? true : false;
            if (a_state != b_state) right_encoder_count++; else right_encoder_count--;
            break;
            
        default: break;
    }
}

void Motor_Left_Set(int32_t duty, int8_t dir) {
    if (dir == 1) {
        DL_GPIO_setPins(MOTOR_AIN1_PORT, MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(MOTOR_AIN2_PORT, MOTOR_AIN2_PIN);
    } else if (dir == 0) {
        DL_GPIO_clearPins(MOTOR_AIN1_PORT, MOTOR_AIN1_PIN);
        DL_GPIO_setPins(MOTOR_AIN2_PORT, MOTOR_AIN2_PIN);
    } else {
        DL_GPIO_clearPins(MOTOR_AIN1_PORT, MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(MOTOR_AIN2_PORT, MOTOR_AIN2_PIN);
    }
    DL_TimerA_setCaptureCompareValue(PWM_0_INST, duty, DL_TIMER_CC_0_INDEX);
}

void Motor_Right_Set(int32_t duty, int8_t dir) {
    if (dir == 1) {
        DL_GPIO_setPins(MOTOR_AIN3_PORT, MOTOR_AIN3_PIN);
        DL_GPIO_clearPins(MOTOR_AIN4_PORT, MOTOR_AIN4_PIN);
    } else if (dir == 0) {
        DL_GPIO_clearPins(MOTOR_AIN3_PORT, MOTOR_AIN3_PIN);
        DL_GPIO_setPins(MOTOR_AIN4_PORT, MOTOR_AIN4_PIN);
    } else {
        DL_GPIO_clearPins(MOTOR_AIN3_PORT, MOTOR_AIN3_PIN);
        DL_GPIO_clearPins(MOTOR_AIN4_PORT, MOTOR_AIN4_PIN);
    }
    DL_TimerA_setCaptureCompareValue(PWM_0_INST, duty, DL_TIMER_CC_1_INDEX);
}

int pid_calc(PID_TypeDef *pid, int target, int current) {
    if (target == 0) {
        pid->integral = 0; 
        return 0;
    }

    int error = target - current;

    pid->integral += error;
    if (pid->integral > pid->max_integral) pid->integral = pid->max_integral;
    if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;

    int derivative = error - pid->last_error;
    pid->last_error = error;
    float ff = target * K_FF;
    float feedback = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);

    float output = ff + feedback;
    if (output > 0) output += DEADZONE_PWM;
    else if (output < 0) output -= DEADZONE_PWM;

    if (output > pid->max_output) output = pid->max_output;
    if (output < -pid->max_output) output = -pid->max_output;

    return (int)output;
}

// void Motor_Control_Loop(void) {
//     int current_L = left_encoder_count;
//     int current_R = right_encoder_count;
//     left_encoder_count = 0;
//     right_encoder_count = 0;
    
//     current_speed_L = current_L;
//     current_speed_R = current_R;

//     int pwm_L = pid_calc(&pid_left, target_speed_L, current_speed_L);
//     int pwm_R = pid_calc(&pid_right, target_speed_R, current_speed_R);

//     if (target_speed_L == 0) Motor_Left_Set(0, -1);
//     else if (pwm_L > 0) Motor_Left_Set(pwm_L, 1);
//     else Motor_Left_Set(-pwm_L, 0);

//     if (target_speed_R == 0) Motor_Right_Set(0, -1);
//     else if (pwm_R > 0) 
//     {
//         pwm_R = pwm_R - 180;
//         Motor_Right_Set(pwm_R, 1);
//     }
//     else{
//         pwm_R = pwm_R + 180;
//         Motor_Right_Set(-pwm_R, 0);
//     }
// }

// void Car_SetSpeed(int speed_L, int speed_R) {
//     target_speed_L = speed_L+2;
//     target_speed_R = speed_R;
// }

void Motor_Control_Loop(void) {
    int current_L = left_encoder_count;
    int current_R = right_encoder_count;
    left_encoder_count = 0;
    right_encoder_count = 0;
    current_speed_L = current_L;
    current_speed_R = current_R;
    int pwm_L = pid_calc(&pid_left, target_speed_L, current_speed_L);
    int pwm_R = pid_calc(&pid_right, target_speed_R, current_speed_R);

    static float sync_integral = 0;
    if (target_speed_L == target_speed_R && target_speed_L != 0) {

        int pulse_diff = current_speed_L - current_speed_R;
        sync_integral += pulse_diff;
        if (sync_integral > 300) sync_integral = 300;
        if (sync_integral < -300) sync_integral = -300;

        float sync_kp = 15.0f; 
        float sync_ki = 1.2f;
        int sync_comp = (int)(pulse_diff * sync_kp + sync_integral * sync_ki);
        pwm_L -= sync_comp;
        pwm_R += sync_comp;
    } else {
        sync_integral = 0; 
    }
    if (pwm_L > 2999) pwm_L = 2999;
    if (pwm_L < -2999) pwm_L = -2999;
    if (pwm_R > 2999) pwm_R = 2999;
    if (pwm_R < -2999) pwm_R = -2999;
    
    if (target_speed_L == 0) Motor_Left_Set(0, -1);
    else if (pwm_L > 0) Motor_Left_Set(pwm_L, 1);
    else Motor_Left_Set(-pwm_L, 0);

    if (target_speed_R == 0) Motor_Right_Set(0, -1);
    else if (pwm_R > 0) Motor_Right_Set(pwm_R, 1); 
    else Motor_Right_Set(-pwm_R, 0);             
}

void Car_SetSpeed(int speed_L, int speed_R) {
    if ((target_speed_L <= 0 && speed_L > 0) || (target_speed_L >= 0 && speed_L < 0)) {
        pid_left.integral = 0;
    }
    if ((target_speed_R <= 0 && speed_R > 0) || (target_speed_R >= 0 && speed_R < 0)) {
        pid_right.integral = 0;
    }
    if(speed_L==speed_R)
    {
        target_speed_L = speed_L;
        target_speed_R = speed_R;
    }
    else {
        target_speed_L = speed_L+2;
        target_speed_R = speed_R;
    }
}