//
// Smpl_GPIO_BodyInfrared
//
// Body Infrared sensor
// pin 1 : Vcc (+5V)
// pin 2 : 1 = detected, 0 = no detection --> connected to GPA0
// pin 3 : Gnd 

#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "LCD.h"

int main(void)
{
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; 	//Enable 12Mhz and set HCLK->12Mhz
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

  DrvGPIO_Open(E_GPA, 0, E_IO_INPUT); // set GPA0 to input mode

	init_LCD(); 
	clear_LCD();
	
	print_Line(0, "Body Infrared   ");	  
	print_Line(1, "GPA0 input:     ");

	while(1) {
	if (DrvGPIO_GetBit(E_GPA,0)==0) print_Line(2, "Object Detected!");
	else                            print_Line(2, "No Detection!   ");   	  
	}	 	  		
}
