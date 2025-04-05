//
// Smpl_ADC4GAS : 
// reading four GAS sensors from ADC 0,1,2,3
// display ADC values on LCD
//
// Gas Sensor-0 : connected to ADC0/GPA0
// Gas Sensor-1 : connected to ADC1/GPA1
// Gas Sensor-2 : connected to ADC2/GPA2
// Gas Sensor-3 : connected to ADC3/GPA3

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "ADC.h"
#include "LCD.h"

int32_t main (void)
{
	int i;
	char TEXT[4][16];
	uint16_t adc_value[4];

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();

  // The maximum ADC clock rate is 16MHz
	// set 4 channels of ADC
  DrvADC_Open(ADC_SINGLE_END, ADC_CONTINUOUS_OP, 0x0F, INTERNAL_HCLK, 1);
	
	init_LCD();  // initialize LCD pannel
	clear_LCD();  // clear LCD panel 

  DrvADC_StartConvert();                   // start A/D conversion	
  //while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done	
	while(1)
	{
		for (i=0; i<4; i++) {
        adc_value[i] = DrvADC_GetConversionData(i);   // read ADC value from ADC register
 		    sprintf(TEXT[i],"ADC%1d:%d",i,adc_value[i]);  // convert ADC value into text
		    print_Line(i, TEXT[i]);		                    // output TEXT to LCD display
		}
		//DrvADC_Close();
	}
}
