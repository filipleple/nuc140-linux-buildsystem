//
// Smpl_USB_Gamepad : using Joystick & Keypad to emulate USB Gamepad
//
// Joystick
// VRx : ADC6 (GPA6)
// VRy : ADC7 (GPA7)
// SW  : GPB15
#include <stdio.h>
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "USB.h"
#include "LCD.h"
#include "Scankey.h"

#define DBG_ 0

extern int32_t HID_MainProcess(void);

void init_ADC(void)
{
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD|=0x00c00000; 		//Disable digital input path
	SYS->GPAMFP.ADC6_AD7=1; 		//Set ADC6 function 
	SYS->GPAMFP.ADC7_SS21_AD6=1;//Set ADC7 function 
				
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 2;	//Select 22Mhz for ADC
	SYSCLK->CLKDIV.ADC_N = 1;		//ADC clock source = 22Mhz/2 =11Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;	//Enable clock source
	ADC->ADCR.ADEN = 1;				  //Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	//single end input
	ADC->ADCR.ADMD   = 2;     	//single-cycle mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0xc0;
	
	/* Step 5. Enable ADC interrupt */
	ADC->ADSR.ADF =1;     			//clear the A/D interrupt flags for safe 
	ADC->ADCR.ADIE = 1;
//	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. Enable WDT module */
	ADC->ADCR.ADST=1;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
#if DBG_
 	STR_UART_T sParam;
#endif
	
  UNLOCKREG();
  SYSCLK->PWRCON.XTL12M_EN = 1;
  DrvSYS_SetPLLMode(0); // enable PLL
	DrvSYS_Delay(5000);
	DrvSYS_SelectHCLKSource(2); // HCLK set to PLL clock	
  DrvSYS_Delay(100);
  SystemCoreClockUpdate(); // update system core clock
	LOCKREG();

#if DBG_	
	DrvGPIO_InitFunction(E_FUNC_UART0); // set UART0 pins	
	// UART parameters setup
  sParam.u32BaudRate 		= 115200;
  sParam.u8cDataBits 		= DRVUART_DATABITS_8;
  sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
  sParam.u8cParity 		  = DRVUART_PARITY_NONE;
  sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); // select UART clock source to 12MHz
	DrvUART_Open(UART_PORT0,&sParam); // set UART Configuration
#endif
	
	init_LCD();
	clear_LCD();
	print_Line(0,"USB_Gamepad");
	init_ADC();
	OpenKeyPad();
	DrvGPIO_Open(E_GPB, 15, E_IO_OPENDRAIN);
	HID_MainProcess();
}
