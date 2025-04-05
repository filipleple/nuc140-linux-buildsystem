// 
// Smpl_I2C_MMA7455 : 3-axis Accelerometer
//
// CS: 0 = SPI, 1 = I2C

// MMA7455 pin connections
// pin1  VCC  :to Vcc5V
// pin2  NC	  : N.C.
// pin3  NC	  : N.C.
// pin4  INT1 : N.C.
// pin5  INT2 : N.C.
// pin6  GND  : to Gnd
// pin7  GND  : to Gnd
// pin8  CS   : to Vcc5V
// pin9  SDO  : N.C.
// pin10 SDA/SDI : to I2C0_SDA (GPA8)
// pin11 SCL  : to I2C0_SCL (GPA9)
// pin12 VCC  : to Vcc5V 

#include <stdio.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "LCD.h"
#include "MMA7455.h"

int32_t main (void)
{
	char TEXT1[16], TEXT2[16], TEXT3[16];	
  	int8_t tmp;
	float accX, accY, accZ;

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN=1;
	DrvSYS_Delay(5000);// Waiting for 12M Xtal stable
	DrvSYS_SelectHCLKSource(0);
	LOCKREG();

	init_LCD(); //Initialize LCD pannel 
	clear_LCD();    
  	print_Line(0,"Smpl_I2C_MMA7455");

	DrvGPIO_InitFunction(E_FUNC_I2C0);  // set I2C0 pins 
	DrvI2C_Open(I2C_PORT0,150000);		  // Initialize I2C to 150Kbps
	init_MMA7455();
	
	while(1)
	{
		// read 3-axis Accelerometer
    tmp  = Read_DataX_MMA7455(); // read X value
		accX = (float)tmp/127 *2;
		tmp  = Read_DataY_MMA7455(); // read Y value
		accY = (float)tmp/127 *2;
    tmp  = Read_DataZ_MMA7455(); // read Z value
		accZ = (float)tmp/127 *2;
    // display on LCD
		sprintf(TEXT1,"accX: %f", accX); print_Line(1,TEXT1);
		sprintf(TEXT2,"accY: %f", accY); print_Line(2,TEXT2);
		sprintf(TEXT3,"accZ: %f", accZ); print_Line(3,TEXT3);
	}
}
