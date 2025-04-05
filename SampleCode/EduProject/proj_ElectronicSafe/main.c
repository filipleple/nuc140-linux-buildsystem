//                                                                                                         */
// proj_ElectronicSafe
//
// Blue   LED flash & Buzz once  when keycode input
// Green  LED flash & Buzz twice when keycode match passcode
// Red    LED flash & Buzz three when keycode mismatch passcode
//
// GPA11 : to control Electronic Magnet of Door Bolt
// GPA12 : to control Blue LED
// GPA13 : to control Green LED
// GPA14 : to control Red LED
// GPA15 : to control external Buzzer
// GPB11 : to control on-board Buzzer
// GPB15 : on-board button input (Nu-LB-NUC140 SW INT) to change passcode

#include <stdio.h>																											 
#include <string.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "Scankey.h"
#include "LCD.h"

#define  BLUE  1
#define  GREEN 2
#define  RED   3

#define  init_Lock  DrvGPIO_Open(E_GPA,11,E_IO_OUTPUT)
#define  OpenLock  DrvGPIO_SetBit(E_GPA,11)
#define  CloseLock DrvGPIO_ClrBit(E_GPA,11)

char TEXT0[16]="Electronic Safe";
char TEXT1[16]="KeyCode :      ";
char TEXT2[16]="PassCode:      ";
char passcode[7] = "123456";
char keycode[7]  = "000000";


void init_RGBLED(void)
{
	// initialize GPIO pins
	DrvGPIO_Open(E_GPA, 12, E_IO_OUTPUT); // GPA12 pin set to output mode
	DrvGPIO_Open(E_GPA, 13, E_IO_OUTPUT); // GPA13 pin set to output mode
	DrvGPIO_Open(E_GPA, 14, E_IO_OUTPUT); // GPA14 pin set to output mode
	// set GPIO pins output Hi to disable LEDs
	DrvGPIO_SetBit(E_GPA, 12); // GPA12 pin output Hi to turn off Blue  LED
	DrvGPIO_SetBit(E_GPA, 13); // GPA13 pin output Hi to turn off Green LED
	DrvGPIO_SetBit(E_GPA, 14); // GPA14 pin output Hi to turn off Red   LED
}

void RGBLED(uint8_t LEDcolor)
{
	switch(LEDcolor) {
		case BLUE:  DrvGPIO_ClrBit(E_GPA,12); // Blue LED on
		            DrvGPIO_SetBit(E_GPA,13);
		            DrvGPIO_SetBit(E_GPA,14);
		            break;
		case GREEN: DrvGPIO_SetBit(E_GPA,12);
		            DrvGPIO_ClrBit(E_GPA,13); // Green LED on
		            DrvGPIO_SetBit(E_GPA,14);
		            break;		
		case RED:   DrvGPIO_SetBit(E_GPA,12);
		            DrvGPIO_SetBit(E_GPA,13);
		            DrvGPIO_ClrBit(E_GPA,14); // Red   LED on
		            break;		
		default:    DrvGPIO_SetBit(E_GPA,12);
		            DrvGPIO_SetBit(E_GPA,13);
		            DrvGPIO_SetBit(E_GPA,14);
                break;  
  }		
	DrvSYS_Delay(100000 * LEDcolor);
  DrvGPIO_SetBit(E_GPA,12);
	DrvGPIO_SetBit(E_GPA,13);
	DrvGPIO_SetBit(E_GPA,14);
}

void init_Buzzer(void)
{
	DrvGPIO_Open(E_GPB, 11, E_IO_OUTPUT); // on-board Buzzer control : low-active
	DrvGPIO_Open(E_GPA, 15, E_IO_OUTPUT);	// external Buzzer control : high-active
	DrvGPIO_SetBit(E_GPB, 11);
	DrvGPIO_ClrBit(E_GPA, 15);	
}

void Buzz(uint8_t beep_no) 
{
	uint8_t i=0;
	while(i<beep_no) {
		DrvGPIO_ClrBit(E_GPB, 11);
		DrvGPIO_SetBit(E_GPA, 15);
		DrvSYS_Delay(100000);
		DrvGPIO_SetBit(E_GPB, 11);
		DrvGPIO_ClrBit(E_GPA, 15);
		DrvSYS_Delay(100000);		
		i++;
	}
}

void EINT1Callback(void)
{
	uint8_t i=0;
  uint8_t number=0;
	
	sprintf(TEXT1,"NewCode :");
	print_Line(1,TEXT1);
  while(1)
	{
		number = ScanKey();		
		if (number!=0) {
			keycode[i] = 0x30+number;
			sprintf(TEXT1+9+i,"%d",number);
			print_Line(1,TEXT1);
			i++;
			Buzz(1);
			RGBLED(BLUE);
			if (i==6) {
					RGBLED(GREEN);					
				  strcpy(passcode,keycode);
				  sprintf(TEXT2,"PassCode:%s", passcode);
				  print_Line(2,TEXT2);
					Buzz(2);
					CloseLock;
					DrvSYS_Delay(300000);
					RGBLED(BLUE);
					sprintf(TEXT1,"KeyCode :       ");
				  print_Line(1,TEXT1);
					print_Line(2,"                ");					
				  i=0;
				  break;
			}
		} 
	}

}

int32_t main (void)
{
	uint8_t  i, number;
	
	UNLOCKREG();                                 // Unlock the protected registers
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);          // Enable 12MHz oscillator
  DrvSYS_Delay(5000);                          // delay for stable clock
	DrvSYS_SelectHCLKSource(0);	                 // HCLK clock source. 0: external 12MHz;
	LOCKREG();                                   // Lock the protected registers

	init_RGBLED();                               // initial RGB LEDs
	init_Buzzer();		                           // initial Buzzer	
	init_Lock;
	
	CloseLock;
	DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);         // initial SWINT button for changing passcode
  DrvGPIO_EnableEINT1(E_IO_RISING, E_MODE_EDGE, EINT1Callback); // set up Interrupt callback	
	
	init_LCD();                             // initial LCD panel
	clear_LCD();

	OpenKeyPad();	                               // initialize 3x3 keypad
	print_Line(0,TEXT0);                          // print title
	print_Line(1,TEXT1);
	
  i=0;	
	while(1)
	{
		number = ScanKey();		
		if (number!=0) {
			keycode[i] = 0x30+number;
			sprintf(TEXT1+9+i,"%d",number);
			print_Line(1,TEXT1);
			i++;
			Buzz(1);
			RGBLED(BLUE);
			if (i==6) {
				if (strcmp(keycode,passcode)==0)
				{
					RGBLED(GREEN);
					sprintf(TEXT2,"PassCode:Correct");
					print_Line(2,TEXT2);					
					Buzz(3);
					OpenLock;
					DrvSYS_Delay(300000);
					print_Line(2,"                ");
				} else {
					RGBLED(RED);					
					print_Line(2,"PassCode:Wrong! ");
					Buzz(2);
					CloseLock;
					DrvSYS_Delay(300000);
					RGBLED(BLUE);
					print_Line(2,"                ");					
				}
				sprintf(TEXT1,"KeyCode :       ");
			  print_Line(1,TEXT1);				
				i = 0;
			}
		} 
	}
}

