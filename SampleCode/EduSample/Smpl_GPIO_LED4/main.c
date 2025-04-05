// 
// Smpl_GPIO_LED4 : GPC12 ~ 15 to control on-board LEDs
//                  low-active output 
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"

// Initial GPIO pins (GPC 12,13,14,15) to Output mode  
void Init_LED()
{
	// initialize GPIO pins
	DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT); // GPC12 pin set to output mode
	DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT); // GPC13 pin set to output mode
	DrvGPIO_Open(E_GPC, 14, E_IO_OUTPUT); // GPC14 pin set to output mode
	DrvGPIO_Open(E_GPC, 15, E_IO_OUTPUT); // GPC15 pin set to output mode
	// set GPIO pins to output Low
	DrvGPIO_SetBit(E_GPC, 12); // GPC12 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 13); // GPC13 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 14); // GPC14 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 15); // GPC15 pin output Hi to turn off LED
}

int main (void)
{
	UNLOCKREG();			    // unlock register for programming
  DrvSYS_Open(50000000);// set System Clock to run at 48MHz 
	                      // 12MHz crystal input, PLL output 48MHz
	LOCKREG();				    // lock register from programming

  Init_LED();  // Initialize LEDs (four on-board LEDs below LCD panel)
 
	while (1)	   // forever loop to keep flashing four LEDs one at a time
	{
		DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 12); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	  DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 13); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	  DrvGPIO_ClrBit(E_GPC, 14); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 14); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	  DrvGPIO_ClrBit(E_GPC, 15); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 15); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	}
}
