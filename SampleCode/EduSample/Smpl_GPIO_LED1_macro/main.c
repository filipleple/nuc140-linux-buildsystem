// 
// Smpl_GPIO_LED1: GPC12 to control on-board LEDs 
//                 output low to enable red LEDs
//                 define macro & replace function call to macro
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"

typedef enum 
{
	E_GPA = 0,
	E_GPB = 1, 
	E_GPC = 2, 
	E_GPD = 3, 
	E_GPE = 4
} E_DRVGPIO_PORT;

#define PORT_OFFSET   0x40
#define u32Reg (uint32_t)&GPIOA->PMD + (E_GPC*PORT_OFFSET)
#define u32RegD (uint32_t)&GPIOA->DOUT + (E_GPC*PORT_OFFSET)
#define DrvGPIO_Open   outpw(u32Reg, inpw(u32Reg)| (0x1<<(12*2)))
#define DrvGPIO_SetBit outpw(u32RegD, inpw(u32RegD)| (0x1 << 12  ))
#define DrvGPIO_ClrBit outpw(u32RegD, inpw(u32RegD)& ~(0x1<< 12  ))
#define DrvSYS_Open    DrvSYS_Open(50000000)
#define DrvSYS_Delay   DrvSYS_Delay(300000)
 
void Init_LED()   // Initialize GPIO pins
{
	DrvGPIO_Open;   // set GPC 12 to Output Mode
	DrvGPIO_SetBit; // output high to turn off LED
}

int main (void)
{
	UNLOCKREG();		  // unlock register for programming
  DrvSYS_Open;	    // set System Clock to run at 48MHz
	LOCKREG();			  // lock register from programming

  Init_LED();       // Initialize LEDs (four on-board LEDs below LCD panel)

	while (1)			    // forever loop to keep flashing four LEDs one at a time
	{
		DrvGPIO_ClrBit; // output Low to turn on LED
  	DrvSYS_Delay;   // delay 
	  DrvGPIO_SetBit; // output Hi to turn off LED
	  DrvSYS_Delay;   // delay
	}
}

