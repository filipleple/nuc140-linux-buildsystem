/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
//#ifdef TEST_PS2_MOUSE

#include <stdio.h>
#include "NUC1xx.h"
#include "UART.h"
#include "GPIO.h"
#include "SYS.h"
#include "PS2.h"

#define DBG_PRINTF(...)     printf(__VA_ARGS__)

/*---------------------------------------------------------------------------------------------------------
		7		   6         5        4       3         2        1       0                                            
	---------------------------------------------------------------------------
	|Yoverflow|Xoverflow|Y sign  |X sign  |    1    | Middle |  Right | Left   |
	---------------------------------------------------------------------------
	|                             X Movement                                   |
	---------------------------------------------------------------------------
	|                             Y Movement                                   |
	---------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


#define DEVICE_ID                   0x00

#define PS2CMD_RESET                0xFF
#define PS2CMD_RESEND               0xFE
#define PS2CMD_SET_DEFAULTS         0xF6
#define PS2CMD_DISABLE_DATA_REPORT  0xF5
#define PS2CMD_ENABLE_DATA_REPORT   0xF4
#define PS2CMD_SET_SAMPLE_RATE      0xF3
#define PS2CMD_GET_DEVICE_ID        0xF2
#define PS2CMD_SET_REMOTE_MODE      0xF0
#define PS2CMD_SET_WRAP_MODE        0xEE//(exit wrap mode using 0xEC or 0xFF)
#define PS2CMD_RESET_WRAP_MODE      0xEC	
#define PS2CMD_READ_DATA            0xEB
#define PS2CMD_SET_STREAM_MODE      0xEA
#define PS2CMD_STATUS_REQUEST       0xE9
#define PS2CMD_SET_RESOLUTION       0xE8
#define PS2CMD_SET_SCALLING2        0xE7
#define PS2CMD_SET_SCALLING1        0xE6

#define PS2ERROR        0xFC
#define PS2ACK          0xFA
#define PS2PASS         0xAA

#define PS2MOD_RESET    0x0
#define PS2MOD_STREAM   0x1
#define PS2MOD_REMOTE   0x2
#define PS2MOD_WRAP     0x3

typedef struct
{
    __IO uint8_t Right_btn:1;
    __IO uint8_t Middle_btn:1;
    __IO uint8_t Left_btn:1;
    __IO uint8_t Always:1;
    __IO uint8_t Scaling:1;
    __IO uint8_t Enable:1;
    __IO uint8_t Mode:1;
    __IO uint8_t Reserve:1;
} PS2_MOUSE_STATUS;

uint8_t g_opMode = PS2MOD_RESET, g_opModeBak;
uint8_t g_sampleRate = 0;
uint8_t g_resolution = 0;
uint8_t g_scalling = 0;
uint8_t g_dataReportEnable = 0;
uint32_t g_mouseData = 0;
uint8_t g_cmd[2] = {0}, g_mouseX, g_mouseY;
PS2_MOUSE_STATUS g_mouseStatus;


uint32_t g_cnt = 0;

/*----------------------------------------------------------------------------
  PS2 mouse ack
  Parameter:
  	code: ack code
  	len: the len of code
 *----------------------------------------------------------------------------*/
