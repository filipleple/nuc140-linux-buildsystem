/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "GPIO.h"
#include "I2C.h"
#include "I2S.h"
#include "USB.h"
#include "SYS.h"
#include "UACHIDSys.h"

#define USB_VID     0x0416  /* Vendor ID */
#define USB_PID     0x1000  /* Product ID */


#define HID_DEFAULT_INT_IN_INTERVAL  20


#if(PLAY_CHANNELS == 1)
#define PLAY_CH_CFG     1
#endif
#if(PLAY_CHANNELS == 2)
#define PLAY_CH_CFG     3
#endif

#if(REC_CHANNELS == 1)
#define REC_CH_CFG     1
#endif
#if(REC_CHANNELS == 2)
#define REC_CH_CFG     3
#endif


#define PLAY_RATE_LO    ( PLAY_RATE        & 0xFF)
#define PLAY_RATE_MD    ((PLAY_RATE >> 8)  & 0xFF)
#define PLAY_RATE_HI    ((PLAY_RATE >> 16) & 0xFF)

#define REC_RATE_LO     ( REC_RATE        & 0xFF)
#define REC_RATE_MD     ((REC_RATE >> 8)  & 0xFF)
#define REC_RATE_HI     ((REC_RATE >> 16) & 0xFF)

#define USB_VID_LO      ( USB_VID       & 0xFF)
#define USB_VID_HI      ((USB_VID >> 8) & 0xFF)

#define USB_PID_LO      ( USB_PID       & 0xFF)
#define USB_PID_HI      ((USB_PID >> 8) & 0xFF)


const uint8_t gau8DeviceDescriptor[] =
{
    LEN_DEVICE,                 /* bLength */
    DESC_DEVICE,                /* bDescriptorType */
    0x10, 0x01,                 /* bcdUSB */
    0x00,                       /* bDeviceClass */
    0x00,                       /* bDeviceSubClass */
    0x00,                       /* bDeviceProtocol */
    UAC_MAX_PACKET_SIZE_EP0,    /* bMaxPacketSize0 */
    USB_VID_LO,     
    USB_VID_HI,                 /* idVendor */
    USB_PID_LO, 
    USB_PID_HI,                 /* idProduct */
    0x00, 0x01,                 /* bcdDevice */
    0x01,                       /* iManufacture */
    0x02,                       /* iProduct */
    0x00,                       /* iSerialNumber */
    0x01                        /* bNumConfigurations */
};

const uint8_t g_HID_au8KeyboardReportDescriptor[] = 
{
      0x05, 0x01,
      0x09, 0x06,
      0xA1, 0x01,
      0x05, 0x07,
      0x19, 0xE0,
      0x29, 0xE7,
      0x15, 0x00,
      0x25, 0x01,
      0x75, 0x01,
      0x95, 0x08,
      0x81, 0x02,
      0x95, 0x01,
      0x75, 0x08,
      0x81, 0x01,
      0x95, 0x05,
      0x75, 0x01,
      0x05, 0x08,
      0x19, 0x01,
      0x29, 0x05,
      0x91, 0x02,             
      0x95, 0x01,             
      0x75, 0x03,             
      0x91, 0x01,
      0x95, 0x06,             
      0x75, 0x08,             
      0x15, 0x00,            
      0x25, 0x65,
      0x05, 0x07,
      0x19, 0x00,
      0x29, 0x65,
      0x81, 0x00,
      0xC0  
};

#define HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE \
    sizeof(g_HID_au8KeyboardReportDescriptor) / sizeof(g_HID_au8KeyboardReportDescriptor[0])
const uint32_t g_HID_u32KeyboardReportDescriptorSize = HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE;

#define HID_REPORT_DESCRIPTOR_SIZE   HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE


