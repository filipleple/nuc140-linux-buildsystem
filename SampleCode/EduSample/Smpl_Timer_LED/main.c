//
// Smpl_Timer_LED : on/off LED every second
//
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"

#define  ONESHOT  0   // counting and interrupt when reach TCMPR number, then stop
#define  PERIODIC 1   // counting and interrupt when reach TCMPR number, then counting from 0 again
#define  TOGGLE   2   // keep counting and interrupt when reach TCMPR number, tout toggled (between 0 and 1)
#define  CONTINUOUS 3 // keep counting and interrupt when reach TCMPR number

volatile uint32_t ledState = 0;
  
void TMR0_IRQHandler()
{
	ledState = ~ ledState;  // changing ON/OFF state
	if(ledState) DrvGPIO_ClrBit(E_GPC,12);
	else         DrvGPIO_SetBit(E_GPC,12);
	TIMER0->TISR.TIF = 1;		// clear Interrupt flag
}

void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
	SYSCLK->APBCLK.TMR0_EN = 1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE = PERIODIC; //Select operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE = 255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 46875;		    // Set TCMPR [0~16777215]
	//Timeout period = (1 / 12MHz) * ( 127 + 1 ) * 46875 = 1 sec

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;	//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

int32_t main(void)
{
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz Crystal
  DrvSYS_Delay(5000); // waiting for 12MHz crystal stable
	SYSCLK->CLKSEL0.HCLK_S = 0;		
	LOCKREG();
	
	DrvGPIO_Open(E_GPC,12, E_IO_OUTPUT); // set GPC12 output for LED
	InitTIMER0();                        // Set Timer Ticking
	
	while(1) 
	{ // Do Nothing
	}
}

