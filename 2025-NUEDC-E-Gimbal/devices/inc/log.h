/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log.h
  * @brief   Terminal color log system header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __LOG_H_
#define __LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Configuration Macros
 * ============================================================================ */

/* Default log level (can be overridden in project settings) */
#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL       LOG_LEVEL_DEBUG
#endif

/* Enable/disable color output */
#ifndef LOG_ENABLE_COLOR
#define LOG_ENABLE_COLOR        1
#endif

/* Enable/disable timestamp */
#ifndef LOG_ENABLE_TIMESTAMP
#define LOG_ENABLE_TIMESTAMP    0
#endif

/* Enable/disable thread-safe queue */
#ifndef LOG_ENABLE_QUEUE
#define LOG_ENABLE_QUEUE        1
#endif

/* Queue configuration (only used if LOG_ENABLE_QUEUE is enabled) */
#ifndef LOG_QUEUE_SIZE
#define LOG_QUEUE_SIZE          16
#endif

#ifndef LOG_MESSAGE_MAX_LEN
#define LOG_MESSAGE_MAX_LEN     128
#endif

#ifndef LOG_TAG_MAX_LEN
#define LOG_TAG_MAX_LEN         16
#endif

/* Output buffer size */
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE         256
#endif

/* Compile-time log level filtering (set to disable lower levels at compile time) */
#ifndef LOG_COMPILE_LEVEL
#define LOG_COMPILE_LEVEL       LOG_LEVEL_DEBUG
#endif

/* ============================================================================
 * Log Levels
 * ============================================================================ */

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
} log_level_t;

/* Log output function pointer type */
typedef void (*log_output_fn_t)(const char *str, uint16_t len);

/* Initialization */
void log_init(log_output_fn_t output_fn);
void log_set_level(log_level_t level);

/* ============================================================================
 * Core Functions
 * ============================================================================ */

void log_write(log_level_t level, const char *tag, const char *fmt, ...);

/* Enable/disable convenience macros (log_d, log_i, log_w, log_e) */
#ifndef LOG_ENABLE_MACROS
#define LOG_ENABLE_MACROS       1
#endif

/* ============================================================================
 * Convenience Macros (with compile-time filtering)
 * ============================================================================ */

#if LOG_ENABLE_MACROS

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_DEBUG
#define log_d(tag, fmt, ...) log_write(LOG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
#define log_d(tag, fmt, ...) ((void)0)
#endif

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_INFO
#define log_i(tag, fmt, ...) log_write(LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__)
#else
#define log_i(tag, fmt, ...) ((void)0)
#endif

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_WARN
#define log_w(tag, fmt, ...) log_write(LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__)
#else
#define log_w(tag, fmt, ...) ((void)0)
#endif

#if LOG_COMPILE_LEVEL <= LOG_LEVEL_ERROR
#define log_e(tag, fmt, ...) log_write(LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)
#else
#define log_e(tag, fmt, ...) ((void)0)
#endif

#endif /* LOG_ENABLE_MACROS */

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H_ */
