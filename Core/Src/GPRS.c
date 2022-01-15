#include "GPRS.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

char msg[30];
char cmd[475];
uint8_t buf[475] = {0};
// IMPORTANT: here you should change yours uarts!!(if you don't want/can't use debug, define it as 0)
UART_HandleTypeDef* gsm = &huart2;
UART_HandleTypeDef* debug = &huart1;

void gsm_send(char command[], uint32_t timeout) {
  HAL_UART_Transmit(gsm, (uint8_t *) command, strlen(command), HAL_MAX_DELAY);
  if (debug) {
	  HAL_UART_Receive(gsm, buf, 475, timeout);
	  HAL_UART_Transmit(debug,(uint8_t *)buf,sizeof(buf),HAL_MAX_DELAY);
	  memset(buf,0,sizeof(buf));
  }
}

void gsm_config_gprs() {
  sprintf(msg, " --- CONFIG GPRS --- \n");
  uint8_t flag=1;

	while(flag){
	sprintf(cmd,"AT\n");
	HAL_UART_Transmit(gsm,(uint8_t *)cmd,strlen(cmd),1000);
	HAL_UART_Receive(gsm, buf, 30, 1000);
	HAL_Delay(1000);

	if(strstr((char *)buf,"OK")){
		sprintf(msg,"Module Connected\n");
		HAL_UART_Transmit(debug,(uint8_t *)msg,strlen(msg),1000);
		flag=0;
	}
	HAL_Delay(1000);
	HAL_UART_Transmit(debug,(uint8_t *)buf,sizeof(buf),1000);
	memset(buf,0,sizeof(buf));
	}
  if (debug)
	  HAL_UART_Transmit(debug, (uint8_t *) msg, strlen(msg), HAL_MAX_DELAY);
  gsm_send("AT+SAPBR=3,1,Contype,GPRS\n", 1000);
  gsm_send("AT+SAPBR=3,1,APN,APN\n", 1000);
}

void gsm_http_post(char postdata[]) {
  sprintf(msg, " --- Start GPRS & HTTP --- \n");
  if (debug)
	  HAL_UART_Transmit(debug, (uint8_t *) msg, strlen(msg), HAL_MAX_DELAY);
  gsm_send("AT+SAPBR=1,1\n", 500);
  gsm_send("AT+HTTPINIT\n", 500);
  gsm_send("AT+HTTPPARA=CID,1\n", 500);
  sprintf(cmd, "AT+HTTPPARA=URL,http://bubuntik.herokuapp.com/?info=%s\n", postdata);
  gsm_send(cmd, 1000);
  gsm_send("AT+HTTPACTION=1\n", 10000);
  gsm_send("AT+HTTPTERM\n", 500);
  gsm_send("AT+SAPBR=0,1\n", 500);
}


void gsm_http_get() {
	sprintf(msg, " --- Sart GPRS & HTTP GET --- \n");
	if (debug)
		HAL_UART_Transmit(debug, (uint8_t *) msg, strlen(msg), HAL_MAX_DELAY);
	gsm_send("AT+SAPBR=1,1\n", 250);
	gsm_send("AT+HTTPINIT\n", 250);
	gsm_send("AT+HTTPPARA=CID,1\n", 250);
	gsm_send("AT+HTTPPARA=URL,http://bubuntik.herokuapp.com/operation\n", 500);
	gsm_send("AT+HTTPACTION=0\n", 3250);
	gsm_send("AT+HTTPREAD\n", 5000);
	gsm_send("AT+HTTPTERM\n", 250);
	gsm_send("AT+SAPBR=0,1\n", 250);
}