static uint32_t Ps2Ack(uint32_t code, uint8_t len)
{
	//while(PS2->STATUS.TXEMPTY == 0);

	if(PS2->STATUS.TXEMPTY)
	{
		DBG_PRINTF("Device->Host: ACK\n");
		_DRVPS2_TXDATA(code, len);
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------
  reset variable to default value base on PS2 mouse data sheet
  Parameter:
  	None
 *----------------------------------------------------------------------------*/
static void Ps2Reset()
{
	g_sampleRate = 100;
	g_resolution = 4;
	g_scalling = 1;
	g_dataReportEnable = 0;
}

/*----------------------------------------------------------------------------
  Init status byte for command of PS2CMD_STATUS_REQUEST
  Parameter:
  	None
 *----------------------------------------------------------------------------*/
static void Ps2InitStatus()
{
	outpb(&g_mouseStatus, 0);
	g_mouseStatus.Right_btn = 0;
	g_mouseStatus.Middle_btn = 0;
	g_mouseStatus.Left_btn = 0;
	if(g_scalling == 2)
		g_mouseStatus.Scaling = 1;
	if(g_dataReportEnable)
		g_mouseStatus.Enable = 1;
	if(g_opMode == PS2MOD_REMOTE)
		g_mouseStatus.Mode = 1;
}

/*----------------------------------------------------------------------------
  the ISR for SysTick
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void)
{

    if(g_opMode == PS2MOD_STREAM && g_dataReportEnable)
    {
        if(PS2->STATUS.TXEMPTY)
        {
            if(g_cnt < 100)
            {
            	g_mouseX = 0x01;
            	g_mouseY = 0x00;
                g_mouseData =  (g_mouseY << 16) | (g_mouseX << 8) | 0x08; /* move right */
            }
            else if(g_cnt < 200)
            {
            	g_mouseX = 0x00;
            	g_mouseY = 0xFF;
                g_mouseData = (g_mouseY << 16) | (g_mouseX << 8) | 0x28; /* move up */
            }
            else if(g_cnt < 300)
            {
             	g_mouseX = 0xFF;
            	g_mouseY = 0x00;
                g_mouseData =  (g_mouseY << 16) | (g_mouseX << 8) | 0x18; /* move left */
            }
            else if(g_cnt < 400)
            {
            	g_mouseX = 0x00;
            	g_mouseY = 0x01;
                g_mouseData =  (g_mouseY << 16) | (g_mouseX << 8) | 0x08; /* move down */
            }
            else if(g_cnt < 500)
                g_cnt = 0;
           
            g_cnt++;

			
			/* PS2 mouse a packet is 3 bytes */
            _DRVPS2_TXDATA(g_mouseData, 3);
            
            if((g_cnt & 0x0F) == 0)
            DBG_PRINTF("Device->Host: Data report 0x%06x\n", g_mouseData);
        }
    }
}

/*----------------------------------------------------------------------------
  PS2 interrupt callback function
  Parameter:
  	u32IntStatus: interrupt status
 *----------------------------------------------------------------------------*/
