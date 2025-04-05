/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Include related header files                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "GPIO.h"
#include "ADC.h"
#include "UART.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define Function Prototypes                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void AdcSingleModeTest(void);
void AdcSingleCycleModeTest(void);
void AdcContScanModeTest(void);
void AdcResultMonitorTest(void);
void AdcIntCallback(uint32_t);
void Cmp0IntCallback(uint32_t);
void Cmp1IntCallback(uint32_t);
uint8_t SingleEndInput_SingleOpModeChannelSelect(uint8_t *);
uint8_t DifferentialInput_SingleOpModeChannelSelect(uint8_t *);
uint8_t SingleEndInput_ScanOpModeChannelSelect(uint8_t *, uint8_t *);
uint8_t DifferentialInput_ScanOpModeChannelSelect(uint8_t *, uint8_t *);

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t gu8AdcIntFlag;
volatile uint8_t gu8AdcCmp0IntFlag;
volatile uint8_t gu8AdcCmp1IntFlag;


/* Main function */
int main(void)
{
    uint8_t u8Option;
    STR_UART_T param;
    
    /* Unlock the locked registers before access */
    UNLOCKREG(x);

    /* System clock rate 32MHz */
    DrvSYS_Open(32000000);

    /* HCLK clock source. 0: external 4~24MHz crystal clock; 2: PLL clock; 4:internal 22.1184MHz RC oscillator */
    DrvSYS_SelectHCLKSource(2);

    /* Configure UART0 related pins */
    DrvGPIO_InitFunction(E_FUNC_UART0);
    
    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0);

    /* UART configuration */
    param.u32BaudRate        = 115200;
    param.u8cDataBits        = DRVUART_DATABITS_8;
    param.u8cStopBits        = DRVUART_STOPBITS_1;
    param.u8cParity          = DRVUART_PARITY_NONE;
    param.u8cRxTriggerLevel  = DRVUART_FIFO_1BYTES;
    param.u8TimeOut          = 0;

    DrvUART_Open(UART_PORT0, &param);

    printf("\n\n");
    printf("Driver version: %x\n", DrvADC_GetVersion());
    
    while(1)
    {
        /* default setting: single end input, single operation mode, all channel disable, ADC clock frequency = 32MHz/(3+1) */
        DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_OP, 0, INTERNAL_HCLK, 1); /* The maximum ADC clock rate is 16MHz */
        printf("Conversion rate: %d samples/second\n", DrvADC_GetConversionRate());
        
        printf("\n\n");
        printf("+----------------------------------------------------------------------+\n");       
        printf("|                            ADC sample code                           |\n");       
        printf("+----------------------------------------------------------------------+\n");       
        printf("  [1] ADC single mode test\n");
        printf("  [2] ADC single cycle mode test\n");
        printf("  [3] ADC continuous scan mode test\n");
        printf("  [4] ADC compare function test\n");
        printf("  [q] Quit\n");
        printf("Please choose a test item.\n");

        u8Option = getchar();

        if(u8Option == '1')
        {
            AdcSingleModeTest();
        }
        else if(u8Option == '2')
        {
            AdcSingleCycleModeTest();
        }
        else if(u8Option == '3')
        {
            AdcContScanModeTest();
        }
        else if(u8Option == '4')
        {
            AdcResultMonitorTest();
        }
        else if( (u8Option == 'q') || (u8Option == 'Q') )
        {
            break;
        }
        printf("\n\n");
    }
    printf("\n--- Exit ADC sample code ---\n");
    DrvADC_Close();
    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: AdcSingleModeTest                                                                             */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Description:                                                                                            */
