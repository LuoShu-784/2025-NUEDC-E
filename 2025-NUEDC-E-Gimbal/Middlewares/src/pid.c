#include "pid.h"

static float limit_abs(float value, float limit)
{
    if (limit < 0.0f) {
        limit = -limit;
    }

    if (value > limit) {
        return limit;
    }

    if (value < -limit) {
        return -limit;
    }

    return value;
}

void pid_init(pid_t *pid, float kp, float ki, float kd, float output_limit, float integral_limit)
{
    if (pid == 0) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->output_limit = output_limit;
    pid->integral_limit = integral_limit;
    pid_reset(pid);
}

void pid_reset(pid_t *pid)
{
    if (pid == 0) {
        return;
    }

    pid->integral = 0.0f;
    pid->last_error = 0.0f;
}

void pid_set_param(pid_t *pid, float kp, float ki, float kd)
{
    if (pid == 0) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void pid_set_limit(pid_t *pid, float output_limit, float integral_limit)
{
    if (pid == 0) {
        return;
    }

    pid->output_limit = output_limit;
    pid->integral_limit = integral_limit;
    pid->integral = limit_abs(pid->integral, pid->integral_limit);
}

float pid_calc(pid_t *pid, float target, float feedback)
{
    return pid_calc_error(pid, target - feedback);
}

float pid_calc_error(pid_t *pid, float error)
{
    if (pid == 0) {
        return 0.0f;
    }

    pid->integral += error;
    pid->integral = limit_abs(pid->integral, pid->integral_limit);

    float output = pid->kp * error
                 + pid->ki * pid->integral
                 + pid->kd * (error - pid->last_error);

    pid->last_error = error;
    return limit_abs(output, pid->output_limit);
}
