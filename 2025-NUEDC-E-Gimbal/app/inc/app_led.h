#ifndef __APP_LED_H_
#define __APP_LED_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "main.h"
#include "cmsis_os2.h"

#define APP_LED_INTERVAL_MS       500U

#define LED_RED             (1U << 0)
#define LED_GREEN           (1U << 1)
#define LED_BLUE            (1U << 2)
#define LED_WHITE           (LED_RED | LED_GREEN | LED_BLUE)
#define LED_COLOR_MASK      (0x07U)

#define LED_MOD_OFF             (1U << 3)
#define LED_MOD_ON              (1U << 4)
#define LED_MOD_BLINK_NORMAL    (1U << 5)
#define LED_MOD_BLINK_FAST      (1U << 6)
#define LED_MOD_BLINK_SLOW      (1U << 7)
#define LED_MOD_MASK            (0xF8U)

#define LED_EVT_ALL             (0xFFU)

#define LED_BLINK_NORMAL_MS     (500U)
#define LED_BLINK_FAST_MS       (150U)
#define LED_BLINK_SLOW_MS       (900U)

void led_ctrl(uint8_t color, uint8_t mode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_LED_H_ */