const uint8_t gau8ConfigDescriptor[] =
{
    LEN_CONFIG,     /* bLength */
    DESC_CONFIG,    /* bDescriptorType */
    0xD9,0x00,      /* wTotalLength */
    0x04,           /* bNumInterfaces */
    0x01,           /* bConfigurationValue */
    0x00,           /* iConfiguration */
    0x80,           /* bmAttributes */
    0x20,           /* Max power */

    /* Standard AC inteface */
    0x09,           /* bLength */
    0x04,           /* bDescriptorType */
    0x00,           /* bInterfaceNumber */
    0x00,           /* bAlternateSetting */
    0x00,           /* bNumEndpoints */
    0x01,           /* bInterfaceClass:AUDIO */
    0x01,           /* bInterfaceSubClass:AUDIOCONTROL */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */

    /* Class-spec AC interface descriptor */
    0x0A,           /* bLength */
    0x24,           /* bDescriptorType:CS_INTERFACE */
    0x01,           /* bDescriptorSubType:HEADER */
    0x00,0x01,      /* bcdADC:1.0 */
    0x46,0x00,      /* wTotalLength */
    0x02,           /* bInCollection */
    0x01,           /* baInterfaceNr(1) */
    0x02,           /* baInterfaceNr(n) */
    
    /* TID 1: Input for usb streaming */
    0x0C,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:INPUT_TERMINAL */
    0x01,               /* bTerminalID */
    0x01,0x01,          /* wTerminalType: 0x0101 usb streaming */
    0x00,               /* bAssocTerminal */
    PLAY_CHANNELS,      /* bNrChannels */
    PLAY_CH_CFG,0x00,   /* wChannelConfig */
    0x00,               /* iChannelNames */
    0x00,               /* iTerminal */

    /* UNIT ID 5: Feature Unit */
    0x08,               /* bLength */
    0x24,               /* bDescriptorType */
    0x06,               /* bDescriptorSubType */
    REC_FEATURE_UNITID, /* bUnitID */
    0x04,               /* bSourceID */
    0x01,               /* bControlSize */
    0x03,               /* bmaControls(0) */
    0x00,               /* iFeature */


    /* TID 2: Output Terminal for usb streaming */
    0x09,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x03,               /* bDescriptorSubType:OUTPUT_TERMINAL */
    0x02,               /* bTerminalID */
    0x01,0x01,          /* wTerminalType: 0x0101 usb streaming */
    0x00,               /* bAssocTerminal */
    REC_FEATURE_UNITID, /* bSourceID */
    0x00,               /* iTerminal */

    /* UNIT ID 6: Feature Unit */
    0x0A,               /* bLength */
    0x24,               /* bDescriptorType */
    0x06,               /* bDescriptorSubType */
    PLAY_FEATURE_UNITID, /* bUnitID */
    0x01,               /* bSourceID */
    0x01,               /* bControlSize */
    0x01,               /* bmaControls(0) */
    0x02,               /* bmaControls(0) */
    0x02,               /* bmaControls(0) */
    0x00,               /* iFeature */
    
    /* TID 3: Output for speaker */
    0x09,           /* bLength*/
    0x24,           /* bDescriptorType:CS_INTERFACE*/
    0x03,           /* bDescriptorSubType:OUTPUT_TERMINAL*/
    0x03,           /* bTerminalID*/
    0x01,0x03,      /* wTerminalType: 0x0301 speaker*/
    0x00,           /* bAssocTerminal*/
    0x06, /* bSourceID*/
    0x00,           /* iTerminal*/

    /* TID 4: Input Terminal for microphone */
    0x0C,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:INPUT_TERMINAL*/
    0x04,               /* bTerminalID*/
    0x01,0x02,          /* wTerminalType: 0x0201 microphone*/
    0x00,               /* bAssocTerminal*/
    REC_CHANNELS,       /* bNrChannels*/
    REC_CH_CFG, 0x00,   /* wChannelConfig*/
    0x00,               /* iChannelNames*/
    0x00,               /* iTerminal*/

    /* Standard AS interface 1, alternate 0 */
    0x09,           /* bLength */
    0x04,           /* bDescriptorType */
    0x01,           /* bInterfaceNumber */
    0x00,           /* bAlternateSetting */
    0x00,           /* bNumEndpoints */
    0x01,           /* bInterfaceClass:AUDIO */
    0x02,           /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */                     
    
    /* Standard AS interface 1, alternate 1 */
    0x09,           /* bLength */
    0x04,           /* bDescriptorType */
    0x01,           /* bInterfaceNumber */
    0x01,           /* bAlternateSetting */
    0x01,           /* bNumEndpoints */
    0x01,           /* bInterfaceClass:AUDIO */
    0x02,           /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */
    
    /* Class-spec AS interface, this interface's endpoint connect to TID 0x02 */
    0x07,           /* bLength */
    0x24,           /* bDescriptorType:CS_INTERFACE */
    0x01,           /* bDescriptorSubType:AS_GENERAL */
    0x02,           /* bTernimalLink */
    0x01,           /* bDelay */
    0x01,0x00,      /* wFormatTag:0x0001 PCM */
    
    /* Type I format type Descriptor */
    0x0B,           /* bLength */
    0x24,           /* bDescriptorType:CS_INTERFACE */
    0x02,           /* bDescriptorSubType:FORMAT_TYPE */
    0x01,           /* bFormatType:FORMAT_TYPE_I */
    REC_CHANNELS,   /* bNrChannels */
    0x02,           /* bSubFrameSize */
    0x10,           /* bBitResolution */
    0x01,           /* bSamFreqType : 0 continuous; 1 discrete */
    REC_RATE_LO,
    REC_RATE_MD,
    REC_RATE_HI,    /* Sample Frequency */
    
    /* Standard AS ISO Audio Data Endpoint */
    0x09,                       /* bLength */
    0x05,                       /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,   /* bEndpointAddress */
    0x05,                       /* bmAttributes */
    ISO_IN_MXPLD,0x00,          /* wMaxPacketSize*/
    0x01,                       /* bInterval*/
    0x00,                       /* bRefresh*/
    0x00,                       /* bSynchAddress*/
    
    /* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,           /* bLength */
    0x25,           /* bDescriptorType:CS_ENDPOINT */
    0x01,           /* bDescriptorSubType:EP_GENERAL */
    0x00,           /* bmAttributes */
    0x00,           /* bLockDelayUnits */
    0x00, 0x00,     /* wLockDelay */

    /* Standard AS interface 2, alternate 0 */
    0x09,           /* bLength */
    0x04,           /* bDescriptorType */
    0x02,           /* bInterfaceNumber */
    0x00,           /* bAlternateSetting */
    0x00,           /* bNumEndpoints */
    0x01,           /* bInterfaceClass:AUDIO */
    0x02,           /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */

    /* Standard AS interface 2, alternate 1 */
    0x09,           /* bLength */
    0x04,           /* bDescriptorType */
    0x02,           /* bInterfaceNumber */
    0x01,           /* bAlternateSetting */
    0x01,           /* bNumEndpoints */
    0x01,           /* bInterfaceClass:AUDIO */
    0x02,           /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */
    
    /* Class-spec AS inf this interface's endpoint connect to TID 0x01 */
    0x07,           /* bLength */
    0x24,           /* bDescriptorType:CS_INTERFACE */
    0x01,           /* bDescriptorSubType:AS_GENERAL */
    0x01,           /* bTernimalLink */
    0x01,           /* bDelay */
    0x01,0x00,      /* wFormatTag:0x0001 PCM */   

    /* Type I format type Descriptor */
    0x0B,           /* bLength */
    0x24,           /* bDescriptorType:CS_INTERFACE */
    0x02,           /* bDescriptorSubType:FORMAT_TYPE */
    0x01,           /* bFormatType:FORMAT_TYPE_I */
    PLAY_CHANNELS,  /* bNrChannels */
    0x02,           /* bSubFrameSize */
    0x10,           /* bBitResolution */
    0x01,           /* bSamFreqType : 0 continuous; 1 discrete */
    PLAY_RATE_LO,
    PLAY_RATE_MD,
    PLAY_RATE_HI,   /* Sample Frequency */
    
    /* Standard AS ISO Audio Data Endpoint, output, addtess 2, Max 0x40 */
    0x09,                       /* bLength */
    0x05,                       /* bDescriptorType */
    ISO_OUT_EP_NUM | EP_OUTPUT, /* bEndpointAddress */
    0x0d,                       /* bmAttributes */
    ISO_OUT_MXPLD, 0x00,        /* wMaxPacketSize */
    0x01,                       /* bInterval */
    0x00,                       /* bRefresh */
    0x00,                       /* bSynchAddress */
       
    /* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,           /* bLength */
    0x25,           /* bDescriptorType:CS_ENDPOINT */
    0x01,           /* bDescriptorSubType:EP_GENERAL */
    0x80,           /* bmAttributes */
    0x00,           /* bLockDelayUnits */
    0x00, 0x00,     /* wLockDelay */

    // I/F descr: HID
    LEN_INTERFACE,  // bLength
    DESC_INTERFACE, // bDescriptorType
    0x03,           // bInterfaceNumber
    0x00,           // bAlternateSetting
    0x01,           // bNumEndpoints
    0x03,           // bInterfaceClass
    //HID_FUNCTION,         // bInterfaceSubClass
    //HID_FUNCTION,         // bInterfaceProtocol
    0x01,           // bInterfaceSubClass
    0x01,           // bInterfaceProtocol   
    0x00,           // iInterface

    // HID Descriptor
    LEN_HID,        // Size of this descriptor in UINT8s.
    DESC_HID,       // HID descriptor type.
    0x10, 0x01,     // HID Class Spec. release number.
    0x00,           // H/W target country.
    0x01,           // Number of HID class descriptors to follow.
    DESC_HID_RPT,   // Dscriptor type.
    
    /* Total length of report descriptor */
    HID_REPORT_DESCRIPTOR_SIZE & 0x00FF,
    (HID_REPORT_DESCRIPTOR_SIZE & 0xFF00) >> 8,

    /* EP Descriptor: interrupt in */
    LEN_ENDPOINT,   // bLength
    DESC_ENDPOINT,  // bDescriptorType
    (HID_IN_EP_NUM | EP_INPUT),         // bEndpointAddress
    EP_INT,         // bmAttributes
    // wMaxPacketSize
    HID_IN_MXPLD & 0x00FF,
    (HID_IN_MXPLD & 0xFF00) >> 8,
    HID_DEFAULT_INT_IN_INTERVAL     // bInterval
};

