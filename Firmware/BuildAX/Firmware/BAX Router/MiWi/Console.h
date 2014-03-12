/********************************************************************
* FileName:		Console.h
* Dependencies: none
* Processor:	PIC18, PIC24F, PIC24H, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Hardware:		PICDEM Z, Explorer 16
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
*  This header file allows usage of the console functions located
*   in Console.c
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   11/09/2006   yfy       Initial revision
*  1.0   01/09/2007   yfy       Initial release
*  2.0   4/15/2009    yfy       MiMAC and MiApp revision
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
********************************************************************/
#ifndef  _CONSOLE_H_
#define  _CONSOLE_H_

/************************ HEADERS **********************************/
#include "GenericTypeDefs.h"
#include "Compiler.h"
//#include "SystemProfile.h"
#include "HardwareProfile.h"
#include "ConfigApp.h"

#define BAUD_RATE 19200

/************************ DEFINITIONS ******************************/
/************************ FUNCTION PROTOTYPES **********************/
#if defined(__dsPIC30F__) || defined(__dsPIC33F__) || defined(__dsPIC33E__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__) || defined(__PIC32MX__)
    #if defined(ENABLE_CONSOLE)
        void ConsoleInit(void);
        #define ConsoleIsPutReady()     (U2STAbits.TRMT)
        void ConsolePut(BYTE c);
        //void ConsolePutString(BYTE *s);
        void ConsolePutROMString(ROM char* str);
    
        #define ConsoleIsGetReady()     (IFS1bits.U2RXIF)
        BYTE ConsoleGet(void);
        //BYTE ConsoleGetString(char *buffer, BYTE bufferLen);
        void PrintChar(BYTE);
        void PrintDec(BYTE);
    #else
        #define ConsoleInit()
        #define ConsoleIsPutReady() 1
        #define ConsolePut(c)
        #define ConsolePutString(s)
        #define ConsolePutROMString(str)

        #define ConsoleIsGetReady() 1
        #define ConsoleGet()        'a'
        #define ConsoleGetString(buffer, bufferLen) 0
        #define PrintChar(a)
        #define PrintDec(a)
    #endif
#elif defined(__18CXX)

    #if defined(ENABLE_CONSOLE)   // Useful for disabling the console (saving power)
        void ConsoleInit(void);
        #if defined(EIGHT_BIT_WIRELESS_BOARD)
            #define ConsoleIsPutReady()     (TXSTA2bits.TRMT)
        #else
            #define ConsoleIsPutReady()     (TXSTAbits.TRMT)
        #endif
        void ConsolePut(BYTE c);
        void ConsolePutString(BYTE *s);
        void ConsolePutROMString(ROM char* str);
    
        #if defined(EIGHT_BIT_WIRELESS_BOARD)
            #define ConsoleIsGetReady()     (PIR3bits.RC2IF)
        #else
            #define ConsoleIsGetReady()     (PIR1bits.RCIF)
        #endif
        BYTE ConsoleGet(void);
        BYTE ConsoleGetString(char *buffer, BYTE bufferLen);
        void PrintChar(BYTE);
        void PrintDec(BYTE);
    #else
        #define ConsoleInit()
        #define ConsoleIsPutReady() 1
        #define ConsolePut(c)
        #define ConsolePutString(s)
        #define ConsolePutROMString(str)
    
        #define ConsoleIsGetReady() 1
        #define ConsoleGet()        'a'
        #define ConsoleGetString(buffer, bufferLen) 0
        #define PrintChar(a)
        #define PrintDec(a)
    #endif
#else
#error Unknown processor.  See Compiler.h
#endif

#define Printf(x) ConsolePutROMString((ROM char*)x)
//#define printf(x) ConsolePutROMString((ROM char*)x)
#endif


