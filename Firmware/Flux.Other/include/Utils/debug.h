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
// Karim Ladha, 2015

#ifndef _DEBUG_H_
#define _DEBUG_H_

// Includes
#include <stdint.h>	// For standard types
#include <stdlib.h>	// For allocation vars
#include <stdarg.h> // For __line__ and __file__
#include "app_config.h"	// For DEBUG_LEVEL_GLOBAL override

// Each file should define a separate level if required. If not set then the
// global setting will be used. It that is not set then debugging is off.
// If the global setting exceeds the local one then it will override it.
// 0 = off
// 1 = assertions only (minimum)
// 2 = previous + text errors
// 3 = previous + text info
// 4 = previous + verbose info

#if 0
// Debug levels - copy to config.h file for better setting readability if desired
#define DBG_OUTPUT_NONE		0	/* No debugging information output on debug channel		*/
#define DBG_OUTPUT_MIN		1	/* Assertions and errors, errors changed to assertions 	*/
#define DBG_OUTPUT_ERRORS	2	/* Assertions and errors, with normal error messages 	*/
#define DBG_OUTPUT_INFO		3	/* As above but with additional information messages 	*/
#define DBG_OUTPUT_VERBOSE	4	/* Verbose debug messages and heap debugging enabled	*/
#endif

// Definitions
// Compile time assertions
#define CTA_MAKE_FUNC_NAME(_line_)		void DEBUG_COMPILE_TIME_ASSERTION_FAILED_ON_LINE__ ## _line_ (void) 
#define CTA_USE_FUNC_NAME(_line_)		CTA_MAKE_FUNC_NAME(_line_)
#define DBG_STATIC_ASSERT(_assertion)	CTA_USE_FUNC_NAME(__LINE__){switch(0){case 0:; case((_assertion)):; }}

// Un-define first to avoid ordering errors and allow re-definition of debug settings
#undef DBG_VERBOSE
#undef DBG_INFO
#undef DBG_DUMP
#undef DBG_FAST
#undef DBG_ERROR
#undef DBG_ASSERT
	
// Local debug level not defined (shouldn't be needed)
#if !defined(DEBUG_LEVEL_LOCAL) && !defined(DEBUG_LEVEL_GLOBAL)
	// Define a global level or local debug level to remove error
	// Include this file after config.h to retrieve debug settings
	// Optionally define DEBUG_LEVEL_LOCAL and DBG_FILE first
	#error "Debug inherently disabled for file."
#else
// Set the debugging behaviour for the source file including this header

// Global minimum debug level, override local if required
#ifndef	DEBUG_LEVEL_LOCAL
	// If no local level set, use global setting
	#define DEBUG_LEVEL_LOCAL	DEBUG_LEVEL_GLOBAL
#elif defined(DEBUG_LEVEL_GLOBAL)
	// If global and local levels are set, check if global overrides local  
	#if ((DEBUG_LEVEL_GLOBAL) > (DEBUG_LEVEL_LOCAL))
		#undef 	DEBUG_LEVEL_LOCAL
		#define DEBUG_LEVEL_LOCAL	DEBUG_LEVEL_GLOBAL
	#endif
#else
	// The debug setting has been selected locally and no global level is set
#endif

// Select functions enabled
#if ((DEBUG_LEVEL_LOCAL) >= 4)
	// Level 4 or more
	#define DBG_VERBOSE(X,...)	DBG_printf(dbg_file,X,##__VA_ARGS__)
#endif

#if ((DEBUG_LEVEL_LOCAL) >= 3)
	// Level 3 or more
    #define DBG_INFO(X,...)		DBG_printf(dbg_file,X,##__VA_ARGS__)
    #define DBG_DUMP(X, Y)		DBG_dump(X,Y)
	#define DBG_FAST(X)			DBG_fast(X)
#endif

// Errors can be text or assertions
#if ((DEBUG_LEVEL_LOCAL) == 1)
	// Level 1, errors changed to assertions - ASSERT:File:text\r\n
	#define DBG_ERROR(X,...)	DBG_assert(0,dbg_file,(uint16_t)__LINE__)
#elif ((DEBUG_LEVEL_LOCAL) >= 2)
	// Level 2+, errors output format - ERROR:File:text\r\n
    #define DBG_ERROR(X,...)	DBG_printf(dbg_file_error,X,##__VA_ARGS__)
#endif

