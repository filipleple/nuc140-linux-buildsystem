//
// Smpl_ADC_IRQ : 
// sampling ADC0~7 continuously
// use Interrupt Callback function to set interrupt flag
// display ADC7 value on LCD when interrupt flag is set

// Note: ADC7 is connected to on-board Variable Resistor
//       rotating VR to change the display value

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "ADC.h"
#include "LCD.h"

volatile uint8_t gu8AdcIntFlag;

void AdcIntCallback(uint32_t u32UserData)
{
    gu8AdcIntFlag = 1;	
}

int32_t main (void)
{
	int i;
	char TEXT[4][16];
	uint16_t adc_value[8];

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();
		
	init_LCD();  // initialize LCD pannel
	clear_LCD();  // clear LCD panel 
	print_Line(0,"Smpl_ADC_IRQ");
	
  // The maximum ADC clock rate is 16MHz
	// open 8 ADC channels for continously sampling
  DrvADC_Open(ADC_SINGLE_END, ADC_CONTINUOUS_OP, 0xFF, INTERNAL_HCLK, 1);
  DrvADC_EnableADCInt(AdcIntCallback, 0);

	i = 7;                 // ADC channel = 7 
	gu8AdcIntFlag =0;      // reset AdcIntFlag
 	DrvADC_StartConvert(); // start ADC sampling
	
	while(1) {	
		while(gu8AdcIntFlag==0); // wait till AdcIntFlag is set by AdcIntCallback
		gu8AdcIntFlag=0;		     // reset AdcIntFlag
    adc_value[i] = DrvADC_GetConversionData(i);   // read ADC value from ADC registers
 		sprintf(TEXT[2],"ADC%1d:%4d",i,adc_value[i]); // convert ADC value into text
		print_Line(2, TEXT[2]);		              // output TEXT to LCD display
	}
	//DrvADC_Close();	
}
