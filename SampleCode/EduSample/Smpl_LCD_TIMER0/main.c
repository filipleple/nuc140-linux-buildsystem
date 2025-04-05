//
// Smpl_LCD_TIMER0 : TIMER 0 set at .25 sec to shift & print string on LCD
//
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "LCD.h"

static int8_t x_pos = 0;

//---------------------------------------------------------------------------------TIMER
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=63;	// Set Prescale [0~255]
	TIMER0->TCMPR = 46875;		// Set TCMPR [0~16777215]
							             	// (1/12000000)*(63+1)* 46875 = 0.25 sec

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
	if (x_pos<(LCD_Xmax-8*3)) {
		printS(x_pos, 1*16, " M0");
	}
	else {
		printS(x_pos, 1*16, "   ");
		x_pos=0;
	}
	TIMER0->TISR.TIF =1;
	x_pos++;
}

int main(void)
{
	UNLOCKREG();
	DrvSYS_Open(50000000); // CPU run at 50MHz
	LOCKREG(); 

	init_LCD(); 
	clear_LCD();	
	print_Line(0, "Smpl_LCD_TIMER");	  
	
	InitTIMER0();
	while(1) {
  }   	  	 	  		
}

