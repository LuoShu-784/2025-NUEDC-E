#ifndef __LED_H_
#define __LED_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stm32f4xx_hal.h"
#include "drivers.h"
#include "main.h"
#include <stdbool.h>

#ifndef LED_NUM
#define LED_NUM   3U
#endif

#ifndef LED_PIN_STATE
#define LED_PIN_STATE   GPIO_PIN_RESET
#endif

#if (LED_NUM < 1U)
#error "LED_NUM must be greater than 0."
#endif

#if (LED_PIN_STATE != GPIO_PIN_RESET) && (LED_PIN_STATE != GPIO_PIN_SET)
#error "LED_PIN_STATE must be GPIO_PIN_RESET or GPIO_PIN_SET."
#endif

#define LED_GPIO_CONFIG_TABLE           \
    {LED_R_GPIO_Port, LED_R_Pin},       \
    {LED_G_GPIO_Port, LED_G_Pin},       \
    {LED_B_GPIO_Port, LED_B_Pin}        \

typedef enum 
{
    LED_R,
    LED_G,
    LED_B,
} led_id_t;

typedef enum
{
    LED_OFF,
    LED_ON
} led_state_t;

void led_on(uint8_t id);
void led_off(uint8_t id);
void led_toggle(uint8_t id);
void led_set_state(uint8_t id, led_state_t state);
led_state_t led_get_state(uint8_t id);

static inline void led_r(led_state_t state)
{
    led_set_state(LED_R, state);
}

static inline void led_g(led_state_t state)
{
    led_set_state(LED_G, state);
}

static inline void led_b(led_state_t state)
{
    led_set_state(LED_B, state);
}

static inline void led_off_all(void)
{
    led_set_state(LED_R, LED_OFF);
    led_set_state(LED_G, LED_OFF);
    led_set_state(LED_B, LED_OFF);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LED_H_ */
