/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>

/*---------------------------------------------------------------------------------------------------------*/
/*  Include hear file                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "TIMER.h"

void ExternalTimerCounterFunc(void);
void ExternalTimerCaptureFunc(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Sample Code Menu                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
static void TestItem (void)
{
    printf("\n\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|               Timer Sample Program                        |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| Timer Callback                                      - [1] |\n");
    printf("| Timer Get Tick                                      - [2] |\n");
    printf("| Timer Delay                                         - [3] |\n");
    printf("| Timer 0/1/2/3 Frequency test                        - [4] |\n");
    printf("| WDT Callback                                        - [5] |\n");
    printf("| Power Down and WDT wakeup test                      - [6] |\n");
    printf("| Timer External Counter Mode test                    - [7] |\n");
    printf("|    Only supported on NUC1x0xxxBx and NUC1x0xxxCx series.  |\n");
    printf("|    Ex:NUC140RD2BN and NUC140VE3CN                         |\n");
    printf("| Timer External Capture Mode test                    - [8] |\n");
    printf("|    Only supported on NUC1x0xxxCx series, ex:NUC140VE3CN.  |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| Quit                                              - [ESC] |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("Please select key (1 ~ 8): ");
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Callback funtion                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t uTimerEventNo[4];;
volatile uint32_t u32Timer0Cnt=0, u32Timer1Cnt=0, u32Timer2Cnt=0, u32Timer3Cnt=0;
void TMR0_Callback(uint32_t u32Param)
{
    if (u32Param == 1)
    {
        printf(" Enter Timer callback #%dth, Current INT ticks is %d\n", ++u32Timer0Cnt, DrvTIMER_GetIntTicks(E_TMR0));
    }else
    {
        ++u32Timer0Cnt;
    }
}

void TMR1_Callback(uint32_t u32Param)
{
    u32Timer1Cnt++;
}

void TMR2_Callback(uint32_t u32Param)
{
    if (u32Param == 2)
    {
        if (GPC_0 == 1)
            GPC_0 = 0;
        else
            GPC_0 = 1;
    }else
    {
        u32Timer2Cnt++;
    }
}

void TMR3_Callback(uint32_t u32Param)
{
    if (u32Param == 2)
    {
        if (GPC_1 == 1)
            GPC_1 = 0;
        else
            GPC_1 = 1;
    }else
    {
        u32Timer3Cnt++;
    }
}

volatile uint8_t b8WDTINT = FALSE;
void WDT_Callback(uint32_t uWakeupFlag)
{    
    b8WDTINT = TRUE;

    DrvWDT_Ioctl(E_WDT_IOC_RESET_TIMER, 0);
    DrvWDT_Close();

    printf("  WDT interrupt !!!\n");    
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
/*  MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main (void)
{
    int32_t tick;
    int8_t item;
    
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

    DrvTIMER_Init();
    printf("\n\nTIMER Sample Code \n");

    do
    {
        /* Set all the default TIMER clock sources are from 12MHz */
        DrvSYS_SelectIPClockSource(E_SYS_TMR0_CLKSRC, 0);
        DrvSYS_SelectIPClockSource(E_SYS_TMR1_CLKSRC, 0);
        DrvSYS_SelectIPClockSource(E_SYS_TMR2_CLKSRC, 0);
        DrvSYS_SelectIPClockSource(E_SYS_TMR3_CLKSRC, 0);
        
        TestItem();
        item = getchar();
        printf("%c\n",item);

        switch (item)
        {
            case '1':    
            {   
                printf("\n[TIMER Callback Function Test]\n");
                printf(" This sample code will set TIMER0 2 ticks/sec by periodic mode \n");
                printf(" and install a callback function which is happend when timer \n");
                printf(" interrupt occurred 2 times \n\n");
 
                /* Using TIMER0 in PERIODIC_MODE, 2 ticks /sec */
                DrvTIMER_Open(E_TMR0, 2, E_PERIODIC_MODE);
                
                /* Install callback "TMR0_Callback" and trigger callback when Interrupt happen twice */
                uTimerEventNo[0] = DrvTIMER_SetTimerEvent(E_TMR0, 2, (TIMER_CALLBACK)TMR0_Callback, 1);
                
                /* Enable TIMER0 Intettupt */
                DrvTIMER_EnableInt(E_TMR0); 
                
                printf(" Waiting for 10 times timer callbacks to exit test ...\n");
    
                /* Start counting */
                u32Timer0Cnt = 0;
                DrvTIMER_Start(E_TMR0);
                
                while (u32Timer0Cnt < 10);

                /* Clear the Timer event */
                DrvTIMER_ClearTimerEvent(E_TMR0, uTimerEventNo[0]);

                /* Close TIMER0 */
                DrvTIMER_Close(E_TMR0);
                
                printf("Case 1 Finished \n");
                break;
            }

            case '2':    
            {   
                printf("[TIMER Get Tick Test]\n");
                printf(" This sample code will use TIMER1 1 tick/sec by periodic mode\n");
                printf(" It will exit the funtion when current INT tick is larger 10 times\n\n");
                
                DrvTIMER_Open(E_TMR1, 1, E_PERIODIC_MODE);
                DrvTIMER_EnableInt(E_TMR1);
                DrvTIMER_Start(E_TMR1);
                tick = 0;
                do
                {
                    if (tick != DrvTIMER_GetIntTicks(E_TMR1))    /* print when 10 multiple  */
                    {
                        tick = DrvTIMER_GetIntTicks(E_TMR1);
                        printf ("Currnet INT tick = %d \n", tick);
                        if (tick > 10) break;
                    }

                } while (1);
                DrvTIMER_ResetIntTicks(E_TMR1);
                DrvTIMER_Close(E_TMR1);
                printf("Case 2 Finished \n");
                break;
            }

            case '3':    
            {   
                printf("[TIMER Delay Test]\n");
                printf(" This sample code will use TIMER0 to create a delay time\n\n");
                
                printf(" Delay for 3 seconds ...\n");
                DrvTIMER_Open(E_TMR0, 1000, E_PERIODIC_MODE);
                DrvTIMER_EnableInt(E_TMR0);
                DrvTIMER_Start(E_TMR0);   /* Start counting */
                DrvTIMER_Delay(E_TMR0, 3000);
                DrvTIMER_Close(E_TMR0);
                printf("Case 3 Finished \n");
                break;
            }
            
            case '4':    
            {   
                uint32_t i;
                uint32_t begin0 = 0;
                
                printf("[TIMER Frequency Test]\n");
                printf(" This sample code will use four timer and set four different settings as below\n");
                printf(" TIMER0: 1 ticks/sec\n");
                printf(" TIMER1: 2 ticks/sec\n");
                printf(" TIMER2: 3 ticks/sec\n");
                printf(" TIMER3: 4 ticks/sec\n");
                printf(" Check their multiple relation \n\n");

                /* Using TIMER0 in PERIODIC_MODE, 1 ticks /sec */
                DrvTIMER_Open(E_TMR0, 1, E_PERIODIC_MODE);  
                DrvTIMER_Open(E_TMR1, 2, E_PERIODIC_MODE);  
                DrvTIMER_Open(E_TMR2, 3, E_PERIODIC_MODE);  
                DrvTIMER_Open(E_TMR3, 4, E_PERIODIC_MODE);  
                
                /* Install callback "TMR0_Callback" and trigger callback when Interrupt happen twice */
                uTimerEventNo[0] = DrvTIMER_SetTimerEvent(E_TMR0, 1, (TIMER_CALLBACK)TMR0_Callback, 0);
                uTimerEventNo[1] = DrvTIMER_SetTimerEvent(E_TMR1, 1, (TIMER_CALLBACK)TMR1_Callback, 0);
                uTimerEventNo[2] = DrvTIMER_SetTimerEvent(E_TMR2, 1, (TIMER_CALLBACK)TMR2_Callback, 0);
                uTimerEventNo[3] = DrvTIMER_SetTimerEvent(E_TMR3, 1, (TIMER_CALLBACK)TMR3_Callback, 0);
                
                /* Enable TIMER0 Intettupt */
                DrvTIMER_EnableInt(E_TMR0); 
                DrvTIMER_EnableInt(E_TMR1); 
                DrvTIMER_EnableInt(E_TMR2); 
                DrvTIMER_EnableInt(E_TMR3); 

                /* Start counting */
                u32Timer0Cnt = u32Timer1Cnt = u32Timer2Cnt = u32Timer3Cnt = 0;
                DrvTIMER_Start(E_TMR0);
                DrvTIMER_Start(E_TMR1);
                DrvTIMER_Start(E_TMR2);
                DrvTIMER_Start(E_TMR3);
                
                for (i=0; i<10; i++)
                {
                    begin0 = u32Timer0Cnt;
                    printf("T0_Tick: %02d, \tT1_Tick: %02d, \tT2_Tick: %02d, \tT3_Tick: %02d  \n",\
                            u32Timer0Cnt, u32Timer1Cnt, u32Timer2Cnt, u32Timer3Cnt);
                    while (begin0 == u32Timer0Cnt);
                }
                
                if ((u32Timer0Cnt*2 == u32Timer1Cnt) && (u32Timer0Cnt*3 == u32Timer2Cnt) && (u32Timer0Cnt*4 == u32Timer3Cnt))
                    printf("  >> Frequency OK.\n");
                else
                    printf("  >> Frequency Error.\n");

                /* Clear the Timer event */
                DrvTIMER_ClearTimerEvent(E_TMR0, uTimerEventNo[0]);
                DrvTIMER_ClearTimerEvent(E_TMR1, uTimerEventNo[1]);
                DrvTIMER_ClearTimerEvent(E_TMR2, uTimerEventNo[2]);
                DrvTIMER_ClearTimerEvent(E_TMR3, uTimerEventNo[3]);

                /* Close TIMER0 */
                DrvTIMER_Close(E_TMR0);
                DrvTIMER_Close(E_TMR1);
                DrvTIMER_Close(E_TMR2);
                DrvTIMER_Close(E_TMR3);
                
                printf("Case 4 Finished \n");
                break;
            }

            case '5':    
            { 
                printf("\n[WDT Callback Function Test]\n");
                printf(" The sample code will set WDT interval is LEVEL6 and WDT clock source is 10K.\n");
                printf(" Wait about 6 seconds to exit WDT test function.\n\n");

                /* Enable 10KHz clock for WDT */
                DrvSYS_SetOscCtrl(E_SYS_OSC10K, ENABLE);   
                             
                /* Waiting for 10KHz stable */
                DrvSYS_Delay(200000);

                DrvSYS_SelectIPClockSource(E_SYS_WDT_CLKSRC, 3); 

                b8WDTINT = FALSE;
                DrvWDT_Open(E_WDT_LEVEL6);
                DrvWDT_InstallISR((WDT_CALLBACK)WDT_Callback);
                DrvWDT_Ioctl(E_WDT_IOC_ENABLE_INT, 0);
                DrvWDT_Ioctl(E_WDT_IOC_START_TIMER, 0);
                printf("Wait WDT interrupt happened ...\n"); 
                while (1)
                {
                    if ( b8WDTINT )
                        break;
                }

                printf("Case 5 Finished.\n");
                break;
            }

            case '6':    
            { 
                printf("\n[Power Down and WDT wakeup Test]\n");
                printf(" This sample code will test the WDT wakeup function. \n");
                printf(" System will enter in Power Down mode around 6s then wakeup by WDT time-out.\n\n");

                printf("Press any key to enter in Power Down mode... \n");
                getchar();

                /* Enable 10KHz clock for WDT */
                DrvSYS_SetOscCtrl(E_SYS_OSC10K, ENABLE);   
                             
                /* Waiting for 10KHz stable */
                DrvSYS_Delay(200000);

                DrvSYS_SelectIPClockSource(E_SYS_WDT_CLKSRC, 3); 

                b8WDTINT = FALSE;
                DrvWDT_Open(E_WDT_LEVEL6);
                DrvWDT_InstallISR((WDT_CALLBACK)WDT_Callback);
                DrvWDT_Ioctl(E_WDT_IOC_ENABLE_INT, 0);
                DrvWDT_Ioctl(E_WDT_IOC_ENABLE_WAKEUP, 0);            
                DrvWDT_Ioctl(E_WDT_IOC_START_TIMER, 0);

                UNLOCKREG();
                SCB->SCR = 4;
                DrvSYS_EnterPowerDown(E_SYS_WAIT_FOR_CPU);   
                /* Enter in Power Down mode and wait for interrupt to levae Power Down mode */
                __WFI();

                while (1)
                {
                    if ( b8WDTINT )
                        break;
                }
                printf("Case 6 Finished. \n");
                break;
            }
           
            case '7':    
            { 
                printf("\n[TIMER Counter mode Test]\n");
                printf(" This sample code will use the TIMER0/1/2/3 pins as external counter input source and \n");
                printf(" run in One Shot Mode, Periodic Mode and Continuous Mode.\n\n");

                ExternalTimerCounterFunc();

                printf("\n"); 
                printf("Case 7 Finished \n"); 
                break;
            }
           
            case '8':    
            { 
				printf("\n[TIMER Capture Mode Test]\n");
                printf(" This sample code will use the TIMER0 external capture function to \n");
                printf(" get the TIMER0 counter value.\n\n");

                ExternalTimerCaptureFunc();

				printf("Case 8 Finished.\n");                
                break;
            }
           
            default:
            {
                break;
            }    
        }
    } while (item != 27);
  
    printf("\nExit TIMER Sample Code ...\n");

    return 0;
}

