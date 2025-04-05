/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "UART.h"
#include "GPIO.h"
#include "SPI.h"
#include "PDMA.h"

#define TEST_COUNT 64

void SpiLoopbackTest(void);
void SpiLoopbackTest_WithPDMA(void);
void SPI0_Callback(uint32_t u32UserData);
void SPI1_Callback(uint32_t u32UserData);
void PDMA_Callback(uint32_t u32UserData);

volatile uint32_t g_Spi0IntFlag;
volatile uint32_t g_Spi1IntFlag;
volatile uint32_t g_PdmaIntFlag;
uint32_t g_au32SlaveRxData[TEST_COUNT];
uint32_t g_u32SlaveRxDataCount;


int main(void)
{
    STR_UART_T param;
    uint8_t u8Option;
    
    /* Unlock the protected registers */
    UNLOCKREG();

    /* Enable the 12MHz oscillator oscillation */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
 
    /* Waiting for 12MHz Xtal stable */
    DrvSYS_Delay(5000);
 
    /* HCLK clock source. 0: external 12MHz. */
    DrvSYS_SelectHCLKSource(0);

    LOCKREG();

    DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */

    DrvGPIO_InitFunction(E_FUNC_UART0);
    
    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0);
    
    param.u32BaudRate        = 115200;
    param.u8cDataBits        = DRVUART_DATABITS_8;
    param.u8cStopBits        = DRVUART_STOPBITS_1;
    param.u8cParity          = DRVUART_PARITY_NONE;
    param.u8cRxTriggerLevel  = DRVUART_FIFO_1BYTES;
    param.u8TimeOut          = 0;
    DrvUART_Open(UART_PORT0, &param);
    
    /* Configure SPI0 related multi-function pins */
    DrvGPIO_InitFunction(E_FUNC_SPI0);
    /* Configure SPI0 as a master, 32-bit transaction */
    DrvSPI_Open(eDRVSPI_PORT0, eDRVSPI_MASTER, eDRVSPI_TYPE1, 32);
    /* Enable the automatic slave select function of SS0. */
    DrvSPI_EnableAutoSS(eDRVSPI_PORT0, eDRVSPI_SS0);
    /* Set the active level of slave select. */
    DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT0, eDRVSPI_ACTIVE_LOW_FALLING);
    /* SPI clock rate 2MHz */
    DrvSPI_SetClockFreq(eDRVSPI_PORT0, 2000000, 0);
    
    /* Configure SPI1 related multi-function pins */
    DrvGPIO_InitFunction(E_FUNC_SPI1);
    /* Configure SPI1 as a slave, 32-bit transaction */
    DrvSPI_Open(eDRVSPI_PORT1, eDRVSPI_SLAVE, eDRVSPI_TYPE1, 32);
    /* Set the trigger mode of slave select pin. */
    DrvSPI_SetTriggerMode(eDRVSPI_PORT1, eDRVSPI_LEVEL_TRIGGER);   /* level trigger */
    /* Set the active level of slave select. */
    DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT1, eDRVSPI_ACTIVE_LOW_FALLING);
    
    printf("\n\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                       SPI Driver Sample Code                         |\n");
    printf("|                                                                      |\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("\n");
    printf("SPI Driver version: %x\n", DrvSPI_GetVersion());
    printf("Configure SPI0 as a master and SPI1 as a slave.\n");
    printf("Bit length of a transaction: 32\n");
    printf("SPI clock rate: %d Hz\n", DrvSPI_GetClock1Freq(eDRVSPI_PORT0));
    printf("The I/O connection for SPI0/SPI1 loopback:\n    SPI0_SS0(GPC0)   <--> SPI1_SS0(GPC8)\n    SPI0_CLK(GPC1)   <--> SPI1_CLK(GPC9)\n");
    printf("    SPI0_MISO0(GPC2) <--> SPI1_MISO0(GPC10)\n    SPI0_MOSI0(GPC3) <--> SPI1_MOSI0(GPC11)\n\n");
    printf("Please connect SPI0 with SPI1, and press any key to start transmission.\n");
    
    while(1)
    {
        printf("Test options:\n");
        printf("  [1] General SPI data transfer\n");
        printf("  [2] SPI data transfer with PDMA\n");
        printf("  [Other keys] Quit\n");
        printf("Please connect SPI0 with SPI1, and choose a test item to start transmission.\n");
        u8Option = getchar();

        if( u8Option == '1' )
            SpiLoopbackTest();
        else if( u8Option == '2' )
            SpiLoopbackTest_WithPDMA();
        else
            break;
    }
    
    printf("\n\nExit SPI driver sample code.\n");

    DrvSPI_Close(eDRVSPI_PORT0);
    DrvSPI_Close(eDRVSPI_PORT1);
    return 0;
}


