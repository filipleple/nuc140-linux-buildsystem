	
// LCD QC12864B module
// pin 1 : Vss   to Gnd
// pin 2 : Vcc   to Vcc+5V
// pin 3 : 0V    to N.C.
// pin 4 : CS    to NUC140 SPI1_CS  (GPC8 /pin61)
// pin 5 : SID   to NUC140 SPI1_DO0 (GPC11/pin58)
// pin 6 : SCLK  to NUC140 SPI1_CLK (GPC9 /pin60)
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

void init_SPI()
{
	// Configure SPI1 as a master to access LCD Q12864B
	DrvGPIO_InitFunction(E_FUNC_SPI1);
	DrvSPI_Open(eDRVSPI_PORT1, eDRVSPI_MASTER, eDRVSPI_TYPE1, 8);
	DrvSPI_SetEndian(eDRVSPI_PORT1, eDRVSPI_MSB_FIRST);
	DrvSPI_DisableAutoSS(eDRVSPI_PORT1);
	DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT1, eDRVSPI_ACTIVE_HIGH_RISING);
	//DrvSPI_EnableInt(eDRVSPI_PORT1, SPI1_Callback, 0);
	DrvSPI_SetClockFreq(eDRVSPI_PORT1, 50000, 0); // set SPI clock = 50KHz
}

void lcdWriteCommand(uint8_t cmd)
{
	SPI1->SSR.SSR=1;		
	SPI1->TX[0] =0x00F8;
	SPI1->CNTRL.GO_BUSY = 1;
  while ( SPI1->CNTRL.GO_BUSY == 1 );
	SPI1->TX[0] =0x00F0 & cmd;
	SPI1->CNTRL.GO_BUSY = 1;
  while ( SPI1->CNTRL.GO_BUSY == 1 );
	SPI1->TX[0] =0x00F0 & (cmd<<4);
	SPI1->CNTRL.GO_BUSY = 1;
  while ( SPI1->CNTRL.GO_BUSY == 1 );
	SPI1->SSR.SSR=0;
}

void lcdWriteData(unsigned char data)
{
	SPI1->SSR.SSR=1;		
	SPI1->TX[0] =0x00FA;
	SPI1->CNTRL.GO_BUSY = 1;
  while ( SPI1->CNTRL.GO_BUSY == 1 );
	SPI1->TX[0] =0x00F0 & data;
	SPI1->CNTRL.GO_BUSY = 1;
  while ( SPI1->CNTRL.GO_BUSY == 1 );
	SPI1->TX[0] =0x00F0 & (data<<4);
	SPI1->CNTRL.GO_BUSY = 1;
  while ( SPI1->CNTRL.GO_BUSY == 1 );
	SPI1->SSR.SSR=0;
}

void init_LCD(void)
{
	init_SPI();
	lcdWriteCommand(0x30);
	DrvSYS_Delay(50);	
	lcdWriteCommand(0x0c);
	DrvSYS_Delay(50);
}

void clear_LCD(void)
{
	lcdWriteCommand(0x01);
}

void print_Line(uint8_t line, unsigned char *string)
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
