//
// Smpl_LCD_Graph_SinCos: draw Sine Wave & Cosine Wave on LCD
//
#include <stdio.h>	
#include <string.h>
#include <math.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "TFT.h"
#include "Font5x7.h"

#define  PI 3.14159265

int32_t main (void)
{
	int x,y;

	UNLOCKREG();
	DrvSYS_Open(50000000);
	LOCKREG();

	init_LCD();  
	clear_LCD();	                        
	
	// draw Sine Wave
	printS(LCD_Xmax/2+30, (LCD_Ymax/2-1),"Sine");
	for (x=0; x<360; x++){
		y = sin(x * PI/180) * LCD_Ymax/2 + LCD_Ymax/2;
	  draw_Pixel(x/2, (LCD_Ymax-1)-y, FG_COLOR, BG_COLOR);
	}	

	// draw Cosine Wave
	printS(LCD_Xmax/2+30, 0,"CoSine");
	for (x=0; x<360; x++){
		y = cos(x * PI/180) * LCD_Ymax/2 + LCD_Ymax/2;
	  draw_Pixel(x/2, (LCD_Ymax-1)-y, FG_COLOR, BG_COLOR);
	}
}
