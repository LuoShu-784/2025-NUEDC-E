#ifndef __APP_MOTOR_H_
#define __APP_MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "main.h"

void motor_init(void);
void motor_task(void *argument);
void motor_irq_handler(CAN_HandleTypeDef *hcanx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_MOTOR_H_ */