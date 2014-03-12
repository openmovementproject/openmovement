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

// Debugging outputs
// Karim Ladha, 2013-2014

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "debug.h"
#include "data.h"

// Not in C spec but C30s implementation of fwrite is BS
extern int write(int handle, void *buffer, unsigned int len);

// This will be diverted to the error destination of utils.c, write() and stderr (usb_write() default)
void DBG_printf(const char* fmt, ...)
{
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs); // Divert to stderr
    va_end(myargs);
}

void DBG_write(void* data, unsigned short len)
{
	// STDERR=2
//	fwrite(data,1,len,stderr); //One byte at a time, not good
	write(2,data,len); // Better option
}

// This will be diverted to the error destination of data.c
void DBG_log_error(const char* fmt,...)
{
	char temp[128], *ptr=temp;
	va_list myargs;
    va_start(myargs, fmt);
	// Error message
    ptr+=vsprintf(ptr, fmt, myargs); 
    va_end(myargs);
	AddDataElement(TYPE_TEXT_ELEMENT,DATA_DEST_ALL, (ptr-temp), TXT_ERR_FLAG , temp); 
}

inline void DBG_assert(unsigned char condition, char* file, unsigned short line)
{
	if(!condition)
	{
		DBG_printf("ASSERTION FAILED: %s, %u\r\n",file,line);
	}
}

void DBG_dump(const unsigned char* data, unsigned short len)
{
	unsigned short i;
	unsigned char trunc = FALSE;
	if(len>24)
	{
		len = 24; // Clamp dump load
		trunc = TRUE;
	}
	for(i=0;i<len;i++)
	{
		DBG_printf("%02X ",(unsigned char)data[i]);
	}
	if(trunc == TRUE)
		DBG_printf("..\r\n");
	else
		DBG_printf("\r\n");
}


