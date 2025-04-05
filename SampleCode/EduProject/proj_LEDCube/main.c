//
// proj_LEDCube : LED 8x8x8 Cube 
//
// LED Cube 8x8x8 Control Board using four TM1818 (16 channel LED driver) & one 74HC138 Demux
// Control Board pin description & connection
// pin1  VCC : to Vcc+5V
// pin2  Y0  : to GPA0
// pin3  Y1  : to GPA1
// pin4  Y2  : to GPA2
// pin5  Y3  : to Vcc
// pin6  Y4  : to Vcc
// pin7  SDI : to GPA3
// pin8  CLK : to GPA4
// pin9  LE  : to GPA5
// pin10 OEN : to GPA6
// pin11 EXT : to Vcc
// pin12 GND : to Gnd

#include <stdio.h>	
#include <string.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "Font5x7.h"

#define  Y0_Hi         DrvGPIO_SetBit(E_GPA,0)
#define  Y0_Lo         DrvGPIO_ClrBit(E_GPA,0)
#define  Y1_Hi         DrvGPIO_SetBit(E_GPA,1)
#define  Y1_Lo         DrvGPIO_ClrBit(E_GPA,1)
#define  Y2_Hi         DrvGPIO_SetBit(E_GPA,2)
#define  Y2_Lo         DrvGPIO_ClrBit(E_GPA,2)
#define  SDI_Hi        DrvGPIO_SetBit(E_GPA,3)
#define  SDI_Lo        DrvGPIO_ClrBit(E_GPA,3)
#define  CLK_Hi        DrvGPIO_SetBit(E_GPA,4)
#define  CLK_Lo        DrvGPIO_ClrBit(E_GPA,4)
#define  LE_Hi         DrvGPIO_SetBit(E_GPA,5)
#define  LE_Lo         DrvGPIO_ClrBit(E_GPA,5)
#define  OEN_Hi        DrvGPIO_SetBit(E_GPA,6)
#define  OEN_Lo        DrvGPIO_ClrBit(E_GPA,6)

#define DELAY_TIME 100000
#define REPEAT_TIMES 75

// initialize GPIO for output pins to Control Board
void Init_LED8x8x8(void)
{
	DrvGPIO_InitFunction(E_FUNC_GPIO);

	DrvGPIO_Open(E_GPA, 0, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 1, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 2, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 3, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 4, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPA, 5, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 6, E_IO_OUTPUT);
	// set to default
	Y0_Lo;
	Y1_Lo;
	Y2_Lo;
	SDI_Lo;
	CLK_Lo;
	LE_Lo;
	OEN_Hi;
}

// display one floor 8x8 LEDs
void display_led8x8(uint8_t floor, uint8_t led[8])
{	
	uint8_t i,j;
	CLK_Lo;
	LE_Lo;	
	OEN_Lo;
	for(i=0;i<8;i++) {
		if (i%2==0) 
		{
	     for(j=0;j<8;j++) {
		      if(((led[i]>>j)&0x01)==0) SDI_Lo;
			    else                	 	  SDI_Hi;
			    CLK_Hi;
			    CLK_Lo;
		      }					
		} else {
			 for(j=0;j<8;j++) {
		      if(((led[i]<<j)&0x80)==0) SDI_Lo;
			    else                	 	  SDI_Hi;
			    CLK_Hi;
			    CLK_Lo;
		      }		
		}
	}
	OEN_Hi;
	LE_Hi;           // latch data
	LE_Lo;	
  switch(floor) {
		case 0: Y2_Lo; Y1_Lo; Y0_Lo; break;
		case 1: Y2_Lo; Y1_Lo; Y0_Hi; break;
		case 2: Y2_Lo; Y1_Hi; Y0_Lo; break;
		case 3: Y2_Lo; Y1_Hi; Y0_Hi; break;
		case 4: Y2_Hi; Y1_Lo; Y0_Lo; break;
		case 5: Y2_Hi; Y1_Lo; Y0_Hi; break;
		case 6: Y2_Hi; Y1_Hi; Y0_Lo; break;
		case 7: Y2_Hi; Y1_Hi; Y0_Hi; break;
		//default:Y2_Lo; Y1_Lo; Y0_Lo; break;
	}		
	OEN_Lo;
} 
				
int32_t main (void)
{
	char TEXT[8] = "NUVOTON";
	uint8_t allon[8] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t alloff[8] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	
	
	uint8_t temp[8], font[8];
	
	uint8_t r;     // repeat times
	uint8_t f;     // floors
	uint8_t i,c,h,v,t; // tmp variables
	UNLOCKREG();  	// unlock protected registers
	DrvSYS_Open(48000000);
	LOCKREG();				                  // lock protected registers

	Init_LED8x8x8();

	while(1)
	{			
		// all on
		for (r=0;r<REPEAT_TIMES*3;r++) 
    for (f=0;f<8;f++)	display_led8x8(f,allon);		
	
		// one horizontal floor at a time
		for (h=0;h<8;h++) {
			for(r=0;r<REPEAT_TIMES;r++)
		  for (f=0;f<8;f++)	{
			  if (f==h) display_led8x8(f,allon);
			  else      display_led8x8(f,alloff);
		  }
			DrvSYS_Delay(DELAY_TIME);
	  }		
						
		// one vertical floor at a time
		for (v=0;v<8;v++) {
		  for (t=0;t<8;t++) temp[t]=0x01<<v;
			for (r=0;r<REPEAT_TIMES;r++) 
		  for (f=0;f<8;f++)	display_led8x8(f,temp);
			DrvSYS_Delay(DELAY_TIME);
		}
		
		// Horizontally show TEXT
  	for (i=0;i<strlen(TEXT);i++) {			
		c=TEXT[i] - 0x20;
		font[0]=0;        
		for (t=1;t<6;t++) font[t]=Font5x7[c*5+t-1];
		font[6]=0;
		font[7]=0;
			
		for (h=0;h<8;h++) {
			for(r=0;r<REPEAT_TIMES;r++)
		  for (f=0;f<8;f++)	{
			  if (f==h) display_led8x8(f,font);
			  else      display_led8x8(f,alloff);
		    }
	    }	
			DrvSYS_Delay(DELAY_TIME);
	  }
	
		// Veritically show A to Z from Font5x7 table 
      for (i=0;i<strlen(TEXT);i++) {	
			c=TEXT[i] - 0x20;
		  font[0]=0;
		  for (t=1;t<6;t++) font[t]=Font5x7[c*5+t-1];
			font[6]=0;
			font[7]=0;
		
     	for (v=0;v<8;v++) {
		  for (t=0;t<8;t++) temp[t]=0;
			for (r=0;r<REPEAT_TIMES;r++) 
		    for (f=0;f<8;f++)	{
				temp[v]=font[f];
				display_led8x8(f,temp);
			  }
		  }
			DrvSYS_Delay(DELAY_TIME);
	  }
				
		// slop line
		for (h=0;h<8;h++) {	
		  for(r=0;r<REPEAT_TIMES;r++)			
		  for (f=0;f<8;f++)	{		  
			  for (i=0;i<8;i++) {
					if   (i==f) temp[(i+h)%8]=0xFF;
					else        temp[(i+h)%8]=0x00;
				}
				display_led8x8(f,temp);
		  }
		DrvSYS_Delay(DELAY_TIME);
	  }			

	}    		 	  
}
