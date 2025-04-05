//
// Smpl_ADC_ResisitveTouch : 4-wire resistive touch panel 
//
// reading Touch Panel from ADC0 & ADC1
// display X,Y on LCD

// X+ : GPA0 (ADC0)
// X- : GPA2
// Y+ :	GPA1 (ADC1)
// Y- : GPA3

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "ADC.h"
#include "LCD.h"

void Delay(int count)
{
	while(count--)
	{
//		__NOP;
	 }
}

int32_t main (void)
{
	char TEXT2[16]="X-ADC0:        ";
	char TEXT3[16]="Y-ADC1:        ";		
	uint16_t temp;

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();

  /* The maximum ADC clock rate is 16MHz */
	init_LCD();  // initialize LCD pannel
	clear_LCD();  // clear LCD panel 
	print_Line(0,"Smpl_ADC_Touch");
	print_Line(1,"4-wire Resistive");
							 					 
	while(1)
	{
    	   DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 0x01, INTERNAL_HCLK, 1); // ADC0
	   DrvGPIO_Open(E_GPA, 1, E_IO_OUTPUT);
	   DrvGPIO_Open(E_GPA, 3, E_IO_OPENDRAIN);
	   DrvGPIO_SetBit(E_GPA,1);
	   DrvGPIO_ClrBit(E_GPA,3);

	   DrvADC_StartConvert();                   // start A/D conversion
    	   while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
    	   temp = DrvADC_GetConversionData(0);
	   DrvGPIO_Close(E_GPA,1);
	   DrvGPIO_Close(E_GPA,3);
	   DrvADC_Close();
	   temp = temp & 0x0ffc;
	   sprintf(TEXT2+7,"%d",temp);  // convert ADC0 value into text
	   print_Line(2, TEXT2);	// output TEXT to LCD
	   Delay(20000);

    	   DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 0x2, INTERNAL_HCLK, 1); // ADC1
	   DrvGPIO_Open(E_GPA, 0, E_IO_OUTPUT);
	   DrvGPIO_Open(E_GPA, 2, E_IO_OPENDRAIN);
	   DrvGPIO_SetBit(E_GPA,0);
	   DrvGPIO_ClrBit(E_GPA,2);

  	   DrvADC_StartConvert();                   // start A/D conversion
    	   while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
    	   temp = DrvADC_GetConversionData(1);
	   DrvGPIO_Close(E_GPA,0);
	   DrvGPIO_Close(E_GPA,2);
	   DrvADC_Close();
	   temp = temp & 0x0ffc;
	   sprintf(TEXT3+7,"%d",temp); // convert ADC1 value into text
	   print_Line(3, TEXT3);		 // output TEXT to LCD
	   Delay(20000);
	}
}
