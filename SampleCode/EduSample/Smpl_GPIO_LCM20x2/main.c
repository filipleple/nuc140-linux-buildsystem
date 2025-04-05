//
// Smpl_GPIO_LCM20x2 : Nu-LB-NUC140 connect to LCM 20x2
//
// LCM 20x2 
// pin1 Gnd
// pin2 Vcc: 						(to +5V)
// pin3 Vo : brightness control 	(4.7Kohm to Gnd)
// pin4 RS : 1=Data, 0=Instruction 	(to GPA0)
// pin5 RW : 1=Read, 0=Write 	   	(to GPA1)
// pin6 E  : Chip Enable		   	(to GPA2)
// pin7~14 : D0~D7				   	(to GPE0~7)
// pin15 A : backlight+ 			(to Vcc) 無背光則不用接
// pin16 K : backlight- 			(to Gnd) 無背光則不用接

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "LCM20x2.h"

main()
{
	char  TEXT0[20]= "Welcome to NTOUEE !!";
	char  TEXT1[20]= "Nuvoton CortexM0 MCU";

  init_LCM20x2();

 	printS(0,0,TEXT0);
 	printS(0,1,TEXT1);
} 
