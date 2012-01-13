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

// util.h - Utility functions
// Dan Jackson, 2010.

//#define USE_USART


// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#if defined(USE_USART) || defined(DEBUG_USART2_TX)
#include <usart.h>
#endif
//#include <delays.h>
//#include <reset.h>
#include "USB/USB.h"
#include "USB/usb_function_cdc.h"
#include "util.h"
#include "usb_cdc.h"


// Variables
#pragma idata

// Edit-line buffer length
static int serialBufferLength = 0;
char commEcho = 1;


#pragma udata

// Edit-line buffer
#define SERIAL_BUFFER_SIZE 64
static char serialBuffer[SERIAL_BUFFER_SIZE] = {0};


#pragma code


// USB/USART combined put char (handles printf)
void _user_putc(unsigned char c)
{
#ifdef USE_USART
    putc1USART(c);
    while(Busy1USART()); 
#endif
#ifdef DEBUG_USART2_TX
    Write2USART(c);
    while(Busy2USART()); 
#endif
    usb_putchar(c);
}


// USB/USART combined get char
int _user_getc(void)
{
#ifdef USE_USART
    if (DataRdy1USART()) { return getc1USART(); }
#endif
    return usb_getchar();
}


// Retrieve a line of console-edited input
const char *_user_gets(void)
{
    unsigned char i;
    int c;
    for (i = 0; i < 100; i++)       // Maximum number of iterations (bails out early if no input)
    {
        c = _user_getc();
        if (c <= 0) { break; }
    
        if (c == 13)                        // CR
        {
            if (commEcho && c != 10) { _user_putc(c); }
            serialBuffer[serialBufferLength] = '\0';
            serialBufferLength = 0;
            if (commEcho) { _user_putc(10); }
            return serialBuffer;
        }
        else if (c == 8 || c == 127)        // BS and DEL
        {
            if (serialBufferLength > 0)
            {
                if (commEcho && c != 10) { _user_putc(8); _user_putc(32); _user_putc(8); }
                serialBufferLength--;
                serialBuffer[serialBufferLength] = '\0';
            }
        }
        else if (c == 10)                   // LF
        {
            ;   // ignore
        }
        else if (serialBufferLength + 1 < SERIAL_BUFFER_SIZE)
        {
            if (commEcho && c != 10) { _user_putc(c); }
            serialBuffer[serialBufferLength] = (char)c;
            serialBufferLength++;
            serialBuffer[serialBufferLength] = '\0';
        }
        else
        {
            if (commEcho && c != 10) { _user_putc('\a'); }
            ;   // Discard (out of buffer)
        }
    }
    return NULL;
}



// Macro to lower-case an ASCII character
#define _CHAR_TO_LOWER(_c) (((_c) >= 'A' && (_c) <= 'Z') ? ((_c) + 'a' - 'A') : (_c))


static char toabuffer[12];	// -2147483647\0

// Integer to ASCII
char *my_itoa(int v)
{
	char *p;
	*(p = toabuffer + 6) = '\0';
	if (v >= 0) {
		do { *(--p) = '0' + (v % 10); } while (v /= 10);
	} else {
		do { *(--p) = '0' - (v % 10); } while (v /= 10);
		*(--p) = '-';
	}
	return p;
}

// Unsigned integer to ASCII
char *my_uitoa(unsigned int v)
{
	char *p;
	*(p = toabuffer + 5) = '\0';
	do { *(--p) = '0' + (v % 10); } while (v /= 10);
	return p;
}

// Long to ASCII
char *my_ltoa(long v)
{
	char *p;
	*(p = toabuffer + 11) = '\0';
	if (v >= 0) {
		do { *(--p) = '0' + (v % 10); } while (v /= 10);
	} else {
		do { *(--p) = '0' - (v % 10); } while (v /= 10);
		*(--p) = '-';
	}
	return p;
}

// Unsigned long to ASCII
char *my_ultoa(unsigned long v)
{
	char *p;
	*(p = toabuffer + 10) = '\0';
	do { *(--p) = '0' + (v % 10); } while (v /= 10);
	return p;
}


// ASCII to integer
unsigned long my_atoi(const char *s)
{
    char sign = 0;
    unsigned long value = 0;
    for (; *s != '\0'; s++)
    {
        if (*s == '-') { if (sign == 0) { sign = 1; } sign = -sign; }
        else if (*s >= '0' && *s <= '9') { if (sign == 0) { sign = 1; } value *= 10; value += (*s - '0'); }
    }
    return (sign == 0) ? 0xffffffff : (sign * value);
}


// ROM ASCII to integer
int atoi_rom(const rom far char *s)
{
    char sign = 0;
    int value = 0;
    for (; *s != '\0'; s++)
    {
        if (*s == '-') { if (sign == 0) { sign = 1; } sign = -sign; }
        else if (*s >= '0' && *s <= '9') { if (sign == 0) { sign = 1; } value *= 10; value += (*s - '0'); }
    }
    return (sign == 0) ? -1 : (sign * value);
}


// ROM to RAM string copy
char *strcpy_rom(char *dest, const rom far char *source)
{
    char *d;
    const rom far char *s;
    for (d = dest, s = source; (*d = *s) != '\0'; d++, s++);
    return dest;
}


// RAM to ROM case-insensitive string comparison 
int stricmp_rom(const char *a, const rom far char *b)
{
    for (;;)
    {
        if (_CHAR_TO_LOWER(*a) != _CHAR_TO_LOWER(*b)) { return 1; }
        if (*a == '\0' || *b == '\0') { return 0; }
        a++; b++;
    }
}


// RAM to ROM case-insensitive string comparison with limit
int strnicmp_rom(const char *a, const rom far char *b, int max)
{
    for (;;)
    {
        if (max-- == 0) { return 0; }
        if (_CHAR_TO_LOWER(*a) != _CHAR_TO_LOWER(*b)) { return 1; }
        if (*a == '\0' || *b == '\0') { return 0; }
        a++; b++;
    }
}



void hexdump(void *buffer, size_t length)
{
	unsigned char *buf = (unsigned char *)buffer;
	char w = 16, b;
	unsigned short o;
	for (o = 0; o < length; o += w)
	{
		char z = w;
		if (o + z >= length) { z = (char)(length - o); }
		printf((const rom far char *)"%03x: ", o);
	    for (b = 0; b < z; b++)
	    {
			printf((const rom far char *)"%02x ", buf[o + b]);
	    }
	    for (b = 0; b < z; b++)
	    {
			printf((const rom far char *)"%c", ((buf[o + b] & 0x7f) < 0x20) ? '.' : (buf[o + b]));
	    }
	    printf((const rom far char *)"\r\n");
	    
// Flush intermittently
while (usbOutFree() < 72) { USBProcessIO(); }
	    
	}
}
