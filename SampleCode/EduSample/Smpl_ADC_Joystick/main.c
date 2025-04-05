//
// Smpl_ADC_Joystick
//
// reading Joystick from ADC0 & ADC1
// display X,Y on LCD

// GND: Gnd
// +5V: Vcc
// Vx : GPA0 (ADC0)
// Vy : GPA1 (ADC1)
// SW : GPB0

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "ADC.h"
#include "LCD.h"

int32_t main (void)
{
  uint16_t Vx, Vy;
  uint8_t  SW;
  char TEXT[16];	

 	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();

	init_LCD();  // initialize LCD pannel
	clear_LCD();  // clear LCD panel 
	print_Line(0,"SmplADC_Joystick");
							 					 
	DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 0x03, INTERNAL_HCLK, 1); // ADC1 & ADC0	
  DrvGPIO_Open(E_GPB, 0, E_IO_INPUT); // SW		
	
	while(1)
	{
		DrvADC_StartConvert();                   // start A/D conversion
    while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
	  Vx = ADC->ADDR[0].RSLT & 0xFFF;
		Vy = ADC->ADDR[1].RSLT & 0xFFF;
		SW = DrvGPIO_GetBit(E_GPB,0);
		sprintf(TEXT,"Vx: %4d",Vx); // print ADC0 value into text
		print_Line(1, TEXT);		     
		sprintf(TEXT,"Vy: %4d",Vy); // print ADC1 value into text
		print_Line(2, TEXT);		     
		sprintf(TEXT,"SW: %4d",SW); // print switch input
		print_Line(3, TEXT);		
    DrvSYS_Delay(100000);		
	}
}

