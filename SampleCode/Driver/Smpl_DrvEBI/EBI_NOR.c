/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "EBI.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Includes of local headers                                                                               */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t	*pu8NorBaseAddr;

/*---------------------------------------------------------------------------------------------------------*/
/* Global file scope (static) variables                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void NOR_W39L010(void);
void NOR_Reset_W39L010(void);
uint8_t NOR_Device_ID_W39L010(void);
uint8_t NOR_Erase_W39L010(uint8_t u8IsNeedCompare);
uint8_t NOR_CheckCMDComplete(uint32_t u32DestAddr, uint8_t u8Data);
uint8_t NOR_Read_W39L010(uint32_t u32DestAddr);
uint8_t NOR_ProgramByte_W39L010(uint32_t u32DestAddr, uint8_t u8Data);
uint8_t ProgramDataTest(void);
uint8_t ContinueDataTest(void);

void DelayNOP(uint32_t u32Cnt)
{
	volatile uint32_t u32LoopCnt = u32Cnt;
	while (u32LoopCnt--)
	{
		__NOP();
	}
}

/* Data Width : 8bits, Data Size : 128K(For NUCxxx series, max. size is 128KB) */
void NOR_W39L010(void)
{
	DRVEBI_CONFIG_T	sEBIConfig;
	
	// Open EBI function
	sEBIConfig.eDataWidth		= E_DRVEBI_DATA_8BIT;
	sEBIConfig.eAddrWidth		= E_DRVEBI_ADDR_16BIT;
	sEBIConfig.u32BaseAddress 	= DRVEBI_BASE_ADDR;
	sEBIConfig.u32Size 			= DRVEBI_MAX_SIZE;
 	DrvEBI_Open(sEBIConfig);

	pu8NorBaseAddr = (uint8_t *)DRVEBI_BASE_ADDR;
	
	// Reset NOR Flash
	NOR_Reset_W39L010();
	
	// Get Device ID
	if (NOR_Device_ID_W39L010() == TRUE)
	{
		printf("NOR W39L010 initial OK !\n");
	}else
	{
		printf("NOR W39L010 initial fail !\n\n");
		return ;
	}
	
	printf("Erase Test ... \n");
	NOR_Erase_W39L010(TRUE);
	printf("\n");

	printf("Program Data Test ... \n");
	ProgramDataTest();
}

void NOR_Reset_W39L010(void)
{
	*(pu8NorBaseAddr+0x5555) = 0xAA;
	*(pu8NorBaseAddr+0x2AAA) = 0x55;
	*(pu8NorBaseAddr+0x0000) = 0xF0;
}

uint8_t NOR_Device_ID_W39L010()
{
	uint8_t u8ManuFactureID, u8DeviceID;
	
	*(pu8NorBaseAddr+0x5555) = 0xAA;
	*(pu8NorBaseAddr+0x2AAA) = 0x55;
	*(pu8NorBaseAddr+0x5555) = 0x90;
	DelayNOP(0x1000);
	
	u8ManuFactureID = *(pu8NorBaseAddr+0x0);
	u8DeviceID 		= *(pu8NorBaseAddr+0x1);
	printf("   >> W39L010 ManufactureID = 0x%X, DeviceID = 0x%X\n", u8ManuFactureID, u8DeviceID);
	DelayNOP(0x1000);
	if ((u8ManuFactureID != 0xDA) ||(u8DeviceID != 0x31))
		return FALSE;
		
	*(pu8NorBaseAddr+0x5555) = 0xAA;
	*(pu8NorBaseAddr+0x2AAA) = 0x55;
	*(pu8NorBaseAddr+0x5555) = 0xF0;
	DelayNOP(0x1000);
	
	return TRUE;
}

