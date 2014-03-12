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

#ifndef DATASTREAM_H
#define DATASTREAM_H

#include "Utils/Fifo.h"

// Data stream (timestamped, FIFO data collection)
typedef struct
{
    fifo_t fifo;
    unsigned long lastDateTime;
    unsigned short lastTimeFractional;
} datastream_t;

// Interrupt mask protection functions - used to block data stream interrupts
#ifdef __C30__
	typedef unsigned char DATA_STREAM_IPL_shadow_t;
	#define DATA_STREAM_INTS_DISABLE()	{IPLshadow = SRbits.IPL; if(SRbits.IPL<DATA_STREAM_INTERRUPT_PRIORITY)SRbits.IPL=DATA_STREAM_INTERRUPT_PRIORITY;}
	#define DATA_STREAM_INTS_ENABLE()	{SRbits.IPL = IPLshadow;}
#elif defined (__C32__)
	typedef unsigned long DATA_STREAM_IPL_shadow_t;
	#define DATA_STREAM_INTS_DISABLE()	{IPLshadow = _CP0_BCS_STATUS((_CP0_STATUS_IPL_MASK&((~DATA_STREAM_INTERRUPT_PRIORITY)<<_CP0_STATUS_IPL_POSITION)),(_CP0_STATUS_IPL_MASK&(DATA_STREAM_INTERRUPT_PRIORITY<<_CP0_STATUS_IPL_POSITION)));} // Assign IPL equal to DATA_STREAM_INTERRUPT_PRIORITY
	#define DATA_STREAM_INTS_ENABLE()	{_CP0_BCS_STATUS((_CP0_STATUS_IPL_MASK&(~IPLshadow)),(_CP0_STATUS_IPL_MASK&IPLshadow));}
#endif

// Initialize the data stream, specifying the storage buffer (capacity * elementSize)
void DataStreamInit(datastream_t *dataStream, size_t elementSize, unsigned int capacity, void *buffer);

// Clear data collection buffer
void DataStreamClear(datastream_t *dataStream);

// Update the timestamp (for the end of the FIFO)
void DataStreamUpdateTimestamp(datastream_t *dataStream);

// Returns the most recent timestamp
void DataStreamTimestamp(datastream_t *dataStream, unsigned long *timestamp, unsigned short *timeFractional, unsigned int *timeStampOffset);


#endif
