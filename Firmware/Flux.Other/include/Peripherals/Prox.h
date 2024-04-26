/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// Driver for the VCNL4000 proximity sensor and future devices
// Karim Ladha 21-12-2012
#ifndef PROX_H
#define PROX_H

#include "HardwareProfile.h" // To pick up defines for NULL implementation

// Includes
typedef union{
	struct {
		unsigned short light;
		unsigned short proximity;
	};
	unsigned char values[4];
} prox_t;

#ifdef HAVE_PROXIMITY_SENSOR

// Globals
extern prox_t prox;
extern char proxPresent;

// Read device ID, sets present flag
unsigned char ProxVerifyDeviceId(void);

// Prox startup
void ProxStartup(void);

// Initiate on demand measurement
void ProxStandby(void);

// Initiate a conversion
void ProxStartSample(void);

// Reads a conversion
prox_t* ProxReadSample(void);

// Checks on current conversion - TRUE means done
unsigned char ProxSampleReady(void);

#else
	// Null function replacements - no need to compile driver
	#define 	proxPresent				0
	#define 	ProxVerifyDeviceId()	0
	#define 	ProxStartup()
	#define 	ProxStandby()
	#define 	ProxStartSample()
	#define 	ProxReadSample()		((void*)NULL)
	#define 	ProxSampleReady()		0
#endif

#endif


