//
// MPU6050 Driver: 3-axis Gyroscope + 3-axis accelerometer + temperature
//
// Interface: I2C
// pin1: Vcc to Vcc (+5V)
// pin2: Gnd to Gnd
// pin3: SCL to I2C0_SCL/GPA9
// pin4: SDA to I2C0_SDA/GPA8
// pin5: XDA -- N.C.
// pin6: XCL -- N.C.
// pin7: AD0 -- N.C.
// pin8: INT -- N.C.

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "LCD.h"
#include "MPU6050_REG.h"

void init_MPU6050(void)
{
	I2C_Write(MPU6050_PWR_MGMT_1, 0x00);	// CLL_SEL=0: internal 8MHz, TEMP_DIS=0, SLEEP=0 
	I2C_Write(MPU6050_SMPLRT_DIV, 0x07);  // Gyro output sample rate = Gyro Output Rate/(1+SMPLRT_DIV)
	I2C_Write(MPU6050_CONFIG, 0x06);      // set TEMP_OUT_L, DLPF=2 (Fs=1KHz)
	I2C_Write(MPU6050_GYRO_CONFIG, 0x18); // bit[4:3] 0=+-250d/s,1=+-500d/s,2=+-1000d/s,3=+-2000d/s
	I2C_Write(MPU6050_ACCEL_CONFIG, 0x01);// bit[4:3] 0=+-2g,1=+-4g,2=+-8g,3=+-16g, ACC_HPF=On (5Hz)
}

uint16_t Read_AccX_MPU6050(void)
{
	uint8_t LoByte, HiByte;
	LoByte = I2C_Read(MPU6050_ACCEL_XOUT_L); // read Accelerometer X_Low  value
	HiByte = I2C_Read(MPU6050_ACCEL_XOUT_H); // read Accelerometer X_High value
	return (HiByte<<8+LoByte);
}

uint16_t Read_AccY_MPU6050(void)
{
	uint8_t LoByte, HiByte;
	LoByte = I2C_Read(MPU6050_ACCEL_YOUT_L); // read Accelerometer X_Low  value
	HiByte = I2C_Read(MPU6050_ACCEL_YOUT_H); // read Accelerometer X_High value
	return (HiByte<<8+LoByte);
}

uint16_t Read_AccZ_MPU6050(void)
{
	uint8_t LoByte, HiByte;
	LoByte = I2C_Read(MPU6050_ACCEL_ZOUT_L); // read Accelerometer X_Low  value
	HiByte = I2C_Read(MPU6050_ACCEL_ZOUT_H); // read Accelerometer X_High value
	return (HiByte<<8+LoByte);
}

uint16_t Read_GyroX_MPU6050(void)
{
	uint8_t LoByte, HiByte;
	LoByte = I2C_Read(MPU6050_GYRO_XOUT_L); // read Accelerometer X_Low  value
	HiByte = I2C_Read(MPU6050_GYRO_XOUT_H); // read Accelerometer X_High value
	return (HiByte<<8+LoByte);
}

uint16_t Read_GyroY_MPU6050(void)
{
	uint8_t LoByte, HiByte;
	LoByte = I2C_Read(MPU6050_GYRO_YOUT_L); // read Accelerometer X_Low  value
	HiByte = I2C_Read(MPU6050_GYRO_YOUT_H); // read Accelerometer X_High value
	return (HiByte<<8+LoByte);
}

uint16_t Read_GyroZ_MPU6050(void)
{
	uint8_t LoByte, HiByte;
	LoByte = I2C_Read(MPU6050_GYRO_ZOUT_L); // read Accelerometer X_Low  value
	HiByte = I2C_Read(MPU6050_GYRO_ZOUT_H); // read Accelerometer X_High value
	return (HiByte<<8+LoByte);
}
