//                                                                                                         */
// Smpl_LCD_Keypad : input 3x3 keypad, output to LCD display
//
#include <stdio.h>																											 
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "Scankey.h"
#include "LCD.h"

int32_t main (void)
{
	int8_t number;
	char TEXT0[16]="Smpl_LCD_Keypad";
	char TEXT1[16]="Keypad:        ";
	
	UNLOCKREG();
  DrvSYS_Open(48000000); // set MCU to run at 48MHz
	LOCKREG();

	init_LCD(); 
	clear_LCD();

	OpenKeyPad();	       // initialize 3x3 keypad
	print_Line(0,TEXT0); // print title
	 
	while(1)
	{
	  number = ScanKey(); 	        // scan keypad to input
		sprintf(TEXT1+8,"%d",number); // print scankey input to string			  
		print_Line(1, TEXT1);         // display string on LCD
		DrvSYS_Delay(5000); 		      // delay 																	 
	}
}

