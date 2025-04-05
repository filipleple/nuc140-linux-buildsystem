/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "GPIO.h"
#include "UART.h"
#include "SYS.h"
#include "FMC.h"

#define KEY_ADDR    0x20003FFC      /* The location of signature for 16KB RAM size */
#define SIGNATURE   0x21557899      /* The signature word is used by AP code to check if simple LD is finished */

#define LDROM_BASE  0x00100000
#define PAGE_SIZE   512

extern uint32_t loaderImageBase;
extern uint32_t loaderImageLimit;


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    int32_t  i32Err;
    uint32_t u32Data, i, u32ImageSize, j, *pu32Loader;
    STR_UART_T sParam;
    uint8_t ch;
  
    UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    
    /* Waiting for 12M Xtal stable */
    DrvSYS_Delay(5000);

    /* Set UART Pin */
    SYS->GPBMFP.UART0_RX    = 1;
    SYS->GPBMFP.UART0_TX    = 1;
   
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
    
    printf("\n\n");
    
    /* Enable ISP function */
    DrvFMC_EnableISP();

    /* Check the signature to check if Simple LD code is finished or not */
    if (inpw(KEY_ADDR) == SIGNATURE)
    {
        /* Just clear SIGNATURE and finish the sample code if Simple LD code has been executed. */
        outpw(KEY_ADDR, 0);
        
        /* Read BS */
        printf("  Boot Mode .................................. ");
        if (DrvFMC_GetBootSelect() == E_FMC_APROM)
            printf("[APROM]\n");
        else
        {
            printf("[LDROM]\n");
            printf("  WARNING: The driver sample code must execute in AP mode!\n");
        }
        goto lexit;
    }
    
    printf("\n\n");
    printf("+-------------------------------------------------------------------------+\n");
    printf("|            NUC1xx Flash Memory Controller Driver Sample Code            |\n");
    printf("+-------------------------------------------------------------------------+\n");
    printf("  NOTE: This sample must be applied to NUC1xx series equipped with 16KB RAM.\n");

    /* Read BS */
    printf("  Boot Mode .................................. ");
    if (DrvFMC_GetBootSelect() == E_FMC_APROM)
        printf("[APROM]\n");
    else
    {
        printf("[LDROM]\n");
        printf("  WARNING: The driver sample code must execute in AP mode!\n");
        goto lexit;
    }

    /* Read Data Flash base address */
    u32Data = DrvFMC_ReadDataFlashBaseAddr();
    printf("  Data Flash Base Address .................... [0x%08x]\n", u32Data);

    /* Check the data in LD ROM to avoid overwrite them */
    DrvFMC_Read(LDROM_BASE, &u32Data);
    if (u32Data != 0xFFFFFFFF)
    {
        printf("\n  WARNING: There is code in LD ROM.\n  If you proceed, the code in LD ROM will be corrupted.\n");
        printf("  Continue? [y/n]:");
        ch = getchar();
        putchar(ch);
        if (ch != 'y')
            goto lexit;
        printf("\n\n");
    }

    /* Enable LDROM update */
    DrvFMC_EnableLDUpdate();

    printf("  Erase LD ROM ............................... ");
    /* Page Erase LDROM */
    for (i = 0; i < 4096; i += PAGE_SIZE)
        DrvFMC_Erase(LDROM_BASE + i);
    
    /* Erase Verify */
    i32Err = 0;
    for (i = LDROM_BASE; i < (LDROM_BASE+4096); i += 4) 
    {     
        DrvFMC_Read(i, &u32Data);
        
        if(u32Data != 0xFFFFFFFF)
        { 
            i32Err = 1;
        }         
    }
    if (i32Err)
        printf("[FAIL]\n");
    else
        printf("[OK]\n");
    

    printf("  Program LD ROM test ........................ ");
    
    /* Program LD ROM and read out data to compare it */
    for (i = LDROM_BASE; i < (LDROM_BASE+4096); i += 4) 
    {
        DrvFMC_Write(i, i);
    }

    i32Err = 0;
    for (i = LDROM_BASE; i < (LDROM_BASE+4096); i += 4) 
    {
        DrvFMC_Read(i, &u32Data);
        if(u32Data != i)
        { 
           i32Err = 1;
        }        
    }
    if (i32Err)
        printf("[FAIL]\n");
    else
        printf("[OK]\n");


    /* Check LD image size */
    u32ImageSize = (uint32_t)&loaderImageLimit - (uint32_t)&loaderImageBase;
    if (u32ImageSize == 0)
    {
        printf("  ERROR: Loader Image is 0 bytes!\n");
        goto lexit;
    }

    if (u32ImageSize > 4096)
    {
        printf("  ERROR: Loader Image is larger than 4KBytes!\n");
        goto lexit;    
    }


    printf("  Program Simple LD Code ..................... ");
    pu32Loader = (uint32_t *)&loaderImageBase;
    for (i = 0; i < u32ImageSize; i += PAGE_SIZE)
    {
        DrvFMC_Erase(LDROM_BASE + i);    
        for (j = 0; j < PAGE_SIZE; j += 4)
        {
            DrvFMC_Write(LDROM_BASE + i + j, pu32Loader[(i + j) / 4]);
        }
    }

    /* Verify loader */
    i32Err = 0;
    for (i = 0; i < u32ImageSize; i += PAGE_SIZE)
    {
        for(j = 0; j < PAGE_SIZE; j += 4)
        {
            DrvFMC_Read(LDROM_BASE + i + j, &u32Data);
            if (u32Data != pu32Loader[(i+j)/4])
                i32Err = 1;
            
            if (i + j >= u32ImageSize)
                break;
        }
    }

    if(i32Err)
    {
        printf("[FAIL]\n");
    }
    else
    {
        printf("[OK]\n");
        
        /* Reset CPU to boot to LD mode */
        printf("\n  >>> Reset to LD mode <<<\n");
        DrvFMC_BootSelect(E_FMC_LDROM);
        DrvSYS_ResetCPU();
    }


lexit:

    /* Disable ISP function */
    DrvFMC_DisableISP();

    /* Lock protected registers */
    LOCKREG();
    
    printf("\nFMC Sample Code Completed.\n");

}



