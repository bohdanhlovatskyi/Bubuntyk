/*
 * telemetry_base_class.h
 *
 *  Created on: Jan 12, 2022
 *      Author: hlovatskyibohdan
 */

#ifndef INC_TELEMETRY_BASE_CLASS_H_
#define INC_TELEMETRY_BASE_CLASS_H_

#include "main.h"

enum Sensor{ACC, GYRO, TEMP};

typedef struct {
	enum Sensor id;
	int data[12];
	size_t data_size;
} TelemetryBase;

#endif /* INC_TELEMETRY_BASE_CLASS_H_ */
