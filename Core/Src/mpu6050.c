#include "mpu6050.h"
#include "i2c.h"
#include <stdio.h>

int MPU6050_Init(void){

    uint8_t buffer[7];
    int res = 0;

    buffer[0] = MPU6050_RA_PWR_MGMT_1;
    buffer[1] = 0x00;
    res = I2C_WriteBuffer(MPU6050_ADDRESS_AD0_LOW,buffer,2);
    if (res != 0) return res;

    buffer[0] = MPU6050_RA_GYRO_CONFIG;
    buffer[1] = 0x8;
    res = I2C_WriteBuffer(MPU6050_ADDRESS_AD0_LOW,buffer,2);
    if (res != 0) return res;

    buffer[0] = MPU6050_RA_ACCEL_CONFIG;
    buffer[1] = 0x10;
    res = I2C_WriteBuffer(MPU6050_ADDRESS_AD0_LOW,buffer,2);

    return res;
}

void MPU6050_GetAllData(int16_t *Data){

  uint8_t accelbuffer[14];

  I2C_ReadBuffer(MPU6050_ADDRESS_AD0_LOW,MPU6050_RA_ACCEL_XOUT_H, accelbuffer, 14);

  /* Registers 59 to 64 – Accelerometer Measurements */
  for (int i = 0; i< 3; i++)
      Data[i] = ((int16_t) ((uint16_t) accelbuffer[2 * i] << 8) + accelbuffer[2 * i + 1]);

  /* Registers 65 and 66 – Temperature Measurement */

  /* Registers 67 to 72 – Gyroscope Measurements */
  for (int i = 4; i < 7; i++)
      Data[i - 1] = ((int16_t) ((uint16_t) accelbuffer[2 * i] << 8) + accelbuffer[2 * i + 1]);

}


void getAccAndGyroData() {
	int16_t Data[6];
	MPU6050_GetAllData(Data);
	printf("Accelero: x - %d, y - %d, z - %d\nGyro: x - %d, y - %d, z - %d\n", Data[0], Data[1], Data[2], Data[3], Data[4] ,Data[5]);
}
