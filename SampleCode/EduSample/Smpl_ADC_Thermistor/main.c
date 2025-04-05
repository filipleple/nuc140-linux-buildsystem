//
// Smpl_ADC_Thermistor : Thermistor
//
// use ADC6 to read Thermistor resistance
// pin GPA6 : ADC6 

// resistance = temperature
// NTSA0103JZ100
// R = 10K     ohm at 25 degree Celsius
// R = 3.496 K ohm at 50 degree Celsius

#include <stdio.h>																										 
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "LCD.h"

void InitADC(void)
{
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD|=0x00800000; 	//Disable digital input path
	SYS->GPAMFP.ADC6_AD7=1; 	//Set ADC function 
				
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 2;	//Select 22Mhz for ADC
	SYSCLK->CLKDIV.ADC_N = 1;	//ADC clock source = 22Mhz/2 =11Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;	//Enable clock source
	ADC->ADCR.ADEN = 1;			//Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	//single end input
	ADC->ADCR.ADMD   = 0;     	//single mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x40;    // channel bit [7:0]
	
	/* Step 5. Enable ADC interrupt */
	ADC->ADSR.ADF =1;     		//clear the A/D interrupt flags for safe 
	ADC->ADCR.ADIE = 1;
//	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. Enable WDT module */
	ADC->ADCR.ADST=1;
}

void Delay(int count)
{
	while(count--);
}


int32_t main (void)
{
  uint32_t R = 9950; 	// pullup resister
	uint32_t r; 		    // thermister
	uint32_t adc_value;

	char TEXT0[16]="ADC6  Thermistor";
	char TEXT1[16]="ADC  :          ";
	char TEXT2[16]="r    :          ";
	char TEXT3[16]="Temp.:        .C";

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; //Enable 12Mhz and set HCLK->12Mhz
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	InitADC();

	init_LCD();  //call initial pannel function
	clear_LCD();
							 	
	print_Line(0,TEXT0);
					 
	while(1)			   
	{
		while(ADC->ADSR.ADF==0);
		ADC->ADSR.ADF=1;
		adc_value = ADC->ADDR[6].RSLT & 0x0FFC;

		r = R * (4096 - adc_value) / adc_value;
		// adc_value = 4096 * R / (R+r);

		sprintf(TEXT1+7,"%4d", adc_value);
		print_Line(1, TEXT1);

		sprintf(TEXT2+7,"%4d ohm", r);
		print_Line(2, TEXT2);

		sprintf(TEXT3+7,"%s .C", "----");
		print_Line(3,TEXT3);

		Delay(1000);
		ADC->ADCR.ADST=1;	   
	}
}

