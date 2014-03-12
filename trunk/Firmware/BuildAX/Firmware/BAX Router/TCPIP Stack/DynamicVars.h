/* 
 * Copyright (c) 2014, Newcastle University, UK.
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

// Dynamic variables
// Karim Ladha, Samuel Finnigan, 2014

#ifndef DYN_VARS_H
#define DYN_VARS_H

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

#define MAX_DYN_FIELD_ID_LEN 	17	// Max field inc. "~ /N8888888.P33 ~" = 17
#define MIN_READ_BUFF_LEN		64

typedef enum {
	DYN_LEN_ERR = -1,
	DYN_ERR = 0,
	DYN_TEXT = 1,
	DYN_BOOL,
	DYN_UINT8,
	DYN_UINT16,
	DYN_SINT16,
	DYN_UINT32,
	DYN_SINT32,
	DYN_FILE,
	DYN_TIME,
	DYN_STATE,			// state_t
	DYN_MODE,			// for verbose printing of stream modes
	DYN_CONDITION		// for parsing of simple conditionals
}DynamicType_t;

// Can make dynamic changes to table if required
#ifndef CONST_DYN_LIST 
	#define CONST_DYN_LIST	const 
#endif

typedef struct {
	CONST_DYN_LIST unsigned short id;
	CONST_DYN_LIST DynamicType_t type;
	CONST_DYN_LIST void* ptr;
}DynEntry_t;

extern DynEntry_t dynEntries[];

// Call with client id and "~XXX~" ptr to load variable, returns length of "~XXX~" field
unsigned short DynamicVarLoad(char* sourceFile, unsigned char clientIndex);

// Call to read bytes from dynamic entry, returns 0 when done
unsigned short DynamicVarRead(unsigned char clientIndex, void* buffer, unsigned short maxLen);


#endif
