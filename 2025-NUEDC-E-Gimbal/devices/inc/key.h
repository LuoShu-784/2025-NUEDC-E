#ifndef __KEY_H_
#define __KEY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "drivers.h"
#include "main.h"

#ifndef KEY_NUM
#define KEY_NUM                         1U
#endif

#define KEY_MODE_POLL                   0U
#define KEY_MODE_INTERRUPT              1U

#ifndef KEY_WORK_MODE
#define KEY_WORK_MODE                   KEY_MODE_INTERRUPT
#endif

#ifndef KEY_PRESS_STATE
#define KEY_PRESS_STATE                 GPIO_PIN_RESET
#endif

#ifndef KEY_CLICK_ENABLE
#define KEY_CLICK_ENABLE                1U
#endif

#ifndef KEY_DOUBLE_CLICK_ENABLE
#define KEY_DOUBLE_CLICK_ENABLE         1U
#endif

#ifndef KEY_LONG_PRESS_ENABLE
#define KEY_LONG_PRESS_ENABLE           1U
#endif

#ifndef KEY_DEBOUNCE_TIME_MS
#define KEY_DEBOUNCE_TIME_MS            20U
#endif

#ifndef KEY_DOUBLE_CLICK_TIME_MS
#define KEY_DOUBLE_CLICK_TIME_MS        450U
#endif

#ifndef KEY_LONG_PRESS_TIME_MS
#define KEY_LONG_PRESS_TIME_MS          1000U
#endif

#if (KEY_WORK_MODE != KEY_MODE_POLL) && (KEY_WORK_MODE != KEY_MODE_INTERRUPT)
#error "KEY_WORK_MODE must be KEY_MODE_POLL or KEY_MODE_INTERRUPT."
#endif

#if (KEY_PRESS_STATE != GPIO_PIN_RESET) && (KEY_PRESS_STATE != GPIO_PIN_SET)
#error "KEY_PRESS_STATE must be GPIO_PIN_RESET or GPIO_PIN_SET."
#endif

#if KEY_NUM == 1U
#define KEY_GPIO_CONFIG_TABLE           \
    {KEY_GPIO_Port, KEY_Pin}
#elif KEY_NUM == 2U
#define KEY_GPIO_CONFIG_TABLE           \
    {KEY_GPIO_Port, KEY_Pin}            
#endif

typedef enum
{
    KEY_RELEASED = 0,
    KEY_PRESSED
} key_state_t;

typedef enum
{
    KEY_EVENT_NONE         = 0x00U,
    KEY_EVENT_CLICK        = 0x01U,
    KEY_EVENT_DOUBLE_CLICK = 0x02U,
    KEY_EVENT_LONG_PRESS   = 0x04U
} key_event_t;

typedef void (*key_callback_t)(uint8_t id, key_event_t event);

void key_init(void);
void key_process(void);
void key_irq_handler(uint16_t gpio_pin);
key_state_t key_get_state(uint8_t id);
uint8_t key_get_events(uint8_t id);
void key_clear_events(uint8_t id, uint8_t event_mask);
void key_set_callback(key_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H_ */
