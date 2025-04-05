//
// Smpl_LCD_Game_Invader : LCD Game - Invader
//
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "SYS.h"
#include "LCD.h"
#include "Scankey.h"

unsigned char displayBuf[128*8]; // display buffer
//Alien0 bitmap
//00000011,11000000,
//00001111,11110000,
//00011111,11111000,
//00110101,10101100,
//01111111,01111110,
//00111001,10011100,
//00010000,00001000,
//00000000,00000000

//Alien1 bitmap
//00010000,00010000,
//00001000,00100000,
//00011111,11110000,
//00110011,11001100,
//01111111,11111110,
//01011111,11111010,
//01010000,00001010,
//00000000,00000000,

//Alien2 bitmap
//00001000,00010000,
//00011111,11111000,
//01011001,10011010,
//01011111,11111010,
//01111111,11111110,
//00101000,00010100,
//01000000,00000010,
//00000000,00000000,

//Alien3 bitmap
//00000011,11000000,
//00011111,11111000,
//00111001,10011110,
//01111111,11111110,
//00001110,01110000,
//00011001,10011000,
//01100000,00000110,
//00000000,00000000,

//Alien4 bitmap
//00000011,11000000,
//00000111,11100000,
//00001101,10110000,
//00011111,11111000,
//00000010,01000000,
//00000101,10100000,
//00001010,01010000,
//00000000,00000000,

//Cannon bitmap
//00000000,00000000,
//00000001,10000000,
//00000001,10000000,
//00000011,11000000,
//00000011,11000000,
//00111111,11111100,
//01111111,11111110,
//00111111,11111100,

//Bullet bitmap
//00000001,10000000,
//00000001,10000000,
//00000000,00000000,
//00000000,00000000,
//00000001,10000000,
//00000001,10000000,
//00000000,00000000,
//00000000,00000000,

// bitmap up-side down
unsigned char Alien1[16] = {
	0x00,0x10,0x38,0x7C,0x36,0x1E,0x17,0x3F,0x3F,0x17,0x1E,0x36,0x7C,0x38,0x10,0x00};
unsigned char Alien2[16] = {
	0x00,0x70,0x18,0x7D,0x36,0x34,0x3C,0x3C,0x3C,0x3C,0x34,0x36,0x7D,0x18,0x70,0x00};
unsigned char Alien3[16] = {
	0x00,0x5C,0x30,0x1E,0x3F,0x1A,0x1A,0x1E,0x1E,0x1A,0x1A,0x3F,0x1E,0x30,0x5C,0x00};
unsigned char Alien4[16] = {
	0x00,0x48,0x4C,0x2E,0x3E,0x1A,0x1B,0x2F,0x2F,0x1B,0x1A,0x3E,0x2E,0x4C,0x48,0x00};
unsigned char Alien5[16] = {
	0x00,0x00,0x00,0x08,0x4C,0x2E,0x5B,0x2F,0x2F,0x5B,0x2E,0x4C,0x08,0x00,0x00,0x00};
unsigned char Bullet[16] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char Cannon[16] = {
	0x00,0x40,0xE0,0xE0,0xE0,0xE0,0xF8,0xFE,0xFE,0xF8,0xE0,0xE0,0xE0,0xE0,0x40,0x00};

