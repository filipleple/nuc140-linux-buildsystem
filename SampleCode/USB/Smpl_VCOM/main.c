//
// Smpl_VCOM_UART0
// Virtual Com port : connecting USB to UART0
// 
// UART0 of NUC140
// pin32  GPB0/RX0
// pin33  GPB1/TX0
//
#include <stdio.h>
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "USB.h"

extern int32_t VCOM_MainProcess(void);

void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

int32_t main (void)
{

 	STR_UART_T sParam;
	
  	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1); // Set to External Crystal
  	DrvSYS_SetPLLMode(0);               // Enable PLL
	Delay(1000);
	DrvSYS_SelectHCLKSource(2);         // Select HCLK Source (2: PLL)
	LOCKREG();
	
	/* Set UART0 Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0); 
	/* UART Setting */
    	sParam.u32BaudRate 		= 9600;
    	sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    	sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    	sParam.u8cParity 		  = DRVUART_PARITY_NONE;
    	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	
	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); // Set UART Clock Source to 12MHz
	DrvUART_Open(UART_PORT0, &sParam);  // Set UART0 Configuration
    
  /* Execute VCOM process */
	VCOM_MainProcess();
}
