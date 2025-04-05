//
// Smpl_GPIO_LED16x16 
//
// 16x16 LED Matrix Display module
// pin description & connection
// Vcc :              to VCC +5V
// Gnd :              to GND
// P24 : 74HC164 CLK  to GPA4 (NUC140-pin75)
// P23 : 74HC164 AB   to GPA3 (NUC140-pin74)
// P22 : 74HC595 RCK  to GPA2 (NUC140-pin73)
// P21 : 74HC595 SRCK to GPA1 (NUC140-pin72)
// P20 : 74HC595 SER  to GPA0 (NUC140-pin71)

#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "LED16x16.h"
#include "greenman.h"

#define  REPEAT 100
	
int32_t main (void)
{
	int i,j;
	int16_t r;
	uint8_t temp[32];
	
	UNLOCKREG();   // unlock protected registers
        DrvSYS_Open(48000000);
	LOCKREG();     // lock protected registers

	init_LED16x16();

	while(1) {		
		for (i=0;i<8;i++) {
		    for(j=0;j<32;j++) temp[j]=greenman[i*32+j];
		    for (r=0;r<REPEAT;r++)
		    display_LED16x16(temp);		
		}
  }		      		
}
