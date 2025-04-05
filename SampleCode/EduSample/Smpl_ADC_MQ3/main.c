//
// Smpl_ADC_MQ3 : 
// GPA0 to read MQ3-DO
// ADC1 to read MQ3-AO
//

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "ADC.h"
#include "LCD.h"

int32_t main (void)
{
	int i;
	char TEXT[16];
	uint16_t adc_value;
	uint16_t gpio_value;

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();

  // The maximum ADC clock rate is 16MHz
	// select ADC1
  DrvADC_Open(ADC_SINGLE_END, ADC_CONTINUOUS_OP, 0x02, INTERNAL_HCLK, 1);
	DrvGPIO_Open(E_GPA,0, E_IO_INPUT);
	
	init_LCD();  // initialize LCD pannel
	clear_LCD();  // clear LCD panel 
	print_Line(0,"Smpl_ADC_MQ3");

  DrvADC_StartConvert();                   // start A/D conversion	
  //while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done	
	while(1)
	{
		    gpio_value = DrvGPIO_GetBit(E_GPA,0);
        adc_value = DrvADC_GetConversionData(1);   // read ADC value from ADC register
		    sprintf(TEXT,"GPA0: %1d",gpio_value);
		    print_Line(1, TEXT);
 		    sprintf(TEXT,"ADC1: %4d",i,adc_value);  // convert ADC value into text
		    print_Line(2, TEXT);		               // output TEXT to LCD display	    
		//DrvADC_Close();
	}
}
