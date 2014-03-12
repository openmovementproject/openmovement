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

// Basic debug out file
// Karim Ladha, 2013-2014

#ifndef _DEBUG_H_
#define _DEBUG_H_

// Includes
#include "HardwareProfile.h"

// Prototypes
void dbg_print_const (const char* ptr);
void dbg_assert(unsigned char test);
void dbg_dump(unsigned char* data, unsigned char len, char separator);
void dbg_print_int (int val);
void dbg_print_hex (int val);

#ifdef DEBUG_ON
	#define ASSERT(_x)		dbg_assert(_x)
	#define DBG_INFO(_s)	dbg_print_const(_s)
	#define DBG_ERROR(_s)	dbg_print_const(_s)
	#define DBG_DUMP(_p,_l,_s)	dbg_dump(_p,_l,_s)
	#define DBG_PRINT_INT(_i) 	dbg_print_int(_i)
	#define DBG_PRINT_HEX(_c) 	dbg_print_hex(_c)
#else
	#define ASSERT(_x)		
	#define DBG_INFO(_s)	
	#define DBG_ERROR(_s)	
	#define DBG_DUMP(_p,_l,_s)
	#define DBG_PRINT_INT(_i)	
	#define DBG_PRINT_HEX(_c)
#endif


#endif