int32_t main (void)
{
	int8_t x,y;
	int8_t keyin;
	int8_t i;
	int8_t FireFlag =0;

	int8_t Alien1_x, Alien1_y;
	int8_t Alien2_x, Alien2_y;
	int8_t Alien3_x, Alien3_y;
	int8_t Alien4_x, Alien4_y;	
	int8_t Alien5_x, Alien5_y;
	int8_t Bullet_x, Bullet_y;
	int8_t Cannon_x, Cannon_y;

	UNLOCKREG();
  DrvSYS_Open(50000000);
	LOCKREG();

	init_LCD();  
	clear_LCD();
	OpenKeyPad();

	printS_5x7(0, 0," NuMiro MCU Invader Demo ");
	draw_Bmp16x8(26,16,FG_COLOR,BG_COLOR, Alien2);	
	printS_5x7(46,16,"= 30 POINTS");
	draw_Bmp16x8(26,24,FG_COLOR,BG_COLOR, Alien3);	
	printS_5x7(46,24,"= 25 POINTS");
	draw_Bmp16x8(26,32,FG_COLOR,BG_COLOR, Alien4);	
	printS_5x7(46,32,"= 20 POINTS");	
	draw_Bmp16x8(26,40,FG_COLOR,BG_COLOR, Alien5);	
	printS_5x7(46,40,"= 10 POINTS");
	draw_Bmp16x8(26,48,FG_COLOR,BG_COLOR, Alien1);	
	printS_5x7(46,48,"= BONUS <<<");	
	printS_5x7(20,56,"Press Key To Start");	
	
	keyin=ScanKey();
  while(keyin==0) { keyin=ScanKey();}

	clear_LCD();		
	y=0; x=0;	printS_5x7(x,y," SCORE 0000       LIFE 03 ");
		
	Alien1_x=0;  Alien1_y=8;
	Alien2_x=0;  Alien2_y=16;
	Alien3_x=0;  Alien3_y=24;
	Alien4_x=0;  Alien4_y=32;
	Alien5_x=0;  Alien5_y=40;	
	Bullet_x=64; Bullet_y=48;
	Cannon_x=64; Cannon_y=56;	
	
	for (i=1;i<4;i++) {
		draw_Bmp16x8(i*16,   Alien2_y,FG_COLOR,BG_COLOR, Alien2); 
	  draw_Bmp16x8(i*16+64,Alien2_y,FG_COLOR,BG_COLOR, Alien2);
	}
	for (i=1;i<4;i++) {
		draw_Bmp16x8(i*16,   Alien3_y,FG_COLOR,BG_COLOR, Alien3); 
	  draw_Bmp16x8(i*16+64,Alien3_y,FG_COLOR,BG_COLOR, Alien3);
	}
	for (i=1;i<4;i++) {
		draw_Bmp16x8(i*16, Alien4_y,FG_COLOR,BG_COLOR, Alien4); 
	  draw_Bmp16x8(i*16+64,Alien4_y,FG_COLOR,BG_COLOR, Alien4);
	}	
	for (i=1;i<4;i++) {
		draw_Bmp16x8(i*16,   Alien5_y,FG_COLOR,BG_COLOR, Alien5); 
	  draw_Bmp16x8(i*16+64,Alien5_y,FG_COLOR,BG_COLOR, Alien5);
	}	
	
	FireFlag=0;
	while(1) {
		draw_Bmp16x8(Alien1_x, Alien1_y,FG_COLOR,BG_COLOR, Alien1);
	  draw_Bmp16x8(Cannon_x,Cannon_y, FG_COLOR,BG_COLOR, Cannon); 		
		if (FireFlag)
    draw_Bmp16x8(Bullet_x,Bullet_y, FG_COLOR,BG_COLOR, Bullet);
		
		DrvSYS_Delay(50000);
		
		draw_Bmp16x8(Alien1_x, Alien1_y,BG_COLOR,BG_COLOR, Alien1);		
	  draw_Bmp16x8(Cannon_x,Cannon_y, BG_COLOR,BG_COLOR, Cannon); 
		if (FireFlag)
    draw_Bmp16x8(Bullet_x,Bullet_y, BG_COLOR,BG_COLOR, Bullet);		
	

		keyin=ScanKey();		// scan keypad
		switch(keyin) {
			case 4: Cannon_x--; break;
			case 5: FireFlag = 1;
			        Bullet_x = Cannon_x;
			        Bullet_y = 48;			
			        break;
			case 6: Cannon_x++; break;
			default:            break;
		}	
		
		Bullet_y--; // decrement Bullet_y every round
		Alien1_x++;	// increment Alien1_x for UFO moving from left to right
		
		// check boundary
		if (Alien1_x>(LCD_Xmax-16)) Alien1_x=0;		
		if (Cannon_x>(LCD_Xmax-16))	Cannon_x=0;
    if (Cannon_x<0)             Cannon_x=LCD_Xmax-16;
    if (Bullet_y<8)             FireFlag=0;
  } 		
}
