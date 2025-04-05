//
// Smpl_UART0_RC522 : read card serail number using compact command
//
// RFID module
// comm port = 9600, N, 1, 8, 1
// pin1 : GND   to GND
// pin2 : VCC   to Vcc +5V
// pin3 : RXD   to NUC140 UART0-TX (GPB1)
// pin4 : TXD   to NUC140 UART0-RX (GPB0)
//
// compact command (1 byte)
// 0x01 : Search Card
// 0x02 : Read the card serial number
// 0x03 : Record the card's serial number into an authorization list
// 0x04 : check if a card is in authorization list
// 0x05 : Remove a card's serial number from authorization list

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "LCD.h"

int32_t main()
{
	uint8_t  read_buf[4];
	uint8_t  write_buf[1];
	char     TEXT1[16] = "SN.             ";
	
	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(50000000);
	LOCKREG();
   	
	DrvGPIO_InitFunction(E_FUNC_UART0);	// Set UART pins

	/* UART Setting */
  sParam.u32BaudRate 		= 9600;
  sParam.u8cDataBits 		= DRVUART_DATABITS_8;
  sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
  sParam.u8cParity 		  = DRVUART_PARITY_NONE;
  sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);

	init_LCD();                 // initialize LCD panel
	clear_LCD();                 // clear LCD panel							 	
	print_Line(0, "Smpl_UART0_RFID"); // print title

  write_buf[0] = 0x02;
  DrvUART_Write(UART_PORT0, write_buf, 1);
  while(1) {
		DrvUART_Read (UART_PORT0, read_buf, 4);
		sprintf(TEXT1+3,"%2x%2x%2x%2x",read_buf[0],read_buf[1],read_buf[2],read_buf[3]);
		print_Line(1,TEXT1);	
	}
}



































































