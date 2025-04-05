// 
// MMA7455 Driver : 3-axis Accelerometer
//
// CS: 0 = SPI, 1 = I2C

// MMA7455 pin connections
// pin1  VCC  :to Vcc5V
// pin2  NC	  : N.C.
// pin3  NC	  : N.C.
// pin4  INT1 : N.C.
// pin5  INT2 : N.C.
// pin6  GND  : to Gnd
// pin7  GND  : to Gnd
// pin8  CS   : to Vcc5V
// pin9  SDO  : N.C.
// pin10 SDA/SDI : to I2C0_SDA (GPA8)
// pin11 SCL  : to I2C0_SCL (GPA9)
// pin12 VCC  : to Vcc5V 

#include <stdio.h>
#include "SYS.h"
#include "GPIO.h"
#include "I2C.h"
#include "MMA7455_REG.h"

void init_MMA7455(void)
{
	I2C_Write(MMA7455_MCTL, 0x05); // 0x16 set to 0101 for +-2g mode
}

int8_t Read_DataX_MMA7455(void)
{
    return I2C_Read(MMA7455_XOUT8);
}

int8_t Read_DataY_MMA7455(void)
{
    return I2C_Read(MMA7455_YOUT8);
}

int8_t Read_DataZ_MMA7455(void)
{
    return I2C_Read(MMA7455_ZOUT8);
}