void SpiLoopbackTest(void)
{    
    uint32_t au32SourceData[TEST_COUNT];
    uint32_t au32DestinationData[TEST_COUNT];
    uint32_t u32DataCount, u32TestCycle;
    int32_t i32Err;
    uint32_t u32Tmp;
    
    printf("\nSPI0/1 Loopback test ");
    
    /* Enable the SPI1 interrupt and install the callback function. */
    DrvSPI_EnableInt(eDRVSPI_PORT1, SPI1_Callback, 0);
    
    /* Clear Tx register of SPI1 to avoid send non-zero data to Master. Just for safe. */
    u32Tmp = 0;
    DrvSPI_SetTxRegister(eDRVSPI_PORT1, &u32Tmp, 1);
    
    /* set the soure data and clear the destination buffer */
    for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
    {
        au32SourceData[u32DataCount] = u32DataCount;
        au32DestinationData[u32DataCount] = 0;
    }
    
    i32Err = 0;
    for(u32TestCycle=0; u32TestCycle<10000; u32TestCycle++)
    {
        u32DataCount=0;
        g_Spi1IntFlag = 0;

        if((u32TestCycle&0x1FF) == 0)
        {
            putchar('.');
        }
        /* set the GO_BUSY bit of SPI1 */
        DrvSPI_SetGo(eDRVSPI_PORT1);
        /* write the first data of source buffer to Tx register of SPI0. And start transmission. */
        DrvSPI_SingleWrite(eDRVSPI_PORT0, &au32SourceData[0]);
    
        while(1)
        {
            if(g_Spi1IntFlag==1)
            {
                g_Spi1IntFlag = 0;
                
                if(u32DataCount<(TEST_COUNT-1))
                {
                    /* Read the previous retrieved data and trigger next transfer. */
                    DrvSPI_SingleRead(eDRVSPI_PORT1, &au32DestinationData[u32DataCount]);
                    u32DataCount++;
                    DrvSPI_SingleWrite(eDRVSPI_PORT0, &au32SourceData[u32DataCount]);
                }
                else
                {
                    /* Just read the previous retrived data but trigger next transfer, because this is the last transfer. */
                    DrvSPI_DumpRxRegister(eDRVSPI_PORT1, &au32DestinationData[u32DataCount], 1);
                    break;
                }
            }
        }
        
        for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
        {
            if(au32DestinationData[u32DataCount]!=au32SourceData[u32DataCount])
            {
                i32Err = 1;
            }
        }

        if(i32Err)
            break;
    }

    if(i32Err)
    {
        printf(" [FAIL]\n\n");
    }
    else
    {
        printf(" [PASS]\n\n");
    }
    
    /* Disable the SPI1 interrupt */
    DrvSPI_DisableInt(eDRVSPI_PORT1);
    
    return ;
}

