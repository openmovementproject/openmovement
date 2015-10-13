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

#ifndef _DEBUG_H_
#define _DEBUG_H_

// Includes
#include <stdarg.h>

// Prototypes
extern void DBG_printf(const char* fmt,...);
extern void DBG_error(const char* file, const char* fmt, ...); 
extern void DBG_log_write(const char* file, const char* fmt, ...); 
extern void DBG_assert(unsigned char condition, const char* file, unsigned short line);
extern void DBG_dump(const unsigned char* data, unsigned short len); 	// HEX
extern void DBG_write(void* data, unsigned short len); 				// RAW (for text)

//-----------------------------------------------------------------------------
// Debugging malloc() implementation
extern void *DBG_malloc(size_t size, const char* file, unsigned short line);
extern void DBG_free(void*, const char* file, unsigned short line);
extern volatile unsigned short mallocCtr;

// 16 bits of debug storage (counter)
extern volatile unsigned short debug16;

// Define this to print all calls to DBG_MALLOC over stdout
//#define DBG_PRINT_MALLOC

#ifdef DBG_PRINT_MALLOC
	#define DBG_MALLOC(SIZE) DBG_malloc(SIZE, __FILE__, __LINE__)
	#define DBG_FREE(PTR)	 DBG_free(PTR, __FILE__, __LINE__)
#else
	#define DBG_MALLOC(SIZE) malloc(SIZE)
	#define DBG_FREE(PTR)	 free(PTR)
#endif

//-----------------------------------------------------------------------------
// 0 is off, 1 is errors, 2 is assertions and erros, 3 is all
// DEBUG LEVEL == 0 == No debugging output
#ifdef GLOBAL_DEBUG_LEVEL
	#if(DEBUG_LEVEL < GLOBAL_DEBUG_LEVEL)
		#undef DEBUG_LEVEL
		#define DEBUG_LEVEL (GLOBAL_DEBUG_LEVEL)
	#endif
#endif

// DEBUG LEVEL > 0
#if (DEBUG_LEVEL > 0)
	#ifdef ERRORS_TO_FILE
		#define DBG_ERROR(...)		DBG_log_write(DBG_FILE,__VA_ARGS__)
	#else
		#define DBG_ERROR(...)		DBG_error(DBG_FILE,__VA_ARGS__)
	#endif
#else
	#define DBG_ERROR(...)
#endif

// DEBUG LEVEL > 1
#if (DEBUG_LEVEL > 1)
	#define ASSERT(_x)			DBG_assert(_x,DBG_FILE,__LINE__)
	#define DBG_LOG_WRITE(...) 	DBG_log_write(DBG_FILE,__VA_ARGS__)
#else
	#define ASSERT(_x)	
	#define DBG_LOG_WRITE(...) 	
#endif

// DEBUG LEVEL > 2
#if (DEBUG_LEVEL > 2)
	#define DBG_INFO(...)		DBG_printf(__VA_ARGS__)
	#define DBG_DUMP(_p,_l)		DBG_dump(_p,_l)
	#define DBG_WRITE(_p,_l)	{DBG_write(_p,_l);}
	#define DBG_PRINT_INT(_i) 	{DBG_printf("%d",_i);}
	#define DBG_PRINT_HEX(_c) 	{DBG_printf("%04X",_i);}
#else
	#define DBG_INFO(_x,...)	
	#define DBG_DUMP(_p,_l)
	#define DBG_WRITE(_p,_l)
	#define DBG_PRINT_INT(_i)	
	#define DBG_PRINT_HEX(_c)
#endif


#endif

