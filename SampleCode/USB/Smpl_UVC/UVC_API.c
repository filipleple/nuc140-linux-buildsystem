/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "USB.h"
#include "UVCSys.h"

extern char imageBegin0, imageBegin1, imageEnd;
uint8_t *imageTbl[] = {(uint8_t *)&imageBegin0, (uint8_t *)&imageBegin1, (uint8_t *)&imageEnd};

#define JPEG1_SIZE 6720
#define JPEG2_SIZE 6005

uint32_t g_u32Togglebit=0;
uint32_t g_u32JpegIndex=0;
uint32_t g_u32RemSize=JPEG1_SIZE;

#define USB_VID		0x0416  /* Vendor ID */
#define USB_PID		0x1100  /* Product ID */


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


#define PLAY_RATE_LO    (PLAY_RATE & 0xFF)
#define PLAY_RATE_MD    ((PLAY_RATE >> 8) & 0xFF)
#define PLAY_RATE_HI    ((PLAY_RATE >> 16) & 0xFF)

#define REC_RATE_LO     (REC_RATE & 0xFF)
#define REC_RATE_MD     ((REC_RATE >> 8) & 0xFF)
#define REC_RATE_HI     ((REC_RATE >> 16) & 0xFF)

#define USB_VID_LO      (USB_VID & 0xFF)
#define USB_VID_HI      ((USB_VID >> 8) & 0xFF)

#define USB_PID_LO      (USB_PID & 0xFF)
#define USB_PID_HI      ((USB_PID >> 8) & 0xFF)

#define UAVC_PROCESSING_UNITID 0x05
#define	UAVC_MAX_PACKET_SIZE_EP0		64
#define UAVC_ISO_PACKET_1               0x1D8

__align(4) const uint8_t gau8DeviceDescriptor[] =
{
	LEN_DEVICE,		/* bLength */
	DESC_DEVICE,	/* bDescriptorType */
	0x00, 0x02,		/* bcdUSB */
	0xEF,			/* bDeviceClass */
	0x02,			/* bDeviceSubClass */
	0x01,			/* bDeviceProtocol */
	UAVC_MAX_PACKET_SIZE_EP0,	/* bMaxPacketSize0 */
	/* idVendor */
	USB_VID & 0x00FF,
	(USB_VID & 0xFF00) >> 8,
	/* idProduct */
	USB_PID & 0x00FF,
	(USB_PID & 0xFF00) >> 8,

	0x00, 0x01,     /* bcdDevice */
	0x01,           /* iManufacture */
	0x02,           /* iProduct */
	0x00,           /* 0x03 iSerialNumber */
	0x01            /* bNumConfigurations */
};


