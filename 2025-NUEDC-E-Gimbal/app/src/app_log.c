#include "app_log.h"
#include "log.h"
#include "soft_uart.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    log_level_t level;
    char tag[APP_LOG_TAG_SIZE];
    char message[APP_LOG_MESSAGE_SIZE];
} log_msg_t;

static osMessageQueueId_t log_queue_handle = NULL;

static bool log_queue_push(const log_msg_t *msg)
{
    if (log_queue_handle == NULL)
    {
        return false;
    }

    osStatus_t status = osMessageQueuePut(log_queue_handle, msg, 0, 0);
    return (status == osOK);
}

__weak void log_output_handler(const char *str, uint16_t len)
{
    if (str == NULL || len == 0) {
        return;
    }
    HAL_SOFT_UART_Transmit(&hsuart, (uint8_t *)str, len, HAL_MAX_DELAY);
}

void app_log_init(void)
{
    log_init(log_output_handler);
    log_set_level(LOG_LEVEL_DEBUG);

    log_queue_handle = osMessageQueueNew(16, sizeof(log_msg_t), NULL);
}

void app_log_process(void)
{
    if (log_queue_handle == NULL)
    {
        return;
    }

    log_msg_t msg;
    while (osMessageQueueGet(log_queue_handle, &msg, NULL, 0) == osOK)
    {
        log_write(msg.level, msg.tag, "%s", msg.message);
    }
}

void logq_write(log_level_t level, const char *tag, const char *fmt, ...)
{
    log_msg_t msg = {.level = level};

    strncpy(msg.tag, tag, sizeof(msg.tag) - 1);
    msg.tag[sizeof(msg.tag) - 1] = '\0';

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.message, sizeof(msg.message), fmt, args);
    va_end(args);

    log_queue_push(&msg);
}


void log_task(void *argument)
{
    osDelay(100);
    UNUSED(argument);

    app_log_init();

    while (1)
    {
        app_log_process();
        osDelay(APP_LOG_INTERVAL_MS);
    }
}