/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "TIMER.h"
#include "UART.h"
#include "CAN.h"

/*----------------------------------------------------------------------------*/
/*  Function Declare                            							  */
/*----------------------------------------------------------------------------*/
extern char GetChar(void);
/*----------------------------------------------------------------------------*/
/*  statistics of all the interrupts           	   							  */
/*----------------------------------------------------------------------------*/
void CAN_ShowMsg(STR_CANMSG_T* Msg);
/*----------------------------------------------------------------------------*/
/*  Callback function 													   	  */
/*----------------------------------------------------------------------------*/
STR_CANMSG_T rrMsg;

void TestFnTxOK()
{
	printf("Test Callback funtion OK\n");
}
void CAN_CallbackFn(uint32_t u32IIDR)
{
    if(u32IIDR==1)
    {
	    printf("Msg-0 INT and Callback\n");
        DrvCAN_ReadMsgObj(0,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
    if(u32IIDR==5+1)
    {
	    printf("Msg-5 INT and Callback \n");
        DrvCAN_ReadMsgObj(5,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
    if(u32IIDR==31+1)
    {
	    printf("Msg-31 INT and Callback \n");
        DrvCAN_ReadMsgObj(31,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
}

/*----------------------------------------------------------------------------*/
/*  Initial M0 Core and UART setting            							  */
/*----------------------------------------------------------------------------*/
void InitSystem()
{
	/* SYSCLK =>12Mhz*/
	UNLOCKREG();
	DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
  DrvSYS_SetOscCtrl(E_SYS_XTL32K,1);
  DrvSYS_SetOscCtrl(E_SYS_OSC22M,1);
  DrvSYS_SetOscCtrl(E_SYS_OSC10K,1);
	DrvSYS_Delay(20000);				/* Delay for Xtal stable */
	
    while(!SYSCLK->CLKSTATUS.OSC22M_STB);
    DrvSYS_Open(48000000);

	/* Select CAN Multi-Function */
	DrvGPIO_InitFunction(E_FUNC_CAN0);
}

/*----------------------------------------------------------------------------*/
/*  Test Function 													   	 	  */
/*----------------------------------------------------------------------------*/
void CAN_ShowMsg(STR_CANMSG_T* Msg)
{
    uint8_t i;
	  printf("Read ID=%8X, Type=%s, DLC=%d,Data=",Msg->Id,Msg->IdType?"EXT":"STD",Msg->DLC);
    for(i=0;i<Msg->DLC;i++)    
        printf("%02X,",Msg->Data[i]);
    printf("\n\n");
}

/*----------------------------------------------------------------------------*/
/*  Send Tx Msg by Basic Mode Function (Without Message RAM)         	 	  */
/*----------------------------------------------------------------------------*/
void Test_BasicMode_Tx()
{
    int32_t delaycount;
    STR_CANMSG_T msg1;
    delaycount=1000;
    DrvCAN_EnterTestMode(CAN_TESTR_BASIC);	                                       
    DrvCAN_EnableInt(CAN_CON_IE|CAN_CON_SIE);

    /* Send Message No.1 */
    msg1.FrameType= DATA_FRAME;
    msg1.IdType   = CAN_STD_ID;
	  msg1.Id       = 0x001;
    msg1.DLC      = 2;
	  msg1.Data[0]  = 0x00;
    msg1.Data[1]  = 0x2;
    DrvCAN_BasicSendMsg(&msg1);
    printf("Send STD_ID:0x1,Data[0,2]\n");
    DrvSYS_Delay(delaycount);
      
    /* Send Message No.2 */
    msg1.FrameType= DATA_FRAME;
    msg1.IdType   = CAN_STD_ID;
	  msg1.Id       = 0x1AC;
    msg1.DLC      = 8;
	  msg1.Data[0]  = 0x11;
    msg1.Data[1]  = 0x12;
    msg1.Data[2]  = 0x13;
    msg1.Data[3]  = 0x14;
	msg1.Data[4]  = 0x15;
    msg1.Data[5]  = 0x16;
    msg1.Data[6]  = 0x17;
    msg1.Data[7]  = 0x18;
  
    DrvCAN_BasicSendMsg(&msg1);
    printf("Send STD_ID:0x1AC,Data[11,12,13,14,15,16,17,18]\n");
    DrvSYS_Delay(delaycount);

    /* Send Message No.3 */
    msg1.FrameType= DATA_FRAME;
    msg1.IdType   = CAN_STD_ID;
	  msg1.Id       = 0x310;
    msg1.DLC      = 8;
	  msg1.Data[0]  = 0x21;
    msg1.Data[1]  = 0x22;
    msg1.Data[2]  = 0x23;
    msg1.Data[3]  = 0x24;
	  msg1.Data[4]  = 0x25;
    msg1.Data[5]  = 0x26;
    msg1.Data[6]  = 0x27;
    msg1.Data[7]  = 0x28;
    DrvCAN_BasicSendMsg(&msg1);
    printf("Send STD_ID:0x310,Data[21,22,23,24,25,26,27,28]\n");
    DrvSYS_Delay(delaycount);

    /* Send Message No.4 */
    msg1.FrameType= DATA_FRAME;    
    msg1.IdType   = CAN_EXT_ID;
    msg1.Id       = 0x3377;
    msg1.DLC      = 8;
	  msg1.Data[0]  = 0x31;
    msg1.Data[1]  = 0x32;
    msg1.Data[2]  = 0x33;
    msg1.Data[3]  = 0x34;
	  msg1.Data[4]  = 0x35;
    msg1.Data[5]  = 0x36;
    msg1.Data[6]  = 0x37;
    msg1.Data[7]  = 0x38;   
    DrvCAN_BasicSendMsg(&msg1);
    printf("Send EXT_ID:0x3377,Data[31,32,33,34,35,36,37,38]\n");
    DrvSYS_Delay(delaycount);

    /* Send Message No.5 */
    msg1.FrameType= DATA_FRAME;    
    msg1.IdType   = CAN_EXT_ID;
    msg1.Id       = 0x7755;
    msg1.DLC      = 8;
	  msg1.Data[0]  = 0x41;
    msg1.Data[1]  = 0x42;
    msg1.Data[2]  = 0x43;
    msg1.Data[3]  = 0x44;
	  msg1.Data[4]  = 0x45;
    msg1.Data[5]  = 0x46;
    msg1.Data[6]  = 0x47;
    msg1.Data[7]  = 0x48;  
    DrvCAN_BasicSendMsg(&msg1);
    printf("Send EXT_ID:0x7755,Data[41,42,43,44,45,46,47,48]\n");
    DrvSYS_Delay(delaycount);
    DrvCAN_LeaveTestMode();

}

/*----------------------------------------------------------------------------*/
/*  Read Rx Msg by Basic Mode Function (Without Message RAM)         	 	  */
/*----------------------------------------------------------------------------*/
void Test_BasicMode_Rx()
{
	STR_CANMSG_T rMsg[5];
    int32_t i;
    DrvCAN_EnterTestMode(CAN_TESTR_BASIC);

    /*  Wait status flag changed and with read IF2 */
    printf("\b Total 40 bytes data(using 5 frames)will be receive by CAN0 from CAN BUS\n");


    for(i=0;i<5;i++)
    {
        DrvCAN_ResetIF(1);
        CAN->sMsgObj[1].u32MCON  =0; 
        DrvCAN_WaitMsg();
	    DrvCAN_BasicReceiveMsg(&rMsg[i]);
    }
    for(i=0;i<5;i++)
        CAN_ShowMsg(&rMsg[i]);
    printf("CAN TEST END\n");
    return;
}

/*----------------------------------------------------------------------------*/
/*  Send Tx Msg by Normal Mode Function (With Message RAM)         	 	      */
/*----------------------------------------------------------------------------*/
void Test_NormalMode_Tx()
{
    STR_CANMSG_T tMsg;

	/* Send a 11-bits message */
    tMsg.FrameType= DATA_FRAME;
    tMsg.IdType   = CAN_STD_ID;
    tMsg.Id       = 0x7FF;
    tMsg.DLC      = 2;
    tMsg.Data[0]  = 7;
    tMsg.Data[1]  = 0xFF;    

    if(DrvCAN_SetTxMsgObj(MSG(0),&tMsg) < 0)     /* Call DrvCAN_SetTxMsgObj() only Confiugre Msg RAM */
    {
        printf("Set Tx Msg Object failed\n");
        return;
    }
	DrvCAN_SetTxRqst(MSG(0));                    /* Call DrvCAN_SetTxRqst() requeset to send the Msg in the RAM */
    printf("MSG(0).Send STD_ID:0x7FF, Data[07,FF]done\n");

    if(DrvCAN_SetTxMsgObj(MSG(1),&tMsg) < 0)    
    {
        printf("Set Tx Msg Object failed\n");
        return;
    }
	/* Send a 29-bits message */
    tMsg.FrameType= DATA_FRAME;
    tMsg.IdType   = CAN_EXT_ID;
    tMsg.Id       = 0x12345;
    tMsg.DLC      = 3;
    tMsg.Data[0]  = 1;
    tMsg.Data[1]  = 0x23;
    tMsg.Data[2]  = 0x45;        

    if(DrvCAN_SetTxMsgObj(MSG(1),&tMsg) < 0)    
    {
        printf("Set Tx Msg Object failed\n");
        return;
    }
	  DrvCAN_SetTxRqst(MSG(1));              
    printf("MSG(1).Send EXT:0x12345 ,Data[01,23,45]done\n");  
 
	/* Send a data message */
    tMsg.FrameType= DATA_FRAME;
    tMsg.IdType   = CAN_EXT_ID;
    tMsg.Id       = 0x7FF01;
    tMsg.DLC      = 4;
    tMsg.Data[0]  = 0xA1;
    tMsg.Data[1]  = 0xB2;
    tMsg.Data[2]  = 0xC3;        
    tMsg.Data[3]  = 0xD4;        

    if(DrvCAN_SetTxMsgObj(MSG(3),&tMsg) < 0)    
    {
        printf("Set Tx Msg Object failed\n");
        return;
    }
	  DrvCAN_SetTxRqst(MSG(3));                
    printf("MSG(3).Send EXT:0x7FF01 ,Data[A1,B2,C3,D4]done\n");  
	  DrvSYS_Delay(1000000); //Delay one second to wait INT
	  printf("Trasmit Done!\nCheck the receive host received data\n");

}

/*----------------------------------------------------------------------------*/
/*  Send Rx Msg by Normal Mode Function (With Message RAM)         	 	      */
/*----------------------------------------------------------------------------*/

void Test_NormalMode_Rx()
{
    if(DrvCAN_SetRxMsgObj(MSG(0),CAN_STD_ID, 0x7FF,TRUE) < 0)
    {
        printf("Set Rx Msg Object failed\n");
        return;
    }

    if(DrvCAN_SetRxMsgObj(MSG(5),CAN_EXT_ID, 0x12345,TRUE) < 0)
    {
        printf("Set Rx Msg Object failed\n");
        return;
    }

    if(DrvCAN_SetRxMsgObj(MSG(31),CAN_EXT_ID, 0x7FF01,TRUE) < 0)
    {
        printf("Set Rx Msg Object failed\n");
        return;
    }
	/*Choose one mode to test*/
#if 1
    /* Polling Mode */
    while(1)
    {
        while(CAN->u32IIDR ==0);            /* Wait IDR is changed */
        printf("IDR = %x\n",CAN->u32IIDR);
        DrvCAN_ReadMsgObj(CAN->u32IIDR -1,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
#else
	/* INT Mode */
	DrvCAN_EnableInt(CAN_CON_IE);
 	DrvCAN_InstallCallback(CALLBACK_MSG, (CAN_CALLBACK)CAN_CallbackFn);
	printf("Wait Msg\n");
	printf("Enter any key to exit\n");
	GetChar();
#endif
}


void Test_TestMaskFilter()
{
    int32_t i32id=0;
    STR_CANMSG_T tMsg;

    printf("Use Messge Object No.1 to send ID=0x700 ~0x70F\n");
    printf("Enter any key to send\n ");

    GetChar();
    while(1)
    {
    	/* Send a 11-bits message */
	    tMsg.FrameType= DATA_FRAME;
	    tMsg.IdType   = CAN_STD_ID;
	    tMsg.Id 	  = 0x700 + i32id;
	    tMsg.DLC	  = 0;

    	if(DrvCAN_SetTxMsgObj(MSG(1),&tMsg) < 0)	 /* Call DrvCAN_SetTxMsgObj() only Confiugre Msg RAM */
	    {
		    printf("Set Tx Msg Object failed\n");
		    return;
	    }
	    DrvCAN_SetTxRqst(MSG(1));
        i32id++;
        if(i32id>=0x10)
            break;
        DrvSYS_Delay(100000);
    }
    printf("Transfer Done\n");
}


void Test_SetMaskFilter()
{
    STR_CANMASK_T MaskMsg;

	DrvCAN_EnableInt(CAN_CON_IE);
 	DrvCAN_InstallCallback(CALLBACK_MSG, (CAN_CALLBACK)CAN_CallbackFn);
	
	/* Set b'0' means don't care*/
	/* Set Messge Object No.0 mask ID */
	MaskMsg.u8Xtd    = 1;
	MaskMsg.u8Dir    = 1;
	MaskMsg.u8IdType = 0;     /* 1:EXT or 0:STD */
	MaskMsg.u32Id 	 = 0x707;
	DrvCAN_SetMsgObjMask(MSG(0),&MaskMsg);
	DrvCAN_SetRxMsgObj(MSG(0), CAN_STD_ID, 0x7FF , TRUE);

    printf("The sample code should be with Item[4]\n\n");

    printf("Set Receive Message Object Contect\n");
    printf("===================================\n");
    printf("Message Object No.0 :Receive STD_ID:0x7FF\n\n\n");
    printf("Set Mask Content\n");
    printf("==========================\n");
    printf("Compare Xtd: %s\n",MaskMsg.u8Xtd?"YES":"NO");
    printf("Compare Dir: %s\n",MaskMsg.u8Dir?"YES":"NO");
    printf("Compare MaskID:%s 0x%x\n\n\n",MaskMsg.u8IdType? "EXT":"STD",MaskMsg.u32Id);
    printf("If there is a message-ID 0x700~0x70F,\n ONLY 0x707/0x70F can pass acceptance filter.\n");

    printf("Waiting Message\n");
	DrvSYS_Delay(100000);
	GetChar();

	DrvCAN_UninstallCallback(CALLBACK_MSG);
   	DrvCAN_DisableInt(CAN_CON_IE);
	
}



/*----------------------------------------------------------------------------*/
/*  Some description about how to create test environment         	 	      */
/*----------------------------------------------------------------------------*/
void Note_Configure()
{
    printf("\n\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("|  About CAN sample code configure                                       |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("|   The sample code provide a simple sample code for you study CAN       |\n");
    printf("|   Before execute it, please check description as below                 |\n");
    printf("|                                                                        |\n");
    printf("|   1.CAN_TX and CAN_RX should be connected to your CAN transceiver      |\n");
    printf("|   2.Using two module board and connect to the same CAN BUS             |\n");
    printf("|   3.Check the terminal resistor of bus is connected                    |\n");
    printf("|   4.Using UART0 as print message port(Both of NUC1xx module boards)    |\n");
    printf("|                                                                        |\n"); 
    printf("|  |--------|       |----------|   CANBUS  |----------|       |--------| |\n");     
    printf("|  |        |------>|          |<--------->|          |<------|        | |\n"); 
    printf("|  |        |CAN_TX |   CAN    |   CAN_H   |   CAN    |CAN_TX |        | |\n");     
    printf("|  | NUC1XX |       |Tranceiver|           |Tranceiver|       | NUC1XX | |\n"); 
    printf("|  |        |<------|          |<--------->|          |------>|        | |\n"); 
    printf("|  |        |CAN_RX |          |   CAN_L   |          |CAN_RX |        | |\n");    
    printf("|  |--------|       |----------|           |----------|       |--------| |\n");     
    printf("|   |                                                           |        |\n");
    printf("|   |                                                           |        |\n");
    printf("|   V                                                           V        |\n");
    printf("| UART0                                                         UART0    |\n");
    printf("|(print message)                                          (print message)|\n");
    printf("+------------------------------------------------------------------------+\n");
}

/*----------------------------------------------------------------------------*/
/*  Test Menu    													   	 	  */
/*----------------------------------------------------------------------------*/
void TestItem(void)
{
    printf("\n");
    printf("+------------------------------------------------------------------ +\n");
    printf("|  Nuvoton CAN BUS DRIVER DEMO                                      |\n");
    printf("+-------------------------------------------------------------------+\n");
    printf("|  [0] Transmit a message by basic mode                             |\n");
    printf("|     (At first, another module board should be set to              |\n");
    printf("|     [Receive a message by basic mode] waiting for receiving data) |\n");
    printf("|  [1] Receive a message by basic mode                              |\n");
    printf("|  [2] Transmit a message by normal mode                            |\n");
    printf("|     (At first, another module board should be set to              |\n");
    printf("|     [Receive a message by normal mode] waiting for receiving data)|\n");                 
    printf("|  [3] Receive a message by normal mode                             |\n");
	  printf("|  [4] Test Mask Filter                                             |\n");
    printf("|     (At first, another module board should be set to              |\n");
    printf("|     [Set Mask Filter] waiting for receiving data)                 |\n");
    printf("|  [5] Set Mask Filter                                              |\n");
    printf("+-------------------------------------------------------------------+\n");
}


void TestFnMsg_RXOK(uint32_t u32Parameter)
{
    STR_CANMSG_T rMsg;
    DrvCAN_ResetIF(1);
    CAN->sMsgObj[1].u32MCON  =0; 

	  DrvCAN_BasicReceiveMsg(&rMsg);
    CAN_ShowMsg(&rMsg);
}

void SelectCANSpeed()
{
    uint32_t unItem;
    int32_t i32Err =0;

    printf("Please select CAN speed you desired\n");
    printf("[0] 1000Kbps\n");
    printf("[1]  500Kbps\n");
    printf("[2]  250Kbps\n");
    printf("[3]  125Kbps\n");
    printf("[4]  100Kbps\n");
    printf("[5]   50Kbps\n");

    unItem = GetChar();
    printf("%c\n",unItem) ;
    if(unItem=='1')
        i32Err = DrvCAN_Open(500);
    else if(unItem=='2')
        i32Err = DrvCAN_Open(250);
    else if(unItem=='3')
        i32Err = DrvCAN_Open(125);
    else if(unItem=='4')
        i32Err = DrvCAN_Open(100);
    else if(unItem=='5')
        i32Err = DrvCAN_Open(50);
    else
        i32Err = DrvCAN_Open(1000);

    if(i32Err<0)
        printf("Set CAN bit rate is fail\n");
}


/*----------------------------------------------------------------------------*/
/*  MAIN function													   	 	  */
/*----------------------------------------------------------------------------*/
int main (void)
{
	uint8_t item;
 	//STR_UART_T param;		
    UNLOCKREG();
    DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
    DrvSYS_SetOscCtrl(E_SYS_OSC22M,1);
    DrvSYS_Delay(20000);				/* Delay for Xtal stable */
   
    while(!SYSCLK->CLKSTATUS.XTL12M_STB);
    while(!SYSCLK->CLKSTATUS.OSC22M_STB);

    DrvSYS_SelectHCLKSource(0);

	  // Init GPIO and configure UART0 
	  //DrvGPIO_InitFunction(E_FUNC_UART0);
  	//param.u32BaudRate        = 115200;
    //param.u8cDataBits        = DRVUART_DATABITS_8;
    //param.u8cStopBits        = DRVUART_STOPBITS_1;
    //param.u8cParity          = DRVUART_PARITY_NONE;
    //param.u8cRxTriggerLevel  = DRVUART_FIFO_1BYTES;
    //param.u8TimeOut        	 = 0;
    //DrvUART_Open(UART_PORT0, &param);

    /* Set GPD14 to control CAN transceiver for Nuvoton board */
 	  DrvGPIO_Open(E_GPD,14,E_IO_OUTPUT);    
    DrvGPIO_ClrBit(E_GPD,14);
 	  DrvGPIO_Open(E_GPB,12,E_IO_OUTPUT);    
    DrvGPIO_ClrBit(E_GPB,12);

	  /* Select CAN Multi-Function */
	  DrvGPIO_InitFunction(E_FUNC_CAN0);
    DrvCAN_Init();
    Note_Configure();
    do
    {
        TestItem();
        item = GetChar();    
        printf("%c\n",item);
        SelectCANSpeed();
        switch (item)
        {
            case '0': Test_BasicMode_Tx();      break;     
            case '1': Test_BasicMode_Rx();      break;     
            case '2': Test_NormalMode_Tx();     break;     
            case '3': Test_NormalMode_Rx();     break;     
            case '4': Test_TestMaskFilter();    break;      
            case '5': Test_SetMaskFilter();     break;      
              
        }
    }while(item != 27);
	
	DrvCAN_Close(); 
}
