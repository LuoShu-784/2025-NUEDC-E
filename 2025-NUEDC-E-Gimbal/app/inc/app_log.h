/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_log.h
  * @brief   Log system initialization and queue interface
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_LOG_H_
#define __APP_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "log.h"
#include <stdarg.h>

/* Initialize log system with CMSIS-RTOS2 queue */
void app_log_init(void);

void log_output_handler(const char *str, uint16_t len);

/* Process queued log messages */
void app_log_process(void);

/* Queue-based logging function (ISR-safe) */
void logq_write(log_level_t level, const char *tag, const char *fmt, ...);

/* Queue-based logging macros */
#if LOG_COMPILE_LEVEL <= LOG_LEVEL_DEBUG
#define logq_d(tag, fmt, ...) logq_write(LOG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
#define logq_d(tag, fmt, ...) ((void)0)
#endif

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_INFO
#define logq_i(tag, fmt, ...) logq_write(LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__)
#else
#define logq_i(tag, fmt, ...) ((void)0)
#endif

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_WARN
#define logq_w(tag, fmt, ...) logq_write(LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__)
#else
#define logq_w(tag, fmt, ...) ((void)0)
#endif

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_ERROR
#define logq_e(tag, fmt, ...) logq_write(LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)
#else
#define logq_e(tag, fmt, ...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_LOG_H_ */
