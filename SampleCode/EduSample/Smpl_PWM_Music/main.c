//
// Smpl_PWM_Music : use PWM to generate musical notes
//
// Output : PWM0 (GPA12)

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "PWM.h"
#include "LCD.h"
#include "Note_Freq.h"

#define  P125ms 125000
#define  P250ms 250000
#define  P500ms 500000
#define  P1S   1000000

int32_t main (void)
{
  uint8_t i;
  uint32_t Clock;	
  uint32_t Frequency;
  uint8_t  PreScaler;
  uint8_t  ClockDivider;
  uint8_t  DutyCycle;
  uint16_t CNR, CMR;
  
  uint16_t music[72] = {
	E6 ,D6u,E6 ,D6u,E6 ,B5 ,D6 ,C6 ,A5 ,A5 , 0 , 0 ,
	C5 ,E5 ,A5 ,B5 ,B5 , 0 ,C5 ,A5 ,B5 ,C6 ,C6 , 0 ,
	E6 ,D6u,E6 ,D6u,E6 ,B5 ,D6 ,C6 ,A5 ,A5 , 0 , 0 ,
	C5 ,E5 ,A5 ,B5 ,B5 , 0 ,E5 ,C6 ,B5 ,A5 ,A5 , 0 ,
	B5 ,C6 ,D6 ,E6 ,E6 , 0 ,G5 ,F6 ,E6 ,D6 ,D6 , 0 ,
	F5 ,E6 ,D6 ,C6 ,C6 , 0 ,E5 ,D6 ,C6 ,B5 ,B5 , 0 };
	
  uint32_t pitch[72] = {
	P250ms, P250ms, P250ms, P250ms,	P250ms, P250ms, P250ms, P250ms,	P250ms, P250ms, P250ms, P250ms,
	P250ms, P250ms, P250ms, P250ms,	P250ms, P250ms, P250ms, P250ms,	P250ms, P250ms, P250ms, P250ms,
	P250ms, P250ms, P250ms, P250ms,	P250ms, P250ms, P250ms, P250ms,	P250ms, P250ms, P250ms, P250ms,
	P500ms, P500ms, P500ms, P500ms,	P500ms, P500ms, P500ms, P500ms,	P500ms, P500ms, P500ms, P500ms,
	P500ms, P500ms, P500ms, P500ms,	P500ms, P500ms, P500ms, P500ms,	P500ms, P500ms, P500ms, P500ms,
	P500ms, P500ms, P500ms, P500ms,	P500ms, P500ms, P500ms, P500ms,	P500ms, P500ms, P500ms, P500ms
	};

	//Enable 12Mhz and set HCLK->12Mhz
	char TEXT0[16],TEXT1[16],TEXT2[16],TEXT3[16];

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	init_LCD();	
	clear_LCD();   	
 	print_Line(0, "Smpl_PWM_Music");
	
// PWM_No = PWM channel number
// PWM_CLKSRC_SEL   = 0: 12M, 1:32K, 2:HCLK, 3:22M
// PWM_PreScaler    : PWM clock is divided by (PreScaler + 1)
// PWM_ClockDivider = 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
	init_PWM(0, 0, 119, 4); // initialize PWM0(GPA12) to output 1MHz square wave
	Clock = 12000000;
	PreScaler = 119;
	ClockDivider = 1;
	DutyCycle = 50;

	while(1)
	{
	  for (i=0; i<72; i++) {
			Frequency = music[i];
			//PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1)
			CNR = Clock / Frequency / (PreScaler + 1) / ClockDivider - 1;
      // Duty Cycle = (CMR0+1) / (CNR0+1)
      CMR = (CNR +1) * DutyCycle /100  - 1;			
			
	    PWM_Out(0, CNR, CMR);
			if (Frequency==0) PWM_Stop(0);
			sprintf(TEXT1,"Freq=%5dHz", music[i]); print_Line(1,TEXT1);
			sprintf(TEXT2,"CNR =%5d", CNR); print_Line(2,TEXT2);
			sprintf(TEXT3,"CMR =%5d", CMR); print_Line(3,TEXT3);
	    DrvSYS_Delay(pitch[i]); // delay between each note
	  }
	}
}
