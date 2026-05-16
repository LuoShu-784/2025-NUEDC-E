/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log.c
  * @brief   Terminal color log system implementation
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* ANSI color codes */
#if LOG_ENABLE_COLOR
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_WHITE   "\033[37m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_RED     "\033[31m"
#else
#define ANSI_COLOR_RESET   ""
#define ANSI_COLOR_WHITE   ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_RED     ""
#endif

/* Log level strings */
static const char *level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

/* Log level colors */
static const char *level_colors[] = {
    ANSI_COLOR_WHITE,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_RED
};

/* Log configuration */
static log_output_fn_t log_output = NULL;
static log_level_t log_min_level = LOG_DEFAULT_LEVEL;

void log_init(log_output_fn_t output_fn)
{
    log_output = output_fn;
    log_min_level = LOG_DEFAULT_LEVEL;
}

void log_set_level(log_level_t level)
{
    log_min_level = level;
}

static void log_output_internal(const char *str, uint16_t len)
{
    if (log_output != NULL)
    {
        log_output(str, len);
    }
}

void log_write(log_level_t level, const char *tag, const char *fmt, ...)
{
    if (level < log_min_level || log_output == NULL)
    {
        return;
    }

    char buffer[LOG_BUFFER_SIZE];
    int offset = 0;

#if LOG_ENABLE_TIMESTAMP
    uint32_t timestamp = HAL_GetTick();
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "[%lu] ", timestamp);
#endif

#if LOG_ENABLE_COLOR
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", level_colors[level]);
#endif

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "[%s] [%s] ",
                      level_strings[level], tag);

    va_list args;
    va_start(args, fmt);
    offset += vsnprintf(buffer + offset, sizeof(buffer) - offset, fmt, args);
    va_end(args);

#if LOG_ENABLE_COLOR
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", ANSI_COLOR_RESET);
#endif

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");

    log_output_internal(buffer, offset);
}
