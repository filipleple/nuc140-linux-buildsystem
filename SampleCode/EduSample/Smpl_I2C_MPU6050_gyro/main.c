//
// Smpl_I2C_MPU6050_gyro : 3-axis Gyroscope
//
// MPU6050 : 3-axis Gyroscope + 3-axis accelerometer + temperature
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
#include "MPU6050.h"

int32_t main (void)
{
	char TEXT1[16], TEXT2[16], TEXT3[16];
	int16_t tmp;
  float	gyroX, gyroY, gyroZ;

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN=1;
	DrvSYS_Delay(5000);					// Waiting for 12M Xtal stalble
	SYSCLK->CLKSEL0.HCLK_S=0;
	LOCKREG();

	init_LCD(); 
	clear_LCD();                
  print_Line(0,"MPU6050 Gyro2000");

	DrvGPIO_InitFunction(E_FUNC_I2C0);  // set I2C pins 
	DrvI2C_Open(I2C_PORT0, 50000);	    // set I2C to 50KHz
	init_MPU6050();                     // Initialize MPU6050
	
	while(1)
	{
		tmp = Read_GyroX_MPU6050();
		gyroX = (float) tmp/32768 *2000;	

		tmp = Read_GyroY_MPU6050();
		gyroY = (float) tmp/32768 *2000;

		tmp = Read_GyroZ_MPU6050();
		gyroZ = (float) tmp/32768 *2000;
	
    // print to LCD
		sprintf(TEXT1,"rX: %f", gyroX); print_Line(1,TEXT1);
		sprintf(TEXT2,"rY: %f", gyroY); print_Line(2,TEXT2);
		sprintf(TEXT3,"rZ: %f", gyroZ); print_Line(3,TEXT3);
	}
}
