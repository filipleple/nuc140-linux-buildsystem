/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "EBI.h"
#include "UART.h"
#include "SYS.h"

extern void SRAM_BS616LV4017(void);	
extern void NOR_W39L010(void);

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

void InitEBISettings(void)
{
	DRVEBI_CONFIG_T	sEBIConfig;
	DRVEBI_TIMING_T	sEBITiming;
    uint32_t u32SysHCLKFreq = SystemCoreClock;

	// Open EBI function
	sEBIConfig.eDataWidth		= E_DRVEBI_DATA_16BIT;
	sEBIConfig.eAddrWidth		= E_DRVEBI_ADDR_16BIT;
	sEBIConfig.u32BaseAddress 	= DRVEBI_BASE_ADDR;
	sEBIConfig.u32Size 			= DRVEBI_MAX_SIZE;
 	DrvEBI_Open(sEBIConfig);
							  
	// Configure EBI timing
    u32SysHCLKFreq = DrvSYS_GetHCLKFreq();
    printf("The MCLK of EBI must be less than 18 MHz for Read/Write. \n");
    printf("But the actual required MCLK of EBI bus is based on the different EBI device. \n");

    if (u32SysHCLKFreq < 18000000)
    {
 	    sEBITiming.eMCLKDIV	 = E_DRVEBI_MCLKDIV_1;
        printf("MCLK of EBI is %d KHz.\n", u32SysHCLKFreq/1000);
    }else
    if (u32SysHCLKFreq >= 36000000)
    {
        sEBITiming.eMCLKDIV	 = E_DRVEBI_MCLKDIV_4;
        printf("MCLK of EBI colck is %d KHz.\n", (u32SysHCLKFreq/4)/1000);
    }else
    {
        sEBITiming.eMCLKDIV	 = E_DRVEBI_MCLKDIV_2;
        printf("MCLK of EBI colck is %d KHz.\n", (u32SysHCLKFreq/2)/1000);
    }
    printf("\n");

 	sEBITiming.u8ExttALE = 0;
	sEBITiming.u8ExtIR2R = 0;
	sEBITiming.u8ExtIW2X = 0;
	sEBITiming.u8ExttAHD = 0;
	sEBITiming.u8ExttACC = 0;
	DrvEBI_SetBusTiming(sEBITiming);

	// Get EBI timing
 	DrvEBI_GetBusTiming(&sEBITiming);
	printf("u8MCLKDIV=%d \nu8ExttALE=%d \nu8ExtIR2R=%d \nu8ExtIW2X=%d \nu8ExttAHD=%d \nu8ExttACC=%d\n\n",
			sEBITiming.eMCLKDIV,  sEBITiming.u8ExttALE, sEBITiming.u8ExtIR2R,  
			sEBITiming.u8ExtIW2X, sEBITiming.u8ExttAHD, sEBITiming.u8ExttACC);  
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/    
int main (void)
{
	uint8_t	u8Item = 0x0;

	UNLOCKREG();

    /* Enable External 12M Xtal */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, ENABLE);

    /* Waiting for 12M Xtal stable */
    DrvSYS_Delay(5000);
                             
    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0);
    
    /* Initial UART debug message function */
    InitUARTDebugPort();       
   
    DrvSYS_Open(32000000);

	printf("\n\n");
    printf("+------------------------------------------------------------------+\n");
    printf("|                      EBI Driver Sample Code                      |\n");
    printf("+------------------------------------------------------------------+\n");
	printf("\n");

    /* Initial EBI settings */
    InitEBISettings();
	
	do {
		printf("******************************************************************\n");
		printf("* Please connect BS616LV4017 or W39L010 to NUC100 Series EBI bus *\n");
		printf("* before EBI testing!!                                           *\n");
		printf("*                                                                *\n");								
		printf("* The testing result will be FAILED, if there is no BS616LV4017  *\n");
		printf("* or W39L010 connecting to NUC100 sereis EBI bus!                *\n");
		printf("******************************************************************\n");
		printf("\n");
		printf("|   >>> Selese item to test <<<    |\n");
		printf("| [0] SRAM  --- BS616LV4017, 16bit |\n");
		printf("| [1] NOR   --- W39L010, 8bit      |\n");
		printf("| [ESC] Exit                       |\n");
		u8Item = getchar();

		printf("\n");
		switch (u8Item)
		{
			case '0':
				SRAM_BS616LV4017();
				break;
				
			case '1':		
				NOR_W39L010();		
				break;
		}
	} while (u8Item != 27);

	// Close EBI function
	DrvEBI_Close();
	
	printf("Exit EBI Test Function \n\n");
	
	return 1;
}




