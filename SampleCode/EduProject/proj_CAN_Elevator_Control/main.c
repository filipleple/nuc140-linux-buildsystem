//
// Smpl_CAN_Elevator_Control : Id = 0x777, Data[0]=floor_no, Data[1]=floor_dir 
//
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "CAN.h"
#include "LCD.h"
#include "Scankey.h"

#define MAX_FLOOR 9

char TEXT0[16] = "Control         ";
char TEXT1[16] = "ID  :           ";
char TEXT2[16] = "Data:           ";
char TEXT3[16] = "Elev:           ";

int8_t request[MAX_FLOOR];   // request of each floor 
int8_t direction[MAX_FLOOR]; // requested direction of every floor
int8_t floor_no;  // current floor
int8_t floor_dir; // elevator direction

void CAN_ShowMsg(STR_CANMSG_T* Msg);

void init_Elevator(void)
{
	int8_t i;
	for (i=0;i<MAX_FLOOR+1;i++) request[i]  =0;
	for (i=0;i<MAX_FLOOR+1;i++) direction[i]=0;
	floor_no  = 1;
	floor_dir = 0;
}

void Elevator_move(int8_t op)
{
	if      (op==1) sprintf(TEXT3+5," %dF Up  ", floor_no);
	else if (op==-1)sprintf(TEXT3+5," %dF Down", floor_no);
	else            sprintf(TEXT3+5," %dF Stop", floor_no);
	print_Line(3,TEXT3);
  DrvSYS_Delay(500000);	
}

void Elevator_open(void)
{
	request[floor_no]=0;
	direction[floor_no]=0;
	sprintf(TEXT3+5," %dF Open", floor_no);
	print_Line(3,TEXT3);
  DrvSYS_Delay(500000);	
}
	
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
  int8_t i,f;
	
	sprintf(TEXT1+6,"%x", Msg->Id);
	print_Line(1,TEXT1);

  for(i=0;i<Msg->DLC;i++) 	
  sprintf(TEXT2+6+i,"%c", Msg->Data[i]);
	print_Line(2,TEXT2);
	
	if (Msg->Id==0x700) {
		f = Msg->Data[0] - 0x30;
		request[f]+=10;
		if      (floor_no<f) direction[f]=1;
		else if (floor_no>f) direction[f]=-1;
		else                 direction[f]=0;				
	} else {
		f = Msg->Id - 0x700;
		request[f]++;		
	  if      (Msg->Data[0]=='U') direction[f]=1;
	  else if (Msg->Data[0]=='D') direction[f]=-1;
	  else                        direction[f]=0;		
	}
}

// TX send ID & Data
void CAN_TX(void)
{
    //int32_t i32id=0;
    STR_CANMSG_T tMsg;

	  tMsg.FrameType= DATA_FRAME;
	  tMsg.IdType   = CAN_STD_ID;     // standard 11-bit ID format
	  tMsg.Id 	    = 0x777;          // CAN Id
	  tMsg.DLC	    = 2;		          // data length = 1 byte
	 
	  tMsg.Data[0] = 0x30+floor_no;
	  if      (floor_dir==1)  tMsg.Data[1] = 'U';
	  else if (floor_dir==-1) tMsg.Data[1] = 'D';
	  else                    tMsg.Data[1] = '-';   	

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

void init_Timer0(void)
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
	int8_t  i;
	int16_t up_wt, down_wt;
	
	up_wt   = 0;
	down_wt = 0;
	for (i=1;i<floor_no;i++) down_wt = down_wt + request[i];
  for (i=floor_no+1; i<(MAX_FLOOR+1); i++) up_wt = up_wt + request[i];
	
	// To decicde motor moving direction
	if (floor_dir==1) {
			if      (up_wt  !=0) floor_dir=1;
		  else if (down_wt!=0) floor_dir=-1;
		  else                 floor_dir=0;
			}			
	else if (floor_dir==-1) {
			if      (down_wt!=0) floor_dir=-1;
		  else if (up_wt  !=0) floor_dir=1;
		  else                 floor_dir=0;
			}				
	else  {
			if      (down_wt>up_wt) floor_dir=-1;
		  else if (up_wt>down_wt) floor_dir=1;
		  else   //up_wt==down_wt
				   if (up_wt!=0) floor_dir=1;
		       else          floor_dir=0;
			}	

	floor_no = floor_no + floor_dir;			

	Elevator_move(floor_dir);
	if (request[floor_no]!=0 && floor_dir==0) Elevator_open();
			
  CAN_TX(); // send counter value to CAN bus			
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
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

	init_Elevator();
	init_LCD(); // Initialize LCD panel
	clear_LCD();
	OpenKeyPad();	   // Initialize Keypad 
	init_Timer0();    // Initialize Timer

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

  CAN_TX();
	DrvSYS_Delay(500000);
	
	while(1) {		
	}      
//	DrvCAN_Close(); 
}







