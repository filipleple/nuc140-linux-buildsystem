/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "GPIO.h"
#include "I2C.h"
#include "SYS.h"
#include "UART.h"
#include "NAU7802.h"
#include "LCD.h"
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	uint32_t ADCresult;
	char adc_value[15];

	init_LCD();  //call initial pannel function
	clear_LCD();
	
	print_Line(0, "Welcome! Nuvoton");	  
	print_Line(1, "NAU7802 24-bits ");
	print_Line(2, "ADC value       ");	  


	InitNAU7802();
	while(1)
	{
		ADCresult=GetADCResult_NAU7802();
		sprintf(adc_value,"%8d",ADCresult);
		print_Line(3, adc_value);
	}
}




