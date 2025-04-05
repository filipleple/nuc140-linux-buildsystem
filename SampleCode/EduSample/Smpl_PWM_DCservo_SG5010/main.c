//
// Smpl_PWM_DCservo_SG5010
//
// using PWM to generate 50Hz (20ms) pulse to DC Servo signal pin
// 0.5 ~ 2.5ms high time (PWM clock at 10us per count)

// SG5010 DC servo
// pin1 : signal to PWM0/GPA12 (NUC140-pin65/NUC120-pin28)
// pin2 : Vcc
// pin3 : Gnd

#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "PWM.h"
#include "LCD.h"

//#define  SERVO_CYCTIME        2000 // 20ms = 50Hz
#define  SERVO_HITIME_MIN       50 // minimum Hi width = 0.5ms
#define  SERVO_HITIME_MAX      250 // maximum Hi width = 2.5ms
#define  STEPTIME               10 // incremental time = 1.0ms

int32_t main (void)
{
  uint8_t i;
  uint8_t hitime;
  char TEXT0[16],TEXT1[16],TEXT2[16],TEXT3[16];
  uint32_t Clock;	
  uint32_t Frequency;
  uint8_t  PreScaler;
  uint8_t  ClockDivider;
  uint8_t  DutyCycle;
  uint16_t CNR, CMR;
	
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN=1;
	DrvSYS_Delay(5000);					// Waiting for 12M Xtal stalble
	SYSCLK->CLKSEL0.HCLK_S=0;
	LOCKREG();

	init_LCD();
	clear_LCD();
	print_Line(0,"Smpl_PWM_SG5010");
	print_Line(1,"PWM Freq: 20ms ");
	
// PWM_No = PWM channel number
// PWM_CLKSRC_SEL   = 0: 12M, 1:32K, 2:HCLK, 3:22M
// PWM_PreScaler    : PWM clock is divided by (PreScaler + 1)
// PWM_ClockDivider = 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
	init_PWM(0, 0, 119, 4); // initialize PWM0(GPA12) to output 1MHz square wave
	Clock = 12000000;
	PreScaler = 119;
	ClockDivider = 1;
	Frequency = 50;

	hitime = SERVO_HITIME_MIN;
	while(1)
	{
	  hitime=hitime + (SERVO_HITIME_MAX - SERVO_HITIME_MIN)/10;
	  if (hitime>SERVO_HITIME_MAX) hitime = SERVO_HITIME_MIN;	
		
	  //PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1)
	  CNR = Clock / Frequency / (PreScaler + 1) / ClockDivider - 1;
	  // Duty Cycle = (CMR0+1) / (CNR0+1)
	  CMR = hitime  - 1;		
	  PWM_Out(0, CNR, CMR);
			
	  sprintf(TEXT1,"HiTime: %d0us ", hitime); print_Line(1,TEXT1);
	  sprintf(TEXT2,"CNR =%d", CNR); print_Line(2,TEXT2);
	  sprintf(TEXT3,"CMR =%d", CMR); print_Line(3,TEXT3);			
	  DrvSYS_Delay(100000);
	}
}
