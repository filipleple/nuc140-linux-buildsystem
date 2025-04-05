//
// Sampl_GPIO_StepMotor_17HS5604 : GPIO to control StepMotor 
// Stepper Motor = 17HS5604, 12V, 0.4A, 1.8 degree
// Driver IC = ULN2003A
//
// driver IC control pins : 
// INA -> GPA3
// INB -> GPA2
// INC -> GPA1
// IND -> GPA0
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"

// Definitions for Step Motor turning degree
#define d360 400

unsigned char CW[8] ={0x08,0x0a,0x02,0x06,0x04,0x05,0x01,0x09}; // Clockwise
unsigned char CCW[8]={0x09,0x01,0x05,0x04,0x06,0x02,0x0a,0x08}; // Counter-Clockwise

void SpinMotor(uint8_t dir, uint16_t step)
{
  int i=0;
  while(step>0)
  {
  if (dir) GPIOA->DOUT=CW[i] | 0xFF00;
	else     GPIOA->DOUT=CCW[i] | 0xFF00;
	DrvSYS_Delay(5000); // Delay 5000us = 5ms
	i++;
	if (i>7) i = 0;
	step--;
  }
}

int main (void)
{
	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1); // Enable the 12MHz oscillator oscillation
	DrvSYS_SelectHCLKSource(0); // HCLK clock source. 0: external 12MHz;
	LOCKREG();
	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0);
	  	
	SpinMotor(1,407); // clockwise for one round
	SpinMotor(0,407); // counter-clockwise for one round
}




