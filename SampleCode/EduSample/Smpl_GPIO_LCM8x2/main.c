//
// Smpl_GPIO_LCM8x2 : Nu-LB-NUC140 connect to a LCM8x2 display module
//
// LCM8x2
// pin1 Vss:						           (to Gnd)
// pin2 Vcc: to +5V					       (to +5V)
// pin3 Vee : brightness control 	 (to Gnd)
// pin4 RS : 1=Data, 0=Instruction (to GPA0)
// pin5 RW : 1=Read, 0=Write 		   (to GPA1)
// pin6 E  : Chip Enable			     (to GPA2)
// pin7~14 : D0~D7					       (to GPE0~7)

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "LCM8x2.h"

main()
{
	char  TEXT0[8]= "Nuvoton ";
	char  TEXT1[8]= "CortexM0";
	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1); // Enable the 12MHz oscillator oscillation
	DrvSYS_SelectHCLKSource(0); // HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator
	LOCKREG();

  init_LCM8x2();

 	printS(0,0,TEXT0);//print at Row=0, Column=0
 	printS(0,1,TEXT1);//print at Row=1, Column=0
} 
