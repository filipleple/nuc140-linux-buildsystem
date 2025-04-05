/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"

#define SIGNATURE       0x125ab234
#define FLAG_ADDR       0x20003FFC


/*---------------------------------------------------------------------------------------------------------*/
/*  Brown out Callback Function                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void BOD_CallbackFn(void)
{
    printf("Brown Out Detect\n");
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Simple calculation test function                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
#define PI_NUM  256
int32_t f[PI_NUM+1];
uint32_t piTbl[19] = {
3141,
5926,
5358,
9793,
2384,
6264,
3383,
2795,
 288,
4197,
1693,
9937,
5105,
8209,
7494,
4592,
3078,
1640,
6284

};

int32_t piResult[19];

int32_t pi(void) 
{
    int32_t i,i32Err;
    int32_t a=10000,b=0,c=PI_NUM,d=0,e=0,g=0;     


    for(;b-c;)
        f[b++]=a/5;
    
    i = 0;
    for(;d=0,g=c*2;c -=14,/*printf("%.4d\n",e+d/a),*/ piResult[i++] = e+d/a,e=d%a)
    {
        if(i==19)
            break;

        for(b=c; d+=f[b]*a,f[b]=d%--g,d/=g--,--b; d*=b);
    }
    i32Err = 0;
    for(i=0;i<19;i++)
    {
        if(piTbl[i] != piResult[i])
            i32Err = -1;    
    }

    return i32Err;
}

void Delay(uint32_t x)
{
    int32_t i;
    
    for(i=0;i<x;i++)
    {
        __NOP();
        __NOP();
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    int32_t  i32ret;
    uint32_t u32data;
    STR_UART_T sParam;
    int32_t i;
    uint32_t u32Freq;
    
    UNLOCKREG();
    
    /* Check the key address registers are unlocked */
    if (DrvSYS_IsProtectedRegLocked() == 0)
    {
        printf("Failed to unlock Key Address Registers\n");
        return -1;  
    }
    
    SYSCLK->PWRCON.XTL12M_EN = 1;
    
    /* Waiting for 12M Xtal stalble */
    DrvSYS_Delay(5000);

    /* Set UART Pin */
    DrvGPIO_InitFunction(E_FUNC_UART0);
    
    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0); 
    
    /* UART Setting */
    sParam.u32BaudRate      = 115200;
    sParam.u8cDataBits      = DRVUART_DATABITS_8;
    sParam.u8cStopBits      = DRVUART_STOPBITS_1;
    sParam.u8cParity        = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

    /* Set UART Configuration */
    DrvUART_Open(UART_PORT0, &sParam);

    printf("+------------------------------------------------------------------------+\n");
    printf("|                       System Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");

    if (M32(FLAG_ADDR) == SIGNATURE)
    {
        printf("  CPU Reset success!\n");
        M32(FLAG_ADDR) = 0;
        printf("  Press any key to continue ...\n");
        getchar();
    }

/*---------------------------------------------------------------------------------------------------------*/
/* Misc system function test                                                                               */
/*---------------------------------------------------------------------------------------------------------*/

    /* Read Product ID */
    u32data = DrvSYS_ReadProductID();
    printf("Product ID 0x%x\n", u32data);
        
    /* Get reset source from last operation */
    u32data = DrvSYS_GetResetSource();
    printf("Reset Source 0x%x\n", u32data);
    
    /* Clear reset source */
    DrvSYS_ClearResetSource(u32data);

    /* Reset IP (I2C0) */
    DrvSYS_ResetIP(E_SYS_I2C0_RST);
    
    /* Unlock Protected address register before reset CPU */
    i32ret = DrvSYS_UnlockProtectedReg();
    if (i32ret == 0)
    {
        printf(" The Protected Address is Unlocked\n"); 
    }

    /* Enable Brown-Out Detector , select Brown-Out Interrupt function and install its callback function */
    DrvSYS_SetBODFunction(1, 0, BOD_CallbackFn);    

    /* Set Brown Our Detector voltage 2.2V */
    DrvSYS_SelectBODVolt(0);
    
    /* Configure PLL clock */
    DrvSYS_Open(50000000);
    
    /* Get PLL clock frequency */
    u32data = DrvSYS_GetPLLClockFreq();
    printf("PLL clock %d Hz\n", u32data);
    
    /* Select IP clock source(ADC clock source from 12M) */
    DrvSYS_SelectIPClockSource(E_SYS_ADC_CLKSRC, 0x00);
    
    /* Enable IP clock(I2C0) */
    DrvSYS_SetIPClock(E_SYS_I2C0_CLK, 1);

    /*---------------------------------------------------------------------------------------------------------*/
    /* PLL clock confiruation test                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/
    
    printf("-------------------------[ Test PLL ]-----------------------------\n");


    u32Freq = 25000000;
    for(i=0;i<7;i++)
    {
        DrvSYS_Open(u32Freq + 4000000 * i);
        printf("  Change system clock to %d Hz ......................... ", DrvSYS_GetHCLKFreq());
        
        /* The delay loop is used to check if the CPU speed is incresing */
        Delay(0x400000);
        
        if(pi())
        {
            printf("[FAIL]\n");
        }
        else
        {
            printf("[OK]\n");
        }
    }   
    
    /* Write a signature work to SRAM to check if it is reset by software */   
    M32(FLAG_ADDR) = SIGNATURE; 
    printf("\n\n  >>> Reset CPU <<<\n");

    /* Waiting for message send out */
    while(UART0->FSR.TE_FLAG == 0);

    /* Reset CPU */
    DrvSYS_ResetCPU();

}




