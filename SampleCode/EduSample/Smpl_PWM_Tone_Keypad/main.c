//
// Smpl_PWM_Tone_Keypad 
//
// use Keypad to select a tone
// use PWM to output the tone
// Output : PWM0 (GPA12)

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "PWM.h"
#include "LCD.h"
#include "Note_Freq.h"
#include "Scankey.h"

int32_t main (void)
{
  uint8_t  number;
	uint16_t tone;
  uint32_t Clock;	
	uint32_t Frequency;
	uint8_t  PreScaler;
	uint8_t  ClockDivider;
	uint8_t  DutyCycle;
	uint16_t CNR, CMR;	

	//Enable 12Mhz and set HCLK->12Mhz
	char TEXT0[16],TEXT1[16],TEXT2[16],TEXT3[16];

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	init_LCD();	
	clear_LCD();
 	print_Line(0,"PWM_Tone_Keypad");
	
	OpenKeyPad();
	
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
	  number = ScanKey(); // keypad input
	  switch(number) {
	  case 1 : tone=C5; break;
	  case 2 : tone=D5; break;
	  case 3 : tone=E5; break;
	  case 4 : tone=F5; break;
	  case 5 : tone=G5; break;
	  case 6 : tone=A5; break;
	  case 7 : tone=B5; break;
	  case 8 : tone=C6; break;
	  case 9 : tone=D6; break;
	  default: tone=0;  break;
 	  }
		
	  Frequency = tone;
	  //PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1)
	  CNR = Clock / Frequency / (PreScaler + 1) / ClockDivider - 1;
    	  // Duty Cycle = (CMR0+1) / (CNR0+1)
    CMR = (CNR +1) * DutyCycle /100  - 1;			
		
	  PWM_Out(0, CNR, CMR);
	  if (Frequency==0) PWM_Stop(0);
		
	  sprintf(TEXT1,"Freq= %5dHz",tone);
	  sprintf(TEXT2,"CNR = %5d", CNR);
	  sprintf(TEXT3,"CMR = %5d", CMR);
	  print_Line(1, TEXT1);
 	  print_Line(2, TEXT2);
 	  print_Line(3, TEXT3);
	}
}
