#ifndef __DRIVERS_H_
#define __DRIVERS_H_

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stm32f4xx_hal.h"

typedef struct
{
    void* port;
    uint16_t pin;
} gpio_t;

#include "soft_uart.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_H_ */