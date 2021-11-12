#ifndef _SENSOR_READINGS_H
#define _SENSOR_READINGS_H
enum SENSORS{ACC, GYRO, DBG_MSG};

#define NUM_OF_SENSORS 3

typedef struct sr {
   int* Buf;
   enum SENSORS SensorId;
} SensorReadings_t;

#endif // _SENSOR_READINGS_H
