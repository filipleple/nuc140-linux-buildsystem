;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/

    AREA _image, DATA, READONLY

    EXPORT  imageBegin0
    EXPORT  imageBegin1
    EXPORT  imageBegin2
    EXPORT  imageBegin3
    EXPORT  imageEnd
    
	ALIGN   4

imageBegin0
	INCBIN JPEG1.jpg
imageBegin1
	INCBIN JPEG2.jpg
imageBegin2
imageBegin3
imageEnd        
    
    
    END