//
// Smpl_7seg_keypad
//
// Input:  3x3 keypad (input = 1~9 when key is pressed, =0 when key is not pressed
// Output: 7-segment LEDs
//
#include <stdio.h>																											 
#include "NUC1xx.h"
#include "SYS.h"
#include "Seven_Segment.h"
#include "Scankey.h"

int32_t main (void)
{
	int8_t number;
	
	UNLOCKREG();
	DrvSYS_Open(50000000);
	LOCKREG();

	OpenKeyPad();					 	
	 
	while(1)
	{
	  number = ScanKey();           // scan keypad to get a number (1~9)
		show_seven_segment(0,number); // display number on 7-segment LEDs
		DrvSYS_Delay(5000);           // delay time for keeping 7-segment display 
		close_seven_segment();	      // turn off 7-segment LEDs								 
	}
}
