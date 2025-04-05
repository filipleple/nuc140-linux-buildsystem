//
// Smpl_UART0_HC05 : bluetooth module receive 8 bytes and display on LCD
//
// Bluetooth module (BC04 or HC05)
// pin1 : KEY   N.C
// pin2 : VCC   to Vcc +5V
// pin3 : GND   to GND
// pin4 : TXD   to NUC140 UART0-RX (GPB0)
// pin5 : RXD   to NUC140 UART0-TX (GPB1)
// pin6 : STATE N.C.

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "LCD.h"

char TEXT[16];
volatile uint8_t comRbuf[9];
volatile uint8_t comRbytes = 0;

void UART_INT_HANDLE(void)
{
	while(UART0->ISR.RDA_IF==1) 
	{
		comRbuf[comRbytes]=UART0->DATA;
		comRbytes++;		
		if (comRbytes==8) {	
			sprintf(TEXT,"%s",comRbuf);
			print_Line(1,TEXT);			
		  comRbytes=0;
		}
	}
}

int32_t main()
{
	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(50000000);
	LOCKREG();
   	
	DrvGPIO_InitFunction(E_FUNC_UART0);	// Set UART pins

	/* UART Setting */
    sParam.u32BaudRate 		  = 9600;
    sParam.u8cDataBits 		  = DRVUART_DATABITS_8;
    sParam.u8cStopBits 		  = DRVUART_STOPBITS_1;
    sParam.u8cParity 		    = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);
	
	init_LCD();                 // initialize LCD panel
	clear_LCD();                 // clear LCD panel							 	
	print_Line(0, "Smpl_UART0_HC05"); // print title
    		   
	while(1)
	{
	}
	//DrvUART_Close(UART_PORT0);
}

























































































