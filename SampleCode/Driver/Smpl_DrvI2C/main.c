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

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t Device_Addr0;
uint8_t Tx_Data0[3];
uint8_t Rx_Data0;
uint8_t DataLen0;
volatile uint8_t EndFlag0 = 0;

uint8_t  Addr1[3] = {0};
uint8_t  DataLen1;
uint8_t  Slave_Buff1[32] = {0};
uint16_t Slave_Buff_Addr1;


/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 (Master) Rx Callback Function                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_Callback_Rx(uint32_t status)
{
    if (status == 0x08)                     /* START has been transmitted and prepare SLA+W */
    {
        DrvI2C_WriteData(I2C_PORT0, Device_Addr0<<1);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }   
    else if (status == 0x18)                /* SLA+W has been transmitted and ACK has been received */
    {
        DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataLen0++]);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }
    else if (status == 0x20)                /* SLA+W has been transmitted and NACK has been received */
    {
        DrvI2C_Ctrl(I2C_PORT0, 1, 1, 1, 0);
    }
    else if (status == 0x28)                /* DATA has been transmitted and ACK has been received */
    {
        if (DataLen0 != 2)
        {
            DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataLen0++]);
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
        }
        else
        {
            DrvI2C_Ctrl(I2C_PORT0, 1, 0, 1, 0);
        }       
    }
    else if (status == 0x10)                /* Repeat START has been transmitted and prepare SLA+R */
    {
        DrvI2C_WriteData(I2C_PORT0, Device_Addr0<<1 | 0x01);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }
    else if (status == 0x40)                /* SLA+R has been transmitted and ACK has been received */
    {
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }       
    else if (status == 0x58)                /* DATA has been received and NACK has been returned */
    {
        Rx_Data0 = DrvI2C_ReadData(I2C_PORT0);
        DrvI2C_Ctrl(I2C_PORT0, 0, 1, 1, 0);
        EndFlag0 = 1;
    }
    else
    {
        printf("Status 0x%x is NOT processed\n", status);
    }           
}

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
        DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataLen0++]);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }
    else if (status == 0x20)                /* SLA+W has been transmitted and NACK has been received */
    {
        
        DrvI2C_Ctrl(I2C_PORT0, 1, 1, 1, 0);
    }   
    else if (status == 0x28)                /* DATA has been transmitted and ACK has been received */
    {
        if (DataLen0 != 3)
        {
            DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataLen0++]);
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
/*  I2C1 (Slave) Callback Function                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void I2C1_Callback_Slave(uint32_t status)
{
    
    if ((status == 0x60) || (status == 0x68))       /* SLA+W has been received and ACK has been returned */
    {
        DataLen1 = 0; 
        DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);
    }   
    else if (status == 0x80)                        /* DATA has been received and ACK has been returned */
    {
        Addr1[DataLen1++] = DrvI2C_ReadData(I2C_PORT1);
        
        if (DataLen1 == 2)
        {
            Slave_Buff_Addr1 = (Addr1[0] << 8) + Addr1[1];
        }

        if ((DataLen1 == 3) && (Slave_Buff_Addr1 < 32))
        {
            Slave_Buff1[Slave_Buff_Addr1] = Addr1[2];
            DataLen1 = 0;       
        }

        DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);
    }   
    else if ((status == 0xB0) || (status == 0xA8))  /* SLA+R has been received and ACK has been returned */
    {
        DrvI2C_WriteData(I2C_PORT1, Slave_Buff1[Slave_Buff_Addr1++]);
        if (Slave_Buff_Addr1 >= 32)
            Slave_Buff_Addr1 = 0;
        DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);
    }
    else if (status == 0xC0)                        /* DATA has been transmitted and NACK has been received */
    {
        DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);
    }
    else if (status == 0xA0)                        /* STOP or Repeat START has been received */
    {
        DataLen1 = 0;
        DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);
    }
    else
    {
        printf("Status 0x%x is NOT processed\n", status);
    }
}



