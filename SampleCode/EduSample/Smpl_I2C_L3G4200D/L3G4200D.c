//
// L3G4200 Driver: 3-axis Gyroscope
//
// CS: 0 = SPI, 1 = I2C 
//
// L3G4200D pin connections
// pin1 SDO N.C.
// pin2 CS  to Vcc +5V 
// pin3 DR  N.C.
// pin4 INT N.C.
// pin5 GND to Gnd
// pin6 SDA to I2C0_SDA (GPA8)
// pin7 SCL to I2C0_SCL (GPA9)
// pin8 Vcc to Vcc5V

// I2C_Addr = SAD[6:1] + SDO + R/W
// when SDO no connection
// I2C slave ADDR = 0xD2 (Write)
// I2C slave ADDR = 0xD3 (Read)

#include <stdio.h>
#include <stdint.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "L3G4200D_REG.h"

void init_L3G4200D(void)
{
   I2C_Write(L3G4200D_CTRL_REG1, 0x0f);   // Xen,Yen,Zen,PD
   I2C_Write(L3G4200D_CTRL_REG2, 0x00);   // HPF
   I2C_Write(L3G4200D_CTRL_REG3, 0x08);   // 
   I2C_Write(L3G4200D_CTRL_REG4, 0x30);   //+-2000dps
   I2C_Write(L3G4200D_CTRL_REG5, 0x00);
}

uint16_t Read_DataX_L3G4200D(void)
{
	uint8_t LoByte, HiByte;
        LoByte=I2C_Read(L3G4200D_OUT_X_L); // read X_Low  value
        HiByte=I2C_Read(L3G4200D_OUT_X_H); // read X_High value
        return(HiByte<<8+LoByte);
}

uint16_t Read_DataY_L3G4200D(void)
{
	uint8_t LoByte, HiByte;
        LoByte=I2C_Read(L3G4200D_OUT_Y_L); // read X_Low  value
        HiByte=I2C_Read(L3G4200D_OUT_Y_H); // read X_High value
        return(HiByte<<8+LoByte);
}

uint16_t Read_DataZ_L3G4200D(void)
{
	uint8_t LoByte, HiByte;
        LoByte=I2C_Read(L3G4200D_OUT_Z_L); // read X_Low  value
        HiByte=I2C_Read(L3G4200D_OUT_Z_H); // read X_High value
        return(HiByte<<8+LoByte);
}
