//
// Smpl_SPI_QC12864B	: serial interfacing with LCD 128x64 module
//	
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
#include "QC12864B.h"

int main(void)
{
	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1); // Enable the 12MHz oscillator oscillation
	DrvSYS_SelectHCLKSource(0); // HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator
	LOCKREG();

	init_LCD();
	clear_LCD();
								  	
  print_Line(0,"Smpl_SPI_Q12864B");
	print_Line(1,"Academic Example");
	print_Line(2,"Nuvoton NuMicro ");
	print_Line(3,"NU-LB-NUC140    ");
}


