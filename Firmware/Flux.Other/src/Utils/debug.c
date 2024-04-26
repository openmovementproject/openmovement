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
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Integration into compiler specific debug environment
#if defined(__C30__) || defined (__C32__)
    /* For these compilers the user needs to ensure the USB CDC is enabled and running for debugging*/
	extern int write(int handle, void *buffer, unsigned int len);
	extern void usb_putchar(uint8_t);
    /* Debug out init and routing to internal stdout method */
	void DBG_Init(void){;}
    /* For faster writes of constants */
	#define DBG_puts(_s)   write(2,(void*)_s,strlen(_s)) /* 2 = stderr -> USB */ //{write_handler_t writeHandlerSave = writeHandler; writeHandler = usb_write; write(1,(void*)_s,strlen(_s)); writeHandler = writeHandlerSave;}	
	#define DBG_putc(_c)   usb_putchar(_c)					
#elif defined(__arm__)
	/* Initially written for an M0 cortex with rowley crossworks*/
	#include "boards.h"
	#include "Uart0.h"
	/* Debug out init and routing to internal stdout method */
	void DBG_Init(void){Uart0Init(U0_TX_PIN_NUMBER,U0_RX_PIN_NUMBER,U0_CTS_PIN_NUMBER,U0_RTS_PIN_NUMBER,U0_HWFC,U0_BAUD_RATE);}
	int __putchar(int ch){return Uart0Putc(ch);}			/* Compiler specific integration to stdout functions*/
    /* For faster writes of constants */
	#define DBG_puts(_s)   	Uart0Puts(_s)					/* stdout */ 
	#define DBG_putc(_c)	Uart0Putc(_c)					/* stdout, also used for printf */ 
#endif

// Definitions
#ifndef DEBUG_MALLOC_CANARY_LENGTH
#define DEBUG_MALLOC_CANARY_LENGTH	16
#endif
	
// Types
// Dynamic memory debugging. Added to start of dynamic allocations.
typedef struct {
	const char *file; 	
	uint16_t line;
	uint16_t size;
} debug_malloc_info_t;

// Globals
// Dynamic memory allocation variables for dynamic memory debug
const uint16_t debug_malloc_added_size = (sizeof(debug_malloc_info_t) + (2 * DEBUG_MALLOC_CANARY_LENGTH));
volatile uint16_t debug_malloc_count = 0;
volatile uint32_t debug_alloc_size = 0;	

// Static buffer for ultoa makes it very fast, but is only
// Safe due to single debug thread. 2^32 len = 10
static char buffer[11];	// 4000000000\0

// Debug setting
#define DEBUG_LEVEL_LOCAL	100
// Make the debug file name constant. Must be created in each file or 'debug.c' will be used.
#define DBG_FILE			"debug.c"
#include "utils/debug.h"

// Source
// Allows remapping of default debug channel (e.g. to LCD, console, file, etc.)
// Default behaviour is using printf. file: message<CR><LF> (adds line endings)
void DBG_printf(const char* file, const char* fmt,...)
{
    va_list myargs;
    const char* scanPtr;
	
	// Print the file name
    va_start(myargs, fmt);
	if(file)
    {
        DBG_puts(file);
        DBG_puts(": ");
    }

	// Scan string to see if printf is actually needed
    scanPtr = fmt;
    while(*scanPtr != '\0' && *scanPtr != '%')scanPtr++;

    if(*scanPtr == '%')
        vprintf(fmt, myargs);	// % arg found before null
    else
        DBG_puts(fmt);			// No args for printf, much faster route

	// Line ending added
	DBG_puts("\r\n");
    va_end(myargs);
}

// Very fast call for small const strings
void DBG_fast(const char* fmt)
{
	DBG_puts(fmt);
}

void DBG_assert(uint8_t condition, const char* file, uint16_t line)
{
	if(!condition)
	{
		DBG_printf(NULL, DBG_ASSERT_TEXT " %s, %u",file,line);
	}
}

