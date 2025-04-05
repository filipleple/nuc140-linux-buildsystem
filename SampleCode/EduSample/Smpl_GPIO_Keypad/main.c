//
// Smpl_GPIO_Keypad : scan keypad 3x3 (1~9) to control GPB0~8
//
// 4-port Relay
// VCC : to 3.3V
// IN1 : to GPB0 (press 1 will output 3.3V, else output 0V)
// IN2 : to GPB1 (press 2 will output 3.3V, else output 0V)
// IN3 : to GPB2 (press 3 will output 3.3V, else output 0V)
// IN4 : to GPB3 (press 4 will output 3.3V, else output 0V)
// GND : to GND

#include "stdio.h"																											 
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "Seven_Segment.h"
#include "Scankey.h"
#include "LCD.h"

/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	char TEXT1[16] = "number:         ";
	int8_t i, number;
	
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; 	//Enable 12Mhz and set HCLK->12Mhz
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	init_LCD();
	clear_LCD();
  
  for (i=0; i<9; i++)	{
	DrvGPIO_Open(E_GPB, i, E_IO_OUTPUT);
	DrvGPIO_ClrBit(E_GPB, i);
	}
	OpenKeyPad();

	print_Line(0,"Smpl_GPIO_Keypad");

	 
	while(1)
	{
	  number = ScanKey();
		sprintf(TEXT1+8,"%d", number);
		print_Line(1,TEXT1);
		if (number!=0)	DrvGPIO_SetBit(E_GPB, number-1);
		else 
			for (i=0; i<9; i++) DrvGPIO_ClrBit(E_GPB, i); 			
	}
}