__align(4) const uint8_t gau8ConfigDescriptor[] =
{
	0x09,   /* bLength */
	0x02,   /* bDescriptorType */
//    0x6F, 0x01,  /* total length */
//    0x80, 0x01,
    0x88, 0x01,


	0x04,			/* bNumInterfaces */
	0x01,			/* bConfigurationValue */
	0x00,			/* iConfiguration */
	0xC0,			/* bmAttributes */
	0x20,			/* MaxPower */

/*  Standard Video Interface Collection IAD(interface Association Descriptor) */
    0x08,   /* bLength */
    0x0B,   /* bDescriptorType */
    0x00,   /* bFirstInterface */
    0x02,   /* bInterfaceCount */
    0x0E,   /* bFunctionClass */
    0x03,   /* bFunctionSubClass */
    0x00,   /* bFunctionProtocol */
    0x02,   /* iFunction */

/* Standard VideoControl Interface Descriptor */
    0x09,   /* bLength */
    0x04,   /* bDescriptorType */
    0x00,   /* bInterfceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x0E,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x00,   /* bInterfaceProtocol */
    0x02,   /* iInterface */
   
/* Class-specific VideoControl Interface Descriptor */
    0x0D,                   /* bLength */
    0x24,                   /* bDescriptorType */
    0x01,                   /* bDescriptorSubType */
    0x00,0x01,              /* bcdUAVC */
    0x42,0x00,              /* wTotalLength */

    0x80, 0x8D, 0x5B, 0x00, /* dwClockFrequency */
    0x01,                   /* bInCollection */
    0x01,                   /* baInterfaceNr */

/* Input Terminal Descriptor (Camera) */
    0x11,        /* bLength */
    0x24,        /* bDescriptorType */
    0x02,        /* bDescriptorSubType */
    0x01,        /* bTerminalID */
    0x01, 0x02,  /* wTerminalType */
    0x00,        /* bAssocTerminal */
    0x00,        /* iTerminal */
    0x00, 0x00,  /* wObjectiveFocalLengthMin */
    0x00, 0x00,  /* wObjectiveFocalLengthMax */
    0x00, 0x00,  /* wOcularFocalLength */
    0x02,        /* bControlSize */
    0x00, 0x00,  /* bmControls */
//    0x08, 0x00, /* Exposure time */

/* Input Terminal Descriptor */
    0x08,
    0x24,
    0x02,
    0x02,
//  0x01, 0x04,
//  0x01,0x02,
    0x00, 0x02,
    0x00,
    0x00,


/*  Output Terminal Descriptor */
    0x09,      /* bLength */
    0x24,      /* bDescriptorType */
    0x03,      /* bDescriptorSubType */
    0x03,      /* bTerminalID */
    0x01, 0x01,/* wTerminalType */
    0x00,      /* bAssocTerminal */
    UAVC_PROCESSING_UNITID, /* bSourceID */
    0x00,      /* iTerminal */

/*  Selector Unit */
    0x08,
    0x24,
    0x04,
    0x04,
    0x02,
    0x01,
    0x02,
    0x00,
    

/*  Processing Uint Descriptor  */
    0x0B,       /* bLength */
    0x24,       /* bDescriptorType */
    0x05,       /* bDescriptorSubType */
    UAVC_PROCESSING_UNITID,  /* bUnitID */
    0x04,       /* bSourceID */

    0x00, 0x00, /* wMaxMultiplier */
    0x02,       /* bControlSize */
    0x4F, 0x00, /* bmControls */
    0x00,       /* iProcessing */

/*  Standard Interrupt Endpoint Descriptor */
    0x07,       /* bLength */
    0x05,       /* bDescriptorType */
    0x83,       /* bEndpointAddress */
    0x03,       /* bmAttributes */
    0x08, 0x00, /* wMaxPacketSize */
    0x20,       /* bInterval */

/*  Class-specific Interrupt Endpoint Descriptor */
    0x05,   /* bLength */
    0x25,   /* bDescriptorType */
    0x03,   /* bDescriptorSubType */
    0x08,0x00,

/*  Standard VideoStreaming Interface Descriptor */
    0x09,   /* bLength */
    0x04,   /* bDescriptorType */
    0x01,   /* bInterfceNumber */
    0x00,   /* bAlternateSetting */
    0x00,   /* bNumEndpoints */
    0x0E,   /* bInterfaceClass */
    0x02,   /* bInterfaceSubClass */
    0x00,   /* bInterfaceProtocol */
    0x00,   /* iInterface */

/*  Class-specific VideoStreaming Header Descriptor */
    0x0E,        /* bLength */
    0x24,        /* bDescriptorType */
    0x01,        /* bDescriptorSubType */
    
    0x01,        /* bNumFormats, MJPEG and still format */
    
    0xA4, 0x00,
    
    0x84,        /* bEndpointAddress */
    0x00,        /* bmInfo */
    0x03,        /* bTerminalLink */
    
    0x02,        /* bStillCaptureMethod, method 2 */
    
    0x01,        /* bTriggerSupport */
    0x00,        /* bTriggerUsage */
    0x01,        /* bControlSize */
    0x00,        /* bmaControls(1) */
//    0x00,      /* bmaControls(2) */


/* Class-specific VideoStreaming Format Descriptor */
    0x0B,   /* bLength */
    0x24,   /* bDescriptorType */
    0x06,   /* bDescriptorSubType */
    0x01,   /* bFormatIndex */
    0x03,   /* bNumFrameDescriptors */
    0x01,   /* bmFlags */
    0x01,   /* bDefaultFrameIndex */
    0x00,   /* bAspectRatioX */
    0x00,   /* bAspectRatioY */
    0x00,   /* bmInterlaceFlags */
    0x00,   /* bCopyProtect */

/* Class-specific VideoStreaming Frame 3 Descriptor */
    0x26,           /* bLength */
    0x24,           /* bDescriptorType */
    0x07,           /* bDescriptorSubType */
    0x01,           /* bFrameIndex */
    0x00,           /* bmCapabilities */

    0x40, 0x01,     /* 320 */

    0xF0, 0x00,     /* wHeight->240 */
    
    0x00, 0x00, 0x03, 0x00,
    
    0x00, 0x00, 0x5A, 0x00,
    
    0x00, 0x60, 0x00, 0x00,
    
    0x15, 0x16, 0x05, 0x00,

    0x00,           /* bFrameIntervalType */

    0x15, 0x16, 0x05, 0x00,

    0x15, 0x16, 0x05, 0x00,

    0x00, 0x00, 0x00, 0x00,


    0x26,           /* bLength */
    0x24,           /* bDescriptorType */
    0x07,           /* bDescriptorSubType */
    0x02,           /* bFrameIndex */
    0x00,           /* bmCapabilities */
    0xA0, 0x00,
    0x78, 0x00,

    0x00, 0x80, 0x01, 0x00,
    0x00, 0x00, 0x2D, 0x00,
    0x00, 0x30, 0x00, 0x00,

    0x15, 0x16, 0x05, 0x00,
    0x00,           /* bFrameIntervalType */

    0x15, 0x16, 0x05, 0x00,

    0x15, 0x16, 0x05, 0x00,

    0x00, 0x00, 0x00, 0x00,

 
    0x26,           /* bLength */
    0x24,           /* bDescriptorType */
    0x07,           /* bDescriptorSubType */
    0x03,           /* bFrameIndex */
    0x00,           /* bmCapabilities */

    0x80, 0x02,     /* 320 */

    0xE0, 0x01,     /* wHeight->240 */

    0x00, 0x60, 0x09, 0x00,

    0x00, 0xA0, 0x8C, 0x00,

    0x00, 0x2C, 0x01, 0x00,
    
    0x2A, 0x2C, 0x0A, 0x00,

    0x00,           /* bFrameIntervalType */

    0x2A, 0x2C, 0x0A, 0x00,

    0x2A, 0x2C, 0x0A, 0x00,


    0x00, 0x00, 0x00, 0x00,    

/* still image */
     0x13,
     0x24, 
     0x03,
     0x00,     
     0x03, 

     0x40, 0x01,       /* 320 */
     0xF0, 0x00,       /* wHeight->240 */
     0xA0, 0x00,
     0x78, 0x00,
     0x80, 0x02,
     0xE0, 0x01,
      
     0x01,
     0x63,


    
/* color match */
     0x06, 
     0x24,
     0x0D,
     0x01,
     0x01,
     0x04,
 
/* Standard VideoStreaming Interface Descriptor */
    0x09,   /* bLength */
    0x04,   /* bDescriptorType */
    0x01,   /* bInterfceNumber */
    0x01,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x0E,   /* bInterfaceClass */
    0x02,   /* bInterfaceSubClass */
    0x00,   /* bInterfaceProtocol */
    0x00,   /* iInterface */

/* Standard VideoStreaming Iso Video Data Endpoint Descriptor */
    0x07,   /* bLength */
    0x05,   /* bDescriptorType */
    0x84,
    0x05,   /* bmAttributes */
    UAVC_ISO_PACKET_1 & 0xFF,
    (UAVC_ISO_PACKET_1 & 0xFF00) >> 8,
    0x01,   /* bInterval */
    0x08,
    0x0B,
    0x02,
    0x02,
    0x01,
    0x00,
    0x00,
    0x00,

	
	/* interface 0,	alt 0 */
	0x09, 0x04, 0x02, 0x00,	0x00, 0x01, 0x01, 0x00,	0x00,                   /* Standard AC inteface */
	0x09, 0x24, 0x01, 0x00,	0x01, 0x1e, 0x00, 0x01,	0x03,                   /* Class-spec AC inf des */
	0x0c, 0x24, 0x02, 0x01,	0x01, 0x02, 0x00, 0x01,	0x00, 0x00, 0x00, 0x00, /* Input Terminal des */
	0x09, 0x24, 0x03, 0x02,	0x01, 0x01, 0x00, 0x01,	0x00,                   /* Output Terminal des */
	
	0x09, 0x04, 0x03, 0x00,	0x00, 0x01, 0x02, 0x00,	0x00,                   /* Standard AS inf 2, alt 0 */
	0x09, 0x04, 0x03, 0x01,	0x01, 0x01, 0x02, 0x00,	0x00,                   /* Standard AS inf 2, alt 1 */
	0x07, 0x24, 0x01, 0x02,	0x01, 0x01, 0x00,                               /* Class-spec AS inf */
	
	0x0b, 0x24, 0x02, 0x01,	0x01, 0x02, 0x10, 0x01,	0x40, 0x1F, 0x00,       /* Type I format type	Des (8K) */
	0x09, 0x05, 0x85, 0x01,	0x10, 0x00, 0x01, 0x00,	0x00,                   /* Standard Endpoint Des */
	0x07, 0x25, 0x01, 0x00,	0x00, 0x00, 0x00,                               /* Class-spec Iso Audio	data endp Des */


};


