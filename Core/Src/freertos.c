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

#include "rtc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define printf(...) sprintf((char*) msg, __VA_ARGS__);\
		gsm_http_post(msg)
		// HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);  \
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */



osStatus_t status;

FATFS FatFs;
FIL telemetryFile;
FIL firmwareFile;


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
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for txDataThread */
osThreadId_t txDataThreadHandle;
const osThreadAttr_t txDataThread_attributes = {
  .name = "txDataThread",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for telemetryThread */
osThreadId_t telemetryThreadHandle;
const osThreadAttr_t telemetryThread_attributes = {
  .name = "telemetryThread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for peripheryThread */
osThreadId_t peripheryThreadHandle;
const osThreadAttr_t peripheryThread_attributes = {
  .name = "peripheryThread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
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
/* Definitions for gpsTimer */
osTimerId_t gpsTimerHandle;
const osTimerAttr_t gpsTimer_attributes = {
  .name = "gpsTimer"
};
/* Definitions for lightTimer */
osTimerId_t lightTimerHandle;
const osTimerAttr_t lightTimer_attributes = {
  .name = "lightTimer"
};
/* Definitions for txThreadSem */
osSemaphoreId_t txThreadSemHandle;
const osSemaphoreAttr_t txThreadSem_attributes = {
  .name = "txThreadSem"
};
/* Definitions for rxThreadSem */
osSemaphoreId_t rxThreadSemHandle;
const osSemaphoreAttr_t rxThreadSem_attributes = {
  .name = "rxThreadSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osMessageQueueId_t telemetryQueueHandle;
const osMessageQueueAttr_t telemetryQueue_attributes = {
  .name = "telemetryQueue"
};


osMutexId_t telemetryFileMutexHandle;
const osMutexAttr_t telemetryFileMutex_attributes = {
  .name = "telemetryFileMutex"
};

osMutexId_t uartMutexHandle;
const osMutexAttr_t uartMutex_attributes = {
 .name = "uartMutex"
};
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void startRxDataThread(void *argument);
void startTxDataThread(void *argument);
void startTelemetryThread(void *argument);
void startPeripheryThread(void *argument);
void accTimerCallback(void *argument);
void temperatureTimerCallback(void *argument);
void gpsTimerCallback(void *argument);
void lightTimerCallback(void *argument);

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
		printf("f_mount error (%i)\r\n", fres);
		Error_Handler();
	} else {
		printf("SD card mounted\n");
	}
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  telemetryFileMutexHandle = osMutexNew(&telemetryFileMutex_attributes);
  uartMutexHandle = osMutexNew(&uartMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of txThreadSem */
  txThreadSemHandle = osSemaphoreNew(1, 1, &txThreadSem_attributes);

  /* creation of rxThreadSem */
  rxThreadSemHandle = osSemaphoreNew(1, 1, &rxThreadSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  osSemaphoreAcquire(txThreadSemHandle, 0);
  osSemaphoreAcquire(rxThreadSemHandle, 0);
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of accTimer */
  accTimerHandle = osTimerNew(accTimerCallback, osTimerPeriodic, NULL, &accTimer_attributes);

  /* creation of temperatureTimer */
  temperatureTimerHandle = osTimerNew(temperatureTimerCallback, osTimerPeriodic, NULL, &temperatureTimer_attributes);

  /* creation of gpsTimer */
  gpsTimerHandle = osTimerNew(gpsTimerCallback, osTimerPeriodic, NULL, &gpsTimer_attributes);

  /* creation of lightTimer */
  lightTimerHandle = osTimerNew(lightTimerCallback, osTimerPeriodic, NULL, &lightTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  if (accTimerHandle != NULL)  {
      status = osTimerStart(accTimerHandle, 10000U);       // start timer
      if (status != osOK) {
        // Timer could not be started
    	Error_Handler();
      }
  } else {
	  Error_Handler();
  }

  if (temperatureTimerHandle != NULL)  {
        status = osTimerStart(temperatureTimerHandle, 10000U);       // start timer
        if (status != osOK) {
          // Timer could not be started
      	Error_Handler();
        }
    } else {
  	  Error_Handler();
    }

  if (gpsTimerHandle != NULL)  {
          status = osTimerStart(gpsTimerHandle, 10000U);       // start timer
          if (status != osOK) {
            // Timer could not be started
        	Error_Handler();
          }
      } else {
    	  Error_Handler();
      }

  if (lightTimerHandle != NULL)  {
      status = osTimerStart(lightTimerHandle, 10000U);       // start timer
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

  /* creation of peripheryThread */
  peripheryThreadHandle = osThreadNew(startPeripheryThread, NULL, &peripheryThread_attributes);

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
	 // thread is with the highest priority, as after the signal about new firmware
	 // has come, we are not interested in data anymore
	FRESULT wr;
	UINT bytesWrote;
	int cmpRes;
	int safeToBoot = 0;

	uint8_t firmwareChunk[16];
  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(rxThreadSemHandle, osWaitForever);


	  printf("Firmware to be uploaded...\n");

	  status = osMutexAcquire(telemetryFileMutexHandle, osWaitForever);
	  if (status != osOK) {
		  printf("Could not take mutex for writing into file");
	  } else {
		  wr = f_open(&firmwareFile, "f.bin", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
		  osMutexRelease(telemetryFileMutexHandle);

		  if(wr != FR_OK) {
			  printf("f_open error (%i)\n", wr);
		  } else {
			  for (;;) {
				  memset(firmwareChunk, 0, sizeof(firmwareChunk));
				  HAL_UART_Receive(&huart2, firmwareChunk, 4, HAL_MAX_DELAY);
				  cmpRes = strcmp(firmwareChunk, "$END");
				  if (cmpRes == 0) {
					  safeToBoot = 1;
					  break;
				  }

				  osMutexAcquire(telemetryFileMutexHandle, osWaitForever);
				  wr = f_write(&firmwareFile, firmwareChunk, 4, &bytesWrote);
				  osMutexRelease(telemetryFileMutexHandle);

				  if (wr != FR_OK) {
					  printf("[ERROR]: f_write firmware (%d)\n", wr);
					  break;
				  }
			  }


			  osMutexAcquire(telemetryFileMutexHandle, osWaitForever);
			  f_close(&firmwareFile);
			  osMutexRelease(telemetryFileMutexHandle);
		  }
	  }

	 if (safeToBoot) {
		 // toglle boot pin and software reset
		 HAL_PWR_EnableBkUpAccess();
		 HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 1);
		 HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, 0);
		 HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, 1);
		 HAL_PWR_DisableBkUpAccess();

		 HAL_NVIC_SystemReset();
	 } else {
		 // try one more time
		 HAL_UART_Receive_IT(&huart2, (uint8_t *)&notification_buffer, 1);
	 }
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
	FRESULT rr = FR_OK;
	// TODO: get rid of magic constants
	BYTE rbuf[128] = {0};
  /* Infinite loop */
  for(;;)
  {
	  if (rr != FR_OK) {
		  // printf("[ERROR]: reading : (%i)\n", rr);
	  }

	  osSemaphoreAcquire(txThreadSemHandle, osWaitForever);

	  // read from sd and write the info into uart (mock gprs)
	  osMutexAcquire(telemetryFileMutexHandle, osWaitForever);
	  rr = f_open(&telemetryFile, "write.txt", FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	  if(rr != FR_OK) {
		  osMutexRelease(telemetryFileMutexHandle);
		  HAL_UART_Receive_IT(&huart2, (uint8_t *)&notification_buffer, 1);
		  continue;
	  }

	  while (f_gets((TCHAR*)rbuf, sizeof(rbuf), &telemetryFile)) {
		  printf("[READ]: %s", rbuf);
	  }

	  f_close(&telemetryFile);
	  f_unlink("write.txt");
	  osMutexRelease(telemetryFileMutexHandle);

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
  FRESULT wr = FR_OK;
  // TODO: get rid of magic constant
  BYTE wbuf[128] = {0};

  /* Infinite loop */
  for(;;)
  {
	 if (wr != FR_OK) {
		 printf("[ERROR]: writing telemetry : (%i)\n", wr);
	 }

	 osMessageQueueGet(telemetryQueueHandle, &tb, NULL, osWaitForever);

	 sprintf(wbuf, "Telemetry{%d, %u, {%d, %d, %d}}\n",
				 tb.id, tb.data_size, tb.data[0],
				 tb.data[1], tb.data[2]);
	 // printf("Writing following string to sd: %s", wbuf);


	 osMutexAcquire(telemetryFileMutexHandle, osWaitForever);
	 wr = f_open(&telemetryFile, "write.txt", FA_OPEN_APPEND | FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	 if(wr != FR_OK) {
		 osMutexRelease(telemetryFileMutexHandle);
		 continue;
	 }

	 wr = f_write(&telemetryFile, wbuf, strlen(wbuf), NULL);
	 f_close(&telemetryFile);
	 osMutexRelease(telemetryFileMutexHandle);
  }
  /* USER CODE END startTelemetryThread */
}

/* USER CODE BEGIN Header_startPeripheryThread */
/**
* @brief Function implementing the peripheryThread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startPeripheryThread */
void startPeripheryThread(void *argument)
{
  /* USER CODE BEGIN startPeripheryThread */

	uint32_t flags;
	TelemetryBase tb;
	int16_t Data[6];
	float pressure, temperature, humidity;

	float TEMT6000_lux;

  /* Infinite loop */
  for(;;)
  {

	  osThreadFlagsWait(0x11111111U, osFlagsNoClear, osWaitForever);
	  flags = osThreadFlagsGet();
	  osThreadFlagsClear(flags);

	  switch (flags) {
	  case (0x00000001U):
	  	    MPU6050_GetAllData(Data);

			tb.id = ACC;
	  	  	tb.data_size = 3;
	  	  	tb.data[0] = Data[0];
	  	  	tb.data[1] = Data[1];
	  	  	tb.data[2] = Data[2];

	  	  	osMessageQueuePut(telemetryQueueHandle, &tb, 0U, 0U);

	  	  	tb.id = GYRO;
	  	  	tb.data_size = 3;
	  	  	tb.data[0] = Data[3];
	  	  	tb.data[1] = Data[4];
	  	  	tb.data[2] = Data[5];

	  	  	break;

	  case (0x00000010U):
			tb.id = HTP;
	  	  	tb.data_size = 3;

			while (!bmp280_read_float(&bmp280, &temperature, &pressure, &humidity));
			tb.data[0] = (int) temperature;
			tb.data[1] = (int) pressure;
			tb.data[2] = (int) humidity;

			break;

	  case (0x00000100U):

			while (!GPS_read());

			tb.id = NEO6M;
			tb.data_size = 3;
			tb.data[0] = (int) GPS.utc_time;
			tb.data[1] = (int) GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
			tb.data[2] = (int) GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);

			break;

	  case (0x00001000U):

			tb.id = LIGHT;
	  	  	tb.data_size = 1;

	  	    while (TEMT6000_OK != TEMT6000_ReadLight(&TEMT6000_lux));

	  	    tb.data[0] = (int) TEMT6000_lux;

			break;
	  default:
		  break;

	  }

	  osMessageQueuePut(telemetryQueueHandle, &tb, 0U, 0U);
  }
  /* USER CODE END startPeripheryThread */
}

/* accTimerCallback function */
void accTimerCallback(void *argument)
{
  /* USER CODE BEGIN accTimerCallback */
	osThreadFlagsSet(peripheryThreadHandle, 0x00000001U);
  /* USER CODE END accTimerCallback */
}

/* temperatureTimerCallback function */
void temperatureTimerCallback(void *argument)
{
  /* USER CODE BEGIN temperatureTimerCallback */
	osThreadFlagsSet(peripheryThreadHandle, 0x00000010U);
  /* USER CODE END temperatureTimerCallback */
}

/* gpsTimerCallback function */
void gpsTimerCallback(void *argument)
{
  /* USER CODE BEGIN gpsTimerCallback */
	osThreadFlagsSet(peripheryThreadHandle, 0x00000100U);
  /* USER CODE END gpsTimerCallback */
}

/* lightTimerCallback function */
void lightTimerCallback(void *argument)
{
  /* USER CODE BEGIN lightTimerCallback */
	osThreadFlagsSet(peripheryThreadHandle, 0x00001000U);
  /* USER CODE END lightTimerCallback */
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
			 osSemaphoreRelease(rxThreadSemHandle);
			 break;
		 case 1:
			 status = osSemaphoreRelease(txThreadSemHandle);
			 break;
		 default:
			 // printf("[ERROR]: Op not allowed: %d\n", val);
			 HAL_UART_Receive_IT(&huart2, (uint8_t *)&notification_buffer, 1);
			 break;
		 };

     }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