static void PS2Mouse_IRQHandler(uint32_t u32IntStatus)
{
	uint32_t reg;
	uint32_t rxData;


	reg = u32IntStatus;

	/* RXINT */
	if(reg & DRVPS2_RXINT)
	{
		_DRVPS2_INTCLR(DRVPS2_RXINT);
		//rxData = PS2->RXDATA;
		rxData = _DRVPS2_RXDATA();
		DBG_PRINTF("rxData=%x\n", rxData);
        
        if((g_opMode == PS2MOD_WRAP) &&
        	(rxData != PS2CMD_RESET)&&(rxData != PS2CMD_RESET_WRAP_MODE))
        {
        		Ps2Ack(rxData, 1);
        }
        else
        {
			if(g_cmd[0])
			{
				/* If g_cmd[0] is not 0, it should be in data phase */
				if(g_cmd[0] == PS2CMD_SET_SAMPLE_RATE)
				{
					DBG_PRINTF("Host->Device: Set sample rate data %d\n", rxData);
					
					//10 20 40 60 80 100 and 200
					if(rxData < 10)
        	            rxData = 10;
        	        if(rxData > 200)
        	            rxData = 200;
        	        g_sampleRate = rxData;
        	        g_cmd[0] = 0;
        	        g_mouseX = 0;
        	        g_mouseY = 0;
					
					/*ACK*/
        	        if(Ps2Ack(PS2ACK, 1) == FALSE)
        	        {   
        	            DBG_PRINTF("Something wrong!! Stop code!\n");
        	            _DRVPS2_SWOVERRIDE(1, 0);//inhibit communication
        	            while(1);
        	        }
        	
        	    }
        	    else if(g_cmd[0] == PS2CMD_SET_RESOLUTION)
        	    {
        	        DBG_PRINTF("Host->Device: Set resolution data %d\n", rxData);
        	
        	        if(rxData < 1)
        	            rxData = 1;
        	        if(rxData > 3)
        	            rxData = 3;
        	        g_resolution = (1 << rxData);
        	        g_cmd[0] = 0;
					
					/*ACK*/
        	        if(Ps2Ack(PS2ACK, 1) == FALSE)
        	        {   
        	            _DRVPS2_SWOVERRIDE(1, 0);//inhibit communication
        	            DBG_PRINTF("Something Wrong!! Stop code!\n");
        	            while(1);
        	        }
        	    }
			}
			else
        	{
				/* Only support PS2CMD_DISABLE_DATA_REPORT command when data report enabled */
				if((rxData == PS2CMD_DISABLE_DATA_REPORT) || (g_dataReportEnable == 0))
				{    
        	        /* Process the command phase */
        	        if(rxData == PS2CMD_RESET)
        	        {
        	            DBG_PRINTF("Host->Device: Reset\n");
        	
        	            /* Reset command */
        	            g_opMode = PS2MOD_RESET;
        	            g_cmd[0] = 0;
        	
        	           /* Clear FIFO */
        	            _DRVPS2_CLRFIFO();
        	            /*ACK*/
						Ps2Ack(PS2ACK, 1);
        	        }
        	        else if(rxData == PS2CMD_RESEND)
        	        {
        	        	g_mouseData = (g_mouseY << 16) | (g_mouseX << 8) | 0x08; /* move right */
						/*ACK*/
						Ps2Ack((g_mouseData<<8)|PS2ACK, 4);
        	        	
        	     	}	
        	        else if(rxData == PS2CMD_SET_SAMPLE_RATE)
        	        {
        	            DBG_PRINTF("Host->Device: Set sample rate\n");
        	
        	            /* Set sample rate */
        	            g_cmd[0] = PS2CMD_SET_SAMPLE_RATE;
        	
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
        	        }
        	        else if(rxData == PS2CMD_GET_DEVICE_ID)
        	        {
        	            DBG_PRINTF("Host->Device: Get device ID\n");
        	
						g_cmd[0] = 0;
						g_mouseX = 0;
						g_mouseY = 0;
						
						DBG_PRINTF("Device->Host: ACK + Device ID(0x%x)\n", DEVICE_ID);
						/*ACK*/
						Ps2Ack((DEVICE_ID << 8) | PS2ACK, 2);
					}
					else if(rxData == PS2CMD_SET_SCALLING2)
					{
        	            DBG_PRINTF("Host->Device: Set scalling 2\n");
        	            g_scalling = 2;//2:1
        	            g_cmd[0] = 0;
        	            /*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_SET_SCALLING1)
					{
        	            DBG_PRINTF("Host->Device: Set scalling 1\n");
        	            g_scalling = 1;//1:1
        	            g_cmd[0] = 0;
        	            /*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_ENABLE_DATA_REPORT)
					{
        	            DBG_PRINTF("Host->Device: Enable data report\n");
        	            
        	            g_cmd[0] = 0;
						if((g_opMode == PS2MOD_STREAM) || (g_opMode == PS2MOD_REMOTE))
						{
							g_mouseX = 0;
							g_mouseY = 0;
						}
						
        	             /* Set the timer for g_sampleRate */
        	            /* The sample rate could be 10 ~ 200 samples/sec */
        	            if(g_opMode == PS2MOD_STREAM)
        	            {
        	            	g_dataReportEnable = 1;
        	            	SysTick_Config(SystemCoreClock / g_sampleRate);
        	            }
        	
        	            /*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_DISABLE_DATA_REPORT)
					{
        	            DBG_PRINTF("Host->Device: Disable data report\n");
        	            g_dataReportEnable = 0;
        	            g_cmd[0] = 0;
        	
        	            SysTick->CTRL = 0;//disable SysTick interrupt
        	            /*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_SET_RESOLUTION)
					{
						DBG_PRINTF("Host->Device: Set resolution\n");
						g_cmd[0] = PS2CMD_SET_RESOLUTION;
        	
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
        	    	else if(rxData == PS2CMD_SET_DEFAULTS)
					{
						DBG_PRINTF("Host->Device: Set default\n");
						g_cmd[0] = 0;
						/* Reset to default configuration */
						Ps2Reset();
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_SET_REMOTE_MODE)
					{
						DBG_PRINTF("Host->Device: Set remote\n");
						g_cmd[0] = 0;
						g_opMode = PS2MOD_REMOTE;
						SysTick->CTRL = 0;
						g_mouseX = 0;
						g_mouseY = 0;
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_SET_WRAP_MODE)
					{
						DBG_PRINTF("Host->Device: Set wrap\n");
						g_cmd[0] = 0;
						g_opModeBak = g_opMode;
						g_opMode = PS2MOD_WRAP;
						g_mouseX = 0;
						g_mouseY = 0;
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_RESET_WRAP_MODE)
					{
						DBG_PRINTF("Host->Device: Set reset wrap\n");
						g_cmd[0] = 0;
						if(g_opMode == PS2MOD_WRAP)
							g_opMode = g_opModeBak;
						g_mouseX = 0;
						g_mouseY = 0;
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_READ_DATA)
					{
						DBG_PRINTF("Host->Device: Read data\n");
						g_cmd[0] = 0;
						
						g_mouseData = (g_mouseY << 16) | (g_mouseX << 8) | 0x08; /* move right */
						g_mouseX = 0;
						g_mouseY = 0;
						/*ACK*/
						Ps2Ack((g_mouseData<<8)|PS2ACK, 4);
					}
					else if(rxData == PS2CMD_SET_STREAM_MODE)
					{
						DBG_PRINTF("Host->Device: set stream mode\n");
						g_cmd[0] = 0;
						g_opMode = PS2MOD_STREAM;
						
						g_mouseX = 0;
						g_mouseY = 0;
						/*ACK*/
						Ps2Ack(PS2ACK, 1);
					}
					else if(rxData == PS2CMD_STATUS_REQUEST)
					{
						DBG_PRINTF("Host->Device: get status\n");
						g_cmd[0] = 0;
						
						g_mouseX = 0;
						g_mouseY = 0;
						/*ACK*/
						Ps2InitStatus();
						g_mouseData = (g_sampleRate << 16) | (g_resolution << 8) | inpb(&g_mouseStatus);
						Ps2Ack((g_mouseData<<8)|PS2ACK, 4);
					}
				}
			}
		}
	}

    /* TXINT */
    if(reg & DRVPS2_TXINT)
    {
        while(DrvPS2_GetIntStatus(DRVPS2_TXINT))
            _DRVPS2_INTCLR(DRVPS2_TXINT);
    }

}

