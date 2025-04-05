//
// Smpl_Timer_SR04 : using one SR04 Ultrasound Sensors
//
// Timer Capture :
// GPB2 / RTS0 / T2EX (NUC140 pin34)
// GPB4 / RX1         (NUC140 pin19)

// SR04 Ultrasound Sensor 
// pin1 Vcc : to Vcc
// pin2 Trig: to GPB4      (NUC140VE3xN pin19)
// pin3 ECHO: to GPB2/T2EX (NUC140VE3xN pin34)
// pin4 Gnd : to GND

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "LCD.h"

// Global definition
#define	_SR04A_ECHO		   (GPB_2)			//NUC140VE3xN, Pin19
#define	_SR04A_TRIG		   (GPB_4)			//NUC140VE3xN, Pin34
#define	_SR04A_TRIG_Low	 (GPB_4=0)
#define	_SR04A_TRIG_High (GPB_4=1)

// Global variables
volatile uint32_t SR04A_Echo_Width = 0;
volatile uint32_t SR04A_Echo_Flag  = FALSE;
//
// Initial TMR2
//
// Timer Clock:	12 MHz
// Prescale:		11
// Compare:		0xffffff
// Mode:			One-Shot mode
// Capture:		Enable, Capture with Falling Edge
void init_TMR2(void)
{	
	//Step 1: T2EX pin Enable (PB.2, Pin34)
	SYS->GPBMFP.UART0_nRTS_nWRL = 1;	
	SYS->ALTMFP.PB2_T2EX = 1;
	
  //Step 2: Timer Controller Reset and Setting Clock Source
	SYS->IPRSTC2.TMR2_RST = 1;          //Timer Controller: Reset
	SYS->IPRSTC2.TMR2_RST = 0;          //Timer Controller: Normal
	SYSCLK->CLKSEL1.TMR2_S = 0;	        //Timer Clock = 12 MHz
	SYSCLK->APBCLK.TMR2_EN = 1;         //Timer C lock Enable

	//Step 3: Timer Controller Setting
	//  TMR0_T = (12 MHz / (11+1) / 1000000)^-1 = 1.000 Second
	TIMER2->TCMPR = 0xffffff;           //Timer Compare Value:  [0~16777215]
	TIMER2->TCSR.PRESCALE = 11;         //Timer Prescale:       [0~255]
	TIMER2->TCSR.MODE = 0;              //Timer Operation Mode: One-Shot

	//Step 4: External Capture Mode Setting
	TIMER2->TEXCON.TEXEN = 1;	          //External Capture Function Enable
	TIMER2->TEXCON.RSTCAPSEL = 0;	      //Capture Mode Select: Capture Mode
	TIMER2->TEXCON.TEX_EDGE = 2;	      //Capture Edge: Rising & Falling

	//Step 5: Timer Interrupt Setting
//	TIMER2->TCSR.IE = 1;				      //Timeout Interrupt Enable
//	TIMER2->u32TISR |= 0x01;		      //Clear Timeout Flag (TIF)
	TIMER2->TEXCON.TEXIEN = 1;		      //Capture Interrupt Enable
	TIMER2->u32TEXISR |= 0x01;		      //Clear Capture Flag (TEXIF)
	NVIC_EnableIRQ(TMR2_IRQn);		      //Timer NVIC IRQ Enable

	//Step 6: Start Timer Capture (Set by Ultrasonic_Trigger() Function)
// 	TIMER2->TCSR.CRST = 1;			      //Timer Counter Reset
// 	TIMER2->TCSR.CEN = 1;				      //Timer Start
}

// TMR2 Interrupt Handler
void TMR2_IRQHandler(void)
{
	TIMER2->TEXCON.RSTCAPSEL = 0;       // set back for falling edge to capture
	TIMER2->TCSR.CEN = 1;					      //Timer Start

	if(TIMER2->TEXISR.TEXIF == 1)	      //Capture Flag (TEXIF)
	{
	 	TIMER2->u32TEXISR |= 0x01;				//Clear Capture Flag (TEXIF)
		SR04A_Echo_Width = TIMER2->TCAP;	//Load Capture Value (Unit: us)
		SR04A_Echo_Flag  = TRUE;
	}
}

// Ultrasonic Trigger
void SR04_Trigger(void)
{
	//Trigger of Ultrasonic Sensor
	_SR04A_TRIG_High;
	DrvSYS_Delay(10);							// 10us for TRIG width
	_SR04A_TRIG_Low;
	
  TIMER2->TEXCON.RSTCAPSEL = 1; // set for rising edge trigger to reset counter
}

void init_GPIO_SR04(void)
{
	//Ultrasonic I/O Pins Initial
	GPIOB->PMD.PMD2 = 0;							//_SR04_ECHO pin, Input						
	GPIOB->PMD.PMD4 = 1;              //_SR04_TRIG pin, Output
  _SR04A_TRIG_Low;                  // set Trig output to Low
}

//------------------------------
// MAIN function
//------------------------------
int main (void)
{
	char     TEXT2[16] = "SR04a: ";
	uint32_t distance_mm;
	
	//System Clock Initial
	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, ENABLE);
	while(DrvSYS_GetChipClockSourceStatus(E_SYS_XTL12M) == 0);
	DrvSYS_Open(50000000);
	LOCKREG();
	
	init_LCD();                  // initialize LCD
	clear_LCD();                  // clear LCD display
	print_Line(0, "Smpl_Timer_SR04 "); // Line 0 display
	                          
	init_TMR2();                      // initialize Timer2 Capture
  init_GPIO_SR04();
  
	while(1) {
		SR04_Trigger();                 // Trigger Ultrasound Sensor for 10us   		
		DrvSYS_Delay(40000);            // Wait 40ms for Echo to trigger interrupt
		
		if(SR04A_Echo_Flag==TRUE)
		{
			SR04A_Echo_Flag = FALSE;			

			distance_mm = SR04A_Echo_Width * (340/2) / 1000;
			sprintf(TEXT2+6, " %4d mm  ", distance_mm);	
			print_Line(2, TEXT2);	        //Line 2: distance [mm]
	  }   
		
		DrvSYS_Delay(10000);           // 10ms from Echo to next Trigger
	}
	
}
