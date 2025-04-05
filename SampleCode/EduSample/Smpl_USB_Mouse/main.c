//
// Smpl_USB_Mouse : using Keypad3x3 to emulate USB Mouse
//
#include <stdio.h>
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "USB.h"
#include "LCD.h"
#include "Scankey.h"

extern int32_t HID_MainProcess(void);
#define DBG_ 0  // for DBG_PRINTF in HID_Keyboard_API.c & HIDSys.c

int32_t main (void)
{
#if DBG_
 	STR_UART_T sParam;
#endif
	
  UNLOCKREG();
  SYSCLK->PWRCON.XTL12M_EN = 1;
  DrvSYS_SetPLLMode(0); // enable PLL
  DrvSYS_Delay(5000);
  DrvSYS_SelectHCLKSource(2); // HCLK set to PLL clock	
  DrvSYS_Delay(100);
  SystemCoreClockUpdate(); // update system core clock
  LOCKREG();

#if DBG_	
  DrvGPIO_InitFunction(E_FUNC_UART0); // set UART0 pins	
  // UART parameters setup
  sParam.u32BaudRate 	  = 115200;
  sParam.u8cDataBits 	  = DRVUART_DATABITS_8;
  sParam.u8cStopBits 	  = DRVUART_STOPBITS_1;
  sParam.u8cParity 	    = DRVUART_PARITY_NONE;
  sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

  DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); // select UART clock source to 12MHz
  DrvUART_Open(UART_PORT0,&sParam); // set UART Configuration
#endif

	init_LCD();
	clear_LCD();
	print_Line(0,"USB_Mouse");
	print_Line(1,"press Keypad !");
	OpenKeyPad();
	HID_MainProcess();
}
