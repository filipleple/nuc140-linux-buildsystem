//
// Smpl_GPIO_QC12864B	: serial interfacing with LCD 128x64 module, GPIO emulate SPI
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
#include "QC128x64.h"

int main(void)
{
	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1); // Enable the 12MHz oscillator oscillation
	DrvSYS_SelectHCLKSource(0); // HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator
	LOCKREG();

	init_SPI();
	init_QC12864B();
	clear_QC12864B();
								  	
	print_QC12864B(0,"Smpl_GPIO_Q12864");
	print_QC12864B(1,"Academic Example");
	print_QC12864B(2,"Nuvoton NuMicro ");
	print_QC12864B(3,"NU-LB-NUC140    ");
}
