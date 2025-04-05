//
// Smpl_SDcard_ADPCM
//
// 8ohm speaker connected to CON3 of NU-LB-NUC140 learning board
// adpcm.wav stored in SDcard, SDcard plug into the socket on the back of NU-LB-NUC140
// this sample use libImaAdpcm4bit.lib to play .wav file (filename = adpcm.wav, sample rate =32K)
//
#include <stdio.h>
#include <string.h>
#include "UART.h"
#include "GPIO.h"
#include "I2C.h"
#include "I2S.h"
#include "SYS.h"
#include "diskio.h"
#include "ff.h"
#include <Audio.h>
#include "adpcm4bit.h"
#include "LCD.h"

uint32_t u32DataSize;

const int8_t i8WavHeader[]=
{0x52,0x49,0x46,0x46,0x34,0xA1,0x00,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20
,0x14,0x00,0x00,0x00,0x11,0x00,0x01,0x00,0x40,0x1F,0x00,0x00,0xDF,0x0F,0x00,0x00
,0x00,0x01,0x04,0x00,0x02,0x00,0xF9,0x01,0x66,0x61,0x63,0x74,0x04,0x00,0x00,0x00
,0x99,0x3D,0x01,0x00,0x64,0x61,0x74,0x61,0x00,0xA1,0x00,0x00};

int8_t i8FileName[13]="adpcm.wav";
#define i16PCMBuffSize 504
#define	u8FileBuffSize 512
int16_t i16PCMBuff[i16PCMBuffSize];
int16_t i16PCMBuff1[i16PCMBuffSize];
uint8_t	u8FileBuff[u8FileBuffSize];
int32_t i32AdpcmStatus=0;
uint32_t u32PCMBuffPointer;
uint32_t u32PCMBuffPointer1;
uint32_t u32FileBuffPointer;
AudioHeader WavFile;

FATFS FatFs[_DRIVES];		/* File system object for logical drive */
 
//int8_t gIsPlaying = 0;
/* For I2C transfer */
__IO uint32_t EndFlag0 = 0;
uint8_t Device_Addr0 = 0x1A;	 			/* WAU8822 Device ID */
uint8_t Tx_Data0[2];
uint8_t DataCnt0;


#define BUFF_LEN    32*12
#define REC_LEN     REC_RATE / 1000

/* Recoder Buffer and its pointer */
uint16_t PcmRecBuff[BUFF_LEN] = {0};
uint32_t u32RecPos_Out = 0;
uint32_t u32RecPos_In = 0;
									
/* Player Buffer and its pointer */
uint32_t PcmPlayBuff[BUFF_LEN] = {0};
uint32_t u32PlayPos_Out = 0;
uint32_t u32PlayPos_In = 0;
void Tx_thresholdCallbackfn1(uint32_t status);
void Tx_thresholdCallbackfn0(uint32_t status);
void Rx_thresholdCallbackfn1(uint32_t status);
void Rx_thresholdCallbackfn0(uint32_t status);
static void RoughDelay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;

    while(delay-- >= 0);
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      I2C0_Callback_Tx()	          	                                                          		   */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function is to send Device ID and data to CODEC with I2C0							   */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_Callback_Tx(uint32_t status)
{
	if (status == 0x08)						/* START has been transmitted */
	{
		DrvI2C_WriteData(I2C_PORT0, Device_Addr0<<1);
		DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
	}	
	else if (status == 0x18)				/* SLA+W has been transmitted and ACK has been received */
	{
		DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataCnt0++]);
		DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
	}
	else if (status == 0x20)				/* SLA+W has been transmitted and NACK has been received */
	{
		
		DrvI2C_Ctrl(I2C_PORT0, 1, 1, 1, 0);
	}	
	else if (status == 0x28)				/* DATA has been transmitted and ACK has been received */
	{
		if (DataCnt0 != 2)
		{
			DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataCnt0++]);
			DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
		}
		else
		{
			DrvI2C_Ctrl(I2C_PORT0, 0, 1, 1, 0);
			EndFlag0 = 1;
		}		
	}
	else
	{
		while(1);
		//printf("Status 0x%x is NOT processed\n", status);
	}		
}

