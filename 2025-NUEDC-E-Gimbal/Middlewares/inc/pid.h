#ifndef __PID_H_
#define __PID_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float last_error;
    float output_limit;
    float integral_limit;
} pid_t;

void pid_init(pid_t *pid, float kp, float ki, float kd, float output_limit, float integral_limit);
void pid_reset(pid_t *pid);
void pid_set_param(pid_t *pid, float kp, float ki, float kd);
void pid_set_limit(pid_t *pid, float output_limit, float integral_limit);
float pid_calc(pid_t *pid, float target, float feedback);
float pid_calc_error(pid_t *pid, float error);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PID_H_ */