const uint8_t gau8StringLang[4] =
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

const uint8_t gau8StringSerial[26] =
{
    26,             /* bLength */
    DESC_STRING,    /* bDescriptorType */
    'B', 0, 
    '0', 0, 
    '2', 0, 
    '0', 0, 
    '0', 0, 
    '8', 0, 
    '0', 0, 
    '3', 0, 
    '2', 0, 
    '1', 0, 
    '1', 0, 
    '5', 0
};

const uint8_t gau8StringAudio[] =
{
    0x1E,           /* bLength */
    DESC_STRING,    /* bDescriptorType */
    'U', 0, 
    'S', 0, 
    'B', 0, 
    ' ', 0, 
    'M', 0, 
    'i', 0, 
    'c', 0, 
    'r', 0, 
    'o', 0, 
    'p', 0, 
    'h', 0, 
    'o', 0, 
    'n', 0, 
    'e', 0
};

const uint8_t gau8VendorStringDescriptor[] = 
{
    16,
    DESC_STRING,
    'N', 0, 
    'u', 0, 
    'v', 0, 
    'o', 0, 
    't', 0, 
    'o', 0, 
    'n', 0
};

const uint8_t gau8ProductStringDescriptor[] = 
{
    42,
    DESC_STRING,
    'U', 0, 
    'S', 0, 
    'B', 0, 
    ' ', 0, 
    'A', 0, 
    'u', 0, 
    'd', 0, 
    'i', 0, 
    'o', 0,
    '&', 0,
    'H', 0,
    'I', 0,
    'D', 0,
    ' ', 0,
    'D', 0,
    'e', 0,
    'v', 0,
    'i', 0,
    'c', 0,
    'e', 0
};


