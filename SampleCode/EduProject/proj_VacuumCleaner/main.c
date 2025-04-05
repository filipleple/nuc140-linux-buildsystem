//
// proj_VacuumCleaner
//
// NuTiny-SDK-NUC140 (Target Device = NUC140VE3CN)
// UART1 to HC05 bluetooth module to receive 8 bytes Text command
// GPIO to control +6V DC motor 
//
// Smartphone App : BTButton
// Buttons : Forward, Backward, Right, Left
// Toggles : Auto/Remote, Record/Playback
//           Auto - auto run built-in patterns (Circle, Square, Zig-Zag, Edge,..)
//           Remote - run controlled by buttons of smartphone/tablet
//           Record - recording each move circularly
//           Playback - playback the recorded move 
//                      from the last move to the beginning of record buffer.

// HC05 Bluetooth module (connected to UART1)
// pin1 KEY  : N.C
// pin2 VCC  : to VCC +5V
// pin3 GND  : to GND
// pin4 TXD  : to UART1-RX/GPB4 (NUC140-pin19)
// pin5 RXD  : to UART1-TX/GPB5 (NUC140-pin20)
// pin6 STATE: N.C.

// Motor Driver [L298N](controlled by GPC10,11,12,13)
// ENA : to PWM0/GPA12 (NUC140-pin65) 
// IN1 : to GPC10      (NUC140-pin59)
// IN2 : to GPC11      (NUC140-pin58)
// IN3 : to GPC12      (NUC140-pin57)
// IN4 : to GPC13      (NUC140-pin56)
// ENB : to PWM1/GPA13 (NUC140-pin64)

// 5V  : to VCC+5V
// GND : to GND
// GND : N.C. 
// VCC : N.C. (Battery Voltage = +6V = 1.5v x4)
// IN  : N.C.
// IN  : N.C.

// GPA15 to read IR bumper interrupt

#include <stdio.h>
#include "NUC1xx.h"
#include "UART.h"
#include "GPIO.h"
#include "SYS.h"
#include "PWM.h"
#include "LCD.h"

#define MAX_RECORD 12*1024

char IR_Button ='F'; // default forward
char BT_Button ='F'; // default Forward
char Toggle1='A'; // default AUTORUN
char Toggle2='R'; // default RECORD

volatile uint8_t comRbuf[3];
volatile uint8_t comRbytes = 0;

volatile uint8_t  rec_buf[12*1024];
volatile uint32_t rec_no =0;

void init_GPIO()
{
	DrvGPIO_Open(E_GPC,10,E_IO_OUTPUT); // DC motor control
	DrvGPIO_Open(E_GPC,11,E_IO_OUTPUT); // DC motor control
	DrvGPIO_Open(E_GPC,12,E_IO_OUTPUT); // DC motor control
	DrvGPIO_Open(E_GPC,13,E_IO_OUTPUT); // DC motor control
	DrvGPIO_ClrBit(E_GPC,10);
	DrvGPIO_ClrBit(E_GPC,11);
	DrvGPIO_ClrBit(E_GPC,12);
	DrvGPIO_ClrBit(E_GPC,13);
}

void Forward()
{
	DrvGPIO_ClrBit(E_GPC,10); 
	DrvGPIO_SetBit(E_GPC,11);
	DrvGPIO_SetBit(E_GPC,12);
	DrvGPIO_ClrBit(E_GPC,13);
}

void Backward()
{
	DrvGPIO_SetBit(E_GPC,10); 
	DrvGPIO_ClrBit(E_GPC,11);
	DrvGPIO_ClrBit(E_GPC,12);
	DrvGPIO_SetBit(E_GPC,13);
}

void Right()
{
	DrvGPIO_ClrBit(E_GPC,10); 
	DrvGPIO_SetBit(E_GPC,11);
	DrvGPIO_ClrBit(E_GPC,12);
	DrvGPIO_ClrBit(E_GPC,13);
}

void Left()
{
	DrvGPIO_ClrBit(E_GPC,10); 
	DrvGPIO_ClrBit(E_GPC,11);
	DrvGPIO_SetBit(E_GPC,12);
	DrvGPIO_ClrBit(E_GPC,13);
}

void Stop()
{
	DrvGPIO_ClrBit(E_GPC,10); 
	DrvGPIO_ClrBit(E_GPC,11);
	DrvGPIO_ClrBit(E_GPC,12);
	DrvGPIO_ClrBit(E_GPC,13);
}

