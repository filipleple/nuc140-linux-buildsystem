#include <stdio.h>
#include <string.h>
#include "SPI.h"
#include "GPIO.h"
#include "SYS.h"
#include "diskio.h"
#include "ff.h"
#include "SDCard.h"
#include "UART.h"
FATFS FatFs;   /* Work area (filesystem object) for logical drive */

int main (void)
{
    FIL fil;        /* File object */
    char line[100]; /* Line buffer */
    FRESULT fr;     /* FatFs return code */


    /* Register work area to the default drive */
    f_mount(0,&FatFs);

    /* Open a text file */
    fr = f_open(&fil, "test.txt", FA_READ);
    if (fr!=FR_OK){
			f_open(&fil, "test.txt", FA_CREATE_ALWAYS);
			f_printf(&fil, "%s", "String");        /* "String" */
		}
			
    /* Read all lines and display it */
    while (f_gets(line, sizeof line, &fil)) {
        printf("%s",line);
    }

    /* Close the file */
    f_close(&fil);

    return 0;
}
