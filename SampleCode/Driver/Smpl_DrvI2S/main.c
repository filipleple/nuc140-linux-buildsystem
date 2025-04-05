/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "I2C.h"
#include "I2S.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define BUFF_LEN    64

uint32_t PcmBuff[BUFF_LEN] = {0};
uint32_t u32BuffPos = 0;

volatile uint32_t EndFlag0 = 0;
uint8_t Device_Addr0 = 0x1A;                /* WAU8822 Device ID */
uint8_t Tx_Data0[2];
uint8_t DataCnt0;


/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 (Master) Tx Callback Function                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_Callback_Tx(uint32_t status)
{
    if (status == 0x08)                     /* START has been transmitted */
    {
        DrvI2C_WriteData(I2C_PORT0, Device_Addr0<<1);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }   
    else if (status == 0x18)                /* SLA+W has been transmitted and ACK has been received */
    {
        DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataCnt0++]);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }
    else if (status == 0x20)                /* SLA+W has been transmitted and NACK has been received */
    {
        
        DrvI2C_Ctrl(I2C_PORT0, 1, 1, 1, 0);
    }   
    else if (status == 0x28)                /* DATA has been transmitted and ACK has been received */
    {
        if (DataCnt0 != 2)
        {
            DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataCnt0++]);
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
        }
        else
        {
            DrvI2C_Ctrl(I2C_PORT0, 0, 1, 1, 0);
            EndFlag0 = 1;
        }       
    }
    else
    {
        printf("Status 0x%x is NOT processed\n", status);
    }       
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2S Tx Threshold Level Callback Function when Tx FIFO is less than Tx FIFO Threshold Level             */
/*---------------------------------------------------------------------------------------------------------*/
void Tx_thresholdCallbackfn(uint32_t status)
{
    uint32_t u32Len, i;
    uint32_t * pBuff;

    pBuff = &PcmBuff[0];

    /* Read Tx FIFO free size */
    u32Len = 8 - _DRVI2S_READ_TX_FIFO_LEVEL();
    
    if (u32BuffPos >= 8)
    {
        for (i = 0; i < u32Len; i++)
        {
            _DRVI2S_WRITE_TX_FIFO(pBuff[i]);
        }

        for (i = 0; i < BUFF_LEN - u32Len; i++)
        {
            pBuff[i] = pBuff[i + u32Len];   
        }

        u32BuffPos -= u32Len;
    }
    else
    {
        for (i = 0; i < u32Len; i++)
        {
            _DRVI2S_WRITE_TX_FIFO(0x00);       
        }           
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2S Rx Threshold Level Callback Function when Rx FIFO is more than Rx FIFO Threshold Level             */
/*---------------------------------------------------------------------------------------------------------*/
void Rx_thresholdCallbackfn(uint32_t status)
{
    uint32_t u32Len, i;
    uint32_t *pBuff;

    if (u32BuffPos < (BUFF_LEN-8))
    {
        pBuff = &PcmBuff[u32BuffPos];

        /* Read Rx FIFO Level */
        u32Len = _DRVI2S_READ_RX_FIFO_LEVEL();
    
        for ( i = 0; i < u32Len; i++ )
        {
            pBuff[i] = _DRVI2S_READ_RX_FIFO();
        }
    
        u32BuffPos += u32Len;
    
        if (u32BuffPos >= BUFF_LEN)
        {
            u32BuffPos =    0;
        }                           
    }
}


void Delay(int count)
{
    volatile uint32_t i;
    for (i = 0; i < count ; i++);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Write 9-bit data to 7-bit address register of WAU8822 with I2C0                                        */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data)
{       
    DataCnt0 = 0;
    EndFlag0 = 0;
    
    Tx_Data0[0] = (uint8_t)((u8addr << 1) | (u16data >> 8));
    Tx_Data0[1] = (uint8_t)(u16data & 0x00FF);

    /* Install I2C0 call back function for write data to slave */
    DrvI2C_InstallCallback(I2C_PORT0, I2CFUNC, I2C0_Callback_Tx);
        
    /* I2C0 as master sends START signal */
    DrvI2C_Ctrl(I2C_PORT0, 1, 0, 0, 0);
        
    /* Wait I2C0 Tx Finish */
    while (EndFlag0 == 0);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  WAU8822 Settings with I2C interface                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void WAU8822_Setup()
{
    printf("WAU8822 Setup\n");
    
    I2C_WriteWAU8822(0,  0x000);   /* Reset all registers */ 
    Delay(0x200);
        
    I2C_WriteWAU8822(1,  0x02F);   /* Enable internal PLL, analog bias buffer, tie-off buffer, and select 3kohm at VREF pin */        
    I2C_WriteWAU8822(2,  0x1B3);   /* Enable L/R Headphone, ADC Mix/Boost, ADC */

    I2C_WriteWAU8822(3,  0x07F);   /* Enable L/R main mixer, DAC */
            
    I2C_WriteWAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */
            
    I2C_WriteWAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */

    I2C_WriteWAU8822(6,  0x1AD);   /* Divide by 6, 16K */

    I2C_WriteWAU8822(7,  0x006);   /* 16K for internal filter cofficients */
    
    I2C_WriteWAU8822(10, 0x008);   /* DAC softmute is disabled, DAC oversampling rate is 128x */

    I2C_WriteWAU8822(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */

    I2C_WriteWAU8822(15, 0x1EF);   /* ADC left digital volume control */
    I2C_WriteWAU8822(16, 0x1EF);   /* ADC right digital volume control */
    
	I2C_WriteWAU8822(43, 0x010);   /* Right speaker amplifier directly connected to RMIX speaker signal gain stage*/
    
    I2C_WriteWAU8822(44, 0x000);   /* LLIN/RLIN is not connected to PGA */

	I2C_WriteWAU8822(45, 0x150);   /* Left channel PGA is set to 0.0dB*/
	I2C_WriteWAU8822(46, 0x150);   /* Right channel PGA is set to 0.0dB*/	
	
    I2C_WriteWAU8822(47, 0x007);   /* LLIN connected, and its Gain value */
    I2C_WriteWAU8822(48, 0x007);   /* RLIN connected, and its Gain value */

	I2C_WriteWAU8822(49, 0x047);   /* The gain value between LLIN line input and LPGA Mix/Boost stage input is set to -3.0dB*/
								   /* The gain value between LAUXIN auxiliary input and LPGA Mix/Boost stage input is set to +6.0dB*/
	
    I2C_WriteWAU8822(50, 0x001);   /* Left DAC connected to LMIX */
    I2C_WriteWAU8822(51, 0x000);   /* Right DAC disconnected to RMIX */
 
 	I2C_WriteWAU8822(54, 0x139);   /* LSPKOUT Volume */
	I2C_WriteWAU8822(55, 0x139);   /* RSPKOUT Volume */

	DrvGPIO_Open(E_GPE,14, E_IO_OUTPUT);	
	DrvGPIO_ClrBit(E_GPE,14);
 
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    uint32_t u32clock; 
    __IO uint32_t u32startFlag = 1;
    
    STR_UART_T sParam;
    S_DRVI2S_DATA_T st; 

    UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    
    /* Waiting for 12M Xtal stalble */
    DrvSYS_Delay(5000);
                     
    /* Tri-state for FS and BCLK of CODEC */
    DrvGPIO_Open(E_GPC, 0, E_IO_OPENDRAIN);
    DrvGPIO_Open(E_GPC, 1, E_IO_OPENDRAIN);
    DrvGPIO_SetBit(E_GPC, 0);
    DrvGPIO_SetBit(E_GPC, 1);
    
    /* Set I2C I/O */
    DrvGPIO_InitFunction(E_FUNC_I2C0);
  
    /* Set UART I/O */
    DrvGPIO_InitFunction(E_FUNC_UART0);

    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0);
    
    /* Select I2S Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_I2S_CLKSRC, 0);    

    /* UART Setting */
    sParam.u32BaudRate       = 115200;
    sParam.u8cDataBits       = DRVUART_DATABITS_8;
    sParam.u8cStopBits       = DRVUART_STOPBITS_1;
    sParam.u8cParity         = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel = DRVUART_FIFO_1BYTES;

    /* Set UART Configuration */
    DrvUART_Open(UART_PORT0, &sParam);
    
    printf("+------------------------------------------------------------------------+\n");
    printf("|                   I2S Driver Sample Code with WAU8822                  |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  NOTE: This sample code needs to work with WAU8822.\n");
    
    /* Open I2C0, and set clock = 100Kbps */
    DrvI2C_Open(I2C_PORT0, 100000);

    /* Get I2C0 clock */
    u32clock = DrvI2C_GetClockFreq(I2C_PORT0);
    printf("I2C0 clock %d Hz\n", u32clock);

    /* Enable I2C0 interrupt and set corresponding NVIC bit */
    DrvI2C_EnableInt(I2C_PORT0);
        
    st.u32SampleRate     = 16000;
    st.u8WordWidth       = DRVI2S_DATABIT_16;
    st.u8AudioFormat     = DRVI2S_STEREO;
    st.u8DataFormat      = DRVI2S_FORMAT_I2S;
    st.u8Mode            = DRVI2S_MODE_SLAVE;
    st.u8TxFIFOThreshold = DRVI2S_FIFO_LEVEL_WORD_0;
    st.u8RxFIFOThreshold = DRVI2S_FIFO_LEVEL_WORD_8;
        
    DrvI2S_Open(&st);

    WAU8822_Setup();

    /* Set I2S I/O */
    DrvGPIO_InitFunction(E_FUNC_I2S);
    
    /* Set MCLK and enable MCLK */
    DrvI2S_SetMCLKFreq(12000000);
    
    u32clock = DrvI2S_GetMCLKFreq();
    printf("MCLK %d\n", u32clock);
    DrvI2S_EnableMCLK();

    /* Enable Rx threshold level interrupt and install its callback function */
    DrvI2S_EnableInt(I2S_RX_FIFO_THRESHOLD, Rx_thresholdCallbackfn);
    
    /* Enable I2S Rx function to receive data */
    DrvI2S_EnableRx();

    while(1)
    {
        if (u32startFlag)
        {
            /* Enable I2S Tx function to send data when data in the buffer is more than half of buffer size */
            if (u32BuffPos >= BUFF_LEN/2)
            {
                DrvI2S_EnableInt(I2S_TX_FIFO_THRESHOLD, Tx_thresholdCallbackfn);    
                DrvI2S_EnableTx();
                u32startFlag = 0;
            }
        }
    }
}