/*----------------------------------------------------------------------------
  PS2 init
  Parameter:
  	None
 *----------------------------------------------------------------------------*/
static void Ps2Init(void)
{
	DrvPS2_Open();
	DrvPS2_EnableInt(DRVPS2_TXINT|DRVPS2_RXINT, PS2Mouse_IRQHandler);
}



/*----------------------------------------------------------------------------
  PS2 mouse main routine (PS2Mouse)
  Parameter:
  	None
 *----------------------------------------------------------------------------*/
void Ps2Mouse(void)
{
    Ps2Init();
    g_cmd[0] = 0;
       
    while(1)
    {
        if(g_opMode == PS2MOD_RESET)
        {
        	printf("Reset\n");
            /* Reset to default configuration */
            Ps2Reset();

            /* Do self test ... */
            
            /* One byte per trnasfer */
            //_DRVPS2_TXFIFO(0);

            /* Report 0xAA for successful self-test or 0xFC for error */
            /* Waiting for transmit */
            while(_DRVPS2_ISTXEMPTY() == 0);
            //PS2->TXDATA[0] = PS2PASS;
            _DRVPS2_TXDATA(PS2PASS, 1);

            /* Report device ID */
            while(_DRVPS2_ISTXEMPTY() == 0);
            _DRVPS2_TXDATA(DEVICE_ID, 1);
            //PS2->TXDATA[0] = DEVICE_ID;
            
            /* Enter Stream mode */
            g_opMode = PS2MOD_STREAM;

        
        }
        else if(g_opMode == PS2MOD_STREAM)
        {
        }
        else if(g_opMode == PS2MOD_REMOTE)
        {
        }
        else if(g_opMode == PS2MOD_WRAP)
        {
        
        
        
        }
    
    }

}

void UartInit(void)
{
	STR_UART_T sParam;

    /* Set UART Pin */
	  DrvGPIO_InitFunction(E_FUNC_UART0);
	
	  /* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

   	/* Select UART Clock Source From 12Mhz */
	  DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 
 
    DrvUART_Open(UART_PORT0,&sParam);
}


#if 0
volatile uint32_t msTicks; 
/*------------------------------------------------------------------------------
  delays number of tick Systicks (happens every 1 ms)
 *------------------------------------------------------------------------------*/
void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}
#else
void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}
#endif


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main()
{
	  UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    /* Waiting for 12M Xtal stalble */
    DrvSYS_Delay(5000);

	  SYSCLK->CLKSEL0.HCLK_S = 0; //using 12M as HCLK src
    DrvSYS_Delay(100);

    /* Core clock is changed, update it */
    SystemCoreClockUpdate();
    
    UartInit();

    printf("\n\n");
    printf("+--------------------------------------------------------------------+\n");
    printf("|                      PS2 Driver Sample Code (Mouse)                |\n");
    printf("|                                                                    |\n");
    printf("+--------------------------------------------------------------------+\n");
    printf("  This sample code is used to simulate a PS2 mouse. The pointer will \n");
    printf("  move around automatically.\n");
    printf("  I/O configuration:\n");
    printf("    PS2DAT <--> PS2 DATA of PC\n");
    printf("    PS2CLK <--> PS2 CLK of PC\n");
    printf("    Target baord GND <--> PS2 GND of PC\n");
    printf("After connect PS2DAT and PS2CLK to PC, reboot PC\n");   
    printf("\n  >>> Start PS2 Mouse <<<\n"); 

    Ps2Mouse();
}

//#endif


