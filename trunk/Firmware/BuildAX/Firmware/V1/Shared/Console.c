/* Based on "Console.c" by Microchip, copyright as below, changes are:
 * Copyright (c) 2012, Newcastle University, UK.
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

// Modified by KL 2012 for TEDDI project to use USB CDC
#include "HardwareProfile.h"
#include "TimeDelay.h"
#include "USB\USB_CDC_MSD.h"
#include "Utils\Util.h"
#include "Settings.h"

/********************************************************************
* FileName:		Console.c
* Dependencies: Console.h
* Processor:	PIC18, PIC24F, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Hardware:		PICDEM Z, Explorer 16, PIC18 Explorer
* Complier:     Microchip C18 v3.04 or higher
*				Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher	
* Company:		Microchip Technology, Inc.
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
*
*********************************************************************
* File Description:
*
*   This file configures and provides the function for using the
*   UART to transmit data over RS232 to the computer.
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   11/09/2006   yfy       Initial revision
*  1.0   01/09/2007   yfy       Initial release
*  2.0   4/24/2009    yfy       Modified for MiApp interface
*  2.1   6/20/2009    yfy       Add LCD support
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
********************************************************************/

/************************ HEADERS **********************************/
#include "WirelessProtocols/Console.h"
#include "SystemProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"


#if defined(ENABLE_CONSOLE)
#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__) || defined(__PIC32MX__)

/************************ VARIABLES ********************************/
ROM unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/************************ FUNCTIONS ********************************/

/*********************************************************************
* Function:         void ConsoleInit(void)
*
* PreCondition:     none
*
* Input:		    none
*
* Output:		    none
*
* Side Effects:	    UART2 is configured
*
* Overview:		    This function will configure the UART for use at 
*                   in 8 bits, 1 stop, no flowcontrol mode
*
* Note:			    None
********************************************************************/
void ConsoleInit(void)
{
	#ifdef USE_USB_CDC
		unsigned long i = 0;
		
		// Store our connection state
		status.usb = USB_BUS_SENSE;
		
		// Don't attempt anything if not connected
		if (!status.usb) return;


		// Switch to PLL
LED_SET(LED_WHITE);
        CLOCK_PLL();
        DelayMs(10);
	
		USBInitializeSystem();
		USBDeviceAttach();

		// This will only work in an interrupt driven USB system - exits on button press, only exits after enumeration is usb is detected
		while (USBDeviceState < CONFIGURED_STATE)
		{
			#ifndef USB_INTERRUPT
			USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
			#endif
			USBProcessIO();
			Delay10us(1);
			
			// Timed-out starting connection (perhaps a charger or disconnected?)
			if (i++ >= 1000000ul) 
			{ 
				// The USB connection has failed -- if we're not using the PLL when the radio is on, turn it off now
USBDeviceDetach();
				#if !defined(RADIO_PLL)
					// Switch to internal oscillator
				    CLOCK_INTOSC();
				#endif    
LED_SET(LED_OFF);
				return;
			}
		}
		
LED_SET(LED_MAGENTA);
 		// Gives host time to assign CDC port
 		i = 0;
		while (USBDeviceState >= CONFIGURED_STATE && i++ < 600000ul)
		{
			#ifndef USB_INTERRUPT
			USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
			#endif
			USBProcessIO();
			Delay10us(1);
			if (usb_haschar())
			{
				status.noconnect = 1;		// Prevent radio connection (would disturb CDC)
				break;
			}	
		}
		//while(!BUTTON);	DelayMs(10); 										// Debounce button on button break
		//for (i=0;i<10;i++){Printf("\r\n");DelayMs(100);}					// Clear the terminal window - helps with timing
	#endif
LED_SET(LED_OFF);
	return;
}

/*********************************************************************
* Function:         void ConsolePutROMString(ROM char* str)
*
* PreCondition:     none
*
* Input:		    str - ROM string that needs to be printed
*
* Output:		    none
*
* Side Effects:	    str is printed to the console
*
* Overview:		    This function will print the inputed ROM string
*
* Note:			    Do not power down the microcontroller until 
*                   the transmission is complete or the last 
*                   transmission of the string can be corrupted.  
********************************************************************/
void ConsolePutROMString(ROM char* str)
{
	printf(str);
USBCDCWait();
}


/*********************************************************************
* Function:         void ConsolePut(BYTE c)
*
* PreCondition:     none
*
* Input:		    c - character to be printed
*
* Output:		    none
*
* Side Effects:	    c is printed to the console
*
* Overview:		    This function will print the inputed character
*
* Note:			    Do not power down the microcontroller until 
*                   the transmission is complete or the last 
*                   transmission of the string can be corrupted.  
********************************************************************/
void ConsolePut(BYTE c)
{
	usb_putchar(c);
USBCDCWait();
}

/*********************************************************************
* Function:         BYTE ConsoleGet(void)
*
* PreCondition:     none
*
* Input:		    none
*
* Output:		    one byte received by UART
*
* Side Effects:	    none
*
* Overview:		    This function will receive one byte from UART
*
* Note:			    Do not power down the microcontroller until 
*                   the transmission is complete or the last 
*                   transmission of the string can be corrupted.  
********************************************************************/
BYTE ConsoleGet(void)
{
	int Temp;
	while(1)
	{
		Temp = usb_getchar();
		if (Temp!=-1) break;
		USBProcessIO();
	}
	return (BYTE)(Temp&0xFF);
}
	


/*********************************************************************
* Function:         void PrintChar(BYTE toPrint)
*
* PreCondition:     none
*
* Input:		    toPrint - character to be printed
*
* Output:		    none
*
* Side Effects:	    toPrint is printed to the console
*
* Overview:		    This function will print the inputed BYTE to 
*                   the console in hexidecimal form
*
* Note:			    Do not power down the microcontroller until 
*                   the transmission is complete or the last 
*                   transmission of the string can be corrupted.  
********************************************************************/
void PrintChar(BYTE toPrint)
{
	char temp[3];
	sprintf(temp,"%02x",toPrint);
	ConsolePutROMString(temp);
}

/*********************************************************************
* Function:         void PrintDec(BYTE toPrint)
*
* PreCondition:     none
*
* Input:		    toPrint - character to be printed. Range is 0-99
*
* Output:		    none
*
* Side Effects:	    toPrint is printed to the console in decimal
*                   
*
* Overview:		    This function will print the inputed BYTE to 
*                   the console in decimal form
*
* Note:			    Do not power down the microcontroller until 
*                   the transmission is complete or the last 
*                   transmission of the string can be corrupted.  
********************************************************************/
void PrintDec(BYTE toPrint)
{
	const char* temp = my_itoa(toPrint);
	ConsolePutROMString(temp);
}

#else
    #error Unknown processor.  See Compiler.h
#endif

#endif  //ENABLE_CONSOLE
