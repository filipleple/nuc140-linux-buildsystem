//
// Smpl_SDCard_ADC7 : ADC7 sample input & write data into SDCard for 10000 samples
//                    filename = test.dat
//                    ADC7 sample = 12-bit, store 16-bit into SDcard
//
#include <stdio.h>
#include <string.h>
#include "SPI.h"
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "SDCard.h"
#include "diskio.h"
#include "ff.h"

void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

void put_rc (FRESULT rc)
{
	const TCHAR *p =
		_T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
		_T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
		_T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
		_T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");
	//FRESULT i;
	uint32_t i;
	for (i = 0; (i != (UINT)rc) && *p; i++) {
		while(*p++) ;
	}
	printf(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}
	
/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime (void)
{
	unsigned long tmr;

    tmr=0x00000;

	return tmr;
}

uint8_t	u8FileBuff[2];
int8_t i8FileName[13]="test.dat";
void write_sdcard(int count)
{
volatile UINT Timer = 0;		/* Performance timer (1kHz increment) */

//FILINFO Finfo;
FATFS FatFs[_DRIVES];		/* File system object for logical drive */


  FRESULT res;		
	//FATFS *fs;				/* Pointer to file system object */	
FIL file1;				/* File objects */	
	uint32_t u32FileBuffPointer=2;	
	disk_initialize(0);

	res	=f_mount(0, &FatFs[0]);	
	if(res)
	{
		put_rc(res);
		printf("Don't mount file system\n");
	}	 
		
   put_rc(f_open(&file1, (TCHAR*)i8FileName, FA_WRITE));
   f_lseek(&file1, count);
   f_write(&file1, u8FileBuff, u32FileBuffPointer, &u32FileBuffPointer);
   f_close(&file1);
}


void InitADC(void)
{
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD|=0x00800000; 	//Disable digital input path
	SYS->GPAMFP.ADC7_SS21_AD6=1; 		//Set ADC function 
				
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 0;	//Select externl for ADC
	SYSCLK->CLKDIV.ADC_N = 0;	//ADC clock source = 12Mhz/1 =12Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;	//Enable clock source
	ADC->ADCR.ADEN = 1;			//Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	//single end input
	ADC->ADCR.ADMD   = 0;     	//single mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x80;
	
	/* Step 5. Enable ADC interrupt */
	ADC->ADSR.ADF =1;     		//clear the A/D interrupt flags for safe 
	ADC->ADCR.ADIE = 1;
//	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. Enable WDT module */
	ADC->ADCR.ADST=1;
}


int32_t main(void)
{		
		uint8_t dataout[2]="s";
		uint8_t dataout2[2]="h";
    int i=0;    
    STR_UART_T sParam;

    UNLOCKREG();
    /* Set UART Pin */
    DrvGPIO_InitFunction(E_FUNC_UART0);
  
    /* UART Setting */
    sParam.u32BaudRate    = 115200;
    sParam.u8cDataBits    = DRVUART_DATABITS_8;
    sParam.u8cStopBits    = DRVUART_STOPBITS_1;
    sParam.u8cParity    = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
    /* Select UART Clock Source From 12Mhz*/
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 

    /* Set UART Configuration */
    DrvUART_Open(UART_PORT0,&sParam);

    /* Enable 12M OSC */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);

    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
    Delay(1000);

    DrvSYS_SelectHCLKSource(2);

    printf("NUC100 USB MassStorage Start!\n");
#if 0
u8FileBuff[0]=0x55;
u8FileBuff[1]=0xaa;
write_sdcard(0);
u8FileBuff[0]=0x11;
u8FileBuff[1]=0x22;
write_sdcard(2);
#endif
InitADC();

while(1)
	{
		while(ADC->ADSR.ADF==0);
		ADC->ADSR.ADF=1;
		u8FileBuff[0]=(ADC->ADDR[7].RSLT>>8)&0x0f;
		u8FileBuff[1]=(ADC->ADDR[7].RSLT>>0)&0x0f;		
		write_sdcard(i);
		if(i>(10000*2)) break;
		i=i+2;
		ADC->ADCR.ADST=1;
	}
    while(1);
}