uint8_t NOR_Erase_W39L010(uint8_t u8IsNeedCompare)
{
	uint8_t u8Status = TRUE;
	
	*(pu8NorBaseAddr+0x5555) = 0xAA;
	*(pu8NorBaseAddr+0x2AAA) = 0x55;
	*(pu8NorBaseAddr+0x5555) = 0x80;
	*(pu8NorBaseAddr+0x5555) = 0xAA;
	*(pu8NorBaseAddr+0x2AAA) = 0x55;
	*(pu8NorBaseAddr+0x5555) = 0x10;
	
	u8Status = NOR_CheckCMDComplete(0x0, 0x0);
	if (u8Status == FALSE)
	{
		printf("   >> Chip Erase ... TIME OUT !!!\n");
		return u8Status;
	}
	DelayNOP(0x100000);
	
	if ( u8IsNeedCompare )
	{
		/* Compare data ...... */
		uint8_t u8DataIn, u8DataOut;
		uint32_t u32NORAddr;
		u8DataIn = 0xFF;
		for (u32NORAddr=0; u32NORAddr<DRVEBI_MAX_SIZE; u32NORAddr++)
		{
			u8DataOut = NOR_Read_W39L010(u32NORAddr);
			if (u8DataOut != u8DataIn)
			{
				printf("Read [0x%05X]:[0x%02X] FAIL !!! (Got [0x%02X])\n", u32NORAddr, u8DataIn, u8DataOut);
				printf("   >> Chip Erase FAIL !!!\n\n");
				return FALSE;
			}
/*
			else
			{
				// for test only 
				if ((u32NORAddr%32) == 0)
					printf("Read [0x%05X]:[0x%02X] !!!\r", u32NORAddr, u8DataOut);
			}
*/			
		}
		printf("   >> Chip Erase OK !!!          \n");
	}
		
	return u8Status;
}

uint8_t NOR_CheckCMDComplete(uint32_t u32DestAddr, uint8_t u8Data)
{
#if 0
	/* Check Toggle Ready */
    uint8_t u8PreData;
    uint8_t u8CurData;
    uint32_t u32TimeOut = 0;

    u8PreData = NOR_Read_W39L010(u32DestAddr);
    u8PreData = u8PreData & (1<<6);   		// read DQ6
    while (u32TimeOut < EBI_TIMEOUT_COUNT)	 
    {
        u8CurData = NOR_Read_W39L010(u32DestAddr);
        u8CurData = u8CurData & (1<<6); 	// read DQ6 again
        if (u8PreData == u8CurData)
        {
			return TRUE;
        }
		u8PreData = u8CurData;
        u32TimeOut++;
    }
    return FALSE;
#else
	/* Check Data Polling */
	uint8_t u8CurData;
    uint32_t u32TimeOut = 0;

    u8Data = u8Data & (1<<7); 	// read D7
    while (u32TimeOut < DRVEBI_TIMEOUT_COUNT)	
    {
		u8CurData = NOR_Read_W39L010(u32DestAddr);
		u8CurData = u8CurData & (1<<7);	// read DQ7
		if (u8Data == u8CurData)
		{
			return TRUE;
		}
		u32TimeOut++;
    }
    return FALSE;
#endif
}

uint8_t NOR_Read_W39L010(uint32_t u32DestAddr)
{
	return  *(pu8NorBaseAddr + u32DestAddr);
}

uint8_t NOR_ProgramByte_W39L010(uint32_t u32DestAddr, uint8_t u8Data)
{
	*(pu8NorBaseAddr+0x5555) = 0xAA;
	*(pu8NorBaseAddr+0x2AAA) = 0x55;
	*(pu8NorBaseAddr+0x5555) = 0xA0;
	
	*(pu8NorBaseAddr + u32DestAddr) = u8Data;
	
 	return NOR_CheckCMDComplete(u32DestAddr, u8Data);
}

