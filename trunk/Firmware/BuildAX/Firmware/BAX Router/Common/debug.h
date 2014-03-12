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
#include "HardwareProfile.h"

// Prototypes
void DBG_printf(const char* fmt,...);
void DBG_log_error(const char* fmt,...);
void DBG_assert(unsigned char condition, char* file, unsigned short line);
void DBG_dump(const unsigned char* data, unsigned short len); 	// HEX
void DBG_write(void* data, unsigned short len); 				// RAW (for text)

#ifdef ERRORS_ON
	#define DBG_ERROR(...)		DBG_log_error(__VA_ARGS__)
#else
	#define DBG_ERROR(...)
#endif

#ifdef DEBUG_ON
	#define ASSERT(_x)			DBG_assert(_x)
	#define DBG_INFO(...)		DBG_printf(__VA_ARGS__)
	#define DBG_DUMP(_p,_l)		DBG_dump(_p,_l)
	#define DBG_WRITE(_p,_l)	{DBG_write(_p,_l);}
	#define DBG_PRINT_INT(_i) 	{DBG_printf("%d",_i);}
	#define DBG_PRINT_HEX(_c) 	{DBG_printf("%04X",_i);}
#else
	#define ASSERT(_x)		
	#define DBG_INFO(_x,...)	
	#define DBG_DUMP(_p,_l)
	#define DBG_WRITE(_p,_l)
	#define DBG_PRINT_INT(_i)	
	#define DBG_PRINT_HEX(_c)
#endif

#endif

