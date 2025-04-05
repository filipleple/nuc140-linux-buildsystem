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
#include "PDMA.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

int32_t UART_TEST_LENGTH =64;
uint8_t SrcArray[64];
uint8_t DestArray[64];
int32_t IntCnt;
volatile int32_t IsTestOver;
extern char GetChar(void);
/*---------------------------------------------------------------------------------------------------------*/
/* Clear buffer funcion                                                                              	   */
/*---------------------------------------------------------------------------------------------------------*/

void ClearBuf(uint32_t u32Addr, uint32_t u32Length,uint8_t u8Pattern)
{
	uint8_t* pu8Ptr;
	uint32_t i;
	
	pu8Ptr = (uint8_t *)u32Addr;
	
	for (i=0; i<u32Length; i++)
	{
		*pu8Ptr++ = u8Pattern;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/* Bulid Src Pattern function                                                                         	   */
/*---------------------------------------------------------------------------------------------------------*/

void BuildSrcPattern(uint32_t u32Addr, uint32_t u32Length)
{
    uint32_t i=0,j,loop;
    uint8_t* pAddr;
    
    pAddr = (uint8_t *)u32Addr;
    
    do {
        if (u32Length > 256)
	    	loop = 256;
	    else
	    	loop = u32Length;
	    	
	   	u32Length = u32Length - loop;    	

        for(j=0;j<loop;j++)
            *pAddr++ = (uint8_t)(j+i);
            
	    i++;        
	} while ((loop !=0) || (u32Length !=0));         
}

/*---------------------------------------------------------------------------------------------------------*/
/* PDMA Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/

void PDMA_Callback_0()
{
	extern int32_t IntCnt;
	printf("\tTransfer Done %02d!\r",++IntCnt);

	if(IntCnt<10)
	{
 		DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_1);		     
		DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_0);
	}
	else
	{
		IsTestOver = TRUE;
	}
}


void PDMA_Callback_1()
{
	extern int32_t IntCnt;
	int32_t i ;
	
	printf("\tTransfer Done %02d!\t",++IntCnt);
	for(i=0;i<UART_TEST_LENGTH;i++)
		printf(" 0x%2x(%c),",inpb(DestArray+i),inpb(DestArray+i));
	printf("\n");
	if(IntCnt<10)
	{
		DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_0);
	}
	else
	{
		IsTestOver = TRUE;
	}
}

void UART_Callback()
{
   UART1->DATA = UART0->DATA;

}

/*---------------------------------------------------------------------------------------------------------*/
/* PDMA Sample Code:                                                                                  	   */
/*         i32option : ['1'] UART1 TX/RX PDMA Loopback                                                     */
/*                     [Others] UART1 RX PDMA test                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA_UART(int32_t i32option)
{
	STR_PDMA_T sPDMA;  
    uint32_t  UARTPort;

    volatile uint32_t i;
	BuildSrcPattern((uint32_t)SrcArray,UART_TEST_LENGTH);

   	UARTPort = UART1_BASE;    	
    //i=UART_TEST_LENGTH;
    ClearBuf((uint32_t)DestArray, UART_TEST_LENGTH,0xFF);
    
	/* PDMA Init */
    DrvPDMA_Init();

	if(i32option =='1')
	{
		printf("  [Using TWO PDMA channel]. \n");
	 	printf("  This sample code will use PDMA to do UART1 loopback 10 times test. \n");
        printf("  Please connect GPB4 <--> GPB5 before testing.\n");
	    printf("  After connecting GPB4 <--> GPB5, press any key to start transfer.\n");  
	    getchar();
	}
	else
	{
		UART_TEST_LENGTH = 2;	   /* Test Length */
 		printf("  [Using ONE PDMA channel]. \n");
	 	printf("  This sample code will use PDMA to do UART1 Rx test ten times. \n");
        printf("  Please connect GPB4 <--> GPB5 before testing.\n");
	    printf("  After connecting GPB4 <--> GPB5, press any key to start transfer.\n"); 
	    getchar();
		printf("  Please input %d bytes to trigger PDMA one time.(Ex: Press 'a''b')\n",UART_TEST_LENGTH);
	}

	if(i32option =='1')
	{
		/* PDMA Setting */
		DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_1,eDRVPDMA_UART1,eDRVPDMA_WRITE_APB);
		
		/* CH1 TX Setting */
		sPDMA.sSrcCtrl.u32Addr 			= (uint32_t)SrcArray;
	    sPDMA.sDestCtrl.u32Addr	 		= UARTPort;   
	    sPDMA.u8TransWidth 				= eDRVPDMA_WIDTH_8BITS;
		sPDMA.u8Mode 					= eDRVPDMA_MODE_MEM2APB;
		sPDMA.sSrcCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_INCREMENTED; 
		sPDMA.sDestCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_FIXED;   
		sPDMA.i32ByteCnt                = UART_TEST_LENGTH;
		DrvPDMA_Open(eDRVPDMA_CHANNEL_1,&sPDMA);
	 	
	}

	/* PDMA Setting */
	DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_0,eDRVPDMA_UART1,eDRVPDMA_READ_APB);

 	/* CH0 RX Setting */
	sPDMA.sSrcCtrl.u32Addr 			= UARTPort;
    sPDMA.sDestCtrl.u32Addr 		= (uint32_t)DestArray;   
    sPDMA.u8TransWidth 				= eDRVPDMA_WIDTH_8BITS;
	sPDMA.u8Mode 					= eDRVPDMA_MODE_APB2MEM;
	sPDMA.sSrcCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_FIXED; 
	sPDMA.sDestCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_INCREMENTED;   
	sPDMA.i32ByteCnt                = UART_TEST_LENGTH;
	DrvPDMA_Open(eDRVPDMA_CHANNEL_0,&sPDMA);

	/* Enable INT */
    DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD );
        
	/* Install Callback function */   
 	if(i32option =='1')
		DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_0,eDRVPDMA_BLKD,(PFN_DRVPDMA_CALLBACK) PDMA_Callback_0); 
	else
	{
		DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_0,eDRVPDMA_BLKD,(PFN_DRVPDMA_CALLBACK) PDMA_Callback_1); 
		DrvUART_EnableInt(UART_PORT0,DRVUART_RDAINT,(PFN_DRVUART_CALLBACK*) UART_Callback); 
	}
	
	/* Enable UART PDMA and Trigger PDMA specified Channel */
	
 	DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_0);

	if(i32option =='1')
	{
		DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_1);
	}

	DrvUART_EnablePDMA(UART_PORT1);
	IntCnt = 0;

	/* Trigger PDMA 10 time and the S/W Flag will be change in PDMA callback funtion */
	while(IsTestOver==FALSE);

	/* Close PDMA Channel */
	DrvPDMA_Close();
	DrvUART_DisableInt(UART_PORT0,DRVUART_RDAINT);
	return;  
	
}



