// 
// Smpl_GPIO_LED4_macro : GPC12 ~ 15 to control on-board LEDs
//                        low-active output to control Red LEDs
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"

#define  INIT_LED0 DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT)
#define  INIT_LED1 DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT)
#define  INIT_LED2 DrvGPIO_Open(E_GPC, 14, E_IO_OUTPUT)
#define  INIT_LED3 DrvGPIO_Open(E_GPC, 15, E_IO_OUTPUT)
#define  LED0_ON   DrvGPIO_ClrBit(E_GPC, 12)
#define  LED0_OFF  DrvGPIO_SetBit(E_GPC, 12)
#define  LED1_ON   DrvGPIO_ClrBit(E_GPC, 13)
#define  LED1_OFF  DrvGPIO_SetBit(E_GPC, 13)
#define  LED2_ON   DrvGPIO_ClrBit(E_GPC, 14)
#define  LED2_OFF  DrvGPIO_SetBit(E_GPC, 14)
#define  LED3_ON   DrvGPIO_ClrBit(E_GPC, 15)
#define  LED3_OFF  DrvGPIO_SetBit(E_GPC, 15)
#define  DELAY     DrvSYS_Delay(300000)

// Initial GPIO pins (GPC 12,13,14,15) to Output mode  
void Init_LED()
{
	// initialize GPIO pins to OUTPUT mode
  INIT_LED0;
	INIT_LED1;
	INIT_LED2;
	INIT_LED3;
	// set GPIO pins to output Low
  LED0_OFF;
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
}

int main (void)
{
	UNLOCKREG();			   // unlock register for programming
  DrvSYS_Open(50000000);// set System Clock to run at 48MHz (PLL with 12MHz crystal input)
	LOCKREG();				   // lock register from programming

  Init_LED();        // Initialize LEDs (four on-board LEDs below LCD panel)

	while (1)				   // forever loop to keep flashing four LEDs one at a time
	{
	  LED0_ON;
	  DELAY;
	  LED0_OFF;
	  DELAY;
	  LED1_ON;
	  DELAY;
	  LED1_OFF;
	  DELAY;
	  LED2_ON;
	  DELAY;
	  LED2_OFF;
	  DELAY;
	  LED3_ON;
	  DELAY;
	  LED3_OFF;
	  DELAY;		
	}
}




