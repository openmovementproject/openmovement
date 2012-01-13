/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// spi.c - SPI interface
// Portions based on Microchip Technology file 'MSPI.c' from Microchip's MiWi stack
// Extended to support SPI2.
// Additions are covererd by the above copyright, the original code is covered by the below copyright.

/********************************************************************
* FileName:		MSPI.c
* Dependencies: MSPI.h
* Processor:	PIC18, PIC24F, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Complier:     Microchip C18 v3.04 or higher
*				Microchip C30 v2.03 or higher	
*               Microchip C32 v1.02 or higher
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice
*
* Copyright © 2007-2010 Microchip Technology Inc.  All rights reserved.
*
* Microchip licenses to you the right to use, modify, copy and distribute 
* Software only when embedded on a Microchip microcontroller or digital 
* signal controller and used with a Microchip radio frequency transceiver, 
* which are integrated into your product or third party product (pursuant 
* to the terms in the accompanying license agreement).  
*
* You should refer to the license agreement accompanying this Software for 
* additional information regarding your rights and obligations.
*
* SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY 
* KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY 
* WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A 
* PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE 
* LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, 
* CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY 
* DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO 
* ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
* LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, 
* TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT 
* NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/

#include "SystemProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "WirelessProtocols/Console.h"
#include "WirelessProtocols/MSPI.h"

// To use own SPI functions (allowing SPI2 to be used), must have defined in HardwareProfile.h:  #define RF_USE_SPI 2
#if defined(RF_USE_SPI) && ((RF_USE_SPI == 1) || (RF_USE_SPI == 2))

	#ifndef HARDWARE_SPI
		#error "HARDWARE_SPI must be defined if using own functions with RF_USE_SPI set"
	#endif

	void SPIPut(unsigned char v)
	{
	#if (RF_USE_SPI == 1)
		PIR1bits.SSPIF = 0; 
	    do { SSPCON1bits.WCOL = 0; SSPBUF = v; } while (SSPCON1bits.WCOL); 
	    while (PIR1bits.SSPIF == 0);
	#elif (RF_USE_SPI == 2)
	    PIR3bits.SSP2IF = 0;
	    do { SSP2CON1bits.WCOL = 0; SSP2BUF = v; } while (SSP2CON1bits.WCOL);
	    while (PIR3bits.SSP2IF == 0);
	#endif
	}

	unsigned char SPIGet(void)
	{
		SPIPut(0x00); 
		#if (RF_USE_SPI == 1)
			return SSPBUF;
		#elif (RF_USE_SPI == 2)
			return SSP2BUF;
		#endif
	}

	//#define SPIInit()   SSPIF = 1
	
#else

    // Otherwise use Microchip's standard SPI functions for the MiWi module...
    #include "../Common/MSPI.c"

#endif
