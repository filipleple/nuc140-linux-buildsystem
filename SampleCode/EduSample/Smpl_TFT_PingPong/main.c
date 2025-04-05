//
// Smpl_TFT_PingPong : rendering a circle bouncing around between two bars
//
#include <stdio.h>	
#include <string.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "TFT.h"
#include "2DGraphics.h"

#define PIXEL_ON  1
#define PIXEL_OFF 0
#define X0 15       // Circle initial X 
#define Y0 10       // Circle initial Y
#define R_BAR_X LCD_Xmax-1 // Right-Bar X-position
#define L_BAR_X 0          // Left-Bar  X-position

int32_t main (void)
{
	int dirX, dirY;
	int movX, movY;
	uint16_t r;
	uint16_t x, y;
	uint16_t fgColor, bgColor;
	
	UNLOCKREG();
	DrvSYS_Open(50000000);	
	LOCKREG();
	
	init_LCD();  
	clear_LCD();	
	
	x = X0;    // circle center x
	y = Y0;    // circle center y
	r = 5;    // circle radius
	movX = 3;  // x movement
	movY = 3;  // y movement
	dirX = 1;  // x direction
	dirY = 1;  // y direction
	
	bgColor = BG_COLOR;
	while(1) 
		{
		 fgColor = FG_COLOR;
	   CircleBresenham(x, y, r, fgColor, bgColor); // draw a circle
		 LineBresenham(L_BAR_X,y-7, L_BAR_X,y+7, fgColor, bgColor); // draw left  line			
		 LineBresenham(R_BAR_X,y-7, R_BAR_X,y+7, fgColor, bgColor); // draw right line

	   DrvSYS_Delay(10000); // adjustable delay for vision
			
     fgColor = BG_COLOR;
	   CircleBresenham(x, y, r, fgColor, bgColor); // erase a circle
     LineBresenham(L_BAR_X,y-7,L_BAR_X,y+7, fgColor, bgColor); // draw left  line				
     LineBresenham(R_BAR_X,y-7,R_BAR_X,y+7, fgColor, bgColor); // draw right line
							 
		 // boundary check for changing direction
     if      ((x-r) < L_BAR_X)  dirX=1;  
	   else if ((x+r) > R_BAR_X)  dirX=-1;
	   else if ((y-r) < 0)        dirY=1;  
	   else if ((y+r) > LCD_Ymax) dirY=-1;
			
	   x = x + dirX * movX; // change x of circle center
     y = y + dirY * movY; // change y of circle center							
   }
}
