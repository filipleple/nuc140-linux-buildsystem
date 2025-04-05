//
// Smpl_CAN_Timer : set Timer0 to increment counter, and send counter value to CAN bus 
//
// TX: generate message with ID=0x777 & Data = 5 bytes
// RX: set Mask Filter to receive ID=700~7FF, and show Msg Data
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "CAN.h"
#include "LCD.h"

static uint16_t Timer0Counter=0;
char TEXT0[16] = "CAN :           ";
char TEXT1[16] = "ID  :           ";
char TEXT2[16] = "Data:           ";
char TEXT3[16] = "                ";

void CAN_ShowMsg(STR_CANMSG_T* Msg);

STR_CANMSG_T rrMsg;

// Interrupt Service Routine / Callback Function
void CAN_CallbackFn(uint32_t u32IIDR)
{
    if(u32IIDR==1)
    {
	    //printf("Msg-0 INT and Callback\n");
        DrvCAN_ReadMsgObj(0,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
    if(u32IIDR==5+1)
    {
	    //printf("Msg-5 INT and Callback \n");
        DrvCAN_ReadMsgObj(5,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
    if(u32IIDR==31+1)
    {
	    //printf("Msg-31 INT and Callback \n");
        DrvCAN_ReadMsgObj(31,TRUE,&rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
}

// Display Message on LCD
void CAN_ShowMsg(STR_CANMSG_T* Msg)
{
  uint8_t i;
	sprintf(TEXT1+6,"%x", Msg->Id);
	print_Line(1,TEXT1);

  for(i=0;i<Msg->DLC;i++) 	
	sprintf(TEXT2+6+i,"%c", Msg->Data[i]);
	print_Line(2,TEXT2);
}

// TX send ID & Data
void CAN_TX(uint16_t number)
{
    STR_CANMSG_T tMsg;

	  tMsg.FrameType= DATA_FRAME;
	  tMsg.IdType   = CAN_STD_ID; // standard 11-bit ID
	  tMsg.Id 	    = 0x777;		  // ID = 0x777
	  tMsg.DLC	    = 5;			    // data length = 5 bytes
	
		// converting counter value to 5-digit number ASCII code 
		tMsg.Data[0]  = 0x30 + number /10000;
		number = number - number/10000 *10000;
		tMsg.Data[1]  = 0x30 + number /1000;
		number = number - number/1000  *1000;
		tMsg.Data[2]  = 0x30 + number /100;
		number = number - number/100   *100;
		tMsg.Data[3]  = 0x30 + number /10;
		number = number - number/10   *10;
		tMsg.Data[4]  = 0x30 + number;

    	if(DrvCAN_SetTxMsgObj(MSG(1),&tMsg) < 0) // Confiugre Msg RAM
	    {
		    //printf("Set Tx Msg Object failed\n");
		    return;
	    }
	    DrvCAN_SetTxRqst(MSG(1));
      DrvSYS_Delay(100000);
}

// RX set Mask Filter
void SetMaskFilter()
{
  STR_CANMASK_T MaskMsg;

	DrvCAN_EnableInt(CAN_CON_IE);
 	DrvCAN_InstallCallback(CALLBACK_MSG, (CAN_CALLBACK)CAN_CallbackFn);
	
	/* Set b'0' means don't care*/
	/* Set Messge Object No.0 mask ID */
	MaskMsg.u8Xtd    = 1;
	MaskMsg.u8Dir    = 1;
	MaskMsg.u8IdType = 0;     /* 1:EXT or 0:STD */
	MaskMsg.u32Id 	 = 0x700;
	DrvCAN_SetMsgObjMask(MSG(0),&MaskMsg);
	DrvCAN_SetRxMsgObj(MSG(0), CAN_STD_ID, 0x7FF , TRUE);
    //printf("If there is a message-ID 0x700~0x70F,\n ONLY 0x707/0x70F can pass acceptance filter.\n");
    //printf("Waiting Message\n");
	DrvSYS_Delay(100000);
//	DrvCAN_UninstallCallback(CALLBACK_MSG);
//  DrvCAN_DisableInt(CAN_CON_IE);
}


void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 46875;		  // Set TCMPR [0~16777215]
								// (1/12000000)*(255+1)*46875 = 1 sec  	
	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
	Timer0Counter+=1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty	
	CAN_TX(Timer0Counter); // send counter value to CAN bus
}

/*----------------------------------------------------------------------------*/
/*  MAIN function													   	 	  */
/*----------------------------------------------------------------------------*/
int main (void)
{
	int32_t CAN_speed = 1000; // Kbps 
  int32_t i32Err =0; 
  UNLOCKREG();
  DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
  DrvSYS_Delay(5000);	/* Delay for Xtal stable */
  DrvSYS_SelectHCLKSource(0);
  LOCKREG();

	init_LCD();
	clear_LCD();
	InitTIMER0();

 	DrvGPIO_Open(E_GPB,12,E_IO_OUTPUT);		// CAN Transceiver setting
  DrvGPIO_ClrBit(E_GPB,12);

	/* Select CAN Multi-Function */
	DrvGPIO_InitFunction(E_FUNC_CAN0);
  DrvCAN_Init();

  i32Err = DrvCAN_Open(CAN_speed);				// set CAN speed
  //if(i32Err<0) printf("Set CAN bit rate is fail\n");

	sprintf(TEXT0+5,"%d Kbps", CAN_speed); 
	print_Line(0,TEXT0);

 	SetMaskFilter();						// set receiving message

	while(1) {
  // Timer Interrupt Handler will call CAN_TX
	}      	
//	DrvCAN_Close(); 
}







