// ADXL345 : Analog Device 3-axis accelerometer
// Interface: I2C/SPI (CS Hi/Lo)
// pin1: Gnd to Vss  (NUC123 pin15)
// pin2: Vcc to Vdd  (NUC123 pin16)
// pin3: CS  to Vdd  (Hi for I2C)          
// pin4: INT1-- N.C.
// pin5: INT2-- N.C.
// pin6: SDO to Gnd  (ALT_Address =0)
// pin7: SDA to SDA  (NUC123 pin4, NUC140 GPA10)
// pin8: SCL to SCLK (NUC123 pin5, NUC140 GPA11)

#include <stdio.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "ADXL345_REG.h"


void init_ADXL345(void)
{
   I2C_Write(ADXL345_DATA_FORMAT,0x0B); 
   I2C_Write(ADXL345_BW_RATE,0x0F);
   I2C_Write(ADXL345_POWER_CTL,0x08);
   I2C_Write(ADXL345_INT_ENABLE,0x80);
   I2C_Write(ADXL345_OFSX,0x00);
   I2C_Write(ADXL345_OFSY,0x00);
   I2C_Write(ADXL345_OFSZ,0x05);
}

uint16_t Read_DataX_ADXL345(void)
{
    uint8_t  LoByte, HiByte;
    LoByte=I2C_Read(ADXL345_DATAX0);
    HiByte=I2C_Read(ADXL345_DATAX1);
    return(HiByte<<8 + LoByte);
}

uint16_t Read_DataY_ADXL345(void)
{
    uint8_t  LoByte, HiByte;
    LoByte=I2C_Read(ADXL345_DATAY0);
    HiByte=I2C_Read(ADXL345_DATAY1);
    return(HiByte<<8 + LoByte);
}

uint16_t Read_DataZ_ADXL345(void)
{
    uint8_t  LoByte, HiByte;
    LoByte=I2C_Read(ADXL345_DATAY0);
    HiByte=I2C_Read(ADXL345_DATAY1);
    return(HiByte<<8 + LoByte);
}
