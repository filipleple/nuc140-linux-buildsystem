//
// Smpl_ADC_PWM_TIMER : ADC7 to read VR1 resistance value, PWM0 output to control LED (GPA12)
//
#include <stdio.h>	
#include <math.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "LCD.h"

#define  ONESHOT  0   // counting and interrupt when reach TCMPR number, then stop
#define  PERIODIC 1   // counting and interrupt when reach TCMPR number, then counting from 0 again
#define  TOGGLE   2   // keep counting and interrupt when reach TCMPR number, tout toggled (between 0 and 1)
#define  CONTINUOUS 3 // keep counting and interrupt when reach TCMPR number

static uint32_t Timer_Count =0; // Timer_Count incremented every 1 ms
	
void InitADC(void)
{
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD|=0x00800000; 	//Disable digital input path
	SYS->GPAMFP.ADC7_SS21_AD6=1; 		//Set ADC function
	//SYS->GPAMFP.ADC6_AD7=1; 		//Set ADC function 	
			
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 2;	//Select 22Mhz for ADC
	SYSCLK->CLKDIV.ADC_N = 1;	//ADC clock source = 22Mhz/2 =11Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;	//Enable clock source
	ADC->ADCR.ADEN = 1;			//Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	//single end input
	ADC->ADCR.ADMD   = 0;     	//single mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x80; // ADC7
	//ADC->ADCHER.CHEN = 0x40; // ADC6
	
	/* Step 5. Enable ADC interrupt */
	ADC->ADSR.ADF =1;     		//clear the A/D interrupt flags for safe 
	ADC->ADCR.ADIE = 1;
//	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. Enable WDT module */
	ADC->ADCR.ADST=1;
}
//--------------------------------------------
void InitPWM(void)
{
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13=1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 3;//Select 22.1184Mhz for PWM clock source

	PWMA->PPR.CP01=1;			//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0=0;			// PWM clock = clock source/(Prescaler + 1)/divider
				         
	/* Step 3. Select PWM Operation mode */
	//PWM0
	PWMA->PCR.CH0MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0=0xFFFF;
	PWMA->CMR0=0xFFFF;

	PWMA->PCR.CH0INV=0;			//Inverter->0:off, 1:on
	PWMA->PCR.CH0EN=1;			//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM0=1;			//Output to pin->0:Diasble, 1:Enable
}

// Timer0 initialize to tick every 1ms
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
	SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE = PERIODIC;		//Select once mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER0->TCMPR = 1000;		// Set TCMPR [0~16777215]
	//Timeout period = (1 / 12MHz) * ( 11 + 1 ) * 1,000 = 1 ms

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;	//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
  Timer_Count++;	
	TIMER0->TISR.TIF =1; 	   // clear Timer0 interrrupt flag
}

/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	char TEXT0[16],TEXT1[16],TEXT2[16],TEXT3[16];
  uint32_t adc_value;	
	
	//Enable 12Mhz and set HCLK->12Mhz
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	InitPWM();
	InitADC();
	InitTIMER0();

	init_LCD();  //call initial pannel function
	clear_LCD();
	print_Line(0,"ADC_PWM_TIMER");
							 	
	/* Synch field transmission & Request Identifier Field transmission*/
					 
	while(1)
	{
		Timer_Count = 0;               // reset timer count 	
		ADC->ADCR.ADST=1;              // start ADC sampling		
		while(ADC->ADSR.ADF==0);       // wait for ADC sample finished
		ADC->ADSR.ADF=1;               // clear ADC flag
		adc_value = ADC->ADDR[7].RSLT; // read ADC7 value
		//adc_value = ADC->ADDR[6].RSLT; // read ADC6 value
		
		PWMA->CMR0 = adc_value <<4;    // set PWM counter with ADC value to drive LED
	
    sprintf(TEXT0,"ADC_PWM_TIMER");
		print_Line(0,TEXT0);
		sprintf(TEXT1,"ADC=%4d",adc_value); // print adc_value to string 
		print_Line(1,TEXT1);
		sprintf(TEXT2,"VOLT=%f v", 3.3 * adc_value / 4096); // convert adc_value to voltage value
		print_Line(2,TEXT2);
	  sprintf(TEXT3,"TIMER=%d ms", Timer_Count); // print out TIMER
		print_Line(3,TEXT3);	
		
		DrvSYS_Delay(200000);         // delay 200000 us		
	}
}

