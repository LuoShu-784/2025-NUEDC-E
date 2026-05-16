#include "main.h"
#include "cmsis_os.h"
#include "app_log.h"
#include "soft_uart.h"
#include "led.h"
#include <stdint.h>

#define APP_LOG_TEST_PERIOD_MS          1000U
#define APP_LOG_TEST_POLL_MS            10U

static uint32_t s_tick = 0;
static uint32_t s_seq = 0;

static void app_log_test_emit(uint32_t now)
{
    logq_d("APPLOG", "debug seq=%lu tick=%lu", (unsigned long)s_seq, (unsigned long)now);
    logq_i("APPLOG", "info  seq=%lu uart_rx=%u", (unsigned long)s_seq, (unsigned)HAL_SOFT_UART_GetRxAvailable(&hsuart));
    logq_w("APPLOG", "warn  seq=%lu tx_free=%u", (unsigned long)s_seq, (unsigned)HAL_SOFT_UART_GetTxAvailable(&hsuart));
    logq_e("APPLOG", "error seq=%lu led_r=%u", (unsigned long)s_seq, (unsigned)led_get_state(LED_R));

    logq_i("LONGTAG1234567890", "long message test: %lu ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz", (unsigned long)s_seq);
    s_seq++;
}

void test_task(void *argument)
{
    UNUSED(argument);
    osDelay(100);

    led_off_all();
    app_log_init();

    logq_i("APPLOG", "app_log test start");
    logq_i("APPLOG", "soft uart tx=PB14 rx=PB15 baud=9600 8N1");
    logq_w("APPLOG", "connect USB-TTL to PB14 for TX monitor");
    logq_w("APPLOG", "connect external TX to PB15 to test RX interrupt path");
    logq_e("APPLOG", "error line test for color and priority");

    s_tick = HAL_GetTick();

    while (1)
    {
        uint32_t now = HAL_GetTick();

        app_log_process();

        if ((uint32_t)(now - s_tick) >= APP_LOG_TEST_PERIOD_MS)
        {
            s_tick += APP_LOG_TEST_PERIOD_MS;
            led_toggle(LED_R);
            app_log_test_emit(now);
        }

        osDelay(APP_LOG_TEST_POLL_MS);
    }
}