/*   ADC single mode test.                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
void AdcSingleModeTest()
{
    uint8_t u8Option;
    E_ADC_INPUT_MODE InputMode;
    uint8_t u8ChannelSelBitwise, u8ChannelNum;
    int32_t i32ConversionData;
    
    printf("\n=== Single mode test ===\n");
    /* Set the ADC operation mode as single mode */
    DrvADC_SetADCOperationMode(ADC_SINGLE_OP);
    
    while(1)
    {
        printf("Select input mode:\n");
        printf("  [1] Single end input\n");
        printf("  [2] Differential input\n");
        printf("  Other keys: exit single mode test\n");
        u8Option = getchar();
        if(u8Option=='1')
            InputMode = ADC_SINGLE_END;
        else if(u8Option=='2')
            InputMode = ADC_DIFFERENTIAL;
        else
            return ;
        
        /* Set the ADC input mode */
        DrvADC_SetADCInputMode(InputMode);
        
        if(InputMode==ADC_SINGLE_END)
            u8ChannelNum = SingleEndInput_SingleOpModeChannelSelect(&u8ChannelSelBitwise);    /* Select the active channel */
        else
            u8ChannelNum = DifferentialInput_SingleOpModeChannelSelect(&u8ChannelSelBitwise); /* Select the active channel */
        
        /* Check the active channel number */
        if(u8ChannelNum==0xFF)
        {
            printf("No channel was selected.\n");
            printf("--- Exit single mode test ---\n");
            return ;
        }
        
        /* Set the ADC channel */
        DrvADC_SetADCChannel(u8ChannelSelBitwise);
        
        /* Reset the ADC interrupt indicator */
        gu8AdcIntFlag = 0;
        /* Enable the ADC interrupt and setup callback function */
        DrvADC_EnableADCInt(AdcIntCallback, 0);
    
        /* Start A/D conversion */
        DrvADC_StartConvert();
        /* Wait ADC interrupt */
        while(gu8AdcIntFlag==0);
        
        /* Disable the ADC interrupt */
        DrvADC_DisableADCInt();
        
        /* Get the conversion result of the specified ADC channel */
        i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
        if(InputMode==ADC_SINGLE_END)
            printf("Conversion result of channel %d: 0x%X (%d)\n\n", u8ChannelNum, i32ConversionData, i32ConversionData);
        else
            printf("Conversion result of differential input pair %d: 0x%X (%d)\n\n", u8ChannelNum/2, i32ConversionData, i32ConversionData);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: AdcSingleCycleModeTest                                                                        */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Description:                                                                                            */
/*   ADC single cycle scan mode test.                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void AdcSingleCycleModeTest()
{
    uint8_t u8Option;
    E_ADC_INPUT_MODE InputMode;
    uint8_t u8ChannelSelBitwise=0, u8ActiveChannelNum=0, u8ChannelNum, u8ChannelCount;
    int32_t i32ConversionData;
    uint8_t au8ActiveChannel[8];
    
    printf("\n=== Single cycle scan mode test ===\n");
    /* Set the ADC operation mode as single cycle scan mode */
    DrvADC_SetADCOperationMode(ADC_SINGLE_CYCLE_OP);
    
    while(1)
    {
        printf("Select input mode:\n");
        printf("  [1] Single end input\n");
        printf("  [2] Differential input\n");
        printf("  Other keys: exit single cycle scan mode test\n");
        u8Option = getchar();
        if(u8Option=='1')
            InputMode = ADC_SINGLE_END;
        else if(u8Option=='2')
            InputMode = ADC_DIFFERENTIAL;
        else
            return ;
        
        /* Set the ADC input mode */
        DrvADC_SetADCInputMode(InputMode);
        
        if(InputMode==ADC_SINGLE_END)
        {
            /* Select the ADC channels */
            u8ActiveChannelNum = SingleEndInput_ScanOpModeChannelSelect(&u8ChannelSelBitwise, au8ActiveChannel);
            /* Check the active channel number */
            if(u8ActiveChannelNum==0)
            {
                printf("No channel was selected.\n");
                printf("--- Exit single cycle scan mode test ---\n");
                return ;
            }
            
            /* Set the ADC channel */
            DrvADC_SetADCChannel(u8ChannelSelBitwise);
            
            /* start A/D conversion */
            DrvADC_StartConvert();
            
            /* Wait conversion done */
            while(DrvADC_IsConversionDone()==FALSE);
            
            for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
            {
                u8ChannelNum = au8ActiveChannel[u8ChannelCount];
                i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
                printf("Conversion result of channel %d: 0x%X (%d)\n\n", u8ChannelNum, i32ConversionData, i32ConversionData);
            }
        }
        else /* Differential input mode */
        {
            /* Select the ADC channels */
            u8ActiveChannelNum = DifferentialInput_ScanOpModeChannelSelect(&u8ChannelSelBitwise, au8ActiveChannel);
            /* Check the active channel number */
            if(u8ActiveChannelNum==0)
            {
                printf("No channel was selected.\n");
                printf("--- Exit single cycle scan mode test ---\n");
                return ;
            }
            
            /* Set the ADC channel */
            DrvADC_SetADCChannel(u8ChannelSelBitwise);
            
            /* start A/D conversion */
            DrvADC_StartConvert();
            
            /* Wait conversion done */
            while(DrvADC_IsConversionDone()==FALSE);
            
            for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
            {
                u8ChannelNum = au8ActiveChannel[u8ChannelCount] * 2;
                i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
                printf("Conversion result of differential input pair %d: 0x%X (%d)\n\n", au8ActiveChannel[u8ChannelCount], i32ConversionData, i32ConversionData);
            }
        }
        
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: AdcContScanModeTest                                                                           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Description:                                                                                            */
/*   ADC continuous scan mode test.                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
void AdcContScanModeTest()
{
    uint8_t u8Option;
    E_ADC_INPUT_MODE InputMode;
    uint8_t u8ChannelSelBitwise=0, u8ActiveChannelNum=0, u8ChannelNum, u8ChannelCount;
    int32_t i32ConversionData;
    uint8_t au8ActiveChannel[8];
    
    printf("\n=== Continuous scan mode test ===\n");
    /* Set the ADC operation mode as continuous scan mode */
    DrvADC_SetADCOperationMode(ADC_CONTINUOUS_OP);
    
    while(1)
    {
        printf("Select input mode:\n");
        printf("  [1] Single end input\n");
        printf("  [2] Differential input\n");
        printf("  Other keys: exit continuous scan mode test\n");
        u8Option = getchar();
        if(u8Option=='1')
            InputMode = ADC_SINGLE_END;
        else if(u8Option=='2')
            InputMode = ADC_DIFFERENTIAL;
        else
            return ;
        
        /* Set the ADC input mode */
        DrvADC_SetADCInputMode(InputMode);
        
        if(InputMode==ADC_SINGLE_END)
        {
            /* Select the ADC channels */
            u8ActiveChannelNum = SingleEndInput_ScanOpModeChannelSelect(&u8ChannelSelBitwise, au8ActiveChannel);
            /* Check the active channel number */
            if(u8ActiveChannelNum==0)
            {
                printf("No channel was selected.\n");
                printf("--- Exit continuous scan mode test ---\n");
                return ;
            }
            
            /* Set the ADC channel */
            DrvADC_SetADCChannel(u8ChannelSelBitwise);
            
            /* start A/D conversion */
            DrvADC_StartConvert();
            
            /* Wait conversion done */
            while(DrvADC_IsConversionDone()==FALSE);
            
            for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
            {
                u8ChannelNum = au8ActiveChannel[u8ChannelCount];
                i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
                printf("Conversion result of channel %d: 0x%X (%d)\n\n", u8ChannelNum, i32ConversionData, i32ConversionData);
            }
            
            /* Wait conversion done */
            while(DrvADC_IsConversionDone()==FALSE);
            
            for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
            {
                u8ChannelNum = au8ActiveChannel[u8ChannelCount];
                i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
                printf("Conversion result of channel %d: 0x%X (%d)\n\n", u8ChannelNum, i32ConversionData, i32ConversionData);
            }
            
            /* Stop A/D conversion */
            DrvADC_StopConvert();
            
            /* Clear the ADC interrupt flag */
            _DRVADC_CLEAR_ADC_INT_FLAG();
            
        }
        else /* Differential input mode */
        {
            /* Select the ADC channels */
            u8ActiveChannelNum = DifferentialInput_ScanOpModeChannelSelect(&u8ChannelSelBitwise, au8ActiveChannel);
            /* Check the active channel number */
            if(u8ActiveChannelNum==0)
            {
                printf("No channel was selected.\n");
                printf("--- Exit continuous scan mode test ---\n");
                return ;
            }
            
            /* Set the ADC channel */
            DrvADC_SetADCChannel(u8ChannelSelBitwise);
            
            /* start A/D conversion */
            DrvADC_StartConvert();
            
            /* Wait conversion done */
            while(DrvADC_IsConversionDone()==FALSE);
            
            for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
            {
                u8ChannelNum = au8ActiveChannel[u8ChannelCount] * 2;
                i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
                printf("Conversion result of differential input pair %d: 0x%X (%d)\n\n", au8ActiveChannel[u8ChannelCount], i32ConversionData, i32ConversionData);
            }
            
            /* Wait conversion done */
            while(DrvADC_IsConversionDone()==FALSE);
            
            for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
            {
                u8ChannelNum = au8ActiveChannel[u8ChannelCount] * 2;
                i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
                printf("Conversion result of differential input pair %d: 0x%X (%d)\n\n", au8ActiveChannel[u8ChannelCount], i32ConversionData, i32ConversionData);
            }
            
            /* Stop A/D conversion */
            DrvADC_StopConvert();

            /* Clear the ADC interrupt flag */
            _DRVADC_CLEAR_ADC_INT_FLAG();
        }
        
    }
    
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: AdcResultMonitorTest                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Description:                                                                                            */
/*   ADC result monitor function test.                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
void AdcResultMonitorTest()
{
    uint8_t u8CmpChannelNum, u8ChannelSelBitwise, u8CmpMatchCount;
    
    printf("\n=== ADC compare function test ===\n");
    /* Set the ADC operation mode as continuous scan mode */
    DrvADC_SetADCOperationMode(ADC_CONTINUOUS_OP);
    
    /* Set the ADC input mode */
    DrvADC_SetADCInputMode(ADC_SINGLE_END);
    
    /* Enable ADC channel 0 */
    u8ChannelSelBitwise = 0x1;
    /* Set the ADC channel */
    DrvADC_SetADCChannel(u8ChannelSelBitwise);
    /* Disable the PA.0 digital input path */
    DrvGPIO_DisableDigitalInputBit(E_GPA, 0);
    /* Configure PA.0 as ADC analog input pin */
    DrvGPIO_InitFunction(E_FUNC_ADC0);
    
    u8CmpChannelNum = 0;
    gu8AdcCmp0IntFlag = 0;
    u8CmpMatchCount = 5;
    /* Enable ADC comparator 0. Compare condition: conversion result < 0x800. */
    DrvADC_EnableADCCmp0(u8CmpChannelNum, LESS_THAN, 0x800, u8CmpMatchCount);
    /* enable ADC comparator 0 interrupt and set the callback function */
    DrvADC_EnableADCCmp0Int(Cmp0IntCallback, 0);
    
    gu8AdcCmp1IntFlag = 0;
    u8CmpMatchCount = 5;
    /* Enable ADC comparator 1. Compare condition: conversion result >= 0x800. */
    DrvADC_EnableADCCmp1(u8CmpChannelNum, GREATER_OR_EQUAL, 0x800,  u8CmpMatchCount);
    /* enable ADC comparator 1 interrupt and set the callback function */
    DrvADC_EnableADCCmp1Int(Cmp1IntCallback, 0);
    
    /* start A/D conversion */
    DrvADC_StartConvert();
    /* Wait ADC compare interrupt */
    while( (gu8AdcCmp0IntFlag==0)&&(gu8AdcCmp1IntFlag==0) );
    
    DrvADC_StopConvert();
    DrvADC_DisableADCCmp0Int();
    DrvADC_DisableADCCmp1Int();
    DrvADC_DisableADCCmp0();
    DrvADC_DisableADCCmp1();
    if(gu8AdcCmp0IntFlag==1)
    {
        printf("The conversion result of channel %d is less than 0x800\n", u8CmpChannelNum);
    }
    else
    {
        printf("The conversion result of channel %d is greater or equal to 0x800\n", u8CmpChannelNum);
    }
    
}

/* ADC interrupt callback function */
void AdcIntCallback(uint32_t u32UserData)
{
    gu8AdcIntFlag = 1;
}

/* ADC interrupt callback function */
void Cmp0IntCallback(uint32_t u32UserData)
{
    gu8AdcCmp0IntFlag = 1;
}

/* ADC interrupt callback function */
void Cmp1IntCallback(uint32_t u32UserData)
{
    gu8AdcCmp1IntFlag = 1;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: SingleEndInput_SingleOpModeChannelSelect                                                      */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   u8ChannelSelBitwise [in]:  Indicate the selected channel.                                             */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   0xFF   -- quit.                                                                                       */
/*   Others -- the active channel #                                                                        */
/*                                                                                                         */
/* Description:                                                                                            */
/*   Select the ADC channel.                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t SingleEndInput_SingleOpModeChannelSelect(uint8_t * u8ChannelSelBitwise)
{
    uint8_t u8Option;
    
    printf("Select ADC channel:\n");
    printf("  [0] Channel 0\n");
    printf("  [1] Channel 1\n");
    printf("  [2] Channel 2\n");
    printf("  [3] Channel 3\n");
    printf("  [4] Channel 4\n");
    printf("  [5] Channel 5\n");
    printf("  [6] Channel 6\n");
    printf("  [7] Channel 7\n");
    printf("  Other keys: exit single mode test\n");
    u8Option = getchar();
   
    if(u8Option=='0')
    {
        *u8ChannelSelBitwise = 1;
        /* Disable the PA.0 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 0);
        /* Configure PA.0 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC0);
    }
    else if(u8Option=='1')
    {
        *u8ChannelSelBitwise = 2;
        /* Disable the PA.1 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 1);
        /* Configure PA.1 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC1);
    }
    else if(u8Option=='2')
    {
        *u8ChannelSelBitwise = 4;
        /* Disable the PA.2 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 2);
        /* Configure PA.2 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC2);
    }
    else if(u8Option=='3')
    {
        *u8ChannelSelBitwise = 8;
        /* Disable the PA.3 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 3);
        /* Configure PA.3 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC3);
    }
    else if(u8Option=='4')
    {
        *u8ChannelSelBitwise = 0x10;
        /* Disable the PA.4 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 4);
        /* Configure PA.4 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC4);
    }
    else if(u8Option=='5')
    {
        *u8ChannelSelBitwise = 0x20;
        /* Disable the PA.5 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 5);
        /* Configure PA.5 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC5);
    }
    else if(u8Option=='6')
    {
        *u8ChannelSelBitwise = 0x40;
        /* Disable the PA.6 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 6);
        /* Configure PA.6 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC6);
    }
    else if(u8Option=='7')
    {
        *u8ChannelSelBitwise = 0x80;
        /* Disable the PA.7 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 7);
        /* Configure PA.7 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC7);
    }
    else
        return 0xFF;
    u8Option = u8Option - '0';
    return u8Option;   /* return the the active channel number */
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DifferentialInput_SingleOpModeChannelSelect                                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   u8ChannelSelBitwise [in]:  Indicate the selected channel.                                             */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   0xFF   -- quit.                                                                                       */
/*   Others -- the active channel #                                                                        */
/*                                                                                                         */
/* Description:                                                                                            */
/*   Select the ADC channels.                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t DifferentialInput_SingleOpModeChannelSelect(uint8_t * u8ChannelSelBitwise)
{
    uint8_t u8Option;
    
    printf("Select ADC channel:\n");
    printf("  [0] Differential input pair 0\n");
    printf("  [1] Differential input pair 1\n");
    printf("  [2] Differential input pair 2\n");
    printf("  [3] Differential input pair 3\n");
    printf("  Other keys: quit\n");
    u8Option = getchar();
    if(u8Option=='0')
    {
        *u8ChannelSelBitwise = 1;
        /* Disable the PA.0 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 0);
        /* Configure PA.0 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC0);
        /* Disable the PA.1 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 1);
        /* Configure PA.1 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC1);
        u8Option = 0;
    }
    else if(u8Option=='1')
    {
        *u8ChannelSelBitwise = 4;
        /* Disable the PA.2 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 2);
        /* Configure PA.2 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC2);
        /* Disable the PA.3 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 3);
        /* Configure PA.3 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC3);
        u8Option = 2;
    }
    else if(u8Option=='2')
    {
        *u8ChannelSelBitwise = 0x10;
        /* Disable the PA.4 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 4);
        /* Configure PA.4 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC4);
        /* Disable the PA.5 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 5);
        /* Configure PA.5 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC5);
        u8Option = 4;
    }
    else if(u8Option=='3')
    {
        *u8ChannelSelBitwise = 0x40;
        /* Disable the PA.6 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 6);
        /* Configure PA.6 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC6);
        /* Disable the PA.7 digital input path */
        DrvGPIO_DisableDigitalInputBit(E_GPA, 7);
        /* Configure PA.7 as ADC analog input pin */
        DrvGPIO_InitFunction(E_FUNC_ADC7);
        u8Option = 6;
    }
    else
        return 0xFF;
    return u8Option;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: SingleEndInput_ScanOpModeChannelSelect                                                        */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   u8ChannelSelBitwise [in]:  Indicate the selected channels.                                            */
/*   pu8ActiveChannel    [out]: Save the channel #.                                                        */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   Number of active channels.                                                                            */
/*                                                                                                         */
/* Description:                                                                                            */
/*   Select the ADC channels.                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t SingleEndInput_ScanOpModeChannelSelect(uint8_t * u8ChannelSelBitwise, uint8_t * pu8ActiveChannel)
{
    uint8_t u8ChannelCount, u8ActiveChannelCount, u8Option;
    E_DRVGPIO_FUNC aeADCPinFunc[]={ E_FUNC_ADC0, E_FUNC_ADC1, E_FUNC_ADC2, E_FUNC_ADC3,
                                    E_FUNC_ADC4, E_FUNC_ADC5, E_FUNC_ADC6, E_FUNC_ADC7 };

    printf("Select ADC channel:\n");
    u8ActiveChannelCount = 0;
    *u8ChannelSelBitwise = 0;
    for(u8ChannelCount=0; u8ChannelCount<8; u8ChannelCount++)
    {
        printf("  Enable channel %d ? (y/n/q)", u8ChannelCount);
        while(1)
        {
            u8Option = getchar();
            if( (u8Option=='y') || (u8Option=='Y') )
            {
                printf("\n");
                *u8ChannelSelBitwise = *u8ChannelSelBitwise | (1<<u8ChannelCount);
                *(pu8ActiveChannel + u8ActiveChannelCount) = u8ChannelCount;
                u8ActiveChannelCount++;
                /* Disable the digital input path */
                DrvGPIO_DisableDigitalInputBit(E_GPA, u8ChannelCount);
                /* Configure the corresponding ADC analog input pin */
                DrvGPIO_InitFunction(aeADCPinFunc[u8ChannelCount]);
                break;
            }
            else if( (u8Option=='n') || (u8Option=='N') )
            {
                printf("\n");
                break;
            }
            else if( (u8Option=='q') || (u8Option=='Q') )
            {
                printf("\n");
                return u8ActiveChannelCount;
            }
        }
    }
    return u8ActiveChannelCount;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DifferentialInput_ScanOpModeChannelSelect                                                     */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   u8ChannelSelBitwise [in]:  Indicate the selected channels.                                            */
