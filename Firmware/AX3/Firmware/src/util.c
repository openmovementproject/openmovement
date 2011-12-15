/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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
// Dan Jackson, Karim Ladha, 2010-2011.

// Includes
#if defined(USE_USART) || defined(DEBUG_USART2_TX)
#include <usart.h>
#endif

#include "HardwareProfile.h"
#include <stdio.h>
#include <USB/USB.h>
#include <USB/usb_function_cdc.h>
#include "USB_CDC_MSD.h"
#include "util.h"


// Edit-line buffer
#pragma idata
static int serialBufferLength = 0;
char commEcho = 0;      // Echo input
#define SERIAL_BUFFER_SIZE 128
#pragma udata
static char serialBuffer[SERIAL_BUFFER_SIZE] = {0};

#pragma code

#define _user_putc(_x) usb_putchar(_x)


// Retrieve a line of console-edited input
const char *_user_gets(void)
{
    unsigned char i;
    int c;

    for (i = 0; i < 100; i++)       // Maximum number of iterations (bails out early if no input)
    {
		c = usb_getchar();

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

// Integer to ASCII
const char *my_itoa(int v)
{
	static char buffer[7];	// -12345\0
	char *p;
	*(p = buffer + 6) = '\0';
	if (v >= 0) {
		do { *(--p) = '0' + (v % 10); } while (v /= 10);
	} else {
		do { *(--p) = '0' - (v % 10); } while (v /= 10);
		*(--p) = '-';
	}
	return p;
}

const char *my_ultoa(unsigned long v)
{
	static char buffer[11];	// 4000000000\0
	char *p;
	*(p = buffer + 10) = '\0';
	do { *(--p) = '0' + (v % 10); } while (v /= 10);
	return p;
}


// Macro to lower-case an ASCII character
#define _CHAR_TO_LOWER(_c) (((_c) >= 'A' && (_c) <= 'Z') ? ((_c) + 'a' - 'A') : (_c))

// Case-insensitive string comparison
int strnicmp(const char *a, const char *b, int max)
{
    for (;;)
    {
        if (max-- == 0) { return 0; }
        if (_CHAR_TO_LOWER(*a) != _CHAR_TO_LOWER(*b)) { return 1; }
        if (*a == '\0' || *b == '\0') { return 0; }
        a++; b++;
    }
}


//unsigned char outputbuffer[128];
//volatile unsigned char *writereg = &outputbuffer[0];

__attribute__ ( (section(".libc"))) 
int write(int handle, void *buffer, unsigned int len)
{
	unsigned int i;
	switch (handle)
	{
		case 0: // handle 0 = stdout
		case 1: // handle 1 = stdin
		case 2: // handle 2 = stderr
		default:
            for (i = len; i; --i)
            {
                // Write to peripheral
                usb_putchar(*(char*)buffer++);
            }
	}
	return len;
}


// Checksum - 16-bit word-size addition, returns two's compliment of sum (bitwise NOT, then add 1) -- then total sum of words including checksum will be zero.
unsigned short checksum(const void *buffer, size_t len)
{
    const unsigned short *data = (const unsigned short *)buffer;
    unsigned short value = 0x0000;						// Initial sum of zero
	len >>= 1;											// Length in words (must be even number of bytes)
    for (; len; --len) { value += *data++; }			// Sum data words
    return (~value) + 1;								// ...take bitwise NOT of sum, then add 1 (total sum of words including checksum will be zero)
}


// Hex dump of memory
void hexdump(void *buffer, size_t offset, size_t length)
{
	unsigned char *buf = (unsigned char *)buffer;
	char w = 16, b;
	unsigned short o;
	for (o = 0; o < length; o += w)
	{
		char z = w;
		if (o + z >= length) { z = (char)(length - o); }
		printf("%04x: ", offset + o);
	    for (b = 0; b < 16; b++)
	    {
			if (b > 0 && b % 4 == 0) { printf(" "); }		// optional, additional spacing
			
			if (b < z) { printf("%02x ", buf[offset + o + b]); }
			else { printf("   "); }
	    }
		printf(" "); 
	    for (b = 0; b < 16; b++)
	    {
		    unsigned char c = buf[offset + o + b];
			if (b < z) { printf("%c", (c < 0x1f || (c >= 0x7f && c <= 0x9f) || c >= 0xff) ? '.' : c); }
			else { printf(" "); }
	    }
	    printf("\r\n");
		USBCDCWait();
	}
}


