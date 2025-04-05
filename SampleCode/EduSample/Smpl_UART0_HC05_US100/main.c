//
// Smpl_UART_HC05_US100 : reading ultrasound distance, then send throu BlueTooth to Smartphone/Tablet
//
// to use PC Bluetooth to add device HC-05
// to open HyperTerminal on PC to see the output from reading ultrasound sensor
// 
// UART0 connect to Bluetooth (HC-05)
// UART2 connect to UltraSound Sensor (US-100)

//
// HC-05 
// pin1 STATE
// pin2 RXD	to NUC140-UART0-TX(GPB1, pin33)
// pin3 TXD to NUC140-UART0-RX(GPB0, pin32)
// pin4 GND
// pin5 VCC
// pin6 KEY
//
// US-100 range : 2cm to 450cm
// jummper select: ON = UART TX/RX, OFF = Level Trig/Echo
// pin1 : Vcc +5V
// pin2 : Trig/TX to NUC140-UART2-TX(GPD15, pin39)
// pin3 : Echo/RX to NUC140-UART2-RX(GPD14, pin38)
// pin4 : Gnd
// pin5 : Gnd
//

#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "LCD.h"

#define  DATASIZE 8

int32_t main()
{
	uint16_t distance;
	uint8_t  write_byte[1];
	uint8_t  read_byte[2];
	uint8_t  dataout[DATASIZE];

	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(48000000);
	LOCKREG();
	init_LCD();
	clear_LCD();
	print_Line(0,"Smpl_BT_US100");
	print_Line(1,"UART2 to US100");
	print_Line(2,"UART0 to HC-05");
	print_Line(3,"PC HyperTerm  ");
   	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);	
	DrvGPIO_InitFunction(E_FUNC_UART2);	

	/* UART Setting */
    sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	if(DrvUART_Open(UART_PORT2,&sParam) != E_SUCCESS);
    		   
	while(1)
	{
 	 	write_byte[0]=0x55;						// trigger SRF04
		DrvUART_Write(UART_PORT2,write_byte,1);	// write to SRF04
		DrvUART_Read(UART_PORT2,read_byte,2); 	// read two bytes from SRF04
		distance = read_byte[0]*256 + read_byte[1];// distance = byte[0] *256 + byte[1];

		//if (distance>=10000) distance = 9999;
		dataout[0] = 0x30 + distance /1000;
		distance   = distance - distance /1000 * 1000;
		dataout[1] = 0x30 + distance /100;
		distance   = distance - distance /100 * 100;
		dataout[2] = 0x30 + distance/10;
		distance   = distance - distance /10 * 10;
		dataout[3] = 0x30 + distance;
		dataout[4] = 'm';
		dataout[5] = 'm';
		dataout[6] = 0x0d;
		dataout[7] = 0x0a;

		DrvUART_Write(UART_PORT0,dataout,DATASIZE);	// send to Bluetooth

	}
	//DrvUART_Close(UART_PORT0);
}

























































































