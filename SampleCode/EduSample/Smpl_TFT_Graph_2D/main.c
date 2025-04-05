//
// Smpl_LCD_Graphics : render rectangle, circle, line, triangle onto LCD
//
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "TFT.h"
#include "2DGraphics.h"

int32_t main (void)
{
	UNLOCKREG();
	DrvSYS_Open(50000000);
	LOCKREG();

	init_LCD();  
	clear_LCD();
	
	RectangleDraw(0,0,127,63, FG_COLOR, BG_COLOR); // draw a rectangle
	RectangleFill(10,10,20,20,FG_COLOR, BG_COLOR); // draw a box
	CircleBresenham(20,30,20,FG_COLOR, BG_COLOR);  // draw a circle
	CircleMidpoint(63,30,20,FG_COLOR, BG_COLOR);	 // draw a circle
	CircleOptimized(105,30,20,FG_COLOR, BG_COLOR); // draw a circle
	LineBresenham(20,0,60,50,FG_COLOR, BG_COLOR);  // draw a line
	LineOptimized(30,20,100,40,FG_COLOR, BG_COLOR);// draw a line	
	Triangle(20,40,90,10,60,60,FG_COLOR, BG_COLOR);// draw a triangle
	
	RectangleDraw(0,0,319,239, FG_COLOR, BG_COLOR); // draw a rectangle
}



