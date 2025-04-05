//
// proj_CAN_Elevator_Floor : CAN Id = 0x700 + floor
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "CAN.h"
#include "LCD.h"
#include "Scankey.h"

char TEXT0[16] = "Floor           ";
char TEXT1[16] = "ID  :           ";
char TEXT2[16] = "Data:           ";
char TEXT3[16] = "                ";

void CAN_ShowMsg(STR_CANMSG_T* Msg);

// Interrupt Service Routine / Callback function
STR_CANMSG_T rrMsg;

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
	if (Msg->Id==0x777) {		
	   sprintf(TEXT1,"Floor: %c", Msg->Data[0]);
	   print_Line(1,TEXT1);
 
	   if      (Msg->Data[1]=='U') sprintf(TEXT2,"Dir  : Up  ");
	   else if (Msg->Data[1]=='D') sprintf(TEXT2,"Dir  : Down");
	   else                        sprintf(TEXT2,"Dir  : Stop");
	   print_Line(2,TEXT2);
	}
}

// TX send ID & Data
void CAN_TX(uint8_t number)
{
    //int32_t i32id=0;
    STR_CANMSG_T tMsg;

   	/* Send a 11-bits message */
	  tMsg.FrameType= DATA_FRAME;
	  tMsg.IdType   = CAN_STD_ID; // standard 11-bit ID
	  tMsg.Id 	  = 0x703; // your own IP 
	  tMsg.DLC	  = 1;		 // data length = 5 bytes 
	 
	  if      (number==2) tMsg.Data[0] = 'U';
	  else if (number==8) tMsg.Data[0] = 'D';
	  else                tMsg.Data[0] = ' ';   

    	if(DrvCAN_SetTxMsgObj(MSG(1),&tMsg) < 0) // Confiugre Msg RAM
	    {
		    //printf("Set Tx Msg Object failed\n");
		    return;
	    }
	    DrvCAN_SetTxRqst(MSG(1));

    //printf("Transfer Done\n");
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
	DrvSYS_Delay(500000);

//	DrvCAN_UninstallCallback(CALLBACK_MSG);
//  DrvCAN_DisableInt(CAN_CON_IE);

}

/*----------------------------------------------------------------------------*/
/*  MAIN function													   	 	  */
/*----------------------------------------------------------------------------*/
int main (void)
{
	int32_t CAN_speed = 1000; // Kbps 
  int32_t i32Err =0; 
	uint8_t keyin;
	
  UNLOCKREG();
  DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
  DrvSYS_Delay(5000);	/* Delay for Xtal stable */
  DrvSYS_SelectHCLKSource(0);
  LOCKREG();

	init_LCD(); // Initialize LCD panel
	clear_LCD();
	OpenKeyPad();	 // Initialize Keypad 

// 	DrvGPIO_Open(E_GPD,14,E_IO_OUTPUT); 	// LCD backlight   
//  DrvGPIO_ClrBit(E_GPD,14);
 	DrvGPIO_Open(E_GPB,12,E_IO_OUTPUT);	// CAN Transceiver setting
  DrvGPIO_ClrBit(E_GPB,12);

	/* Select CAN Multi-Function */
	DrvGPIO_InitFunction(E_FUNC_CAN0);
  DrvCAN_Init();

    i32Err = DrvCAN_Open(CAN_speed);	// set CAN speed
    //if(i32Err<0) printf("Set CAN bit rate is fail\n");

	sprintf(TEXT0+7,":%dKb", CAN_speed); 
	print_Line(0,TEXT0);

 	SetMaskFilter();			// set receiving mask filter

	while(1) {
	keyin = ScanKey();   // scan keypad to input a number
	if (keyin!=0) CAN_TX(keyin);	     // CAN sending data
	sprintf(TEXT3,"Key = %d",keyin);	   	
	print_Line(3,TEXT3);
	DrvSYS_Delay(500000);
	}      
	
//	DrvCAN_Close(); 
}







