#include "app_led.h"
#include "led.h"

static osEventFlagsId_t s_event = NULL;
static osTimerId_t s_timer = NULL;
static uint8_t s_color = 0;

static void set_led_color(uint8_t color_mask)
{
    led_set_state(LED_R, (color_mask & LED_RED)   ? 1 : 0);
    led_set_state(LED_G, (color_mask & LED_GREEN) ? 1 : 0);
    led_set_state(LED_B, (color_mask & LED_BLUE)  ? 1 : 0);
}

static void timer_callback(void *argument)
{
    static bool toggle = false;

    if (toggle) 
    {
        set_led_color(s_color);
    } 
    else 
    {
        led_off_all();
    }

    toggle = !toggle;
}

void app_led_ctrl(uint8_t color, uint8_t mode)
{
    osEventFlagsSet(s_event, color | mode);
}

void led_task(void *argument)
{
    osDelay(100);
    UNUSED(argument);

    led_off_all();

    s_event = osEventFlagsNew(NULL);
    s_timer = osTimerNew(timer_callback, osTimerPeriodic, NULL, NULL);

    if (s_event == NULL || s_timer == NULL) 
    {
        led_on(LED_R);
        while (1) osDelay(1000);
        return;
    }

    uint32_t flags = 0;

    while (1)
    {
        flags = osEventFlagsWait(s_event, LED_EVT_ALL, osFlagsWaitAny, osWaitForever);

        if (flags & osFlagsError) continue;
        
        osTimerStop(s_timer);
        if (flags & LED_COLOR_MASK) {
            s_color = flags & LED_COLOR_MASK;
        }

        if (flags & LED_MOD_OFF) {
            led_off_all();
        } else if (flags & LED_MOD_ON) {
            set_led_color(s_color);
        } else if (flags & LED_MOD_BLINK_NORMAL) {
            osTimerStart(s_timer, LED_BLINK_NORMAL_MS);
        } else if (flags & LED_MOD_BLINK_FAST) {
            osTimerStart(s_timer, LED_BLINK_FAST_MS);
        } else if (flags & LED_MOD_BLINK_SLOW) {
            osTimerStart(s_timer, LED_BLINK_SLOW_MS);
        }
        osDelay(1000);
    }
}
