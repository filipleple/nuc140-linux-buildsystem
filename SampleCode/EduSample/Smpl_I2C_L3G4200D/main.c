//
// Smpl_I2C_L3G4200	: 3-axis Gyroscope
//
// CS: 0 = SPI, 1 = I2C 
//
// L3G4200D pin connections
// pin1 SDO N.C.
// pin2 CS  to Vcc +5V 
// pin3 DR  N.C.
// pin4 INT N.C.
// pin5 GND to Gnd
// pin6 SDA to I2C0_SDA (GPA8)
// pin7 SCL to I2C0_SCL (GPA9)
// pin8 Vcc to Vcc5V

#include <stdio.h>
#include <stdint.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "LCD.h"
#include "L3G4200D.h"

int32_t main (void)
{
	char TEXT1[16], TEXT2[16], TEXT3[16];
  uint16_t tmp;
	float gyroX, gyroY, gyroZ;
 
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN=1;
	DrvSYS_Delay(5000);					// Waiting for 12M Xtal stalble
	DrvSYS_SelectHCLKSource(0);
	LOCKREG();

	init_LCD();					  // Initialize LCD  
	clear_LCD();   
  print_Line(0,"Smpl_I2C_L3G4200");

	DrvGPIO_InitFunction(E_FUNC_I2C0);  // set I2C0 pins 
	DrvI2C_Open(I2C_PORT0,400000);  // Initialize I2C to 400KHz
	init_L3G4200D();   // Initialize L3G4200D
	
	while(1)
	{									  
		tmp = Read_DataX_L3G4200D();
		gyroX = (float)tmp/32768 *2000;

		tmp = Read_DataY_L3G4200D();
		gyroY = (float)tmp/32768 *2000;

 		tmp = Read_DataZ_L3G4200D();
		gyroZ = (float)tmp/32768 *2000;
		
		sprintf(TEXT1,"X: %f", gyroX);
		sprintf(TEXT2,"Y: %f", gyroY);		
		sprintf(TEXT3,"Z: %f", gyroZ);
		print_Line(1,TEXT1);
		print_Line(2,TEXT2);
		print_Line(3,TEXT3);
	}
}