volatile uint32_t u32CounterINT = 0;
void TMR_CounterCallback(void)
{
    u32CounterINT++;
}

void CounterGenerator(uint32_t pin, uint32_t u32Counters)
{
    if (u32Counters <= 0)
    {
        return ;
    }

    while (u32Counters--)
    {
        DrvGPIO_SetBit(E_GPC, pin);
        DrvGPIO_ClrBit(E_GPC, pin);
    }
}

void ExternalTimerCounterFunc(void)
{
    uint8_t i, j;
    uint8_t u8TimerCH;
    uint32_t u32Boundary;
    uint32_t u32CounterValue[] = {123, 1234, 12345, 123456};

	printf("Use IO pins to create counter source.   \n");
    printf("Please connect GPC.00 to GPB.08(TMR0),  \n");
    printf("               GPC.01 to GPB.09(TMR1),  \n");
    printf("               GPC.02 to GPB.10(TMR2),  \n");
    printf("               GPC.03 to GPB.11(TMR3).  \n");
	printf("Then press any key to start test...     \n");
    getchar();

    DrvGPIO_Open(E_GPC, 0, E_IO_OUTPUT);
    DrvGPIO_Open(E_GPC, 1, E_IO_OUTPUT);
    DrvGPIO_Open(E_GPC, 2, E_IO_OUTPUT);
    DrvGPIO_Open(E_GPC, 3, E_IO_OUTPUT);
    DrvGPIO_ClrBit(E_GPC, 0);
    DrvGPIO_ClrBit(E_GPC, 1);
    DrvGPIO_ClrBit(E_GPC, 2);
    DrvGPIO_ClrBit(E_GPC, 3);

    UNLOCKREG();

	printf("\n>>> One Shot mode <<<\n");
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			u8TimerCH = E_TMR0 + i;
			u32Boundary = u32CounterValue[j];

			DrvTIMER_Init();
			DrvTIMER_OpenCounter((E_TIMER_CHANNEL)u8TimerCH, u32Boundary, E_ONESHOT_MODE);
			uTimerEventNo[0] = DrvTIMER_SetTimerEvent((E_TIMER_CHANNEL)u8TimerCH, 1, (TIMER_CALLBACK)TMR_CounterCallback, 0);
			DrvTIMER_EnableInt((E_TIMER_CHANNEL)u8TimerCH);	
			DrvTIMER_StartCounter((E_TIMER_CHANNEL)u8TimerCH);
	
	 		while (DrvTIMER_GetCounters((E_TIMER_CHANNEL)u8TimerCH) != 0);  	
	
			u32CounterINT = 0;
			CounterGenerator(i, u32Boundary-1);
			CounterGenerator(i, 1);
	
			while(1)
			{
				if ((u32CounterINT == 1) && (DrvTIMER_GetCounters((E_TIMER_CHANNEL)u8TimerCH) == 0))
				{
					printf("Timer[%d] -- Counter = %d\n", u8TimerCH, u32Boundary);
					break;
				}
			}
            DrvTIMER_ClearTimerEvent((E_TIMER_CHANNEL)u8TimerCH, uTimerEventNo[0]);
			DrvTIMER_Close((E_TIMER_CHANNEL)u8TimerCH);
		}
	}

	printf("\n>>> Periodic mode <<<\n");
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			u8TimerCH = E_TMR0 + i;
			u32Boundary = u32CounterValue[3-j];

			DrvTIMER_Init();
			DrvTIMER_OpenCounter((E_TIMER_CHANNEL)u8TimerCH, u32Boundary, E_PERIODIC_MODE);
			uTimerEventNo[0] = DrvTIMER_SetTimerEvent((E_TIMER_CHANNEL)u8TimerCH, 1, (TIMER_CALLBACK)TMR_CounterCallback, 0);
			DrvTIMER_EnableInt((E_TIMER_CHANNEL)u8TimerCH);	
			DrvTIMER_StartCounter((E_TIMER_CHANNEL)u8TimerCH);
	
	 		while (DrvTIMER_GetCounters((E_TIMER_CHANNEL)u8TimerCH) != 0);  	
	
			u32CounterINT = 0;
			CounterGenerator(i, u32Boundary);

			CounterGenerator(i, u32Boundary);

			CounterGenerator(i, u32Boundary-1);
			CounterGenerator(i, 1);
	
			while(1)
			{
				if ((u32CounterINT == 3) && (DrvTIMER_GetCounters((E_TIMER_CHANNEL)u8TimerCH) == 0))
				{
					printf("Timer[%d] -- Counter = %d\n", u8TimerCH, u32Boundary);
					break;
				}
			}
            DrvTIMER_ClearTimerEvent((E_TIMER_CHANNEL)u8TimerCH, uTimerEventNo[0]);
			DrvTIMER_Close((E_TIMER_CHANNEL)u8TimerCH);
		}
	}

    printf("\n>>> Continuous mode <<<\n");
    for (i=0; i<4; i++)
    {
        u8TimerCH = E_TMR0 + i;

        DrvTIMER_Init();
        DrvTIMER_OpenCounter((E_TIMER_CHANNEL)u8TimerCH, u32Boundary, E_CONTINUOUS_MODE);
        DrvTIMER_SetTimerEvent((E_TIMER_CHANNEL)u8TimerCH, 1, (TIMER_CALLBACK)TMR_CounterCallback, 0);
        DrvTIMER_EnableInt((E_TIMER_CHANNEL)u8TimerCH); 
        DrvTIMER_StartCounter((E_TIMER_CHANNEL)u8TimerCH);

        while (DrvTIMER_GetCounters((E_TIMER_CHANNEL)u8TimerCH) != 0);

        u32CounterINT = 0;
        for (j=0; j<4; j++)
        {
            u32Boundary = u32CounterValue[j];
            if (u8TimerCH == E_TMR0)
            {
                TIMER0->TCMPR = u32Boundary;
            }else
            if (u8TimerCH == E_TMR1)
            {
                TIMER1->TCMPR = u32Boundary;
            }else
            if (u8TimerCH == E_TMR2)
            {
                TIMER2->TCMPR = u32Boundary;
            }else
            if (u8TimerCH == E_TMR3)
            {
                TIMER3->TCMPR = u32Boundary;
            }

            CounterGenerator(i, u32Boundary-1);
            CounterGenerator(i, 1);
            while(1)
            {
                if (u32CounterINT == (j+1))
                {
                    printf("Timer[%d] -- Counter = %d\n", u8TimerCH, u32Boundary);
                    break;
                }
            }
        }
        DrvTIMER_ClearTimerEvent((E_TIMER_CHANNEL)u8TimerCH, uTimerEventNo[0]);
        DrvTIMER_Close((E_TIMER_CHANNEL)u8TimerCH);
    }

	LOCKREG();

    printf("\n"); 
}

