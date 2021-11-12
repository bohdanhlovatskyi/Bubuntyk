#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include <stdint.h>

#define MPU6050_ADDRESS_AD0_LOW 0x68
#define MPU6050_RA_ACCEL_XOUT_H 0x3B
#define MPU6050_RA_GYRO_CONFIG  0x1B
#define MPU6050_RA_ACCEL_CONFIG 0x1C
#define MPU6050_RA_PWR_MGMT_1   0x6B

int MPU6050_Init(void);
void MPU6050_GetAllData(int16_t *Data);

#endif /* INC_MPU6050_H_ */
