//
// Smpl_PWM_DeadZone : demo DeadZone between PWM 0 & 1 output
// PWM 0/1 output rising edge will be pushed out for number of PWM clock
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "PWM.h"

int32_t main (void)
{
	S_DRVPWM_TIME_DATA_T sPt;
	uint8_t u8DeadZoneLength;
	
	UNLOCKREG();
	DrvSYS_Open(50000000);
	LOCKREG();

	DrvPWM_Open(); // Initial PWM
	DrvGPIO_InitFunction(E_FUNC_PWM01);	//Enable PWM Function pins
	DrvPWM_SelectClockSource(DRVPWM_TIMER0, DRVPWM_EXT_12M);	//Select PWM engine clock source
	
	//PWM Timer property
	sPt.u8Mode = DRVPWM_AUTO_RELOAD_MODE;
	sPt.u8HighPulseRatio = 30;		//High Pulse period : Total Pulse period = 30 : 100
	sPt.i32Inverter = 0;
	sPt.u32Duty = 1000;
	sPt.u8PreScale = 119;
	sPt.u8ClockSelector = DRVPWM_CLOCK_DIV_1;
 	 //Set PWM Timer0 Configuration
	DrvPWM_SetTimerClk(DRVPWM_TIMER0, &sPt); 	
	//Enable Output for PWM Timer 0 and Timer 1
	DrvPWM_SetTimerIO(DRVPWM_TIMER0, 1);
	DrvPWM_SetTimerIO(DRVPWM_TIMER1, 1);

	//Enable Timer0 and Time1 dead zone function and Set dead zone length to 10
	u8DeadZoneLength = 16;
	DrvPWM_EnableDeadZone(DRVPWM_TIMER0, u8DeadZoneLength, ENABLE);
	//Enable the PWM Timer 0
	DrvPWM_Enable(DRVPWM_TIMER0, 1);
	
	while(1)
	{
		DrvSYS_Delay(1000000);		//Delay 1 Second
		
		//Per second change the dead zone length of PWM0/1 Output
		u8DeadZoneLength += 16;
		DrvPWM_EnableDeadZone(DRVPWM_TIMER0, u8DeadZoneLength, ENABLE);
	}
}

