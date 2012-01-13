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


#ifndef UTIL_H
#define UTIL_H

extern char commEcho;


// USB/USART combined get/put char
extern void _user_putc(unsigned char c);        // Handles printf
extern int _user_getc(void);

// Retrieve a line of console-edited input
extern const char *_user_gets(void);

// String utility functions
extern unsigned long my_atoi(const char *s);
extern char *my_itoa(int v);
extern char *my_uitoa(unsigned int v);
extern char *my_ltoa(long v);
extern char *my_ultoa(unsigned long v);
extern int atoi_rom(const rom far char *s);
extern char *strcpy_rom(char *dest, const rom far char *source);
extern int stricmp_rom(const char *a, const rom far char *b);
extern int strnicmp_rom(const char *a, const rom far char *b, int max);

// Hex dump of memory to the console
extern void hexdump(void *buffer, size_t length);

#endif

