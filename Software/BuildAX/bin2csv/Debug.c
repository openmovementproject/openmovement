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
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "BaxUtils.h"
#include "Debug.h"


volatile unsigned short mallocCtr;	// Extern in debug.h

// Not in C spec but C30s implementation of fwrite is BS (bytewise only)
extern int write(int handle, void *buffer, unsigned int len);

// Write to error log file
void DBG_log_write(const char* file, const char* fmt, ...)
{
	// Make entry on stack
	FILE* error_file = NULL;
	char temp[128], *ptr=temp;
	va_list myargs;
    va_start(myargs, fmt);
	// Error message
	ptr+=sprintf(ptr,"\r\n%s,",RtcToString(RtcNow())); // TODO
	ptr+=sprintf(ptr,"%s,",file);
    ptr+=vsprintf(ptr, fmt, myargs); 
    va_end(myargs);

	// Check file is open
	if(error_file == NULL)
	{
		error_file = fopen(ERROR_FILE,"a");
	}
	// Write to file
	if(error_file != NULL)
	{	
		// Write out error message to file
		fwrite(temp,1,(ptr-temp),error_file);
	}
	// Close the file
	fclose(error_file);
	error_file = NULL;
	return;
}

__inline void DBG_assert(unsigned char condition, const char* file, unsigned short line)
{
	if(!condition)
	{
		#ifdef ERRORS_TO_FILE
			DBG_log_write(file, "ASSERT line:%u", line);	
		#else
			DBG_printf("\r\nASSERT:%s, line:%u",file,line);
		#endif
	}
}

void DBG_error(const char* file, const char* fmt,...)
{
    va_list myargs;
    va_start(myargs, fmt);
	fprintf(stderr, "\r\nERROR:%s:",file);
	vfprintf(stderr, fmt, myargs); // Divert to stderr
	va_end(myargs);
}

/*
	USB output for info, debugging or errors
	If you don't define ERRORS_TO_FILE then the errors will be passed
	to this section as well.
*/


// Faster for known lengths / consts / dumps / time critical stuff
void DBG_write(void* data, unsigned short len)
{
	// STDERR=2
	fwrite(data,1,len,stderr); //One byte at a time, not good
//	write(2,data,len); // Better option
}

// This will be diverted to the error destination of utils.c, write() and stderr (usb_write() default)
void DBG_printf(const char* fmt, ...)
{
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs); // Divert to stderr
    va_end(myargs);
}

// Binary dump
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

// Debugging Malloc implementation
void *DBG_malloc(size_t size, const char* file, unsigned short line) {
	void *ptr = NULL;
	ptr = malloc(size);
	
	if(!ptr)
	{
		DBG_error("debug", "Malloc Failure at %s:%u", file, line);
		return NULL;
	}
	
	mallocCtr++;	// Malloc counter
	DBG_printf(" malloc  addr 0x%04X, count %02u, in %s:%u (%ub)\r\n", ptr, mallocCtr, file, line, size);
	
	return ptr;
}


void DBG_free(void *ptr, const char* file, unsigned short line) {
	free(ptr);
	mallocCtr--;	// Free counter
	DBG_printf(" free    addr 0x%04X, count %02u, in %s:%u\r\n", ptr, mallocCtr, file, line);
}
