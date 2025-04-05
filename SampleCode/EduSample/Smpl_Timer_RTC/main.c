//
// Smpl_Timer_RTC : set RTC & its alarm
//
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD.h"

static uint8_t Alarm_E=1;

// RTC register access
void set_TLR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->TLR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	 ;
}
void set_CLR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->CLR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	 ;
}
void set_TAR(int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->TAR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	;
}
void set_CAR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->CAR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	;
}

// start Real-Time Clock
void START_RTC(void)
{
  	while(1)
  	{
		RTC->INIR = 0xa5eb1357;
		if(inpw(&RTC->INIR)==1)
        	break; 
  	}   
  	while(1)
  	{
  		RTC->AER.AER = 0xA965;
    	if(inpw(&RTC->AER)&0x10000)// AER bit
        	break;  
    }
}

// Initialize RTC
void init_RTC(void)
{
	UNLOCKREG();		
	/* Step 1. Enable and Select RTC clock source */     
	SYSCLK->PWRCON.XTL32K_EN = 1;//Enable 32Khz for RTC clock source
	SYSCLK->APBCLK.RTC_EN =1;//Enable RTC clock source	

	/* Step 2. Initiate and unlock RTC module */
	 
	START_RTC();

	/* Step 3. Initiate Time and Calendar  setting */
	RTC->TSSR.HR24_HR12     =1;//Set 24hour mode
	//Set time and calendar for loading register, Calendar„³ 09/1/19, Time„³ 13:20:00
	set_CLR(0,9,0,1,1,9);
	set_TLR(1,3,2,0,0,0);

	/* Step 4. Set alarm interrupt */
	//Set time and calendar for alarm register , Calendar„³ 09/1/19, Time„³ 13:30:00
	set_CAR(0,9,0,1,1,9);
	set_TAR(1,3,2,0,2,0);		
	//Enable alarm interrupt
	RTC->RIER.AIER = 1;
	RTC->RIER.TIER = 1;
	NVIC_EnableIRQ(RTC_IRQn);
}

// RTC Interrupt Handler
void RTC_IRQHandler(void)
{
	uint32_t clock;
	char TEXT_RTC[16]="RTC :           ";
	/* tick */
	if(inpw(&RTC->RIIR)&0x2)
 	{
		clock=inpw(&RTC->TLR)&0xFFFFFF;
		sprintf(TEXT_RTC+6,"%2x",(clock>>16)&0xFF);
		sprintf(TEXT_RTC+9,"%2x",((clock)>>8)&0xFF);
		sprintf(TEXT_RTC+12,"%2x",clock&0xFF);
		TEXT_RTC[8]=':';
		TEXT_RTC[11]=':';
		print_Line(1, TEXT_RTC);
		outpw(&RTC->RIIR,2);
	}
	/* alarm */
	if(inpw(&RTC->RIIR)&0x1) 
	{
		print_Line(2, "RTC Alarm !!!");
		GPIOC->DOUT&=0xFF;
		Alarm_E=0;
		outpw(&RTC->RIIR,1);
	}
}


int32_t main (void)
{
	UNLOCKREG();
	SYSCLK->PWRCON.XTL32K_EN = 1;//Enable 32KHz for RTC clock source
	SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz crystal
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	init_LCD();  
	clear_LCD();
	print_Line(0,"Smpl_Timer_RTC");	                        

	init_RTC();
		 	
	/* Synch field transmission & Request Identifier Field transmission*/
	while(Alarm_E)
	{
		UNLOCKREG();
		WDT->WTCR.WTR =1;//Write 1 to clear for safety
		LOCKREG();	
	}
	while(1)
	{
		__NOP();
	}
}



