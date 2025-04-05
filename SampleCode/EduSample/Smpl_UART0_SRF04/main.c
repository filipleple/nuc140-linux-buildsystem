//
// Smpl_UART0_SRF04
//
// SRF04 Ultrasound Sensor by UART interface
//
// SRF04 range : 2cm to 450cm
// jummper select: ON = UART TX/RX, OFF = Level Trig/Echo
// pin1 : Vcc +5V
// pin2 : Trig/TX  to TX0/GPB1 (NUC140-pin32)
// pin3 : Echo/RX  to RX0/GPB0 (NUC140-pin33)
// pin4 : Gnd
// pin5 : Gnd

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "LCD.h"

int32_t main()
{
	uint32_t distance;
	uint8_t  write_byte[1];
	uint8_t  read_byte[2];

	char TEXT[16] = "                ";
	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(50000000);
	LOCKREG();
   	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);	

	/* UART Setting */
    sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);

	init_LCD();			// initialize LCD panel
	clear_LCD();		// clear LCD panel
							 	
	print_Line(0, "Smpl_UART0_SRF04");
    		   
	while(1)
	{
 	 	write_byte[0]=0x55;						// trigger SRF04
		DrvUART_Write(UART_PORT0,write_byte,1);	// write to SRF04
		DrvUART_Read(UART_PORT0,read_byte,2); 	// read two bytes from SRF04
		distance = read_byte[0]*256 + read_byte[1];// distance = byte[0] *256 + byte[1];
		sprintf(TEXT+0,"Distance: %dmm",distance);
		print_Line(1, TEXT);
	}
	//DrvUART_Close(UART_PORT0);
}

























































































