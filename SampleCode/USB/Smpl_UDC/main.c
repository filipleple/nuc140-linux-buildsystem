/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "GPIO.h"
#include "FMC.h"
#include "SYS.h"
#include "UART.h"
#include "USB.h"
#include "UDC.h"

#define CONFIG_BASE      0x00300000
#define DATA_FLASH_BASE  0x00010000


void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t u32data0 = 0, u32data1 = 0;
    STR_UART_T sParam;

    UNLOCKREG();
    /* Set UART Pin */
    DrvGPIO_InitFunction(E_FUNC_UART0);
  
    /* UART Setting */
    sParam.u32BaudRate    = 115200;
    sParam.u8cDataBits    = DRVUART_DATABITS_8;
    sParam.u8cStopBits    = DRVUART_STOPBITS_1;
    sParam.u8cParity      = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
    /* Select UART Clock Source From 12Mhz*/
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 

    /* Set UART Configuration */
    DrvUART_Open(UART_PORT0,&sParam);

    /* Enable 12M OSC */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);

    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
    Delay(1000);

    DrvSYS_SelectHCLKSource(2);
    
    Delay(100);

    printf("+------------------------------------------------------+\n");
    printf("|          NUC100 USB MassStorage Sample Code          |\n");
    printf("+------------------------------------------------------+\n");                    
    printf("NOTE: This sample code needs to work with 128K Flash.\n");                    


    /* Enable ISP function */
    DrvFMC_EnableISP();

    /* Check if Data Flash Size is 64K. If not, to re-define Data Flash size and to enable Data Flash function */
    DrvFMC_Read(CONFIG_BASE, &u32data0);
    u32data1 = DrvFMC_ReadDataFlashBaseAddr();

    if ( ((u32data0 & 0x01) == 1) || (u32data1 != DATA_FLASH_BASE) )
    {
        DrvFMC_EnableConfigUpdate();
        DrvFMC_Erase(CONFIG_BASE);
        DrvFMC_Write(CONFIG_BASE, u32data0 & ~0x01);
        DrvFMC_Write(CONFIG_BASE+4, DATA_FLASH_BASE);

        DrvFMC_Read(CONFIG_BASE, &u32data0);
        DrvFMC_Read(CONFIG_BASE + 4, &u32data1);
    
        if ( ((u32data0 & 0x01) == 1) || (u32data1 != DATA_FLASH_BASE) )
        {
            printf("Error: Program Config Failed!\n");
            DrvFMC_DisableISP();
            LOCKREG();
            return -1;    
        }

        /* Reset Chip to reload new CONFIG value */
        DrvSYS_ResetChip();
    }
    LOCKREG();

    printf("NUC100 USB MassStorage Start!\n");
        
    /* Initialize USB Device function */
    udcInit();
    
    /* Initialize mass storage device */
    udcFlashInit();  
    
    /* Start USB Mass Storage */
    udcMassBulk();
  
    return 0;
}



