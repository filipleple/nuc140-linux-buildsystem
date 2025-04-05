/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "UART.h"
#include "SYS.h"

void GPABCallback(uint32_t u32GpaStatus, uint32_t u32GpbStatus)
{
    printf("GPAB Interrupt ! GPA:0x%04x  GPB:0x%04x. \n", u32GpaStatus, u32GpbStatus);
}

void GPCDECallback(uint32_t u32GpcStatus, uint32_t u32GpdStatus, uint32_t u32GpeStatus)
{
    printf("GPCDE Interrupt ! GPC:0x%04x  GPD:0x%04x  GPE:0x%04x. \n", u32GpcStatus, u32GpdStatus, u32GpeStatus);
}

void EINT0Callback(void)
{
    static volatile uint32_t uEINT0Cnt = 0;

    printf("EINT0 Interrupt ! (%d) \n", uEINT0Cnt++);
}

void EINT1Callback(void)
{
    static volatile uint32_t uEINT1Cnt = 0;

    printf("EINT1 Interrupt ! (%d) \n", uEINT1Cnt++);
}

void InitUARTDebugPort(void)
{
    STR_UART_T param;

    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0);

    param.u32BaudRate        = 115200;
    param.u8cDataBits        = DRVUART_DATABITS_8;
    param.u8cStopBits        = DRVUART_STOPBITS_1;
    param.u8cParity          = DRVUART_PARITY_NONE;
    param.u8cRxTriggerLevel  = DRVUART_FIFO_1BYTES;
    param.u8TimeOut          = 0;

    if (DEBUG_PORT == 1)
    {
        /* Set UART1 pins */
        DrvGPIO_InitFunction(E_FUNC_UART1);
        
        /* Set UART1 configuration */
        DrvUART_Open(UART_PORT1, &param);
    }else
    {
        /* Set UART0 pins */
        DrvGPIO_InitFunction(E_FUNC_UART0);
        
        /* Set UART0 configuration */
        DrvUART_Open(UART_PORT0, &param);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/    

int main (void)
{
    int32_t i32Err;

    UNLOCKREG();
            
    SYSCLK->PWRCON.OSC10K_EN = 1;

    /* Enable External 12M Xtal */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, ENABLE);

    /* Waiting for 12M Xtal stable */
    DrvSYS_Delay(5000);
                             
    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0);
    
    /* Initial UART debug message function */
    InitUARTDebugPort();       
                             
    DrvSYS_Open(50000000);
   
	printf("\n\n");
    printf("+-------------------------------------------------------------------+\n");
    printf("|                      GPIO Driver Sample Code                      |\n");
    printf("+-------------------------------------------------------------------+\n");

    /*-----------------------------------------------------------------------------------------------------*/
    /* Basic Setting                                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/    
    /*-----------------------------------------------------------------------------------------------------*/
    /* Configure Bit0 in GPIOA to Output pin and Bit1 in GPIOA to Input pin then close it                  */
    /*-----------------------------------------------------------------------------------------------------*/
	printf("  >> Please connect GPA0 and GPA1 first <<\n");
    printf("        Press any key to start test \n\n");
    getchar();
	
	DrvGPIO_Open(E_GPA, 0, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPA, 1, E_IO_INPUT);
	
    i32Err = 0;
    printf("  GPIO Input/Output test ................................ ");

	DrvGPIO_ClrBit(E_GPA, 0);
	if (DrvGPIO_GetBit(E_GPA, 1) != 0)
	{
		i32Err = 1;
	}

	DrvGPIO_SetBit(E_GPA, 0);	
	if (DrvGPIO_GetBit(E_GPA, 1) != 1)
	{
		i32Err = 1;
	}

    DrvGPIO_Close(E_GPA, 0);
	DrvGPIO_Close(E_GPA, 1);
 
    if (i32Err)
    {
	    printf("[FAIL]\n");
        printf("\n  (Please make sure GPA0 and GPA1 are connected) \n");
    }
    else
    {
        printf("[OK] \n");
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* GPIO Interrupt Test                                                                                 */
    /*-----------------------------------------------------------------------------------------------------*/
    
    printf("\n");     
    printf("  Configure GPA-11 as input mode and INT rising edge trigger. \n");    
    printf("  Configure GPC-03 as quasi-input mode and INT falling level trigger. \n");    
    printf("  Configure GPB-14 as External INT0 and falling edge trigger. \n");    
    printf("  Configure GPB-15 as External INT1 and both falling and rising edge trigger. \n");    

    /* Configure general GPIO interrupt */
    DrvGPIO_Open(E_GPA, 11, E_IO_INPUT);

    /* The Quasi-bidirection mode could be used as input with pull up enable */
    DrvGPIO_Open(E_GPC, 3, E_IO_QUASI);

    DrvGPIO_SetIntCallback(GPABCallback, GPCDECallback);

    /* IO_FALLING means low level trigger if it is in level trigger mode */
    DrvGPIO_EnableInt(E_GPA, 11, E_IO_RISING, E_MODE_EDGE);

    /* IO_FALLING means low level trigger if it is in level trigger mode */
    DrvGPIO_EnableInt(E_GPC, 3, E_IO_FALLING, E_MODE_LEVEL); 

    DrvGPIO_SetDebounceTime(10, E_DBCLKSRC_10K);    
    DrvGPIO_EnableDebounce(E_GPA, 11);
    DrvGPIO_EnableDebounce(E_GPC, 3);
    DrvGPIO_EnableDebounce(E_GPB, 14);
    DrvGPIO_EnableDebounce(E_GPB, 15);

    /* Configure external interrupt */
    DrvGPIO_InitFunction(E_FUNC_EXTINT0);
    DrvGPIO_InitFunction(E_FUNC_EXTINT1);
    DrvGPIO_EnableEINT0(E_IO_FALLING, E_MODE_EDGE, EINT0Callback);
    DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback);

    /* Waiting for interrupts */
    while(1);
}




