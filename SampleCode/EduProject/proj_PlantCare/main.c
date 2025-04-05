//
// Smpl_ADC_Moist_pH : Plant Care
//
// ADC0 (GPA0) to read Moisture
// ADC1 (GPA1) to read pH

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "ADC.h"
#include "LCD.h"

void init_TIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=85;   // Set Prescale to 85 (1/3 sec)
	TIMER0->TCMPR = 46875;		  // (1/12000000)*(255+1)* 46875 = 1 sec

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

// read ADC every TIMER0 time-out
void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
	uint16_t adc_Moist, adc_pH;
	char TEXT[16];	
		DrvADC_StartConvert();                   // start A/D conversion
    while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
	  adc_Moist = ADC->ADDR[0].RSLT & 0xFFF;
		adc_pH    = ADC->ADDR[1].RSLT & 0xFFF;
		sprintf(TEXT,"Moist: %d",adc_Moist);
		print_Line(1, TEXT);		     
		sprintf(TEXT,"pH   : %d",adc_pH);
		print_Line(2, TEXT);
	  TIMER0->TISR.TIF =1; 	 
}

int32_t main (void)
{
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; //Enable 12Mhz and set HCLK->12Mhz
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();
	
	DrvGPIO_InitFunction(E_FUNC_TMR0);  // Set TMR0 pins
	DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 0x03, INTERNAL_HCLK, 1); // ADC1 & ADC0_mois
	
	init_LCD();  //call initial pannel function
	clear_LCD();							 	
	print_Line(0,"Smpl_PlantCare");
	
  init_TIMER0();	
	while(1)			   
	{
	}
}

