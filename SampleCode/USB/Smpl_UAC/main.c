/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "GPIO.h"
#include "SYS.h"
#include "UART.h"
#include "USB.h"

extern int32_t UAC_MainProcess(void);

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

    /* Unlock the locked registers */
    UNLOCKREG();

	/* Enable 12M OSC */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);

    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	  Delay(1000);
	  DrvSYS_SelectHCLKSource(2);    
    Delay(100);

    SystemCoreClockUpdate();
    
    UAC_MainProcess();

}




