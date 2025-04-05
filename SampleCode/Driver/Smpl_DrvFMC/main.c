/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "FMC.h"
#include "UART.h"
#include "SYS.h"

#define KEY_ADDR            0x20003FFC  /* The location of signature for 16KB RAM size */
#define SIGNATURE           0x21557899  /* The signature word is used by AP code to check if simple LD is finished */

#define CONFIG0_TEST_CODE   0x0F9000FF
#define CONFIG1_TEST_CODE   0x9abcdef0

#define CONFIG_BASE         0x00300000
#define LDROM_BASE          0x00100000
#define PAGE_SIZE           512


void UartInit(void)
{
    /* Enable UART clock */
    SYSCLK->APBCLK.UART0_EN = 1;

    /* Select UART clock source */
    SYSCLK->CLKSEL1.UART_S = 0;

    /* Data format */
    UART0->LCR.WLS = 3;

    /* Configure the baud rate */
    *((__IO uint32_t *)&UART0->BAUD) = 0x3F000066; /* External 12MHz, 115200 bps */

    /* Set UART Pin */
    SYS->GPBMFP.UART0_RX    = 1;
    SYS->GPBMFP.UART0_TX    = 1;
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    int32_t  i32Err;
    uint32_t u32Data, i;
    uint32_t apBase, apLimit;
    uint32_t config0Bak, config1Bak;
    
    UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    
    /* Waiting for 12M Xtal stable */
    DrvSYS_Delay(5000);
    
    /* Set UART Configuration */
    UartInit();

    printf("+-----------------------------------------------------+\n");
    printf("|       NUC1xx FMC Driver Sample Code for LDROM       |\n");
    printf("+-----------------------------------------------------+\n");

    /* Enable ISP function */
    DrvFMC_EnableISP();

    /* Read BS */
    printf("  Boot Mode .......................... ");
    if (DrvFMC_GetBootSelect() == E_FMC_APROM)
    {
        printf("[APROM]\n");     
        printf("  WARNING: This code should be in LD mode but it is in AP mode now.\n");
        while(1); 
    }
    else
        printf("[LDROM]\n");


    /* Read Data Flash base address */
    u32Data = DrvFMC_ReadDataFlashBaseAddr();
    printf("  Data Flash Base Address ............ [0x%x]\n", u32Data);


    apBase = 0x4000;
    apLimit= 0x8000;

    printf("  Erase AP ROM:0x%x..0x%x ........ ", apBase, apLimit-1);

    /* Page Erase LDROM */
    for (i = apBase; i < apLimit; i += PAGE_SIZE)
        DrvFMC_Erase(i);

    /* Erase Verify */
    i32Err = 0;
    for (i = apBase; i < apLimit; i += 4) 
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
    

    printf("  Program AP ROM:0x%x..0x%x ...... ", apBase, apLimit-1);
    /* Program AP ROM and read out data to compare it */
    for (i = apBase; i < apLimit; i += 4) 
    {
        DrvFMC_Write(i, i);
    }

    i32Err = 0;
    for (i = apBase; i < apLimit; i += 4) 
    {
        DrvFMC_Read(i, &u32Data);
        if (u32Data != i)
        { 
           i32Err = 1;
        }
    }
    if (i32Err)
        printf("[FAIL]\n");
    else
        printf("[OK]\n");

    /* Read and backup the configuration settings */
    printf("  Read config0 ....................... ");
    DrvFMC_Read(CONFIG_BASE, &config0Bak);
    printf("[0x%x]\n", config0Bak);

    printf("  Read config1 ....................... ");
    DrvFMC_Read(CONFIG_BASE+4, &config1Bak);
    printf("[0x%x]\n", config1Bak);


    /* Configuration region write test */
    printf("  Erase config region ................ ");
    DrvFMC_EnableConfigUpdate();
    DrvFMC_Erase(CONFIG_BASE);
    DrvFMC_Read(CONFIG_BASE, &u32Data);
    if (u32Data != 0xFFFFFFFF)
        printf("[FAIL]\n");
    else
        printf("[OK]\n");

    printf("  Program config region .............. ");
    DrvFMC_Write(CONFIG_BASE, CONFIG0_TEST_CODE);
    DrvFMC_Write(CONFIG_BASE+4, CONFIG1_TEST_CODE);
    i32Err = 0;    
    DrvFMC_Read(CONFIG_BASE, &u32Data);
    if (u32Data != CONFIG0_TEST_CODE)
        i32Err = 1;
    DrvFMC_Read(CONFIG_BASE+4, &u32Data);
    if (u32Data != CONFIG1_TEST_CODE)
        i32Err = 1;

    if (i32Err)
        printf("[FAIL]\n");
    else
        printf("[OK]\n");

    printf("  Restore config settings ............ ");
    DrvFMC_Erase(CONFIG_BASE);
    DrvFMC_Write(CONFIG_BASE, config0Bak);
    DrvFMC_Write(CONFIG_BASE+4, config1Bak);
    DrvFMC_DisableConfigUpdate();

    i32Err = 0;
    DrvFMC_Read(CONFIG_BASE, &u32Data);
    if (u32Data != config0Bak)
        i32Err = 1;
    
    DrvFMC_Read(CONFIG_BASE+4, &u32Data);
    if (u32Data != config1Bak)
        i32Err = 1;

    if (i32Err)
        printf("[FAIL]\n");
    else
        printf("[OK]\n");
   
    /* Write the signature to tell AP code that LD code has finished */
    outpw(KEY_ADDR, SIGNATURE);

    printf("\nFMC Simple LD Code Completed.\n");

    printf("\n  >>> Reset to back to AP mode <<< \n");

    /* Make sure message has printed out */
    while(UART0->FSR.TE_FLAG == 0);
    
    DrvFMC_BootSelect(E_FMC_APROM);
    DrvSYS_ResetCPU();

    /* Disable ISP function */
    DrvFMC_DisableISP();

    /* Lock protected registers */
    LOCKREG();
}




