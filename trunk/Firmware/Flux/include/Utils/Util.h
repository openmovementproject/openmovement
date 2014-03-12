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
// Dan Jackson & Karim Ladha, 2010-2012.

#ifndef UTIL_H
#define UTIL_H

extern char commEcho;

#if defined(PIC24) || defined(__C30__) || defined(__dsPIC33E__)
typedef void (*write_handler_t)(const void *buffer, unsigned int len);
extern write_handler_t writeHandler;
int write(int handle, void *buffer, unsigned int len);
#endif

// Retrieve a line of console-edited input
extern const char *_user_gets(void);

// String utility functions
extern const char *my_itoa(int );
extern int my_nitoa(char *dest, int v);	// In-place integer to ASCII
extern const char *my_uitoa(unsigned int v);
extern const char *my_ultoa(unsigned long v);
extern const char *fixed_itoa(short value, char n);
extern unsigned long my_atoi(const char *s);
#if !(defined(_MSC_VER) && defined(_INC_STRING))
extern int strnicmp(const char *a, const char *b, int max);
#endif

// Checksum
unsigned short checksum(const void *data, size_t len);

// Hex dump of memory to the console
extern void printhexdump(void *buffer, size_t offset, size_t length);

// Text to binary conversion
int DecodeHex(unsigned char *output, const char *input, int max);
int DecodeBase64(unsigned char *output, const char *input);
int EncodeBase64(char *output, const unsigned char *input, int length);

// Filename utility functions
extern char ValidFilename(const char *filename, char allowWildcard);



// --- Serial timestamp functions ---
// ??? Consider moving these to rtc.h (or, better still, a new Timestamp.h ?)

// Return the day number of the year (0-based), given a year, month (1-based), and day-of-month (1-based)
unsigned short DayOfYear(unsigned short year, unsigned char month, unsigned char day);

// Return the day since the epoch (1st Jan 2000)
unsigned short DaysSinceEpoch(unsigned short year, unsigned char month, unsigned char day);

// Return the number of seconds since the epoch (2000-01-01 00:00:00)
unsigned long SecondsSinceEpoch(unsigned short year, unsigned char month, unsigned char day, unsigned char hours, unsigned char minutes, unsigned char seconds);

// Compliments the SecondsSinceEpoch_DateTime() macro - convert a timestamp of seconds since the epoch (1/1/2000) to a packed date/time (castable to DateTime type)
//unsigned long DateTimeFromEpoch(unsigned long epochSeconds);

// Macros for use with DateTime type
#define DayOfYear_DateTime(_dt) DayOfYear(((unsigned char)(((_dt) >> 26) & 0x3f)), ((unsigned char)(((_dt) >> 22) & 0x0f)), ((unsigned char)(((_dt) >> 17) & 0x1f)))
#define DaysSinceEpoch_DateTime(_dt) DaysSinceEpoch(((unsigned char)(((_dt) >> 26) & 0x3f)), ((unsigned char)(((_dt) >> 22) & 0x0f)), ((unsigned char)(((_dt) >> 17) & 0x1f)))
#define SecondsSinceEpoch_DateTime(_dt) SecondsSinceEpoch(((unsigned char)(((_dt) >> 26) & 0x3f)), ((unsigned char)(((_dt) >> 22) & 0x0f)), ((unsigned char)(((_dt) >> 17) & 0x1f)), ((unsigned char)(((_dt) >> 12) & 0x1f)), ((unsigned char)(((_dt) >>  6) & 0x3f)), ((unsigned char)(((_dt)) & 0x3f)))

#endif