void InitialTimerCounterMode(void)
{
	uint8_t i;
	uint8_t u8TimerCH;

    DrvTIMER_Init();

    for (i=0; i<4; i++)
    {
		u8TimerCH = E_TMR0 + i;

    	DrvTIMER_OpenCounter((E_TIMER_CHANNEL)u8TimerCH, 0xFFFFFF, E_PERIODIC_MODE);
    	DrvTIMER_DisableInt((E_TIMER_CHANNEL)u8TimerCH);	
        DrvTIMER_DisableCounterDebounce((E_TIMER_CHANNEL)u8TimerCH);

        /* Select counter TX detect phase, rising or falling detect */
        if ((i%2) == 0)
            DrvTIMER_SelectCounterDetectPhase((E_TIMER_CHANNEL)u8TimerCH, E_PHASE_RISING);
        else
            DrvTIMER_SelectCounterDetectPhase((E_TIMER_CHANNEL)u8TimerCH, E_PHASE_FALLING);

        DrvTIMER_StartCounter((E_TIMER_CHANNEL)u8TimerCH);
    }
}

void ExternalTimerCaptureFunc(void)
{
    uint32_t uLoop;
    uint8_t u8Char, u8Mode;

    printf("Select counter source:                          \n");
    printf("    [0]: frequency is 1250 Hz and from GPC.00   \n");
    printf("    [1]: frequency is from external source      \n");
    u8Char = getchar();
    switch (u8Char)
    {
        case '1':    
        {
            u8Mode = 1;
            break; 
        } 

        default:
        {
            u8Mode = 0;
            break; 
        }
    } 

    printf("\n");
    if (u8Mode == 1)
    {
        printf("Connect external source to GPB.08(TMR0) as the TIMER0 counter source.                   \n");
        printf("Connect GPC.01 to GPB.15(T0EX) to create TIMER0 capture source and frequency is 1 Hz.   \n");
        printf("Then you should get the capture value is the same as the external source frequency.     \n");
    }else
    {
        printf("Connect GPC.00 to GPB.08(TMR0) to create TIMER0 counter source and frequency is 1250 Hz.\n");
        printf("Connect GPC.01 to GPB.15(T0EX) to create TIMER0 capture source and frequency is 1 Hz.   \n");
        printf("Then you should get the capture value is 1250/s.                                        \n");
    }
    printf("Press any key to start test... \n");
    getchar(); 

    /* Using TIMER2 in PERIODIC_MODE, 2500 ticks /sec */
    DrvTIMER_Open(E_TMR2, 2500, E_PERIODIC_MODE);
    
    /* Install callback "TMR2_Callback" and trigger callback when Interrupt happen once */
    uTimerEventNo[2] = DrvTIMER_SetTimerEvent(E_TMR2, 1, (TIMER_CALLBACK)TMR2_Callback, 2);
    
    /* Enable TIMER2 Intettupt */
    DrvTIMER_EnableInt(E_TMR2); 
            
    DrvGPIO_Open(E_GPC, 0, E_IO_OUTPUT);

    /* Using TIMER3 in PERIODIC_MODE, 2 ticks /sec */
    DrvTIMER_Open(E_TMR3, 2, E_PERIODIC_MODE);
    
    /* Install callback "TMR3_Callback" and trigger callback when Interrupt happen once */
    uTimerEventNo[3] = DrvTIMER_SetTimerEvent(E_TMR3, 1, (TIMER_CALLBACK)TMR3_Callback, 2);
    
    /* Enable TIMER3 Intettupt */
    DrvTIMER_EnableInt(E_TMR3); 
        
    DrvGPIO_Open(E_GPC, 1, E_IO_OUTPUT);

    u32Timer2Cnt = u32Timer3Cnt = 0;

    /* Trigger counter source */
    DrvTIMER_Start(E_TMR2);

    /* Trigger capture source */
    DrvTIMER_Start(E_TMR3);
   
    /* Open TIMER0 Counter mode */
	DrvTIMER_OpenCounter(E_TMR0, 0xFFFFFF, E_PERIODIC_MODE);
	DrvTIMER_StartCounter(E_TMR0);

    /* Open TIMER0 Capture mode */
    DrvTIMER_OpenCapture(E_TMR0, E_CAPTURE);
     
    DrvTIMER_SelectCaptureEdge(E_TMR0, E_EDGE_RISING);

    /* Enable TIMER0 capture function */   
    DrvTIMER_EnableCapture(E_TMR0);
                                                     
    /* Clear TEX INT flag to receive new CDR */    
    DrvTIMER_ClearCaptureIntFlag(E_TMR0);

    printf("\n");
    uLoop = 0;
    while (1)
    {
        if (DrvTIMER_GetCaptureIntFlag(E_TMR0) == 0x1)
        {
            uLoop++;
            printf("   [%2d]- %8d \n", uLoop, DrvTIMER_GetCaptureData(E_TMR0));
            DrvTIMER_ClearCaptureIntFlag(E_TMR0);

            /* Write new TCMPR will reset the specified TDR value to 0 */
            if ((uLoop%2) == 1)
            {
                TIMER0->TCMPR = 0xFFFFFE;
            }else
            {
                TIMER0->TCMPR = 0xFFFFFF;
            }

            if (uLoop == 10)
            {
                DrvTIMER_CloseCapture(E_TMR0);
                break;
            }
        }
    }    
    
    DrvTIMER_ClearTimerEvent(E_TMR2, uTimerEventNo[3]);
    DrvTIMER_ClearTimerEvent(E_TMR3, uTimerEventNo[3]);

    DrvTIMER_Close(E_TMR0);
    DrvTIMER_Close(E_TMR2);
    DrvTIMER_Close(E_TMR3);

    printf("\n");
}

