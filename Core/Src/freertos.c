/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
#include "telemetry_base_class.h"
#include "fatfs.h"
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



osStatus_t status;

FATFS FatFs;
FIL telemetryFile;



/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for rxDataThread */
osThreadId_t rxDataThreadHandle;
const osThreadAttr_t rxDataThread_attributes = {
  .name = "rxDataThread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for txDataThread */
osThreadId_t txDataThreadHandle;
const osThreadAttr_t txDataThread_attributes = {
  .name = "txDataThread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for telemetryThread */
osThreadId_t telemetryThreadHandle;
const osThreadAttr_t telemetryThread_attributes = {
  .name = "telemetryThread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for accTimer */
osTimerId_t accTimerHandle;
const osTimerAttr_t accTimer_attributes = {
  .name = "accTimer"
};
/* Definitions for temperatureTimer */
osTimerId_t temperatureTimerHandle;
const osTimerAttr_t temperatureTimer_attributes = {
  .name = "temperatureTimer"
};
/* Definitions for telemetryFileMutex */
osMutexId_t telemetryFileMutexHandle;
const osMutexAttr_t telemetryFileMutex_attributes = {
  .name = "telemetryFileMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osMessageQueueId_t telemetryQueueHandle;
const osMessageQueueAttr_t telemetryQueue_attributes = {
  .name = "telemetryQueue"
};
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void startRxDataThread(void *argument);
void startTxDataThread(void *argument);
void startTelemetryThread(void *argument);
void accTimerCallback(void *argument);
void temperatureTimerCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

	// initialize sd card now to check whether it is working
	// second arg is basically prefic of path to file
	FRESULT fres = f_mount(&FatFs, "", 1);
	if (fres != FR_OK) {
		myprintf("f_mount error (%i)\r\n", fres);
		Error_Handler();
	} else {
		myprintf("SD card mounted\n");
	}
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of telemetryFileMutex */
  telemetryFileMutexHandle = osMutexNew(&telemetryFileMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of accTimer */
  accTimerHandle = osTimerNew(accTimerCallback, osTimerPeriodic, NULL, &accTimer_attributes);

  /* creation of temperatureTimer */
  temperatureTimerHandle = osTimerNew(temperatureTimerCallback, osTimerPeriodic, NULL, &temperatureTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  if (accTimerHandle != NULL)  {
      status = osTimerStart(accTimerHandle, 2000U);       // start timer
      if (status != osOK) {
        // Timer could not be started
    	Error_Handler();
      }
  } else {
	  Error_Handler();
  }

  if (temperatureTimerHandle != NULL)  {
        status = osTimerStart(temperatureTimerHandle, 2000U);       // start timer
        if (status != osOK) {
          // Timer could not be started
      	Error_Handler();
        }
    } else {
  	  Error_Handler();
    }
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  telemetryQueueHandle = osMessageQueueNew(16, sizeof(TelemetryBase), &telemetryQueue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of rxDataThread */
  rxDataThreadHandle = osThreadNew(startRxDataThread, NULL, &rxDataThread_attributes);

  /* creation of txDataThread */
  txDataThreadHandle = osThreadNew(startTxDataThread, NULL, &txDataThread_attributes);

  /* creation of telemetryThread */
  telemetryThreadHandle = osThreadNew(startTelemetryThread, NULL, &telemetryThread_attributes);

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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_startRxDataThread */
/**
* @brief Function implementing the rxDataThread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startRxDataThread */
void startRxDataThread(void *argument)
{
  /* USER CODE BEGIN startRxDataThread */
  /* Infinite loop */
  for(;;)
  {
	 osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);

	 // thread is with the highest priority, as after the signal about new firmware
	 // has come, we are not interested in data anymore


	 HAL_UART_Receive_IT(&huart2, (uint8_t *)&notification_buffer, 1);
  }
  /* USER CODE END startRxDataThread */
}

/* USER CODE BEGIN Header_startTxDataThread */
/**
* @brief Function implementing the txDataThread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startTxDataThread */
void startTxDataThread(void *argument)
{
  /* USER CODE BEGIN startTxDataThread */
  /* Infinite loop */
  for(;;)
  {
	  osThreadFlagsWait(0x00000002U, osFlagsWaitAny, osWaitForever);

	  // there is no need to take mutex, as currenlty this is the only task
	  // that actually uses uart (if we omit the debug part)

	  // read from sd and write the info into uart (mock gprs)


	  HAL_UART_Receive_IT(&huart2, (uint8_t *)&notification_buffer, 1);
  }
  /* USER CODE END startTxDataThread */
}

/* USER CODE BEGIN Header_startTelemetryThread */
/**
* @brief Function implementing the telemetryThread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startTelemetryThread */
void startTelemetryThread(void *argument)
{
  /* USER CODE BEGIN startTelemetryThread */
  TelemetryBase tb;
  FRESULT wr;
  // TODO: get rid of magic constant
  BYTE wbuf[30];

  /* Infinite loop */
  for(;;)
  {
	 status = osMessageQueueGet(telemetryQueueHandle, &tb, NULL, osWaitForever);
	 if (status == osOK) {
		 // write into sd card

		 sprintf(wbuf, "Telemetry{%d, %u, {%d, %d, %d}}\n",
				 tb.id, tb.data_size, tb.data[0],
				 tb.data[1], tb.data[2]);
		 myprintf("Writing following string to sd: %s", wbuf);


		 // TODO: add mutex here
		 wr = f_open(&telemetryFile, "write.txt", FA_WRITE | FA_OPEN_ALWAYS);


		 if(wr == FR_OK) {
		   	myprintf("I was able to open '%s' for writing\n", TELEMETRY_FILE);
		 } else {
		   	myprintf("f_open error (%i)\n", wr);
		 }


		 UINT bytesWrote;
		 // TODO: and also I assume we should add mutex here
		 wr = f_write(&telemetryFile, wbuf, 19, &bytesWrote);
		 if(wr == FR_OK) {
		   	myprintf("Wrote %i bytes to 'write.txt'!\n", bytesWrote);
		 } else {
		   	myprintf("f_write error (%d)\n", (int) bytesWrote);
		 }

		 f_close(&telemetryFile);
	 }
  }
  /* USER CODE END startTelemetryThread */
}

/* accTimerCallback function */
void accTimerCallback(void *argument)
{
  /* USER CODE BEGIN accTimerCallback */
	TelemetryBase acc;

	acc.id = ACC;
	acc.data_size = 3;
	for (size_t i = 0; i < acc.data_size; i++) {
		acc.data[i] = i;
	}

	// note that this might be called from isr, if the
	// time parameter is set to 0
	osMessageQueuePut(telemetryQueueHandle, &acc, 0U, 0U);
  /* USER CODE END accTimerCallback */
}

/* temperatureTimerCallback function */
void temperatureTimerCallback(void *argument)
{
  /* USER CODE BEGIN temperatureTimerCallback */
	TelemetryBase acc;

	acc.id = ACC;
	acc.data_size = 3;
	for (size_t i = 0; i < acc.data_size; i++) {
		acc.data[i] = acc.data_size - i;
	}

	osMessageQueuePut(telemetryQueueHandle, &acc, 0U, 0U);
  /* USER CODE END temperatureTimerCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/*
 * [IMPORTANT] TODO: Note that this functoin is unsafe (as we might interrupt uart usage)
 * Actually not, as we would transmit only after this
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
     if (huart == &huart2) {
    	 int val = (int) (notification_buffer[0] - '0');
		 switch (val) {
		 case 0:
			 osThreadFlagsSet(rxDataThreadHandle, 0x00000001U);
			 break;
		 case 1:
			 osThreadFlagsSet(txDataThreadHandle, 0x00000002U);
			 break;
		 default:
			 myprintf("[ERROR]: Op not allowed: %d\n", val);
			 break;
		 };
     }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
