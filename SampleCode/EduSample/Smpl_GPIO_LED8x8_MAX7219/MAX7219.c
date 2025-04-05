//
// MAX7219 Driver : drive 8x8 LEDs
//
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"

// Define GPA2,1,0 as CLK, CS, DIN pins
#define  CLK_HI  DrvGPIO_SetBit(E_GPA, 2)
#define  CLK_LO  DrvGPIO_ClrBit(E_GPA, 2)
#define  CS_HI   DrvGPIO_SetBit(E_GPA, 1)
#define  CS_LO   DrvGPIO_ClrBit(E_GPA, 1)
#define  DIN_HI  DrvGPIO_SetBit(E_GPA, 0)
#define  DIN_LO  DrvGPIO_ClrBit(E_GPA, 0)

// Initialize GPIOs for connecting MAX7219
void init_GPIO(void)
{
	DrvGPIO_InitFunction(E_FUNC_GPIO);
	DrvGPIO_Open(E_GPA, 0,  E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 1,  E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 2,  E_IO_OUTPUT);	
	// set to default
	DrvGPIO_ClrBit(E_GPA, 0);
 	DrvGPIO_ClrBit(E_GPA, 1);
	DrvGPIO_ClrBit(E_GPA, 2);
}

void WriteData_MAX7219(uint8_t DATA)         
{
   uint8_t i;    
   CS_LO;		
   for(i=8;i>=1;i--) {		  
     CLK_LO;
	 if (DATA&0x80) DIN_HI;
	 else           DIN_LO;
         DATA=DATA<<1;
     CLK_HI;
     DrvSYS_Delay(10);
   }                                 
}

void write_MAX7219(uint8_t address,uint8_t dat)
{ 
   CS_LO;
     WriteData_MAX7219(address);           
     WriteData_MAX7219(dat);     
   CS_HI;                        
}

void init_MAX7219(void)
{
	init_GPIO();
  write_MAX7219(0x09, 0x00);
  write_MAX7219(0x0a, 0x03);
  write_MAX7219(0x0b, 0x07);
  write_MAX7219(0x0c, 0x01);
  write_MAX7219(0x0f, 0x00);
}