void init_TIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 4687;		// Set TCMPR [0~16777215] for 0.1sec
								// (1/12000000)*(255+1)* 46875 = 1 sec

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
	uint8_t Command;
	
	TIMER0->TISR.TIF =1; // clear Timer0 interrupt
	
	if (Toggle1=='A') Command = IR_Button; // IR bumper to decide the command
	else          		Command = BT_Button; // BT button to decide the command
		
  if (Toggle2=='R') {          // Record mode
		rec_buf[rec_no] = Command; // store command to buffer
	  rec_no++;
	} else {                     // Playback mode
	  Command = rec_buf[rec_no]; // play command from record buffer
		rec_no--;                  // playback command revesely 
	}
			
	switch(Command) {
		case 'F': if (Toggle2=='R') Forward(); 
		          else              Backward();
		          break;
		case 'B': if (Toggle2=='R') Backward();
			        else              Forward(); 
		          break;
		case 'R': if (Toggle2=='R') Right();
		          else              Left();
              break;
		case 'L': if (Toggle2=='R') Left();
		          else              Right();
           		break;
		case 'S': Stop();
		          break;
    default : Stop(); 
		          break;
	}
	 
}

void UART_INT_HANDLE(void)
{
	while(UART1->ISR.RDA_IF==1) 
	{
		comRbuf[comRbytes]=UART1->DATA;
		comRbytes++;		
		if (comRbytes>=3) {	
			BT_Button=comRbuf[0];
			Toggle1=comRbuf[1];
			Toggle2=comRbuf[2];
		  comRbytes=0;
		}
	}
}

void GPIOAB_INT_CallBack(uint32_t GPA_IntStatus, uint32_t GPB_IntStatus)
{
 	 if ((GPA_IntStatus>>15) & 0x01) {
		 if      (IR_Button=='F') IR_Button='R';
	   else if (IR_Button=='R') IR_Button='B';
	   else if (IR_Button=='B') IR_Button='L';
	   else if (IR_Button=='L') IR_Button='F';
	   else                     IR_Button='F';
	 }
}

void GPIOCDE_INT_CallBack(uint32_t GPC_IntStatus, uint32_t GPD_IntStatus, uint32_t GPE_IntStatus)
{ 
 	 //if ((GPC_IntStatus>>15) & 0x01) BT_Button = BT_Button++;
	 //print_Line(3,"GPC interrupt !!");
}

int32_t main()
{
	STR_UART_T sParam;
	//char TEXT[16];
	uint16_t PWM_CNR, PWM_CMR;

	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN=1;
	DrvSYS_Delay(5000);					// Waiting for 12M Xtal stalble
	SYSCLK->CLKSEL0.HCLK_S=0;
	LOCKREG();
   	
	DrvGPIO_InitFunction(E_FUNC_UART1);	// Set UART pins
	DrvGPIO_InitFunction(E_FUNC_PWM01); // Set PWM01 pins		
	DrvGPIO_InitFunction(E_FUNC_TMR0);  // Set TMR0 pins
	
	// set UART Configuration
    sParam.u32BaudRate 		  = 9600;
    sParam.u8cDataBits 		  = DRVUART_DATABITS_8;
    sParam.u8cStopBits 		  = DRVUART_STOPBITS_1;
    sParam.u8cParity 		    = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
 	if(DrvUART_Open(UART_PORT1,&sParam) != E_SUCCESS);
	DrvUART_EnableInt(UART_PORT1, DRVUART_RDAINT, UART_INT_HANDLE);

  // set GPA15 for reading IR bummper
	DrvGPIO_Open(E_GPA,15,E_IO_INPUT);
  DrvGPIO_EnableInt(E_GPA, 15, E_IO_RISING, E_MODE_EDGE);
  DrvGPIO_SetDebounceTime(5, 1);
  DrvGPIO_EnableDebounce(E_GPA, 15);	
  DrvGPIO_SetIntCallback(GPIOAB_INT_CallBack, GPIOCDE_INT_CallBack);
	
  init_LCD();
  clear_LCD();
	init_GPIO();
	init_TIMER0();
	init_PWM(0,0,119,4);
	init_PWM(1,0,119,4);
	
	//PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1)
	PWM_CNR = 12000000 / 10000 / (119 + 1) / 1 - 1; // Freq = 10KHz, DutyCycle = 90%
  // Duty Cycle = (CMR0+1) / (CNR0+1)
  PWM_CMR = PWM_CNR*9/10  - 1;		
	PWM_Out(0, PWM_CNR, PWM_CMR); // set PWM0 to 10000 Hz, 90% duty cycle
	PWM_Out(1, PWM_CNR, PWM_CMR); // set PWM1 to 10000 Hz, 90% duty cycle
    				
	print_Line(0,"Vacuum Cleaner");
	
	while(1)
	{
	}
	//DrvUART_Close(UART_PORT1);
}
