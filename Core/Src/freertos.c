/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "usart.h"
#include "mpu6050.h"
#include "sensor_readings.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MSG_SIZE 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for acc_task */
osThreadId_t acc_taskHandle;
const osThreadAttr_t acc_task_attributes = {
  .name = "acc_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for gyro_task */
osThreadId_t gyro_taskHandle;
const osThreadAttr_t gyro_task_attributes = {
  .name = "gyro_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensor_q */
osMessageQueueId_t sensor_qHandle;
const osMessageQueueAttr_t sensor_q_attributes = {
  .name = "sensor_q"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void acc_task_start(void *argument);
void gyro_task_start(void *argument);

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

  /* Create the queue(s) */
  /* creation of sensor_q */
  // sensor_qHandle = osMessageQueueNew (16, sizeof(uint16_t), &sensor_q_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  // VERY_IMPORTANT: comment the line of code above
  sensor_qHandle = osMessageQueueNew (16, sizeof(SensorReadings_t), &sensor_q_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of acc_task */
  acc_taskHandle = osThreadNew(acc_task_start, NULL, &acc_task_attributes);

  /* creation of gyro_task */
  gyro_taskHandle = osThreadNew(gyro_task_start, NULL, &gyro_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  SensorReadings_t msg;
  char dbg_msg[MSG_SIZE];
  osStatus_t status;
  /* Infinite loop */
  for (;;) {
    status = osMessageQueueGet(sensor_qHandle, &msg, NULL, 0U);   // wait for message
    if (status == osOK) {
       if (msg.SensorId > NUM_OF_SENSORS)
    	   continue;
       sprintf(dbg_msg, "s: %d | d: %d, %d, %d \n",
    		   msg.SensorId,
			   msg.Buf[0],
			   msg.Buf[1],
			   msg.Buf[2]
		);
       HAL_UART_Transmit(&huart1, (uint8_t *) dbg_msg, strlen(dbg_msg), 50);
       HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
       status = osError;
    } else {
      osThreadYield();
    }
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_acc_task_start */
/**
* @brief Function implementing the acc_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_acc_task_start */
void acc_task_start(void *argument)
{
  /* USER CODE BEGIN acc_task_start */
  SensorReadings_t acc;
  SensorReadings_t gyro;
  int16_t Data[6];

  // note that static init seems better in this case
  int acc_buf[3];
  int gyro_buf[3];

  osStatus_t status;

   /* Infinite loop */
  for(;;)
  {
	MPU6050_GetAllData(Data);

	acc_buf[0] = Data[0];
	acc_buf[1] = Data[1];
	acc_buf[2] = Data[2];

	gyro_buf[0] = Data[3];
	gyro_buf[1] = Data[4];
	gyro_buf[2] = Data[5];

	acc.Buf = (int *) acc_buf;
	acc.SensorId = ACC;

	gyro.Buf = (int *) gyro_buf;
	gyro.SensorId = GYRO;

	status = osMessageQueuePut(sensor_qHandle, &acc, 0U, 0U);
	if (status != osOK) {
		// TODO: add re-send possibility here
	}

	status = osMessageQueuePut(sensor_qHandle, &gyro, 0U, 0U);
	if (status != osOK) {
		// TODO: add re-send possibility here
	}
	osDelay(500);
	// osThreadYield();
  }

  osThreadExit();
  /* USER CODE END acc_task_start */
}

/* USER CODE BEGIN Header_gyro_task_start */
/**
* @brief Function implementing the gyro_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_gyro_task_start */
void gyro_task_start(void *argument)
{
  /* USER CODE BEGIN gyro_task_start */
  SensorReadings_t t;
  int test[3] = {3, 4, 5};
  /* Infinite loop */
  for(;;) {
	   t.Buf    	 = (int *) test;                                         // do some work...
	   t.SensorId    = DBG_MSG;
	   osMessageQueuePut(sensor_qHandle, &t, 0U, 0U);
	   osDelay(500);
	   // osThreadYield();
   }

   osThreadExit();
  /* USER CODE END gyro_task_start */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