void SpiLoopbackTest_WithPDMA(void)
{
    uint32_t au32SourceData[TEST_COUNT];
    uint32_t au32DestinationData[TEST_COUNT];
    uint32_t u32DataCount, u32TestCycle;
    int32_t i32Err;
    uint32_t u32Tmp;
    STR_PDMA_T sPDMA;
    
    printf("\nSPI0/1 Loopback test with one PDMA Rx channel");
    
    /* Clear Tx register of SPI1 to avoid send non-zero data to Master. Just for safe. */
    u32Tmp = 0;
    DrvSPI_SetTxRegister(eDRVSPI_PORT1, &u32Tmp, 1);
    
    /* set the soure data and clear the destination buffer */
    for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
    {
        au32SourceData[u32DataCount] = u32DataCount;
        au32DestinationData[u32DataCount] = 0;
    }
    
    /* PDMA Init */
    DrvPDMA_Init();
    
    /* RxPDMA Setting */
    DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_0, eDRVPDMA_SPI1, eDRVPDMA_READ_APB);
    
    /* CH0 RX Setting */
    sPDMA.sSrcCtrl.u32Addr          = (uint32_t)&(SPI1->RX[0]);
    sPDMA.sDestCtrl.u32Addr         = (uint32_t)au32DestinationData;
    sPDMA.u8TransWidth              = eDRVPDMA_WIDTH_32BITS;
    sPDMA.u8Mode                    = eDRVPDMA_MODE_APB2MEM;
    sPDMA.sSrcCtrl.eAddrDirection   = eDRVPDMA_DIRECTION_FIXED; 
    sPDMA.sDestCtrl.eAddrDirection  = eDRVPDMA_DIRECTION_INCREMENTED;
    sPDMA.i32ByteCnt                = TEST_COUNT*4;
    DrvPDMA_Open(eDRVPDMA_CHANNEL_0,&sPDMA);
    
    /* Enable INT */
    DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD);
    
    /* Install Callback function */
    DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD, (PFN_DRVPDMA_CALLBACK) PDMA_Callback);
    
    i32Err = 0;
    for(u32TestCycle=0; u32TestCycle<10000; u32TestCycle++)
    {
        if((u32TestCycle&0x1FF) == 0)
            putchar('.');
        
        /* Enable SPI PDMA and Trigger PDMA specified Channel */
        DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_0);
        DrvSPI_SetPDMA(eDRVSPI_PORT1, eDRVSPI_RX_DMA, TRUE);
        
        /* set the GO_BUSY bit of SPI1 */
        DrvSPI_SetGo(eDRVSPI_PORT1);
        
        /* Enable the SPI0 interrupt and install the callback function. */
        DrvSPI_EnableInt(eDRVSPI_PORT0, SPI0_Callback, 0);
        
        g_PdmaIntFlag = 0;
        g_Spi0IntFlag = 0;
        u32DataCount = 0;
        
        /* write the first data of source buffer to Tx register of SPI0. And start transmission. */
        DrvSPI_SingleWrite(eDRVSPI_PORT0, &au32SourceData[0]);
        while(1)
        {
            if(g_Spi0IntFlag==1)
            {
                g_Spi0IntFlag = 0;
            
                if(u32DataCount<(TEST_COUNT-1))
                {
                    /* Trigger next transfer. */
                    u32DataCount++;
                    DrvSPI_SingleWrite(eDRVSPI_PORT0, &au32SourceData[u32DataCount]);
                }
                else
                    break;
            }
        }
        
        /* Disable the SPI0 interrupt */
        DrvSPI_DisableInt(eDRVSPI_PORT0);
        
        /* Wait PDMA transfer done */
        while(g_PdmaIntFlag==0);
        
        
        /* Compare the transfer data */
        for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
        {
            if(au32DestinationData[u32DataCount]!=au32SourceData[u32DataCount])
                i32Err = 1;
        }

        if(i32Err)
            break;
    }
    
    /* Disable SPI PDMA */
    DrvSPI_SetPDMA(eDRVSPI_PORT1, eDRVSPI_RX_DMA, FALSE);
    
    /* Close PDMA */
    DrvPDMA_Close();
    
    if(i32Err)
    {   
        printf(" [FAIL]\n\n");
    }
    else
    {
        printf(" [PASS]\n\n");
    }
    
    return;
}

void SPI0_Callback(uint32_t u32UserData)
{
    g_Spi0IntFlag = 1;
}

void SPI1_Callback(uint32_t u32UserData)
{
    g_Spi1IntFlag = 1;
}

void PDMA_Callback(uint32_t u32UserData)
{
    g_PdmaIntFlag = 1;
}


