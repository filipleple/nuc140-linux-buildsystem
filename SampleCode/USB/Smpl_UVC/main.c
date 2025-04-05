/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "SYS.h"
#include "USB.h"
#include "UVCSys.h"

extern int32_t UVC_MainProcess(void);

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
	extern VIDEOSTREAMCMDDATA  VideoStreamCmdCtlData;
	
	/* Initialize Video Probe and Commit Control data */
	memset((uint8_t *)&VideoStreamCmdCtlData,0x0,sizeof(VIDEOSTREAMCMDDATA));
    VideoStreamCmdCtlData.bmHint = 0x0100;
    VideoStreamCmdCtlData.bFormatIndex = 1;
    VideoStreamCmdCtlData.bFrameIndex = 1;
    VideoStreamCmdCtlData.dwFrameInterval = 0x051615;

    /* Unlock the locked registers */
    UNLOCKREG();
	SYSCLK->PLLCON.OE = 0;
    SYSCLK->PWRCON.XTL12M_EN = 1;

    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	Delay(1000);
	DrvSYS_SelectHCLKSource(2);    
    Delay(100);
    
    UVC_MainProcess();

}




