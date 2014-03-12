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

#include "bt config.h"

// DEFINITIONS
#ifdef BT_DEBUG_ON
	#include <stdarg.h>			// For __line__ and __file__ in assert
	#include <stdio.h>			// For printf
	#define DBG_MASK			DBG_CLASS_HCI
	#define DBG_ENABLE 			1	// Creates functions in .c file
	#define DBG_ALL_ERRORS		1	// Overides mask for errors only
#else
	#define DBG_MASK			0
	#define DBG_ENABLE 			0
	#define DBG_ALL_ERRORS		0
#endif

// Use to set mask for all debugging
#define DBG_CLASS_ALL		0xffff
#define DBG_CLASS_APP 		0x0001
#define DBG_CLASS_PHY 		0x0002
#define DBG_CLASS_HCI 		0x0004
#define DBG_CLASS_L2CAP 	0x0008
#define DBG_CLASS_SDP 		0x0010
#define DBG_CLASS_RFCOMM 	0x0020
#define DBG_CLASS_EHCILL 	0x0040
#define DBG_CLASS_BLE		0x0800
#define DBG_CLASS_DUMP_ALL	0x1000

#define DEBUG_APP 		(DBG_MASK & DBG_CLASS_APP)
#define DEBUG_RFCOMM 	(DBG_MASK & DBG_CLASS_RFCOMM)
#define DEBUG_SDP 		(DBG_MASK & DBG_CLASS_SDP)
#define DEBUG_L2CAP 	(DBG_MASK & DBG_CLASS_L2CAP)
#define DEBUG_HCI 		(DBG_MASK & DBG_CLASS_HCI)
#define DEBUG_PHY 		(DBG_MASK & DBG_CLASS_PHY)
#define DEBUG_BLE		(DBG_MASK & DBG_CLASS_BLE)
#define DEBUG_PHY_EHCILL	(DBG_MASK & DBG_CLASS_EHCILL)
#define DEBUG_PHY_DUMP 		(DBG_MASK & DBG_CLASS_DUMP_ALL)

// OUTPUT
#define DBG_info		DBG_printf
#define DBG_error		DBG_printf

// PROTOTYPES
void DBG_printf(const char* file,const char* fmt,...);
void DBG_dump(const unsigned char* data, unsigned short len);
void DBG_assert(unsigned char condition, const char* file, unsigned short line);
void DBG_fast(const char* data,unsigned char len);
#endif

