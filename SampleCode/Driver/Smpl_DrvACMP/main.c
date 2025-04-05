/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "UART.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void ACMP_IRQHandler(void);


/*---------------------------------------------------------------------------------------------------------*/
/* ACMP Callback function                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
static uint32_t acmpIntCnt = 0;
void ACMP_IRQHandler(void)
{
    uint32_t u32DelayCount;
    
    /* input de-bounce */
    for(u32DelayCount=0; u32DelayCount<10000; u32DelayCount++);
    
    acmpIntCnt++;
    if(ACMP->CMPSR.CMPF0)
    {
        if(ACMP->CMPSR.CO0 == 1)
            printf("CP0 > CN0 (%d)\n", acmpIntCnt);
        else
            printf("CP0 <= CN0 (%d)\n", acmpIntCnt);
        
        ACMP->CMPSR.CMPF0 = 1;
    }

    if(ACMP->CMPSR.CMPF1)
    {
        if(ACMP->CMPSR.CO1 == 1)
            printf("CP1 > CN1 (%d)\n", acmpIntCnt);
        else
            printf("CP1 <= CN1 (%d)\n", acmpIntCnt);
        ACMP->CMPSR.CMPF1 = 1;
    }
}



/*---------------------------------------------------------------------------------------------------------*/
/* Main function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main()
{
    STR_UART_T sParam;
    uint32_t u32DelayCount;

    UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    /* Waiting for 12M Xtal stable */
    DrvSYS_Delay(5000);
    
    /* Configure UART related pins */
    DrvGPIO_InitFunction(E_FUNC_UART0);
    
    /* UART Setting */
    sParam.u32BaudRate      = 115200;
    sParam.u8cDataBits      = DRVUART_DATABITS_8;
    sParam.u8cStopBits      = DRVUART_STOPBITS_1;
    sParam.u8cParity        = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

    /* Select UART Clock Source From 12Mhz*/
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 

    DrvUART_Open(UART_PORT0, &sParam);

    printf("\n");
    printf("+----------------------------------------------------------------+\n");
    printf("|                     ACMP Sample Code                           |\n");           
    printf("+----------------------------------------------------------------+\n");
    printf("  PC.6 (CPP0): analog comparator 0 positive input\n");
    printf("  PC.14 (CPP1): analog comparator 1 positive input\n");
    printf("  The internal bandgap voltage, around 1.2V, is selected as the negative input of both comparators.\n");
    printf("\n");

    for(u32DelayCount=0; u32DelayCount<10000; u32DelayCount++);

    /* Disable the PC.6 digital input path */
    DrvGPIO_DisableDigitalInputBit(E_GPC, 6);
    /* Configure ACMP0 related multi-function pins */
    DrvGPIO_InitFunction(E_FUNC_ACMP0);
    /* Configure PC.7/CPN0 as a GPIO pin */
    SYS->GPCMFP.CPN0_AD5 = 0;
    
    /* Disable the PC.14 digital input path */
    DrvGPIO_DisableDigitalInputBit(E_GPC, 14);
    /* Configure ACMP1 related multi-function pins */
    DrvGPIO_InitFunction(E_FUNC_ACMP1);
    /* Configure PC.15/CPN1 as a GPIO pin */
    SYS->GPCMFP.CPN1_AD3 = 0;

    /* Enable ACMP clock source */
    SYSCLK->APBCLK.ACMP_EN = 1;

    /* Configure ACMP0 */
    ACMP->CMPCR[0].CMPEN = 1;
    ACMP->CMPCR[0].CMPIE = 1;
    ACMP->CMPCR[0].CMPCN = 1;
    ACMP->CMPCR[0].CMP_HYSEN = 0;

    /* Configure ACMP1 */
    ACMP->CMPCR[1].CMPEN = 1;
    ACMP->CMPCR[1].CMPIE = 1;
    ACMP->CMPCR[1].CMPCN = 1;
    ACMP->CMPCR[1].CMP_HYSEN = 0;

    NVIC_EnableIRQ(ACMP_IRQn);

    while(1);
}   