/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      I2C_WriteWAU8822()	          	                                               			           */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The function is to write 9-bit data to 7-bit address register of WAU8822 with I2C interface. 	   */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
static void I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data)
{		
	DataCnt0 = 0;
	EndFlag0 = 0;
	
	Tx_Data0[0] = (uint8_t)((u8addr << 1) | (u16data >> 8));
	Tx_Data0[1] = (uint8_t)(u16data & 0x00FF);

	/* Install I2C0 call back function for write data to slave */
	DrvI2C_InstallCallback(I2C_PORT0, I2CFUNC, I2C0_Callback_Tx);
		
	/* I2C0 as master sends START signal */
	DrvI2C_Ctrl(I2C_PORT0, 1, 0, 0, 0);
		
	/* Wait I2C0 Tx Finish */
	while (EndFlag0 == 0);
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      WAU8822_Setup()	          	                                                  			           */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The function is to configure CODEC WAU8822 with I2C interface. 									   */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
static void WAU8822_Setup(void)
{
	I2C_WriteWAU8822(0,  0x000);   /* Reset all registers */ 
	RoughDelay(0x200);
		
	I2C_WriteWAU8822(1,  0x02F);        
	I2C_WriteWAU8822(2,  0x1B3);   /* Enable L/R Headphone, ADC Mix/Boost, ADC */
	I2C_WriteWAU8822(3,  0x07F);   /* Enable L/R main mixer, DAC */
	//I2C_WriteWAU8822(3,  0x1FF);   /* Enable L/R main mixer, DAC */			
	I2C_WriteWAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */			
	I2C_WriteWAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */
	
	//I2C_WriteWAU8822(6,  0x1AD);   /* Divide by 6, 16K */
	I2C_WriteWAU8822(6,  0x1ED);   /* Divide by 12, 8K */
	//I2C_WriteWAU8822(7,  0x006);   /* 16K for internal filter cofficients */
	I2C_WriteWAU8822(7,  0x00A);   /* 8K for internal filter cofficients */

	I2C_WriteWAU8822(10, 0x008);   /* DAC softmute is disabled, DAC oversampling rate is 128x */
	I2C_WriteWAU8822(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */
	I2C_WriteWAU8822(15, 0x1EF);   /* ADC left digital volume control */
	I2C_WriteWAU8822(16, 0x1EF);   /* ADC right digital volume control */
		
	I2C_WriteWAU8822(43, 0x010);   

	I2C_WriteWAU8822(44, 0x000);   /* LLIN/RLIN is not connected to PGA */
	I2C_WriteWAU8822(45, 0x150);   /* LLIN connected, and its Gain value */
	I2C_WriteWAU8822(46, 0x150);   /* RLIN connected, and its Gain value */
	I2C_WriteWAU8822(47, 0x007);   /* LLIN connected, and its Gain value */
	I2C_WriteWAU8822(48, 0x007);   /* RLIN connected, and its Gain value */
	I2C_WriteWAU8822(49, 0x047);//006);

	//I2C_WriteWAU8822(50, 0x161);   /* Left DAC connected to LMIX */
	//I2C_WriteWAU8822(51, 0x160);//001);   /* Right DAC connected to RMIX */

	I2C_WriteWAU8822(50, 0x001);   /* Left DAC connected to LMIX */
	I2C_WriteWAU8822(51, 0x000);//001);   /* Right DAC connected to RMIX */
 
 	I2C_WriteWAU8822(54, 0x139);   /* LSPKOUT Volume */
	I2C_WriteWAU8822(55, 0x139);   /* RSPKOUT Volume */

	DrvGPIO_Open(E_GPE,14, E_IO_OUTPUT);	
	DrvGPIO_ClrBit(E_GPE,14);
}

/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      Tx_thresholdCallbackfn()	          	                                                           */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function when Data in Tx FIFO is less than Tx FIFO Threshold Level. 				   */
/*      It is used to transfer data in Play Buffer to Tx FIFO. 											   */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      status    I2S Status register value.                     										   */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void Tx_thresholdCallbackfn0(uint32_t status)
{
	uint32_t i;
	int32_t i32Data;
	
	for	( i = 0; i < 4; i++)
	{
		i32Data=(i16PCMBuff[u32PCMBuffPointer++])<<16;
		_DRVI2S_WRITE_TX_FIFO(i32Data);
		if(--u32DataSize==0)
		{
			DrvI2S_DisableInt(I2S_TX_FIFO_THRESHOLD);
			DrvI2S_DisableTx();
			return;
		}
	}
	if(u32PCMBuffPointer==i16PCMBuffSize)
	{
		DrvI2S_EnableInt(I2S_TX_FIFO_THRESHOLD, Tx_thresholdCallbackfn1);
		if(u32PCMBuffPointer1!=0)
			while(1);
	} 
}
void Tx_thresholdCallbackfn1(uint32_t status)
{
	uint32_t i;
	int32_t i32Data;
	
	for	( i = 0; i < 4; i++)
	{
		i32Data=(i16PCMBuff1[u32PCMBuffPointer1++])<<16;
		_DRVI2S_WRITE_TX_FIFO(i32Data);
		if(--u32DataSize==0)
		{
			DrvI2S_DisableInt(I2S_TX_FIFO_THRESHOLD);
			DrvI2S_DisableTx();
			return;
		}
	}
	if(u32PCMBuffPointer1==i16PCMBuffSize)
	{
		DrvI2S_EnableInt(I2S_TX_FIFO_THRESHOLD, Tx_thresholdCallbackfn0);
		if(u32PCMBuffPointer!=0)
			while(1);		
	}	
}



/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      Rx_thresholdCallbackfn()	          	                                                           */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function when Data in Rx FIFO is more than Rx FIFO Threshold Level. 				   */
/*      It is used to transfer data in Rx FIFO to Recode Buffer 										   */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      status    I2S Status register value.                     										   */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void Rx_thresholdCallbackfn0(uint32_t status)
{
	uint32_t  i;
	int32_t i32Data;
	
	for	( i = 0; i < 4; i++)
	{
		i32Data=_DRVI2S_READ_RX_FIFO()>>16;
		i16PCMBuff[u32PCMBuffPointer++]=i32Data;
	}
	if(u32PCMBuffPointer==i16PCMBuffSize)
	{
		DrvI2S_EnableInt(I2S_RX_FIFO_THRESHOLD, Rx_thresholdCallbackfn1);
		if(u32PCMBuffPointer1!=0)
			while(1);
	} 
}
void Rx_thresholdCallbackfn1(uint32_t status)
{
	uint32_t  i;
	int32_t i32Data;
	
	for	( i = 0; i < 4; i++)
	{
		i32Data=_DRVI2S_READ_RX_FIFO()>>16;
		i16PCMBuff1[u32PCMBuffPointer1++]=i32Data;
	}
	if(u32PCMBuffPointer1==i16PCMBuffSize)
	{
		DrvI2S_EnableInt(I2S_RX_FIFO_THRESHOLD, Rx_thresholdCallbackfn0);
		if(u32PCMBuffPointer!=0)
			while(1);
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
	printf("rc=%u FR_%s\n",(UINT)rc, p);
}

uint32_t u32SwapBuffer(void* address)
{
	uint8_t* u8Char;
	u8Char = (uint8_t*) address;
	return (*u8Char|*(u8Char+1)<<8|*(u8Char+2)<<16|*(u8Char+3)<<24);
				
}
uint16_t u16SwapBuffer(void* address)
{
	uint8_t* u8Char;
	u8Char = (uint8_t*) address;
	return (*u8Char|*(u8Char+1)<<8);
				
}
void InitWAU8822(void)
{
	S_DRVI2S_DATA_T st;
	/* Tri-state for FS and BCLK of CODEC */
	DrvGPIO_Open(E_GPC, 0, E_IO_OPENDRAIN);
	DrvGPIO_Open(E_GPC, 1, E_IO_OPENDRAIN);
	DrvGPIO_SetBit(E_GPC, 0);
	DrvGPIO_SetBit(E_GPC, 1);
	
	/* Set I2C0 I/O */
    DrvGPIO_InitFunction(E_FUNC_I2C0);    	
	/* Open I2C0, and set clock = 100Kbps */
	DrvI2C_Open(I2C_PORT0, 100000);		
	/* Enable I2C0 interrupt and set corresponding NVIC bit */
	DrvI2C_EnableInt(I2C_PORT0);
	/* Configure CODEC */
	WAU8822_Setup();
	printf("Configure WAU8822\n");

	/* Configure I2S */
	st.u32SampleRate 	 = 8000;
    st.u8WordWidth 	 	 = DRVI2S_DATABIT_16;
    st.u8AudioFormat 	 = DRVI2S_STEREO;  		
	st.u8DataFormat  	 = DRVI2S_FORMAT_I2S;   
    st.u8Mode 		 	 = DRVI2S_MODE_SLAVE;
    st.u8TxFIFOThreshold = DRVI2S_FIFO_LEVEL_WORD_4;
    st.u8RxFIFOThreshold = DRVI2S_FIFO_LEVEL_WORD_4;
	DrvI2S_SelectClockSource(0);
	DrvI2S_Open(&st);
	/* Set I2S I/O */	
    DrvGPIO_InitFunction(E_FUNC_I2S); 
	/* Set MCLK and enable MCLK */
	DrvI2S_SetMCLKFreq(12000000);	
	DrvI2S_EnableMCLK();
}
/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      UAC_MainProcess()	      	                                                                       */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The main loop of UAC funciton.                                                                     */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
int32_t UAC_MainProcess(void)
{

	uint32_t p1, s1, s2;
	FATFS *fs;				/* Pointer to file system object */
	DIR dir;				/* Directory object */
	FILINFO Finfo;
	FIL file1;				/* File objects */
	char *ptr="\\";
	FRESULT res;
	char TEXT3[16]="Loop Count:     ";							  
	uint32_t	count=0;

	res	= (FRESULT)disk_initialize(0);	
	if(res)
	{
		put_rc(res);
		printf("\n\nDon't initialize SD card\n");
	}

	res	=f_mount(0, &FatFs[0]);	
	if(res)
	{
		put_rc(res);
		printf("Don't mount file system\n");
	}	 


 	// List direct information	
	put_rc(f_opendir(&dir, ptr)); 
	put_rc(f_open(&file1, (TCHAR*)i8FileName, FA_READ));
	u32FileBuffPointer=u8FileBuffSize;
	f_lseek(&file1,0);
	put_rc(f_read(&file1, u8FileBuff, u32FileBuffPointer, &u32FileBuffPointer));
	while(u32FileBuffPointer!=u8FileBuffSize);
	u32FileBuffPointer=0;
	if(memcmp(u8FileBuff+u32FileBuffPointer, "RIFF", 4)==0)
	{
		u32FileBuffPointer+=4;
		printf("RIFF format\n");
		WavFile.u32ChunkSize=u32SwapBuffer(u8FileBuff+u32FileBuffPointer);
		u32FileBuffPointer+=4;
		if(memcmp(u8FileBuff+u32FileBuffPointer, "WAVEfmt ", 8)==0)//Buff[BuffPointer++]=='W'&&Buff[BuffPointer++]=='A'&&Buff[BuffPointer++]=='V'&&Buff[BuffPointer++]=='E'&&Buff[BuffPointer++]=='f'&&Buff[BuffPointer++]=='m'&&Buff[BuffPointer++]=='t'&&Buff[BuffPointer++]==0x20)
		{
			u32FileBuffPointer+=8;
			WavFile.u32Subchunk1Size=u32SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=4;		
		  WavFile.u16AudioFormat=u16SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=2;				
			WavFile.u16NumChannels=u16SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=2;			
			WavFile.u32SampleRate=u32SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=4;
			WavFile.u32ByteRate=u32SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=4;
			WavFile.u16BlockAlign=u16SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=2;
			WavFile.u16BitsPerSample=u16SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=2;
  			u32FileBuffPointer+=4;
			if(memcmp(u8FileBuff+u32FileBuffPointer, "fact", 4)!=0)
				while(1);
			u32FileBuffPointer+=8;
			WavFile.u32SampleNumber=u32SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=4;

			if(memcmp(u8FileBuff+u32FileBuffPointer, "data", 4)==0)
			{
				u32FileBuffPointer+=4;
				WavFile.u8HeaderStatus=1;
				WavFile.u32Subchunk2Size=u32SwapBuffer(u8FileBuff+u32FileBuffPointer);u32FileBuffPointer+=4;
				u32DataSize=WavFile.u32Subchunk2Size;
			}else
				while(1);
		}			  
	}
	print_Line(2, "Playing   ");
	while(1)
	{		
		sprintf(TEXT3+11,"%d",count++);
		print_Line(3, TEXT3);
		u32DataSize=WavFile.u32SampleNumber;
		u32FileBuffPointer=0x28+WavFile.u32Subchunk1Size;
		f_lseek(&file1,u32FileBuffPointer);
		u32PCMBuffPointer=u32PCMBuffPointer1=0;

 		InitWAU8822();
	
	    /* Eanble play hardware */
		DrvI2S_EnableInt(I2S_TX_FIFO_THRESHOLD, Tx_thresholdCallbackfn0);
		//DrvI2S_EnableInt(I2S_RX_FIFO_THRESHOLD, Rx_thresholdCallbackfn);
		DrvI2S_EnableTx();
		//DrvI2S_EnableRx();
		printf("Enable I2S and play first sound\n");
	
		while(1)
		{
			if(u32PCMBuffPointer>=i16PCMBuffSize)
			{
				u32FileBuffPointer=u8FileBuffSize;
				put_rc(f_read(&file1, u8FileBuff, u32FileBuffPointer, &u32FileBuffPointer));
				AdpcmDec4(u8FileBuff,(short*)i16PCMBuff,i16PCMBuffSize);
				u32PCMBuffPointer=0;
			}
			if(u32PCMBuffPointer1>=i16PCMBuffSize)
			{
				AdpcmDec4(u8FileBuff+(i16PCMBuffSize/2+4),(short*)i16PCMBuff1,i16PCMBuffSize);
				u32PCMBuffPointer1=0;
			}
			if(u32DataSize==0)
				break;
		}
	}		

}

void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/* Main function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	STR_UART_T sParam;

	//extern uint32_t SystemFrequency;
    /* Unlock the locked registers */
  UNLOCKREG();
	DrvSYS_Open(50000000);
	LOCKREG();
	
	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	/* Select UART Clock Source From 12MHz */
	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 
	/* Set UART Configuration */
	DrvUART_Open(UART_PORT0,&sParam);	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);	

  init_LCD();  //call initial pannel function
	clear_LCD();


	print_Line(0, "Smpl_ADPCM @8KHz");
	print_Line(1, "16bits, Mono");
	printf("\n\nNUC100 Series ADPCM!\n");
	printf("Please insert SDcard and connect line in with J1\n");  
  UAC_MainProcess();
}
