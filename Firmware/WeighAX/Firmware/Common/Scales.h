/* 
 * Copyright (c) 2012-2013, Newcastle University, UK.
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
 
#ifndef SCALES_H
#define SCALES_H

// Include

// Types
typedef struct
{
    unsigned char syncCode;       	// @0 	0x55
    unsigned char weightMSB;       	// @1	High byte of weight short	
    unsigned char weightLSB;        // @2  	Low byte of weight short
	unsigned char units;			// @3 	0-kg,1-lbs,2St:lbs.d
	unsigned char checksum;			// @4	Unknown format
	// USER SET BYTES
	unsigned short weight;			// Weight, native
	unsigned long	time;			// Time of measurement
} scaleData_t;

// Globals
extern volatile scaleData_t gScaleData;

extern unsigned char scales_buffer[];
extern unsigned short scales_buffer_tail;
extern unsigned short scales_buffer_head;
extern volatile unsigned char gScalesDataReady;

// Prototypes
void ScalesCommInit(void);
void ScalesCommOff(void);
void ScalesCommWait(void);
void ScalesCommClear(void);
unsigned char ScalesDataReady(void);
unsigned char ScalesDataWaiting(void);
unsigned char ScalesParseData(void);

#endif
