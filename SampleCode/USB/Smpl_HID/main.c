/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "USB.h"

#define DEBUG 1

extern int32_t HID_MainProcess(void);

void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
#ifdef DEBUG
 	STR_UART_T sParam;
#endif
    UNLOCKREG();

    SYSCLK->PWRCON.XTL12M_EN = 1;
   
 #ifdef DEBUG
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);
	
	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Select UART Clock Source From 12Mhz */
	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 

	/* Set UART Configuration */
	DrvUART_Open(UART_PORT0,&sParam);
#endif
    
    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	Delay(1000);

	/* Switch to PLL clock */
	DrvSYS_SelectHCLKSource(2);    

    Delay(100);

    /* Update system core clock */
    SystemCoreClockUpdate();

#ifdef DEBUG    
    printf("NUC100 USB HID\n");
#endif
	
    /* Execute HID process */
	HID_MainProcess();

}




