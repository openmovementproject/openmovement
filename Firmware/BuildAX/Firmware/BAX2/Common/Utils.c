/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// Utility and debug functions
// Karim Ladha, 2013-2014

#include <stdlib.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Analog.h"
#include "Utils.h"

// Debug setting
#ifndef DEBUG_ON
//	#define DEBUG_ON
#endif
#include "Debug.h"

// Basic delay function
void DelayMs(unsigned short ms)
{
	// Delay cycles - approx
	unsigned long count = ms*10;
	for(;count!=0;count--)
	{
		Delay100us();
	}
}

unsigned char EepromRead(unsigned char* dest, unsigned short len, unsigned short address)
{
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	while(len)
	{
		EEADR = address&0xff;
		EEADRH = address>>8;
		EECON1bits.RD = 1;
		*dest++ = EEDATA;
		len--;
		address++;
	}
	return TRUE;	
}
unsigned char EepromWrite(unsigned char* source, unsigned short len, unsigned short address)
{
	unsigned char gie;
	gie = INTCONbits.GIE; // Context
	INTCONbits.GIE = 0;
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EECON1bits.WREN = 1;
	while(len)
	{
		EEADR = address&0xff;
		EEADRH = address>>8;
		EEDATA = *source++ ;
		// Timing specific bit
		EECON2 = 0x55;
		EECON2 = 0xAA;
		EECON1bits.WR = 1;
		while(EECON1bits.WR);
		// Check
		if(EECON1bits.WRERR) return FALSE; 
		len--;
		address++;
	}
	EECON1bits.WREN = 0;
	INTCONbits.GIE = gie; // Restore
	return TRUE;
}

void SleepOnWdt(void)
{
	INTCONbits.GIE = 0; // Turn off vectors
	Si44_IRQ_OFF();
	WDTCONbits.SWDTEN = 1;
	Sleep();
	WDTCONbits.SWDTEN = 0;
	Si44_IRQ_EN();
	INTCONbits.GIE = 1; // Turn on vectors
}

void Delay100us(void)
{
	unsigned char count;
	switch((OSCCON>>4)&0x7) {
		case(0b111): count = 39; break; // 16MHz
		case(0b110): count = 17; break; // 8MHz
		case(0b101): count = 7; break; 	// 4MHz
		case(0b100): count = 1; break; 	// 2MHz
		case(0b011): count = 0;break;  	// 1MHz
		case(0b000): count = 78;break;	// 100ms at 32kHz
		default:break;
	};
	if(OSCTUNEbits.PLLEN)count = 82;	// 64MHz
	// Nops
	for(;count;count--){Nop();}
}

void PrintConst (const char* ptr)
{
	while((*ptr) && (*ptr<'~'))
	{
		COMM_PUTC(*ptr++);
	}
	return;
}
void PrintInt (signed short val)
{
	char *p = Int2Dec(val);
	while(*p){ COMM_PUTC(*p++);}
	return;
}
void PrintUint (unsigned short val)
{
	char *p = Uint2Dec(val);
	while(*p){ COMM_PUTC(*p++);}
	return;
}
void PrintHex (void* data, unsigned short len, char separator)
{
	unsigned char* ptr = data;
	while(len--)
	{
		PrintConst((const char*)(Byte2Hex(*ptr++)));
		if(separator)COMM_PUTC(separator);
	}
}

unsigned long my_atoi(const char *s)
{
    char sign = 0;
    unsigned long value = 0;
	while (*s == ' '|| *s == '=' || *s == ':') { s++; }
    if (*s == '-') { sign = -1; s++; }
    for (; *s >= '0' && *s <= '9'; s++)
    {
        if (sign == 0) { sign = 1; } 
		value *= 10; 
		value += (*s - '0');
    }
    return (sign == 0) ? 0xffffffff : (sign * value);
}

char* Int2Dec(signed short val)
{
	static char buffer[7];	// -12345\0
	char *p;
	*(p = buffer + 6) = '\0';
	if (val >= 0) {
		do { *(--p) = '0' + (val % 10); } while (val /= 10);
	} else {
		do { *(--p) = '0' - (val % 10); } while (val /= 10);
		*(--p) = '-';
	}
	return p;
}

char* Uint2Dec(unsigned short val)
{
	static char buffer[6];	// 12345\0
	char *p;
	*(p = buffer + 6) = '\0';
	do { *(--p) = '0' + (val % 10); } while (val /= 10);
	return p;
}

char* Byte2Hex(unsigned char val)
{
	static char out[3];
	out[0] = val>>4;
	out[0] = (out[0]<0xA)?(out[0] + '0'):(out[0] + ('A' - 0xA));
	out[1] = val&0xf;
	out[1] = (out[1]<0xA)?(out[1] + '0'):(out[1] + ('A' - 0xA));
	out[2] = '\0';
	return out;
}

char* UartGetLine(void)
{
	// Get a line with a CR from the serial port
	static char buffer[MAX_LINE_LEN];
	unsigned short index = 0;
	char inChar; 
	// Reset vars etc.

	for(;;)
	{
		unsigned short timeout = 60000; // 60 Seconds timeout (reset every char)
		unsigned char cursorTimer = 0;	// Cursor
		// Hang here
		while((!COMM_HASCHAR()) && --timeout)
		{
			if(cursorTimer++ == 0)PrintConst("_\x08");
			if(cursorTimer == 128)PrintConst(" \x08");	
			DelayMs(1);
		}

		// Check timeout
		if(!COMM_HASCHAR()) return NULL;
	
		// Read char
		inChar = COMM_GETC();

		// Echo
		COMM_PUTC(inChar);
		cursorTimer = 0;

		switch (inChar) {
			case (0x1b)	: {return NULL;}			// ESC.
			case ('\0')	:							// NULL.
			case ('\n')	:							// LF.
			case ('\r')	:{buffer[index] = '\0'; 	// CR. Process
						return buffer;}				// Return line
			case (0x08)	:{if(index>0)index--; 		// BS. delete
						break;}
			default : 	{buffer[index++] = inChar;
						index %= MAX_LINE_LEN;
						break;}
		};// switch	
	}// for
}

void WriteRandom(void* dest, unsigned char num)
{
	unsigned int sum = 0;	
	unsigned char bitPos, bytePos, byte, temp;

	// Get a random seed once
	SampleSensors();
	sum = sensorVals.light + sensorVals.temp + sensorVals.fvr + sensorVals.humid;

	// Generate random numbers one bit at a time using changing seed
	for(bytePos=0;bytePos<num;bytePos++)
	{
		byte = 0;
		for(bitPos=0;bitPos<8;bitPos++)
		{
			// Add radomness be changing seed
			SampleChannel(PIR_CHANNEL); //(PIR is always on)
			sum += adcResults.pir;
			sum += SampleFvrForVdd();
			srand(sum);
			temp = rand();
			byte |= (temp&0x01)<<bitPos;
		}
		((unsigned char*)dest)[bytePos] = byte; 
	}
}
