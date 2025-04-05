//
// Smpl_I2C_ADXL345
//
// ADXL345 : Analog Device 3-axis accelerometer
// Interface: I2C/SPI (CS Hi/Lo)
// pin1: Gnd to Vss  (NUC123 pin15)
// pin2: Vcc to Vdd  (NUC123 pin16)
// pin3: CS  to Vdd  (Hi for I2C)          
// pin4: INT1-- N.C.
// pin5: INT2-- N.C.
// pin6: SDO to Gnd  (ALT_Address =0)
// pin7: SDA to SDA  (NUC123 pin4, NUC140 GPA10)
// pin8: SCL to SCLK (NUC123 pin5, NUC140 GPA11)

#include <stdio.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "LCD.h"
#include "ADXL345.h"

int32_t main (void)
{
	char TEXT1[16], TEXT2[16], TEXT3[16];

	uint16_t accX, accY, accZ;

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN=1;
	DrvSYS_Delay(5000); // Waiting for 12M Xtal stalble
	SYSCLK->CLKSEL0.HCLK_S=0;
	LOCKREG();

	init_LCD(); 
	clear_LCD();                
  print_Line(0,"Smpl_I2C_ADXL");

	DrvGPIO_InitFunction(E_FUNC_I2C0);  // Set I2C pins
	DrvI2C_Open(I2C_PORT0, 100000);        // Initialize I2C to 100KHz
	init_ADXL345();                     // Initialize ADXL345	
	
	while(1)
	{
		accX=Read_DataX_ADXL345();
		accY=Read_DataY_ADXL345();
		accZ=Read_DataZ_ADXL345();		
		
		sprintf(TEXT1,"x:%5d",accX);		
		sprintf(TEXT2,"y:%5d",accY);	
		sprintf(TEXT3,"z:%5d",accZ);			
		print_Line(1,TEXT1);
		print_Line(2,TEXT2);
		print_Line(3,TEXT3);
	}
}
