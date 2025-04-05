//
// Smpl_WDT : Watch-Dog Tiimer
// 
// InitWDT() setup for WDT Interrupt
// 	WDT->WTCR.WTRE = 0; //WDT Reset Enable = 0
//	WDT->WTCR.WTIF = 1; //WDT Interrupt Flag, write 1 to clear
//	WDT->WTCR.WTIE = 1; //WDT Interrupt Enable =1
// InitWDT() setup for WDT Reset 
// 	WDT->WTCR.WTRE = 1; //WDT Reset Enable = 1
//	WDT->WTCR.WTIF = 1; //WDT Interrupt Flag, write 1 to clear
//	WDT->WTCR.WTIE = 0; //WDT Interrupt Enable =0
// main() while loop display a counter on LCD to tell if reset ever occur

#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD.h"

// Initialize Watch-Dog Timer
void init_WDT(void)
{
	UNLOCKREG();		
	/* Step 1. Enable and Select WDT clock source */         
	SYSCLK->CLKSEL1.WDT_S =3;//Select 10Khz for WDT clock source  	
	SYSCLK->APBCLK.WDT_EN =1;//Enable WDT clock source	

	/* Step 2. Select Timeout Interval */
	WDT->WTCR.WTIS=5; 			
	/* Step 3. Disable Watchdog Timer Reset function */
	WDT->WTCR.WTRE = 0; 
	/* Step 4. Enable WDT interrupt */	
	WDT->WTCR.WTIF = 1;//Write 1 to clear for safety		
	WDT->WTCR.WTIE = 1;
	NVIC_EnableIRQ(WDT_IRQn);
	/* Step 5. Enable WDT module */
	//Enable WDT
	WDT->WTCR.WTE = 1;
	 //Clear WDT counter
	WDT->WTCR.WTR = 1;		
	LOCKREG();	 
}

// Watch-Dog Timer Interrupt Handler
void WDT_IRQHandler(void) 
{ 
	UNLOCKREG(); 
	WDT->WTCR.WTIF =1;
	WDT->WTCR.WTR = 1;
	LOCKREG();
	print_Line(2, "WDT Interrupt !!!");
}

int32_t main (void)
{
	uint32_t counter = 0;
	char TEXT[16];
	UNLOCKREG();
	SYSCLK->PWRCON.XTL32K_EN = 1;//Enable 32KHz for RTC clock source
	SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz crystal
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	init_LCD();  
	clear_LCD();

	print_Line(0,"Smpl_Timer_WDT");
	print_Line(1,"WatchDog Timer");
	
	init_WDT();
		 	
	while(1)
	{
		counter++;
		sprintf(TEXT, "%d", counter);
		print_Line(3, TEXT);
	}
}