// Integer printing functions:
// Typical printf versions of this are very slow
const char* dbg_itoa_from(unsigned short v, char* pos)
{
	div_t res; 
	do {res = div((int)v, (int)10); v = res.quot; *(--pos) = '0' + (res.rem);} while(v > 0);
	return pos;	
}
const char* DBG_ultoa(unsigned long lv)
{
	ldiv_t lres; 
	char *pos = &buffer[10];
	*pos = '\0';
	while(lv > 0x00007fff){lres = ldiv((long)lv, (long)10); lv = lres.quot; *(--pos) = '0' + (lres.rem);} 
	return dbg_itoa_from((unsigned short) lv, pos);
}
const char* DBG_uitoa(unsigned short v)
{
	char* pos = &buffer[10];
	*pos = '\0';
	return dbg_itoa_from(v, pos);
}
const char* DBG_bin2hex(const void* var, size_t size) // LE only variable to hex conversion
{
	char *pos = &buffer[0];
	uint8_t temp;
	if(size > 4)
		size = 4;
	while(size--)
	{
		temp = (*(uint8_t*)var) >> 4;
		if(temp < 0xA)	*pos++ = temp + '0';
		else 			*pos++ = temp - 0xA + 'A';
		
		temp = (*(uint8_t*)var) & 0xF;
		if(temp < 0xA)	*pos++ = temp + '0';
		else 			*pos++ = temp - 0xA + 'A';
		
		var = (uint8_t*)var + 1;
	}
	*pos = '\0';
	return buffer;
}

void DBG_dump(const uint8_t* data, uint16_t len)
{
	char temp;

	uint8_t trunc = 0;
    // Clamp dump load
	if(len>24)
	{
		len = 24; 
		trunc = 1;
	}
	// Hex dump with spaces
	for(;len>0;len--)
	{
		
		temp = '0' + (*data >> 4);
		if(temp>'9')temp += ('A' - '9' - 1);  
		DBG_putc(temp);
		temp = '0' + (*data & 0xf);
		if(temp>'9')temp += ('A' - '9' - 1); 
		DBG_putc(temp);
		DBG_putc(' ');
        data++;
	}
	// Add '..' for truncated
	if(trunc == 1)
		DBG_puts("..\r\n");
	else
		DBG_puts("\r\n");
	
	return;
}

// Dynamic memory allocation, debug implementation 
void* DBG_malloc(size_t size, const char* file, unsigned short line) 
{
	debug_malloc_info_t* info;
	void *ptr = NULL;
	const char* result = " ";

	// Allocate memory plus extra info and canaries
	info = ptr = malloc(size + debug_malloc_added_size);

	// Initialise the trace data
	if(ptr != NULL)
	{
		// Set the debug data
		debug_malloc_count++;
		debug_alloc_size += size;

		// Set allocation info
		info->file = file;
		info->line = line;
		info->size = size;

		// Set canary sections start and end
		ptr += sizeof(debug_malloc_info_t);
		memset(ptr, 0xCC, DEBUG_MALLOC_CANARY_LENGTH);
		// Set pointer past start canary section
		ptr += DEBUG_MALLOC_CANARY_LENGTH;
		memset(ptr + size, 0xCC, DEBUG_MALLOC_CANARY_LENGTH);
	}
	
	// Debug output
	if(ptr == NULL) {result = " failed";}	// Set failed message
	DBG_printf(dbg_file, "malloc() [+%ub in %s : %u, sum = %sb],%s",size, file, line, DBG_ultoa(size + debug_alloc_size), result);
	
	// Return pointer
	return ptr;
}

void DBG_free(void* ptr, const char* file, unsigned short line) 
{
	debug_malloc_info_t* info;
	uint8_t count, *preCanary, *postCanary;
	const char* checkResult;
	
	// Assert pointer is valid
	if(ptr == NULL)
	{
		DBG_printf(dbg_file, "free error [%s : %u]", file, line);		
		return;
	}

	// Get info
	info = (debug_malloc_info_t*)(ptr - DEBUG_MALLOC_CANARY_LENGTH - sizeof(debug_malloc_info_t));
	
	// Check canaries
	checkResult = "bounds ok";
	postCanary = ptr + info->size;
	preCanary = ptr - 1;
	for(count = DEBUG_MALLOC_CANARY_LENGTH; count > 0; count--)
	{
		if(*postCanary++ != 0xCC){checkResult = "overwritten";break;}
		if(*preCanary-- != 0xCC){checkResult = "underwritten";break;}
	}
	
	// Update the debug data
	debug_alloc_size -= info->size;
	debug_malloc_count--;

	// Output
	DBG_printf(dbg_file, "free() [-%ub in %s : %u (from %s : %u), sum = %lub, %s]", info->size, file, line, info->file, info->line, debug_alloc_size, checkResult);

	// Free
	free(info);
		
	return;
}


