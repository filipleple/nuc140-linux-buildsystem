//
// Sampl_GPIO_DCMotor 
// L298N is used to drive two DC Motor (+12V DC, 200~250rpm)
//
// L298N pin connections
// pin1 Sense A : to Gnd
// pin2 Output1 : to MotorA +
// pin3 Output2 : to MotorA -
// pin4 Vs      : to +12V
// pin5 Input1  : to GPA0
// pin6 Enable A: to +5V
// pin7 Input2  : to GPA1
// pin8 Gnd     : to Gnd
// pin9 Vss     : to +5V
// pin10 Input3 : to GPA2
// pin11 EnableB: to +5V
// pin12 Input4 : to GPA3
// pin13 Output3: to MotorB +
// pin14 Output4: to MotorB -
// pin15 Sense B: to Gnd
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "LCD.h"

void InitGPIO()
{
	DrvGPIO_Open(E_GPA,0,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA,1,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA,2,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA,3,E_IO_OUTPUT);
	DrvGPIO_ClrBit(E_GPA,0);
	DrvGPIO_ClrBit(E_GPA,1);
	DrvGPIO_ClrBit(E_GPA,2);
	DrvGPIO_ClrBit(E_GPA,3);
}

void Forward()
{
	DrvGPIO_SetBit(E_GPA,0); 
	DrvGPIO_ClrBit(E_GPA,1);
	DrvGPIO_SetBit(E_GPA,2);
	DrvGPIO_ClrBit(E_GPA,3);
}

void Backward()
{
	DrvGPIO_ClrBit(E_GPA,0); 
	DrvGPIO_SetBit(E_GPA,1);
	DrvGPIO_ClrBit(E_GPA,2);
	DrvGPIO_SetBit(E_GPA,3);
}

void Right()
{
	DrvGPIO_SetBit(E_GPA,0); 
	DrvGPIO_ClrBit(E_GPA,1);
	DrvGPIO_ClrBit(E_GPA,2);
	DrvGPIO_SetBit(E_GPA,3);
}

void Left()
{
	DrvGPIO_ClrBit(E_GPA,0); 
	DrvGPIO_SetBit(E_GPA,1);
	DrvGPIO_SetBit(E_GPA,2);
	DrvGPIO_ClrBit(E_GPA,3);
}

void Stop()
{
	DrvGPIO_ClrBit(E_GPA,0); 
	DrvGPIO_ClrBit(E_GPA,1);
	DrvGPIO_ClrBit(E_GPA,2);
	DrvGPIO_ClrBit(E_GPA,3);
}

int main (void)
{
	UNLOCKREG();
  DrvSYS_Open(48000000);
	LOCKREG();

	init_LCD();
	clear_LCD();
	print_Line(0,"Smpl_GPIO_DCMoto");

	InitGPIO();
	print_Line(1,"Move Forward !!!");
	Forward(); 
	DrvSYS_Delay(1000000);
	print_Line(1,"Move Backward !!");
	Backward();
	DrvSYS_Delay(1000000);
	print_Line(1,"Turn Right !!!!!");
	Right();
	DrvSYS_Delay(1000000);
	print_Line(1,"Turn Left !!!!!!");
	Left();
	DrvSYS_Delay(1000000);
	print_Line(1,"Stop !!!!!!!!!!!");
	Stop();
}
