#ifndef __LASER_H_
#define __LASER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stm32f4xx_hal.h"
#include "drivers.h"
#include "main.h"
#include <stdbool.h>


#ifndef LASER_PIN_STATE
#define LASER_PIN_STATE   GPIO_PIN_RESET
#endif

static inline laser_on(void)
{
    HAL_GPIO_WritePin(LASER_GPIO_Port, LASER_Pin, LASER_PIN_STATE);
}

static inline laser_off(void)
{
    HAL_GPIO_WritePin(LASER_GPIO_Port, LASER_Pin, !LASER_PIN_STATE);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LASER_H_ */