#if ((DEBUG_LEVEL_LOCAL) >= 1)
	// Assertion string to highlight them when using verbose outputs
	#ifndef DBG_ASSERT_TEXT
	#define DBG_ASSERT_TEXT		"ASSERT"
	#endif
	// Error string to differentiate debug info from debug errors
	#ifndef DBG_ERROR_TEXT
	#define DBG_ERROR_TEXT 		"ERROR!:"
	#define DBG_ERROR_TEXT_LEN	7
	#endif

	#ifdef DBG_FILE
		// File name constants if defined. Only instatiated once to minimise memory use
		// To remove the 'unused variable' warnings (no debug calls in source) just remove DBG_FILE definition
		static const char dbg_local_file_name[] = DBG_ERROR_TEXT DBG_FILE;
		static const char* const dbg_file_error = dbg_local_file_name;
		static const char* const dbg_file = dbg_local_file_name + DBG_ERROR_TEXT_LEN;
		// Optional but useful source location function
		#ifdef DBG_GET_FILE_ENABLED
		static inline __attribute__((always_inline))const char* DBG_get_file(void){return dbg_file;}
		#endif
	#else
		// Exactly one default file name string constant created externally (in debug.c)
		extern const char* dbg_file;
	#endif
	// Level 1, errors reported as assertions instead removing string constants for reduced memory use
	#define DBG_ASSERT(X)		DBG_assert(X,dbg_file,(uint16_t)__LINE__)
#endif

#if ((DEBUG_LEVEL_LOCAL) <= 0)
	// All debug function definitions are empty; No debugging calls are compiled in the source file
#endif

// Debugging dynamic memory (set debug heap >= 1 or local level >= 3)
// Includes memory tracking for leak identification and canary padding 
// to identify boundary violations. Tracks source files and line numbers
// of allocation and freeing whilst monitoring heap size and allocation 
// total count. Calling free will memory bounding integrity (canaries).
#if (((DEBUG_LEVEL_LOCAL) >= 3) || ((DEBUG_HEAP) >= 1))
	// Dynamic memory debug
	#define DBG_MALLOC(_s)	DBG_malloc(_s,dbg_file,(uint16_t)__LINE__)
	#define DBG_FREE(_p)	DBG_free(_p,dbg_file,(uint16_t)__LINE__)
#else
	// No debug functions used at all used
	// Dynamic memory debug
	#define DBG_MALLOC(_s)	malloc(_s)
	#define DBG_FREE(_p)	free(_p)
#endif
// Ensure it is clear that the calloc function is not part of the debug module
#define DBG_CALLOC		"This dynamic function is not currently implemented"

// Allow for include ordering sequence errors and allow debug level redefinition
// A source file can re-include this file to change the debug settings from any point
#undef DEBUG_LEVEL_LOCAL 
#endif // ifdef DEBUG_LEVEL_LOCAL

// Any functions not used, defined as empty and not compiled
#ifndef DBG_VERBOSE
	#define DBG_VERBOSE(X,...)
#endif
#ifndef DBG_INFO
	#define DBG_INFO(X,...)
#endif
#ifndef DBG_DUMP
	#define DBG_DUMP(X,Y)
#endif
#ifndef DBG_FAST
	#define DBG_FAST(X)
#endif
#ifndef DBG_ERROR
	#define DBG_ERROR(X,...)	
#endif
#ifndef DBG_ASSERT
	#define DBG_ASSERT(X)
#endif

// PROTOTYPES OF DEBUG FUNCTIONS, ALWAYS CALLED VIA DEFINITION MACROS
extern void DBG_Init(void);
extern void DBG_assert(uint8_t condition, const char* file, uint16_t line);
extern void DBG_error(const char* file,const char* fmt,...);
extern void DBG_printf(const char* file,const char* fmt,...);
extern void DBG_dump(const uint8_t* data, uint16_t len);
extern void DBG_fast(const char* fmt);
extern void* DBG_malloc(size_t size, const char* file, unsigned short line);
extern void* DBG_calloc(size_t size, const char* file, unsigned short line);
extern void DBG_free(void* ptr, const char* file, unsigned short line);
// UTILITY FUNCTIONS TO SPEED UP DEBUG VARIABLE TO TEXT OUTPUT
extern const char* DBG_ultoa(unsigned long lv);
extern const char* DBG_uitoa(unsigned short v);

#endif
// EOF