/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    uint32_t u32data, i;
    STR_UART_T sParam;
    
    UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    
    /* Waiting for 12M Xtal stalble */
    DrvSYS_Delay(5000);

    /* Set UART I/O */
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
 
    printf("+--------------------------------------------------------+\n");
    printf("|                 I2C Driver Sample Code                 |\n");
    printf("+--------------------------------------------------------+\n");
    printf("  I/O Configuration:\n");
    printf("    GPA8 <--> GPA10\n");
    printf("    GPA9 <--> GPA11\n");
    printf("\n");

    /* Set I2C I/O */
    DrvGPIO_InitFunction(E_FUNC_I2C0);
    DrvGPIO_InitFunction(E_FUNC_I2C1);

    /* Open I2C0 and I2C1, and set clock = 100Kbps */
    DrvI2C_Open(I2C_PORT0, 100000);
    DrvI2C_Open(I2C_PORT1, 100000);

    /* Get I2C0 clock */
    u32data = DrvI2C_GetClockFreq(I2C_PORT0);
    printf("I2C0 clock %d Hz\n", u32data);

    /* Set I2C0 slave addresses */
    DrvI2C_SetAddress(I2C_PORT0, 0, 0x15, 0);
    DrvI2C_SetAddress(I2C_PORT0, 1, 0x35, 0);
    DrvI2C_SetAddress(I2C_PORT0, 2, 0x55, 0);
    DrvI2C_SetAddress(I2C_PORT0, 3, 0x75, 0);

    /* Set I2C1 slave addresses */
    DrvI2C_SetAddress(I2C_PORT1, 0, 0x16, 0);
    DrvI2C_SetAddress(I2C_PORT1, 1, 0x36, 0);
    DrvI2C_SetAddress(I2C_PORT1, 2, 0x56, 0);
    DrvI2C_SetAddress(I2C_PORT1, 3, 0x76, 0);
    
    for (i = 0; i < 32; i++)
    {
        Slave_Buff1[i] = 0;
    }
    
    /* Set AA bit, I2C1 as slave (To simulate a 24LXX EEPROM) */
    DrvI2C_Ctrl(I2C_PORT1, 0, 0, 0, 1);
        
    /* Enable I2C0 interrupt and set corresponding NVIC bit */
    DrvI2C_EnableInt(I2C_PORT0);
        
    /* Enable I2C1 interrupt and set corresponding NVIC bit */
    DrvI2C_EnableInt(I2C_PORT1);
        
    /* Install I2C1 call back function for slave */
    DrvI2C_InstallCallback(I2C_PORT1, I2CFUNC, I2C1_Callback_Slave);

    Device_Addr0 = 0x16;
    Tx_Data0[0] = 0x00;
    Tx_Data0[1] = 0x00;
        
    for (i = 0; i < 32; i++)
    {
        Tx_Data0[2] = 0x10 + i;
        DataLen0 = 0;
        EndFlag0 = 0;
        
        /* Install I2C0 call back function for write data to slave */
        DrvI2C_InstallCallback(I2C_PORT0, I2CFUNC, I2C0_Callback_Tx);
        
        /* I2C0 as master sends START signal */
        DrvI2C_Ctrl(I2C_PORT0, 1, 0, 0, 0);
        
        /* Wait I2C0 Tx Finish */
        while (EndFlag0 == 0);
        EndFlag0 = 0;

        /* Uninstall I2C0 call back function for write data to slave */
        DrvI2C_UninstallCallBack(I2C_PORT0, I2CFUNC);
        
        /* Install I2C0 call back function for read data from slave */
        DrvI2C_InstallCallback(I2C_PORT0, I2CFUNC, I2C0_Callback_Rx);

        DataLen0 = 0;
        Device_Addr0 = 0x16;
        DrvI2C_Ctrl(I2C_PORT0, 1, 0, 0, 0);
        
        /* Wait I2C0 Rx Finish */
        while (EndFlag0 == 0);

        /* Compare data */
        if (Rx_Data0 != Tx_Data0[2])
        {
            printf("I2C0 Byte Write/Read Failed, Data 0x%x\n", Rx_Data0);
            return -1;
        }           
    }
    printf("I2C0(Master) to I2C1(Slave) Test OK\n");

    /* Disable I2C0 interrupt and clear corresponding NVIC bit */
    DrvI2C_DisableInt(I2C_PORT0);
    
    /* Disable I2C1 interrupt and clear corresponding NVIC bit */
    DrvI2C_DisableInt(I2C_PORT1);

    /* Close I2C0 and I2C1 */
    DrvI2C_Close(I2C_PORT0);
    DrvI2C_Close(I2C_PORT1);

    return 0;
}




