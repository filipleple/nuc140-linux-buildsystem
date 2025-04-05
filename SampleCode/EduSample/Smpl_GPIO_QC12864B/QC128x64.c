//
// QC128x64B LCD Driver
//	
// LCD QC12864B module
// pin 1 : Vss   to Gnd
// pin 2 : Vcc   to Vcc+5V
// pin 3 : 0V    to N.C.
// pin 4 : CS    to NUC140 GPC8 /pin61 emulate SPI_CS
// pin 5 : SID   to NUC140 GPC11/pin58 emulate SPI_DO
// pin 6 : SCLK  to NUC140 GPC9 /pin60 emulate SPI_CLK
// pin 7~14: N.C.
// pin 15: PSB   to Gnd (0=Serial-port mode, 1=Parallel-port mode)
// pin 16: N.C.
// pin 17: RST   to Vcc
// pin 18: N.C.
// pin 19: LED_A to +5V (Backlight Anode)
// pin 20: LED_K to Gnd (Backlight Cathode)

#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "SPI.h"

void init_SPI(void)
{
	DrvGPIO_Open(E_GPC, 8, E_IO_OUTPUT); // SPI_CS
	DrvGPIO_Open(E_GPC, 9, E_IO_OUTPUT); // SPI_CLK
	DrvGPIO_Open(E_GPC,11, E_IO_OUTPUT); // SPI_DO
	DrvGPIO_ClrBit(E_GPC, 8);
	DrvGPIO_ClrBit(E_GPC, 9);
	DrvGPIO_ClrBit(E_GPC,11);
}

void SPI_Write(unsigned char zdata)
{
	unsigned int i;
	for(i=0; i<8; i++)
	{
		if((zdata << i) & 0x80)
		{
			DrvGPIO_SetBit(E_GPC, 11); // SID = 1;
		}
		else 
		{
			DrvGPIO_ClrBit(E_GPC, 11); // SID = 0;
		}
		DrvGPIO_ClrBit(E_GPC,9); //SCLK = 0;
		DrvSYS_Delay(10);
		DrvGPIO_SetBit(E_GPC,9); //SCLK = 1;
	}
}

void SPI_CS(uint8_t pol)
{
	if (pol!=0) DrvGPIO_SetBit(E_GPC, 8);
	else        DrvGPIO_ClrBit(E_GPC, 8);
}

void lcdWriteCommand(uint8_t cmd)
{
	SPI_CS(1);
	SPI_Write(0xF8);
	SPI_Write(0xF0 & cmd);
	SPI_Write(0xF0 & (cmd<<4));
	SPI_CS(0);
}

void lcdWriteData(unsigned char data)
{
	SPI_CS(1);
	SPI_Write(0xFA);
	SPI_Write(0xF0 & data);
	SPI_Write(0xF0 & (data<<4));
	SPI_CS(0);
}

void init_QC12864B(void)
{
	lcdWriteCommand(0x30);
	DrvSYS_Delay(50);	
	lcdWriteCommand(0x0c);
	DrvSYS_Delay(50);
}

void clear_QC12864B(void)
{
	lcdWriteCommand(0x01);
}

void print_QC12864B(uint8_t line, unsigned char *string)
{
	uint8_t i, addr;
	if      (line==0) addr = 0x80;
	else if (line==1) addr = 0x90;
	else if (line==2) addr = 0x88;
	else if (line==3) addr = 0x98;
	else              addr = 0x80;
	lcdWriteCommand(addr);
	for (i=0; i<16; i++) lcdWriteData(*string++);
}
