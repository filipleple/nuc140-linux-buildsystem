//
// LED16x16 Driver 
//
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"

#define  CLK_Hi        DrvGPIO_SetBit(E_GPA,4)
#define  CLK_Lo        DrvGPIO_ClrBit(E_GPA,4)
#define  AB_Hi         DrvGPIO_SetBit(E_GPA,3)
#define  AB_Lo         DrvGPIO_ClrBit(E_GPA,3)
#define  RCK_Hi        DrvGPIO_SetBit(E_GPA,2)
#define  RCK_Lo        DrvGPIO_ClrBit(E_GPA,2)
#define  SRCK_Hi       DrvGPIO_SetBit(E_GPA,1)
#define  SRCK_Lo       DrvGPIO_ClrBit(E_GPA,1)
#define  SER_Hi        DrvGPIO_SetBit(E_GPA,0)
#define  SER_Lo        DrvGPIO_ClrBit(E_GPA,0)	

void init_LED16x16(void)
{
	DrvGPIO_InitFunction(E_FUNC_GPIO);

	DrvGPIO_Open(E_GPA, 4, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 3, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 2, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 1, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 0, E_IO_OUTPUT);	
	// set to default
  CLK_Lo;
	AB_Lo;
	RCK_Lo;
	SRCK_Lo;
	SER_Lo;
}

void display_LED16x16(uint8_t data[32])
{	
	uint8_t i,ia,j,tmp;		

  AB_Lo;
	for(i=0;i<16;i++)
	{
		CLK_Lo;
		RCK_Lo;
		SRCK_Lo;
		for(ia=2; ia>0; ia--)	
		{	
			tmp = ~ data[i*2+ia-1];	// even-byte will shift to Right, odd-byte at Left			
			for(j=0; j<8; j++) 
			{	
				SRCK_Lo;
				if ((tmp>>j) & 0x01) SER_Hi; 
				else                 SER_Lo;
        SRCK_Hi;			
		  }
		}
		RCK_Hi;
		CLK_Hi;
		AB_Hi;
	}
  SRCK_Lo;
	RCK_Lo;
	CLK_Lo;
}
