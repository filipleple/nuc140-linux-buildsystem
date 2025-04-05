//
// NuMicro Nu-LB-NUC140
// Sample Code : Smpl_GPIO_LCM16x2
//
// LCD 16x2 Module
// pin1 Gnd							(to Gnd)
// pin2 Vcc: to +5V					(to +5V)
// pin3 Vo : brightness control 	(to Gnd)
// pin4 RS : 1=Data, 0=Instruction	(to GPA0)
// pin5 RW : 1=Read, 0=Write 		(to GPA1)
// pin6 E  : Chip Enable			(to GPA2)
// pin7~14 : D0~D7
// pin15 A : backlight+ 			(to Vcc) 無背光則不用接
// pin16 K : backlight- 			(to Gnd) 無背光則不用接

// DB[7:0]  : Description
// 0000_0001: Clear Display
// 0000_001x: Return to Home (Display RAM address=0)
// 0000_01DS: Cursor move direction, Display Shift
// 0000_1DCB: Display/Cursor/Blinking on/off
// 0001_SRxx: Curosor move, shift R/L
// 01xx_xxxx: set CGRAM
// 1xxx_xxxx: set DDRAM
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"


main()
{
	char  TEXT0[16]= "Welcome to NTOU ";
	char  TEXT1[16]= "Cortex-M0 MCU !!";

  init_LCM16x2();

 	printS(0,0,TEXT0);
 	printS(0,1,TEXT1);
} 
