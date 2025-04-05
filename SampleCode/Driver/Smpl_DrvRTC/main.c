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
#include "RTC.h"

extern int IsDebugFifoEmpty(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t  g_u32TICK = FALSE;
volatile int32_t   g_bAlarm  = FALSE;

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Tick Callback function                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void RTC_TickCallBackfn(void)
{
    S_DRVRTC_TIME_DATA_T sCurTime;
    
    /* Get the currnet time */
    DrvRTC_Read(DRVRTC_CURRENT_TIME, &sCurTime);        
    
    printf("Current Time: %d/%02d/%02d %02d:%02d:%02d \n",
        sCurTime.u32Year, sCurTime.u32cMonth, sCurTime.u32cDay, 
        sCurTime.u32cHour, sCurTime.u32cMinute, sCurTime.u32cSecond);               
    
    g_u32TICK++;
}

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Alarm Callback function                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
void RTC_AlarmCallBackfn(void)
{   
    printf("Alarm INT !! \n");
    
    g_bAlarm = TRUE; 
}

static void TestItem (void)
{
    printf("\n\n");
    printf("+-----------------------------------------------------------+ \n");
    printf("|                    RTC Sample Program                     | \n");
    printf("+-----------------------------------------------------------+ \n");
    printf("| [1] Time Display Test                                     | \n");
    printf("| [2] Alarm Test                                            | \n");
    printf("| [3] PowerDown and wakeup by RTC alarm Test                | \n");
    printf("+-----------------------------------------------------------+ \n");
    printf("| [0] Quit                                                  | \n");
    printf("+-----------------------------------------------------------+ \n");
    printf("Select key : \n");
}

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

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Test Sample                                                                                         */
/*  1. Time Display Test                                                                                   */   
/*     Use RTC Tick interrupt to display time every one second.                                            */
/*  2. Alarm Test                                                                                          */
/*     Get the current and alarm after 10 seconds                                                          */   
/*  3. PowerDown Test                                                                                      */
/*     System PowerDown and wakeup by RTC alaem after 15 seconds                                           */   
/*---------------------------------------------------------------------------------------------------------*/
int32_t main()                                 
{   
    int32_t bLoop = TRUE;
    uint8_t u8Item;
    S_DRVRTC_TIME_DATA_T sInitTime;
    S_DRVRTC_TIME_DATA_T sCurTime;  
    
    UNLOCKREG();
            
    /* Enable External 12M Xtal */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, ENABLE);

    /* Waiting for 12M Xtal stable */
    DrvSYS_Delay(5000);
                             
    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0);
    
    /* Initial UART debug message function */
    InitUARTDebugPort();       
                             
    DrvSYS_Open(50000000);
     
    /* RTC Initialize */
    if (DrvRTC_Init() != E_SUCCESS)
    {
        printf("RTC Initial Fail !! \n");
        return FALSE;
    }
    
    /* Time Setting */
    sInitTime.u32Year       = 2010;
    sInitTime.u32cMonth     = 12;   
    sInitTime.u32cDay       = 7;
    sInitTime.u32cHour      = 13;
    sInitTime.u32cMinute    = 20;
    sInitTime.u32cSecond    = 0;
    sInitTime.u32cDayOfWeek = DRVRTC_TUESDAY;
    sInitTime.u8cClockDisplay = DRVRTC_CLOCK_24;            
    
    /* Initialization the RTC timer */
    if (DrvRTC_Open(&sInitTime) != E_SUCCESS)
    {
        printf("RTC Open Fail !! \n");
        return FALSE;
    }       
    
    while ( bLoop )
    {
        TestItem();
        u8Item = getchar();
        
        switch(u8Item)
        {
            case '1':
                printf("[1]. RTC Time Display Test. (Exit after 5 seconds)\n");
    
                /* Set Tick setting */
                DrvRTC_SetTickMode(DRVRTC_TICK_1_SEC);
    
                DrvRTC_EnableInt(DRVRTC_TICK_INT, RTC_TickCallBackfn);  
                
                g_u32TICK = 0;
                while(g_u32TICK < 5);
    
                /* Disable RTC Tick Interrupt */
                DrvRTC_DisableInt(DRVRTC_TICK_INT);                         
                break;
    
            case '2':
                printf("[2]. RTC Alarm Test. (Alarm after 10 seconds)\n");
                    
                g_bAlarm = FALSE;   
            
                /* Get the currnet time */
                DrvRTC_Read(DRVRTC_CURRENT_TIME, &sCurTime);
            
                printf("Start Time: %d/%02d/%02d %02d:%02d:%02d \n",
                        sCurTime.u32Year, sCurTime.u32cMonth, sCurTime.u32cDay,
                        sCurTime.u32cHour, sCurTime.u32cMinute, sCurTime.u32cSecond);
                
                /* The alarm time setting */    
                sCurTime.u32cSecond = sCurTime.u32cSecond + 10;     
                
                /* Set the alarm time (Install the call back function and enable the alarm interrupt)*/
                DrvRTC_Write(DRVRTC_ALARM_TIME, &sCurTime);
                        
                /* Enable RTC Alarm Interrupt & Set Alarm call back function*/
                DrvRTC_EnableInt(DRVRTC_ALARM_INT, RTC_AlarmCallBackfn);
                
                while ( !g_bAlarm );
                
                printf("Alarm Time: %d/%02d/%02d %02d:%02d:%02d \n",
                        sCurTime.u32Year, sCurTime.u32cMonth, sCurTime.u32cDay,
                        sCurTime.u32cHour, sCurTime.u32cMinute, sCurTime.u32cSecond);
    
                /* Disable RTC Alarm Interrupt */
                DrvRTC_DisableInt(DRVRTC_ALARM_INT);                                                        
                break;              
    
            case '3':
                printf("[3]. PowerDown and wakeup by RTC alarm Test. (Wakeup after 15 seconds)\n");
                    
                g_bAlarm = FALSE;   
            
                /* Get the currnet time */
                DrvRTC_Read(DRVRTC_CURRENT_TIME, &sCurTime);            
                printf("Start Time: %d/%02d/%02d %02d:%02d:%02d \n",
                        sCurTime.u32Year, sCurTime.u32cMonth, sCurTime.u32cDay,
                        sCurTime.u32cHour, sCurTime.u32cMinute, sCurTime.u32cSecond);
                while (IsDebugFifoEmpty() == 0);                    
                
                /* The alarm time setting */    
                sCurTime.u32cSecond = sCurTime.u32cSecond + 15;     
                
                /* Set the alarm time */
                DrvRTC_Write(DRVRTC_ALARM_TIME, &sCurTime);
                        
                /* Enable RTC Alarm Interrupt & Set Alarm call back function*/
                DrvRTC_EnableInt(DRVRTC_ALARM_INT, RTC_AlarmCallBackfn);
                    
                UNLOCKREG();

                /* Enable deep sleep */ 
                SCB->SCR = 4;

                /* Enable wait for CPU function and enter in Power Down */
                DrvSYS_EnterPowerDown(E_SYS_WAIT_FOR_CPU);

                /* Wait for interrupt and enter in Power Down mode */
                __WFI();
                
                /* Clear Wait CPU bit to 0 */
                SYSCLK->PWRCON.PD_WAIT_CPU = 0;

                DrvSYS_Delay(5000);
                DrvRTC_Read(DRVRTC_CURRENT_TIME, &sCurTime);
                printf("Alarm Time: %d/%02d/%02d %02d:%02d:%02d \n",
                    sCurTime.u32Year, sCurTime.u32cMonth, sCurTime.u32cDay,
                    sCurTime.u32cHour, sCurTime.u32cMinute, sCurTime.u32cSecond);

                /* Disable RTC Alarm Interrupt */
                DrvRTC_DisableInt(DRVRTC_ALARM_INT);
                break;
    
            case '0':
                printf("[0] \n");
                bLoop = FALSE;
                break;
    
            default:
                break;          
        }       
    }
    
    /* Disable RTC Clock */     
    DrvRTC_Close();
    
    printf("RTC sample test pass. \n");
    
    while (1);
}



	 