uint8_t g_au8KeyboardReport[8];
const uint32_t g_u32KeyboardReportSize = sizeof(g_au8KeyboardReport) / sizeof(g_au8KeyboardReport[0]);

void HID_GetInReport(uint8_t *buf);

 
/* For I2C transfer */
__IO uint32_t EndFlag0 = 0;
uint8_t Device_Addr0 = 0x1A;                /* WAU8822 Device ID */
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

static void RoughDelay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;
    while(delay-- >= 0);
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      I2C0_Callback_Tx()                                                                                 */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function is to send Device ID and data to CODEC with I2C0                             */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_Callback_Tx(uint32_t status)
{
    if (status == 0x08)                     /* START has been transmitted */
    {
        DrvI2C_WriteData(I2C_PORT0, Device_Addr0<<1);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }   
    else if (status == 0x18)                /* SLA+W has been transmitted and ACK has been received */
    {
        DrvI2C_WriteData(I2C_PORT0, Tx_Data0[DataCnt0++]);
        DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 0);
    }
    else if (status == 0x20)                /* SLA+W has been transmitted and NACK has been received */
    {
        
        DrvI2C_Ctrl(I2C_PORT0, 1, 1, 1, 0);
    }   
    else if (status == 0x28)                /* DATA has been transmitted and ACK has been received */
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
/*      I2C_WriteWAU8822()                                                                                 */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The function is to write 9-bit data to 7-bit address register of WAU8822 with I2C interface.       */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
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
/*      WAU8822_Setup()                                                                                    */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The function is to configure CODEC WAU8822 with I2C interface.                                     */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
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
    I2C_WriteWAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */         
    I2C_WriteWAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */
    
    I2C_WriteWAU8822(6,  0x1AD);   /* Divide by 6, 16K */
    I2C_WriteWAU8822(7,  0x006);   /* 16K for internal filter cofficients */

    I2C_WriteWAU8822(10, 0x008);   /* DAC softmute is disabled, DAC oversampling rate is 128x */
    I2C_WriteWAU8822(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */
    I2C_WriteWAU8822(15, 0x1EF);   /* ADC left digital volume control */
    I2C_WriteWAU8822(16, 0x1EF);   /* ADC right digital volume control */
        
    I2C_WriteWAU8822(44, 0x000);   /* LLIN/RLIN is not connected to PGA */
    I2C_WriteWAU8822(47, 0x050);   /* LLIN connected, and its Gain value */
    I2C_WriteWAU8822(48, 0x050);   /* RLIN connected, and its Gain value */

    I2C_WriteWAU8822(50, 0x001);   /* Left DAC connected to LMIX */
    I2C_WriteWAU8822(51, 0x001);   /* Right DAC connected to RMIX */

    I2C_WriteWAU8822(54, 0x133);   /* LSPKOUT Volume */
    I2C_WriteWAU8822(55, 0x133);   /* RSPKOUT Volume */
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      Tx_thresholdCallbackfn()                                                                           */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function when Data in Tx FIFO is less than Tx FIFO Threshold Level.                   */
/*      It is used to transfer data in Play Buffer to Tx FIFO.                                             */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      status    I2S Status register value.                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void Tx_thresholdCallbackfn(uint32_t status)
{
    uint32_t i;
    uint32_t * pBuff;
    
    if ((u32PlayPos_Out < u32PlayPos_In - 4) || (u32PlayPos_Out > u32PlayPos_In))
    {
        pBuff = &PcmPlayBuff[u32PlayPos_Out];

        for ( i = 0; i < 4; i++)
        {
            _DRVI2S_WRITE_TX_FIFO(pBuff[i]);
        }
        
        u32PlayPos_Out += 4;
        
        if (u32PlayPos_Out >= BUFF_LEN)
        {
            u32PlayPos_Out = 0;
        }   
    }
    else
    {
        for ( i = 0; i < 4; i++)
        {
            _DRVI2S_WRITE_TX_FIFO(0x00);       
        }
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      Rx_thresholdCallbackfn()                                                                           */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function when Data in Rx FIFO is more than Rx FIFO Threshold Level.                   */
/*      It is used to transfer data in Rx FIFO to Recode Buffer                                            */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      status    I2S Status register value.                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void Rx_thresholdCallbackfn(uint32_t status)
{
    int32_t  i;
    uint16_t *pBuff;
    
    pBuff = &PcmRecBuff[u32RecPos_In];
    
    for ( i = 0; i < 4; i++ )
    {
        pBuff[i] = (uint16_t)(_DRVI2S_READ_RX_FIFO() & 0xFFFF);
    }
    
    u32RecPos_In += 4;
    
    if (u32RecPos_In >= BUFF_LEN)
    {
        u32RecPos_In = 0;
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      SendRecData()                                                                                      */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function when ISO IN ack. It could be used to prepare the record data for next ISO IN */
/*      transfer.                                                                                          */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void SendRecData(void)
{
    uint16_t *pBuff;

    if ((u32RecPos_Out < u32RecPos_In - REC_LEN) || (u32RecPos_Out > u32RecPos_In))
    {
        pBuff = &PcmRecBuff[u32RecPos_Out];
        
        UAC_SendOneAudioPacket((uint8_t *)pBuff, REC_LEN * 2);
        
        u32RecPos_Out += REC_LEN;
        
        if (u32RecPos_Out >= BUFF_LEN)
        {
            u32RecPos_Out = 0;
        }
    }
    else
    {
        UAC_SendOneAudioPacket((uint8_t *)pBuff, 0);
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      DeviceEnable()                                                                                     */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      To enable the device to play or record audio data.                                                 */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      u8Object    To select the device. It could be UAC_MICROPHONE or UAC_SPEAKER.                       */
/*                                                                                                         */
/*      u8State     To indicate the calling stage. 0 = Call by set interface. 1 = Call by first ISO IN ack.*/
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DeviceEnable(uint8_t u8Object, uint8_t u8State)
{

    if (u8Object == UAC_MICROPHONE)
    {
        /* Enable record hardware */
        DrvI2S_EnableInt(I2S_RX_FIFO_THRESHOLD, Rx_thresholdCallbackfn);
        DrvI2S_EnableInt(I2S_RX_UNDERFLOW, NULL);
        DrvI2S_EnableInt(I2S_RX_OVERFLOW, NULL);
        DrvI2S_EnableRx();
    }
    else
    {
        /* Eanble play hardware */
        DrvI2S_EnableInt(I2S_TX_FIFO_THRESHOLD, Tx_thresholdCallbackfn);
        DrvI2S_EnableInt(I2S_TX_UNDERFLOW, NULL);
        DrvI2S_EnableInt(I2S_TX_OVERFLOW, NULL);
        DrvI2S_EnableTx();
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      DeviceDisable()                                                                                    */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      To disable the device to play or record audio data.                                                */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      u8Object    To select the device. It could be UAC_MICROPHONE or UAC_SPEAKER.                       */
/*                                                                                                         */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DeviceDisable(uint8_t u8Object)
{

    if (u8Object ==  UAC_MICROPHONE )
    {
        /* Disable record hardware/stop record */
        DrvI2S_DisableInt(I2S_RX_FIFO_THRESHOLD);
        DrvI2S_DisableRx();
    }
    else
    {
        /* Disable play hardware/stop play */
        DrvI2S_DisableInt(I2S_TX_FIFO_THRESHOLD);
        DrvI2S_DisableTx();
    }   
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      GetPlayData()                                                                                      */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      To get data from ISO OUT to play buffer.                                                           */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      pi16src    The data buffer of ISO OUT.                                                             */
/*      i16Samples The sample number in data buffer.                                                       */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void GetPlayData(int16_t *pi16src, int16_t i16Samples)
{
    /* Get play data from ISO out packets */
    uint32_t u32len, i;
    uint32_t *pBuff;
    uint32_t *pSrc;

    u32len = PLAY_RATE/1000;
        
    pBuff = &PcmPlayBuff[u32PlayPos_In];
    pSrc = (uint32_t *) pi16src;
        
    for ( i = 0; i < u32len; i++ )
    {
        pBuff[i] = pSrc[i];
    }
    
    u32PlayPos_In += u32len;
    
    if (u32PlayPos_In >= BUFF_LEN)
    {
        u32PlayPos_In = 0;
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      UAC_MainProcess()                                                                                  */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The main loop of UAC funciton.                                                                     */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      None                                                                                               */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                                                                                               */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
int32_t UACHID_MainProcess(void)
{
    E_DRVUSB_STATE eUsbState;
    S_DRVI2S_DATA_T st;
    int32_t i32Ret = E_SUCCESS;

    
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
    
    /* Select I2S Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_I2S_CLKSRC, 0);    

    /* Configure I2S */
    st.u32SampleRate     = 16000;
    st.u8WordWidth       = DRVI2S_DATABIT_16;
    st.u8AudioFormat     = DRVI2S_STEREO;       
    st.u8DataFormat      = DRVI2S_FORMAT_I2S;   
    st.u8Mode            = DRVI2S_MODE_SLAVE;
    st.u8TxFIFOThreshold = DRVI2S_FIFO_LEVEL_WORD_4;
    st.u8RxFIFOThreshold = DRVI2S_FIFO_LEVEL_WORD_4;
        
    DrvI2S_Open(&st);
    
    /* Configure CODEC */
    WAU8822_Setup();

    /* Set I2S I/O */
    DrvGPIO_InitFunction(E_FUNC_I2S); 

    /* Set MCLK and enable MCLK */
    DrvI2S_SetMCLKFreq(12000000);   
    DrvI2S_EnableMCLK(); 
 
    /* Install the UAC control callbacks */
    UAC_Init(DeviceEnable, DeviceDisable, SendRecData, GetPlayData);

    if (i32Ret == E_SUCCESS)
    {
        /* Install the USB event handler to USB event callback */       
        i32Ret = DrvUSB_Open((void *)DrvUSB_DispatchEvent);
        HID_SetReportDescriptor(g_HID_au8KeyboardReportDescriptor, g_HID_u32KeyboardReportDescriptorSize);
        
        /* Set the HID report buffer */
        HID_SetReportBuf(g_au8KeyboardReport, g_u32KeyboardReportSize);
        UAC_Open((void *)HID_GetInReport);
        
               
        /* If the USB has been attached already, we should force the bus reset. */
        eUsbState = DrvUSB_GetUsbState();
        if (eUsbState >= eDRVUSB_ATTACHED)
        {
            /* Force Bus Reset for 150 ms*/
            _DRVUSB_ENABLE_SE0();
            RoughDelay(150000);
            _DRVUSB_DISABLE_SE0();
        }
                    
        // Disable USB-related interrupts.
        _DRVUSB_ENABLE_MISC_INT(0);

        // Enable float-detection interrupt.
        _DRVUSB_ENABLE_FLDET_INT();
        
        _DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);
 
        while(1)
        {      
            //DrvUSB_DispatchEvent();

            eUsbState = DrvUSB_GetUsbState();
            
            if (eUsbState == eDRVUSB_DETACHED)
            {
                /* Just waiting for USB attach */
                while (eUsbState == eDRVUSB_DETACHED)
                {
                    eUsbState = DrvUSB_GetUsbState();
                }
            }
        }
    }
    
    return 0;
}

void HID_GetInReport(uint8_t *buf)
{
    int32_t i;
    uint32_t key;
    
    key = GPIOA->PIN;
    {
        /* No any key pressed */
        if (((key >> 3) & 0xF) == 0xF)
        {
            for(i=0;i<8;i++)
            {
                buf[i] = 0;
            }
        }
        
        /* Key A (GPIOA3) Pressed */
        if ((key & (1<<3)) == 0)
        {
            buf[2] = 0x04; /* a */
        }
        
        /* Key B (GPIOA4) Pressed */
        if ((key & (1<<4)) == 0)
        {
            buf[2] = 0x05; /* b */   
        }
        
        /* Key C (GPIOA5) Pressed */
        if ((key & (1<<5)) == 0)
        {
            buf[2] = 0x06; /* c */ 
        }
        
        /* Key D (GPIOA6) Pressed */
        if ((key & (1<<6)) == 0)
        {
            /* abc */
            buf[2] = 0x04;
            buf[3] = 0x05;
            buf[4] = 0x06;
        }
    }
}