__align(4) const uint8_t gau8StringLang[4] =
{
	4,				/* bLength */
	DESC_STRING,	/* bDescriptorType */
	0x09, 0x04
};

__align(4) const uint8_t gau8StringSerial[26] =
{
	26,				/* bLength */
	DESC_STRING,	/* bDescriptorType */
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

__align(4) const uint8_t gau8StringAudio[] =
{
	0x1E,			/* bLength */
	DESC_STRING,	/* bDescriptorType */
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

__align(4) const uint8_t gau8VendorStringDescriptor[] = 
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

__align(4) const uint8_t gau8ProductStringDescriptor[] = {
	20,
	DESC_STRING,
	'U', 0, 
	'S', 0, 
	'B', 0, 
	' ', 0, 
	'A', 0, 
	'u', 0, 
	'd', 0, 
	'i', 0, 
	'o', 0
};
 
int8_t gIsPlaying = 0;

static void RoughDelay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;

    while(delay-- >= 0);
}

/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      SendData()                                                                                         */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The callback function when ISO IN ack. It could be used to prepare the data for next ISO IN        */
/*      transfer.                                                                                          */
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
void SendData(void)
{  

    static int8_t jpgBuf[64] = {0};
    uint32_t u32ByteCount;
		 
	jpgBuf[0] = 0x02;
	jpgBuf[1] = 0x80;


	if( (g_u32JpegIndex==0) || (g_u32RemSize>62) )
	{	 
		if(g_u32Togglebit &  0x1)
		{
			for(u32ByteCount=2; u32ByteCount<64; u32ByteCount++)
				jpgBuf[u32ByteCount] = *(imageTbl[0]+g_u32JpegIndex++);
		}
		else
		{
			for(u32ByteCount=2; u32ByteCount<64; u32ByteCount++)
				jpgBuf[u32ByteCount] = *(imageTbl[1]+g_u32JpegIndex++);
				
		}
		g_u32RemSize -= 62;
		UVC_SendOnePacket((uint8_t *)jpgBuf, ISO_IN_MXPLD);
	}
	else
	{
		jpgBuf[1] = 0x82 | (g_u32Togglebit & 0x1);
		
		if(g_u32Togglebit &  0x1)
		{
			for(u32ByteCount=0; u32ByteCount<g_u32RemSize; u32ByteCount++)
				jpgBuf[u32ByteCount+2] = *(imageTbl[0]+g_u32JpegIndex++);
			UVC_SendOnePacket((uint8_t *)jpgBuf, g_u32RemSize+2);
			g_u32RemSize = JPEG2_SIZE;
			g_u32JpegIndex = 0;
		}
		else
		{
			for(u32ByteCount=0; u32ByteCount<g_u32RemSize; u32ByteCount++)
				jpgBuf[u32ByteCount+2] = *(imageTbl[1]+g_u32JpegIndex++);
			UVC_SendOnePacket((uint8_t *)jpgBuf, g_u32RemSize+2);
			g_u32RemSize = JPEG1_SIZE;
			g_u32JpegIndex = 0;
		}
		 g_u32Togglebit++;	   
		
	}

}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      DeviceEnable()	          	                                                                       */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      To enable the device to play or record audio data.                                                 */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      u8Object    To select the device. It could be UVC_MICROPHONE or UVC_SPEAKER.                       */
/*                                                                                                         */
/*      u8State     To indicate the calling stage. 0 = Call by set interface. 1 = Call by first ISO IN ack.*/
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DeviceEnable(uint8_t u8Object, uint8_t u8State)
{

}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      DeviceDisable()	          	                                                                       */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      To disable the device to play or record audio data.                                                */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      u8Object    To select the device. It could be UVC_MICROPHONE or UVC_SPEAKER.                       */
/*                                                                                                         */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DeviceDisable(uint8_t u8Object)
{
	   
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      GetPlayData()	          	                                                                       */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      To get data from ISO OUT to play buffer.                                                           */
/*                                                                                                         */
/* INPUTS                                                                                                  */
/*      pi16src    The data buffer of ISO OUT.                                                             */
/*      i16Samples The sample number in data buffer.                                                       */
/*                                                                                                         */
/* OUTPUTS                                                                                                 */
/*      None                            				                                                   */
/*                                                                                                         */
/* RETURN                                                                                                  */
/*                                                                                                         */
/*      None                                                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void GetPlayData(int16_t *pi16src, int16_t i16Samples)
{
    /* Get play data from ISO out packets */
}


/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* FUNCTION                                                                                                */
/*      UVC_MainProcess()	      	                                                                       */
/*                                                                                                         */
/* DESCRIPTION                                                                                             */
/*      The main loop of UVC funciton.                                                                     */
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
int32_t UVC_MainProcess(void)
{
	E_DRVUSB_STATE eUsbState;
	int32_t i32Ret = E_SUCCESS;

    /* Install the UVC control callbacks */
    UVC_Init(DeviceEnable, DeviceDisable, SendData, GetPlayData);

	if (i32Ret == E_SUCCESS)
	{
	    /* Install the USB event handler to USB event callback */
		i32Ret = DrvUSB_Open((void *)0);
        UVC_Open(0);
               
        /* If the USB has been attached already, we should force the bus reset. */
        eUsbState = DrvUSB_GetUsbState();
        if (eUsbState >= eDRVUSB_ATTACHED)
        {
            /* Force Bus Reset for 150 ms*/
            _DRVUSB_ENABLE_SE0();
            RoughDelay(150000);
            _DRVUSB_DISABLE_SE0();
        }
            
		
		/* Disable USB-related interrupts. */
		_DRVUSB_ENABLE_MISC_INT(0);

		/* Enable float-detection interrupt. */
		_DRVUSB_ENABLE_FLDET_INT();
        
		_DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);
 
        while(1)
        {
        
            DrvUSB_DispatchEvent();

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



