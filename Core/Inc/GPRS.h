#ifndef __GPRS_H__
#define __GPRS_H__

#include "usart.h"

void gsm_send(char command[], uint32_t timeout);

void gsm_config_gprs();

void gsm_http_post(char postdata[]);

void gsm_http_get();
#endif /*__ GPRS_H__ */
