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

#define RXBUFSIZE 3072

/*---LIN Check sum mode-------------------*/
#define MODE_CLASSIC    2
#define MODE_ENHANCED   1
		   
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/*---Using in UART Test -------------------*/
volatile uint8_t comRbuf[1024];
volatile uint16_t comRbytes = 0;		/* Available receiving bytes */
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;
volatile int32_t g_bWait 	= TRUE;
uint8_t u8SendData[12] ={0};

/*---Using in RS485 Test -------------------*/
uint8_t u8RecData[RXBUFSIZE]  ={0};
int32_t w_pointer =0;
volatile int32_t r_pointer = 0;
int32_t IsRS485ISR_TX_PORT = FALSE;
int32_t IsRS485ISR_RX_PORT = FALSE;

/*---Using in LIN Test -------------------*/
uint8_t testPattern[] ={0x00,0x55,0xAA,0xFF,0x00,0x55,0xFF,0xAA};

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(uint32_t u32IntStatus);
int32_t DataCompare(uint8_t InBuffer[],uint8_t OutBuffer[],int32_t len);
extern char GetChar(void);


/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(uint32_t u32IntStatus)
{

 	uint8_t bInChar[1]={0xFF};

	if(u32IntStatus & DRVUART_RDAINT)
	{
		printf("\nInput:");
		
		/* Get all the input characters */
		while(UART0->ISR.RDA_IF==1) 
		{
			/* Get the character from UART Buffer */
			DrvUART_Read(UART_PORT0,bInChar,1);
			printf("%c ", bInChar[0]);
	
			/* Check if buffer full */
			if(comRbytes < RXBUFSIZE)
			{
				/* Enqueue the character */
				comRbuf[comRtail] = bInChar[0];
				comRtail = (comRtail == (RXBUFSIZE-1)) ? 0 : (comRtail+1);
				comRbytes++;
			}			
		}
		printf("\nTransmission Test:");
	}
	else if(u32IntStatus & DRVUART_THREINT)
	{   
		   
        uint16_t tmp;
        tmp = comRtail;
		if(comRhead != tmp)
		{
			bInChar[0] = comRbuf[comRhead];
			DrvUART_Write(UART_PORT0,bInChar,1);
			comRhead = (comRhead == (RXBUFSIZE-1)) ? 0 : (comRhead+1);
			comRbytes--;
		}
		if(bInChar[0] == '0')	
		{	
			g_bWait = FALSE;
		}
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/

void LIN_HANDLE()    
{
	int32_t i = 0;
	volatile uint32_t REG = 0;

	if(UART1->ISR.LIN_RX_BREAK_INT == 1)
	{
		UART1->ISR.LIN_RX_BREAK_IF = 1;
		g_bWait = FALSE;
	}

  	if(!g_bWait)
	{
		if(UART1->ISR.RDA_IF==1)
		{
		   	u8RecData[r_pointer++] = UART1->DATA;
		}
		
		if(r_pointer==11)
		{
			printf("  %02x \t",u8RecData[1]);    		/* ID */
			for(i =2;i<10;i++) 
			{
				printf("%02x,",u8RecData[i] );	 		/* Data Bytes */
			}
			printf("  %02x \t",u8RecData[10] );       	/* CheckSum */

			if(DataCompare(u8SendData,u8RecData,10))
			{
				printf("\tOK\n");
				r_pointer=0;
			}
			else
			{
				printf("...Failed\n");
			}
		}
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/* RS485 Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/

void RS485_INT_HANDLE()  
{
	
	volatile char addr;
	volatile char regRX;
	

			
	if((UART1->ISR.RLS_INT ==1) && (UART1->ISR.RDA_INT == 1))  	/* RLS INT & RDA INT */
	{  	
		if((UART1->FSR.RS485_ADD_DETF ==1) && (UART1->FUNSEL.FUN_SEL == FUN_RS485))  /* ADD_IF, RS485 mode */
		{			
			addr = UART1->DATA;
			UART1->FSR.RS485_ADD_DETF =1;		  				/* clear ADD_IF flag */
		}
	}
	else if((UART1->ISR.RDA_INT == 1))/* Rx Ready */	/* Time-out INT */
	{
		regRX = UART1->DATA;

		if(IsRS485ISR_TX_PORT)
			UART1->DATA	= regRX;	
		else
			u8RecData[r_pointer++] = regRX;
	}

	else if((UART1->ISR.TOUT_INT == 1))/* Rx Ready */	/* Time-out INT */
	{
		regRX = UART1->DATA;

		if(IsRS485ISR_TX_PORT)
			UART1->DATA	= regRX;	
		else
			u8RecData[r_pointer++] = regRX;
	}

	else if(UART1->ISR.BUF_ERR_INT == 1)    					/* Buff Error INT */
	{
		printf("\nBuffer Error...\n");
		while(1);	 
	}
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Sub-Function for LIN        				                                                           */
/*---------------------------------------------------------------------------------------------------------*/

/* Compute the checksum byte */
/* Offset :               */
/*    [1] : Compute not include ID  (LIN1.1) */
/*    [2] : Compute n include ID  (LIN2.0)   */

uint32_t cCheckSum(uint8_t DataBuffer[], uint32_t Offset)  
{
 	uint32_t i,CheckSum =0;

		
	for(i=Offset,CheckSum=0;i<=9;i++)
	{
		CheckSum+=DataBuffer[i];
	  	if (CheckSum>=256)
	  		CheckSum-=255;
	}
	return (255-CheckSum);	
}

/* Compute the Parity Bit */
int8_t Parity(int i)
{
   int8_t number = 0 ;
   int8_t ID[6]; 
   int8_t p_Bit[2];
   int8_t mask =0;   

   if(i>=64)
   	  printf("The ID is not match protocol\n");
   for(mask=0;mask<7;mask++)	   
   	  ID[mask] = (i & (1<<mask))>>mask;

   p_Bit[0] = (ID[0] + ID[1] + ID[2] + ID[4])%2;
   p_Bit[1] = (!((ID[1] + ID[3] + ID[4] + ID[5])%2));

   number = i + (p_Bit[0] <<6) + (p_Bit[1]<<7);
   return number;

}

int32_t DataCompare(uint8_t InBuffer[],uint8_t OutBuffer[],int32_t len)
{
	int i=0;
	for(i=0;i<len;i++)
	{
		if(InBuffer[i]!=OutBuffer[i])
		{
			printf("In[%d] = %x , Out[%d] = %d\n",i,InBuffer[i],i,OutBuffer[i]);
			return FALSE;
		}
	}
	return TRUE;
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Only Send the LIN protocol header  (Sync.+ID + parity field)		                                   */
/*---------------------------------------------------------------------------------------------------------*/

void LIN_HeaderSend(E_UART_PORT uart_ch,int32_t id)
{
	w_pointer =0 ;
	
	DrvUART_SetFnLIN(uart_ch,MODE_TX | MODE_RX,13);	/* Set LIN operation mode */
	
	u8SendData[w_pointer++] = 0x55 ; 				/* SYNC */
	
	u8SendData[w_pointer++] = Parity(id);			/* ID */
	
	DrvUART_Write(uart_ch,u8SendData,2);	
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Only Send the LIN protocol response  (Data + CheckSum field)		                                   */
/*---------------------------------------------------------------------------------------------------------*/

void LIN_ResponseSend(E_UART_PORT uart_ch,int32_t checkSumOption,uint8_t Pattern[])
{
	int32_t i;

	for(i=0;i<8;i++)
		u8SendData[w_pointer++] = Pattern[i] ;

	u8SendData[w_pointer++] = cCheckSum(u8SendData,checkSumOption) ;  

	DrvUART_Write(uart_ch,u8SendData+2,9);	
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Sub-Function for LIN END       				                                                           */
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/* UART Function Test 				                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*      It sends the received data to HyperTerminal.													   */
/*---------------------------------------------------------------------------------------------------------*/

void UART_FunctionTest()
{
	printf("\nUART Sample Demo. (Press '0' to exit)\n");

	/* Enable Interrupt and install the call back function */
	DrvUART_EnableInt(UART_PORT0, (DRVUART_RLSINT | DRVUART_THREINT | DRVUART_RDAINT),UART_INT_HANDLE);
	while(g_bWait);	
		
	/* Disable Interrupt */
	DrvUART_DisableInt(UART_PORT0,DRVUART_RLSINT | DRVUART_THREINT | DRVUART_RDAINT);
	g_bWait =TRUE;
	printf("\nUART Sample Demo End.\n");
		
}

/*---------------------------------------------------------------------------------------------------------*/
/*  IRDA Function Test    		    				                                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*      IRDA Looplack test by IRDA mode             													   */
/*---------------------------------------------------------------------------------------------------------*/

void IRDA_FunctionTest()
{
	STR_IRCR_T sIrda;
	uint8_t bInChar[1]={0xFF};

    printf("\n\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|               IRDA Function Test                          |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| 1). Input char by UART0 terminal.                          |\n");
    printf("| 2). UART1 will send a char according to step 1.           |\n");
    printf("| 3). Return step 1. (Press '0' to exit)                     |\n");
    printf("+-----------------------------------------------------------+\n\n");
	GetChar();

	printf("\nIRDA Sample Demo. \n");

	/* Please call DrvUART_Open() to configure UART before using DrvUART_OpenIRDA() */
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART1);
	sIrda.u8cTXSelect = ENABLE;
    sIrda.u8cInvTX    = FALSE;
    sIrda.u8cInvRX    = TRUE;
  	DrvUART_SetFnIRDA(UART_PORT1,&sIrda);

	/* Enable Interrupt and install the call back function */
	do
	{
		bInChar[0] = GetChar();
		printf("   Input: %c , Send %c out\n",bInChar[0],bInChar[0]);
		DrvUART_Write(UART_PORT1,bInChar,1);
	}while(bInChar[0] !='0');
	
	DrvUART_Write(UART_PORT1,bInChar,4);
	printf("\nIrDA Sample Demo End.\n");
		
}

/*---------------------------------------------------------------------------------------------------------*/
/*  LIN Function Test    		    				                                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*      Loopback Test by LIN mode                   													   */
/*---------------------------------------------------------------------------------------------------------*/

void LIN_FunctionTest()
{
	int32_t i=0;
	STR_UART_T sParam;
    
    printf("\n\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|               LIN Function Test                           |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| The program is used to test LIN BUS.                      |\n");
    printf("| It will send ID 0~10 by a fixed pattern                   |\n");
    printf("| Enter any key to start                                    |\n");
    printf("+-----------------------------------------------------------+\n\n");
	GetChar();
	
	printf("\nLIN Sample Demo. \n");
	sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT1,&sParam) != E_SUCCESS)
		printf("UART1 open failed\n");
								

	DrvUART_EnableInt(UART_PORT1, DRVUART_RLSINT|DRVUART_RDAINT|DRVUART_LININT, (PFN_DRVUART_CALLBACK*)LIN_HANDLE); 	
	
	printf("+-----------------------------------------------------------+\n");
	printf("|[ID]   [DATA]                   [CheckSum] [Result]        |\n");
	printf("+-----------------------------------------------------------+\n");
    
	for(i=0x00;i<10;i++)	
	{
		g_bWait =TRUE;
		LIN_HeaderSend(UART_PORT1,i);
		while(g_bWait);
		LIN_ResponseSend(UART_PORT1,MODE_ENHANCED,testPattern);
		DrvSYS_Delay(5000);
	}

	printf("\nLIN Sample Demo End.\n");

	DrvUART_DisableInt(UART_PORT1, DRVUART_RDAINT|DRVUART_LININT); 

	DrvUART_Close(UART_PORT1); 

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Sub-Function in RS485 Function Test    		    				                                       */
/*---------------------------------------------------------------------------------------------------------*/
void RS485Send(uint8_t *BufferPtr, uint32_t Length)
{
  uint32_t i;

  for ( i = 0; i < Length; i++ )
  {
    while ( UART1->FSR.TE_FLAG !=1);

	if (i==0)    /* Address Byte*/ 
	{
		UART1->LCR.EPE = 0;
	  	UART1->LCR.SPE = 1;	
		UART1->LCR.PBE = 1;	
	}
	else	     /* Data Byte*/
	{
 		UART1->LCR.EPE = 1;
	  	UART1->LCR.SPE = 1;	
		UART1->LCR.PBE = 1;	
	}
	UART1->DATA = *BufferPtr;

	BufferPtr++;
  }
  return;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  RS485 Tranmit Test    		    				                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void RS485_TransmitTest()
{
	int32_t i;
	STR_UART_T sParam;
	STR_RS485_T sParam_RS485;

    printf("\n\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|               RS485 Function Test                         |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| 1). Switch RS485 Mode.                                    |\n");
    printf("| 2). Send %4d bytes though RS485 interface                |\n",RXBUFSIZE);
    printf("| 3). Test with Item [4].RS485 receive test                 |\n");
    printf("| 4). Press any key to start.                               |\n");
    printf("+-----------------------------------------------------------+\n\n");
	GetChar();

	/* variables set default value */
	IsRS485ISR_RX_PORT = FALSE;
	IsRS485ISR_TX_PORT = TRUE;
	r_pointer = 0;

	/* Set UART Configuration */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	sParam.u8TimeOut        = 0x7F;
 	DrvUART_Open(UART_PORT1,&sParam);


	/* Set RS485 Configuration */
	sParam_RS485.u8cAddrEnable = ENABLE; 
	sParam_RS485.u8cAddrValue  = 0xC0;	         	/* Address */
	sParam_RS485.u8cModeSelect = MODE_RS485_AAD|MODE_RS485_AUD;
	sParam_RS485.u8cDelayTime  = 0;
	sParam_RS485.u8cRxDisable  = TRUE;
	DrvUART_SetFnRS485(UART_PORT1,&sParam_RS485);

	u8RecData[0] = 0xC0;							/* RS485_SLAVE_ADR; */

	for(i=1;i<RXBUFSIZE;i++)
	{
		u8RecData[i] = i & 0xFF;
	}

	RS485Send((uint8_t *)u8RecData,RXBUFSIZE);
    printf("Transfer Done\n");

}

/*---------------------------------------------------------------------------------------------------------*/
/*  RS485 Receive Test    		    				                                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*        Wait Data and store in u8RecData buffer                                                          */
/*---------------------------------------------------------------------------------------------------------*/
void RS485_ReceiveTest()
{
	int32_t i;
	STR_UART_T sParam;
	STR_RS485_T sParam_RS485;
	
    printf("\n\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|               RS485 Function Test                         |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| 1). Wait %4d data bytes.                                  |\n",RXBUFSIZE);
    printf("| 2). Press any key to start.                               |\n");
    printf("+-----------------------------------------------------------+\n\n");
	GetChar();
 	/* variables set default value */
	IsRS485ISR_RX_PORT = TRUE;
	IsRS485ISR_TX_PORT = FALSE;
	r_pointer = 0;

	/* Set UART Configuration */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_ODD;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	sParam.u8TimeOut        = 0x7F;
 	DrvUART_Open(UART_PORT1,&sParam);


	/* Set RS485 Configuration */
	sParam_RS485.u8cAddrEnable = ENABLE; 
	sParam_RS485.u8cAddrValue  = 0xC0;	        /* Address */
	sParam_RS485.u8cModeSelect = MODE_RS485_AAD|MODE_RS485_AUD;
	sParam_RS485.u8cDelayTime  = 0;
	sParam_RS485.u8cRxDisable  = 0;
	DrvUART_SetFnRS485(UART_PORT1,&sParam_RS485);


	r_pointer = 0;

	/* Check Rx empty, otherwise read Rx */
	printf("Starting to recevice %d bytes data...\n", RXBUFSIZE);
	
	DrvUART_EnableInt(UART_PORT1, DRVUART_RLSINT|DRVUART_RDAINT|DRVUART_TOUTINT|DRVUART_BUFERRINT,
								 (PFN_DRVUART_CALLBACK*)RS485_INT_HANDLE); 	

	while(r_pointer<(RXBUFSIZE-1));

	/* Compare Data */
	for(i=0;i<(RXBUFSIZE-1);i++)
	{
		if(u8RecData[i] != ((i+1)&0xFF) )
		{
			printf("Compare Data Failed\n");
		}
	}
	printf("\n Receive OK & Check OK\n");
	printf(" Any key to end this test \n");
	GetChar();

	IsRS485ISR_RX_PORT = FALSE;
	DrvUART_Close(UART_PORT1);

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Sample Code Test Menu		    				                                                       */
/*---------------------------------------------------------------------------------------------------------*/
static void TestItem (void)
{

    printf("\n\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|               UART Sample Program                         |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| UART function test                                  - [1] |\n");
    printf("| IrDA funtion test                                   - [2] |\n");
    printf("| LIN function test                                   - [3] |\n");
    printf("| RS485 Receive funtion test                          - [4] |\n");
    printf("| RS485 Transmit funtion test                         - [5] |\n");
	printf("+-----------------------------------------------------------+\n");
    printf("| Quit                                        	    - [ESC] |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("Select key : ");

}



/*---------------------------------------------------------------------------------------------------------*/
/* Main Function     				                                                                       */
/*---------------------------------------------------------------------------------------------------------*/

int32_t main()
{
	int8_t item;

	STR_UART_T sParam;

	/* SYSCLK =>12Mhz*/
	UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;
    
 	/* Waiting for 12M Xtal stalble */
    DrvSYS_Delay(5000);

	/* Run 48Mhz */
    DrvSYS_Open(48000000);
   	
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
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS)
	{
		printf("UART0 open failed\n");
		return FALSE;
	}

	/* Set UART Configuration */
 	sParam.u32BaudRate 		= 57600;
    
	if(DrvUART_Open(UART_PORT1,&sParam) != E_SUCCESS)
	{
		printf("UART1 open failed\n");
		return FALSE;
	}

	
    do
    {
        TestItem();
        item = getchar();
        printf("%c\n",item);
        switch(item)
        {
			case '1':
				UART_FunctionTest();	/* UART Function Test */
				break;
				
			case '2':
				IRDA_FunctionTest();	/* IrDA Function Test */
				break;

			case '3':
				LIN_FunctionTest();		/* LIN Function Test */
				break;
				
			case '4':
				RS485_ReceiveTest();	/* RS485 Receive Test (Need two module board to test) */
				break;

			case '5':
				RS485_TransmitTest();   /* RS485 Transmit Test (Need two module board to test) */
				break;

            default:
                break;
				
		}
    }while(item != 27);
}	
























































































































