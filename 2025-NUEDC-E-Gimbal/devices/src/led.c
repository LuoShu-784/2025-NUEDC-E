#include "led.h"
#include "main.h"
#include <stdint.h>

static gpio_t led_gpio[LED_NUM] = {
    LED_GPIO_CONFIG_TABLE
};

static inline void led_write(uint8_t id, led_state_t state)
{
    if (id >= LED_NUM)
    {
        return;
    }

    HAL_GPIO_WritePin((GPIO_TypeDef *)led_gpio[id].port,
                      led_gpio[id].pin,
                      (state == LED_ON) ? !LED_PIN_STATE : LED_PIN_STATE);
}

static inline led_state_t led_read(uint8_t id)
{
    GPIO_PinState pin_state;

    if (id >= LED_NUM)
    {
        return LED_OFF;
    }

    pin_state = HAL_GPIO_ReadPin((GPIO_TypeDef *)led_gpio[id].port, led_gpio[id].pin);

    return (pin_state == LED_PIN_STATE) ? LED_OFF : LED_ON;
}

void led_on(uint8_t id)
{
    led_write(id, LED_ON);
}

void led_off(uint8_t id)
{
    led_write(id, LED_OFF);
}

void led_toggle(uint8_t id)
{
    if (id >= LED_NUM)
    {
        return;
    }

    HAL_GPIO_TogglePin((GPIO_TypeDef *)led_gpio[id].port, led_gpio[id].pin);
}

void led_set_state(uint8_t id, led_state_t state)
{
    led_write(id, state);
}

led_state_t led_get_state(uint8_t id)
{
    return led_read(id);
}