uint8_t ProgramDataTest(void)
{
	uint8_t u8DataIn, u8DataOut, u8WriteOnce = TRUE;
	uint32_t u32NORAddr;
	uint8_t	u8Data = 0x00;	

	u8DataIn = u8Data;
	if (u8DataIn == 0x00)
		u8WriteOnce = FALSE;
		
	while (1)
	{		
		/* Erase flash first */
		NOR_Erase_W39L010(FALSE);
		DelayNOP(0x10000);
		
		/* Program flash and compare data */
		printf("  >> Program Flash Test ... \n");
		for (u32NORAddr=0; u32NORAddr<DRVEBI_MAX_SIZE; u32NORAddr++)
		{
			if (NOR_ProgramByte_W39L010(u32NORAddr, u8DataIn) == FALSE)
			{
				printf("Program [0x%05X]:[0x%02X] FAIL !!!\n", u32NORAddr, u8DataIn);
				return FALSE;
			}
/*			else
			{
				// for test only
				if ((u32NORAddr%32) == 0)
					printf("Program [0x%05X]:[0x%02X] !!!\r", u32NORAddr, u8DataIn);
			}
*/			
		}
		for (u32NORAddr=0; u32NORAddr<DRVEBI_MAX_SIZE; u32NORAddr++)
		{
			u8DataOut = NOR_Read_W39L010(u32NORAddr);
			if (u8DataOut != u8DataIn)
			{
				printf("Read [0x%05X]:[0x%02X] FAIL !!! (Got [0x%02X])\n", u32NORAddr, u8DataIn, u8DataOut);
				printf("Program flash FAIL !!!          \n");
				return FALSE;
			}
/*			else
			{
				// for test only 
				if ((u32NORAddr%32) == 0)
					printf("Read [0x%05X]:[0x%02X] !!!     \r", u32NORAddr, u8DataOut);
			}
*/			
		}
		printf("  >> Program flash [0x%02X] OK !!!          \n\n", u8DataIn);
		
		if ( u8WriteOnce )
			break;
			
		if (u8DataIn == 0x00)
			u8DataIn = 0xFF;
		else if (u8DataIn == 0xFF)
			u8DataIn = 0x55;
		else if (u8DataIn == 0x55)
			u8DataIn = 0xAA;
		else if (u8DataIn ==  0xAA)
			u8DataIn = 0x5A;
		else if (u8DataIn == 0x5A)
			u8DataIn = 0xA5;
		else if (u8DataIn == 0xA5)
		{
			u8DataIn = 0xF0;
			ContinueDataTest();
			break;	
        }else
			break;	
	}

	return TRUE;
}

uint8_t ContinueDataTest(void)
{
	uint8_t u8DataIn, u8DataOut;
	uint32_t u32NORAddr;
    uint32_t IncreaseCnt;

	/* Erase flash first */
	NOR_Erase_W39L010(FALSE);
	DelayNOP(0x10000);
	
	/* Program flash and compare data */
	printf("  >> Program Flash Test ... \n");
    IncreaseCnt = 0;
	for (u32NORAddr=0; u32NORAddr<DRVEBI_MAX_SIZE; u32NORAddr++)
	{
        IncreaseCnt++;
        u8DataIn = (u32NORAddr%256)+u32NORAddr;
		if (NOR_ProgramByte_W39L010(u32NORAddr, u8DataIn) == FALSE)
		{
			printf("Program [0x%05X]:[0x%02X] FAIL !!! \n\n", u32NORAddr, u8DataIn);
			return FALSE;
		}

	}
    IncreaseCnt = 0;
	for (u32NORAddr=0; u32NORAddr<DRVEBI_MAX_SIZE; u32NORAddr++)
	{
        IncreaseCnt++;
        u8DataIn = (u32NORAddr%256)+u32NORAddr;
        u8DataOut = NOR_Read_W39L010(u32NORAddr);
		if (u8DataOut != u8DataIn)
		{
			printf("Read [0x%05X]:[0x%02X] FAIL !!! (Got [0x%02X]) \n\n", u32NORAddr, u8DataIn, u8DataOut);
			printf("Program flash FAIL !!!           \n\n");
			return FALSE;
		}

	}
	printf("  >> Continue Data Program OK !!!          \n\n");
		
	return TRUE;
}

