#ifndef __APP_H_
#define __APP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stm32f4xx_hal.h"
#include "laser.h"
#include "soft_uart.h"

void init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_H_ */