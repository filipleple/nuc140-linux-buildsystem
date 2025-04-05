//
// Smpl_Timer_WDT_RTC : Timer0, Watch-Dog Timer, RTC & its alarm
//
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD.h"

static uint16_t TimerCounter=0;
static uint8_t Alarm_E=1;

// Initialize Watch-Dog Timer
void init_WDT(void)
{
	UNLOCKREG();		
	/* Step 1. Enable and Select WDT clock source */         
	SYSCLK->CLKSEL1.WDT_S =3;//Select 10Khz for WDT clock source  	
	SYSCLK->APBCLK.WDT_EN =1;//Enable WDT clock source	

	/* Step 2. Select Timeout Interval */
	WDT->WTCR.WTIS=5;//Select level 7			
	/* Step 3. Disable Watchdog Timer Reset function */
	WDT->WTCR.WTRE = 0;
	/* Step 4. Enable WDT interrupt */	
	WDT->WTCR.WTIF =1;//Write 1 to clear for safety		
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
	UNLOCKREG();
	print_Line(3, "WDT Interrupt !");
}

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
		sprintf(TEXT_RTC+6,"%02x",(clock>>16)&0xFF);
		sprintf(TEXT_RTC+9,"%02x",((clock)>>8)&0xFF);
		sprintf(TEXT_RTC+12,"%02x",clock&0xFF);
		TEXT_RTC[8]=':';
		TEXT_RTC[11]=':';
		print_Line(0, TEXT_RTC);
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

// Initialize Timer0
void init_TIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
    SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 2765;		// Set TCMPR [0~16777215]
								// (1/12000000)*(255+1)*(2765)= 125.01usec or 7999.42Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0
//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

// Timer0 Interrupt Handler
void TMR0_IRQHandler(void)
{
	char TEXT_TIMER[12]="Timer:";
	TimerCounter+=1;
	sprintf(TEXT_TIMER+6,"%d",TimerCounter);
	print_Line(1, TEXT_TIMER);
 	TIMER0->TISR.TIF =1; 	   
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
	                        
	init_TIMER0();
	init_RTC();
	init_WDT();
		 	
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



