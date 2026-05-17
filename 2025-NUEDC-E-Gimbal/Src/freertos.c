/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for LED */
osThreadId_t LEDHandle;
const osThreadAttr_t LED_attributes = {
  .name = "LED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KEY */
osThreadId_t KEYHandle;
const osThreadAttr_t KEY_attributes = {
  .name = "KEY",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for MOTOR */
osThreadId_t MOTORHandle;
const osThreadAttr_t MOTOR_attributes = {
  .name = "MOTOR",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for MAIN */
osThreadId_t MAINHandle;
const osThreadAttr_t MAIN_attributes = {
  .name = "MAIN",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime1,
};
/* Definitions for TEST */
osThreadId_t TESTHandle;
const osThreadAttr_t TEST_attributes = {
  .name = "TEST",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LOG */
osThreadId_t LOGHandle;
const osThreadAttr_t LOG_attributes = {
  .name = "LOG",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void led_task(void *argument);
void key_task(void *argument);
void motor_task(void *argument);
void app_main_task(void *argument);
void test_task(void *argument);
void log_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of LED */
  LEDHandle = osThreadNew(led_task, NULL, &LED_attributes);

  /* creation of KEY */
  KEYHandle = osThreadNew(key_task, NULL, &KEY_attributes);

  /* creation of MOTOR */
  MOTORHandle = osThreadNew(motor_task, NULL, &MOTOR_attributes);

  /* creation of MAIN */
  MAINHandle = osThreadNew(app_main_task, NULL, &MAIN_attributes);

  /* creation of TEST */
  TESTHandle = osThreadNew(test_task, NULL, &TEST_attributes);

  /* creation of LOG */
  LOGHandle = osThreadNew(log_task, NULL, &LOG_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_led_task */
/**
  * @brief  Function implementing the LED thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_led_task */
__weak void led_task(void *argument)
{
  /* USER CODE BEGIN led_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END led_task */
}

/* USER CODE BEGIN Header_key_task */
/**
* @brief Function implementing the KEY thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_key_task */
__weak void key_task(void *argument)
{
  /* USER CODE BEGIN key_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END key_task */
}

/* USER CODE BEGIN Header_motor_task */
/**
* @brief Function implementing the MOTOR thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_motor_task */
__weak void motor_task(void *argument)
{
  /* USER CODE BEGIN motor_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END motor_task */
}

/* USER CODE BEGIN Header_app_main_task */
/**
* @brief Function implementing the MAIN thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_app_main_task */
__weak void app_main_task(void *argument)
{
  /* USER CODE BEGIN app_main_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END app_main_task */
}

/* USER CODE BEGIN Header_test_task */
/**
* @brief Function implementing the TEST thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_test_task */
__weak void test_task(void *argument)
{
  /* USER CODE BEGIN test_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END test_task */
}

/* USER CODE BEGIN Header_log_task */
/**
* @brief Function implementing the LOG thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_log_task */
__weak void log_task(void *argument)
{
  /* USER CODE BEGIN log_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END log_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