/*   pu8ActiveChannel    [out]: Save the channel #.                                                        */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   Number of active channels.                                                                            */
/*                                                                                                         */
/* Description:                                                                                            */
/*   Select the ADC channels.                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t DifferentialInput_ScanOpModeChannelSelect(uint8_t * u8ChannelSelBitwise, uint8_t * pu8ActiveChannel)
{
    uint8_t u8ChannelCount, u8ActiveChannelCount, u8Option;
    E_DRVGPIO_FUNC aeADCPinFunc[]={ E_FUNC_ADC0, E_FUNC_ADC1, E_FUNC_ADC2, E_FUNC_ADC3,
                                    E_FUNC_ADC4, E_FUNC_ADC5, E_FUNC_ADC6, E_FUNC_ADC7 };
    
    printf("Select ADC channel:\n");
    u8ActiveChannelCount = 0;
    *u8ChannelSelBitwise = 0;
    for(u8ChannelCount=0; u8ChannelCount<4; u8ChannelCount++)
    {
        printf("  Enable differential pair %d ? (y/n/q)", u8ChannelCount);
        while(1)
        {
            u8Option = getchar();
            if( (u8Option=='y') || (u8Option=='Y') )
            {
                printf("\n");
                *u8ChannelSelBitwise = *u8ChannelSelBitwise | (1<<(u8ChannelCount*2));
                *(pu8ActiveChannel + u8ActiveChannelCount) = u8ChannelCount;
                u8ActiveChannelCount++;
                /* Disable the digital input path */
                DrvGPIO_DisableDigitalInputBit(E_GPA, u8ChannelCount*2);
                /* Configure the corresponding ADC analog input pin */
                DrvGPIO_InitFunction(aeADCPinFunc[u8ChannelCount*2]);
                /* Disable the digital input path */
                DrvGPIO_DisableDigitalInputBit(E_GPA, u8ChannelCount*2+1);
                /* Configure the corresponding ADC analog input pin */
                DrvGPIO_InitFunction(aeADCPinFunc[u8ChannelCount*2+1]);
                break;
            }
            else if( (u8Option=='n') || (u8Option=='N') )
            {
                printf("\n");
                break;
            }
            else if( (u8Option=='q') || (u8Option=='Q') )
            {
                printf("\n");
                return u8ActiveChannelCount;
            }
        }
    }
    return u8ActiveChannelCount;
}

