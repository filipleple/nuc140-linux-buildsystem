/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "Scankey.h"
#include "LCD.h"
#include "EEPROM_24LC64.h"

int main(void)
{
	uint32_t i2cdata=0;
	unsigned char temp;
	char TEXT0[16]="                ";
	char TEXT1[16]="Addr:           ";
	char TEXT2[16]="Write:          ";
	char TEXT3[16]="Read:           ";

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();	
	
	init_LCD();  //call initial pannel function
	clear_LCD();
	
	print_Line(0, "I2C with 24LC65");
	print_Line(1, "test read and  ");
	print_Line(2, "write function ");	  
	print_Line(3, "press key1-key9");

	DrvGPIO_InitFunction(E_FUNC_I2C1); 	
  OpenKeyPad();

	while(1)
	{	
	  temp=ScanKey();

		if (temp!=0) sprintf(TEXT0,"Key%d is pressed",temp);
		else         sprintf(TEXT0,"Key not pressed");
		print_Line(0,TEXT0);
		print_Line(1,"               ");
		print_Line(2,"               ");
	  print_Line(3,"               ");
		 	  	  
		Write_24LC64(0x00000000+temp,temp+temp*10+temp);
		i2cdata= Read_24LC64(0x00000000+temp);
		sprintf(TEXT1+6,"%x",temp);
		sprintf(TEXT2+6,"%x",temp+11);
		sprintf(TEXT3+6,"%x",i2cdata);
		print_Line(1,TEXT1);
		print_Line(2,TEXT2);
		print_Line(3,TEXT3);
		
    DrvSYS_Delay(200000);		
	}
}