/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                           	   			   */
/*---------------------------------------------------------------------------------------------------------*/

int32_t main()
{
   	uint8_t unItem;
	STR_UART_T sParam;
	
	/* SYSCLK =>12Mhz*/
	UNLOCKREG();
    DrvSYS_SetOscCtrl(E_SYS_XTL12M,ENABLE);
	DrvSYS_Delay(5000);

	/* Run 48Mhz */
	DrvSYS_Open(48000);
	DrvSYS_Delay(5000);

	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);
	DrvGPIO_InitFunction(E_FUNC_UART1);
	
	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
	/* Select UART Clock Source From 12Mhz */
	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 
	DrvUART_Open(UART_PORT0,&sParam);
	DrvUART_Open(UART_PORT1,&sParam);

	do
	{
		/* PDMA Sample Code: UART1 Tx/Rx Loopback */
		printf("\n\n");
		printf("+------------------------------------------------------------------------+\n");
	    printf("|                         PDMA Driver Sample Code                        |\n");
	    printf("|                                                                        |\n");
	    printf("+------------------------------------------------------------------------+\n");                    
		printf("| [1] Using TWO PDMA channel to test. < TX1(CH1)-->RX1(CH0) >            |\n");
	    printf("| [2] Using ONE PDMA channel to test. < TX1-->RX1(CH0) >                 |\n");
		printf("+------------------------------------------------------------------------+\n");                    
		//printf("  press any key to continue ...\n");  
		unItem = getchar();
	
		IsTestOver =FALSE;
		PDMA_UART(unItem);
    printf("\n\n  PDMA sample code is complete.\n");
    }while(unItem!=0x27);
	return 0;
}	
























































































































