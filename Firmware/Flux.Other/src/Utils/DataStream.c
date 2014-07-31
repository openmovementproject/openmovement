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

#include <stdlib.h>
#include <Compiler.h>
#include "HardwareProfile.h"
#include "Peripherals/Rtc.h"
#ifndef USE_FAT_FS
#include "MDD File System/FSIO.h"
#else
#include "FatFs/FatFsIo.h"
#endif
#include "Utils/Fifo.h"
#include "Utils/FileStream.h"
#include "Utils/DataStream.h"


// Initialize the data stream, specifying the storage buffer (capacity * elementSize)
void DataStreamInit(datastream_t *dataStream, size_t elementSize, unsigned int capacity, void *buffer)
{
    FifoInit(&dataStream->fifo, elementSize, capacity, buffer);
    DataStreamClear(dataStream);
}


// Clear data collection buffer
void DataStreamClear(datastream_t *dataStream)
{
	DATA_STREAM_IPL_shadow_t IPLshadow;
	DATA_STREAM_INTS_DISABLE();
    FifoClear(&dataStream->fifo);
    dataStream->lastDateTime = 0;
    dataStream->lastTimeFractional = 0x0000;
	DATA_STREAM_INTS_ENABLE();
}


// Update the timestamp (for the end of the FIFO)
void DataStreamUpdateTimestamp(datastream_t *dataStream)
{
	DATA_STREAM_IPL_shadow_t IPLshadow;
	DATA_STREAM_INTS_DISABLE();
    dataStream->lastDateTime = RtcNowFractional(&dataStream->lastTimeFractional);
	DATA_STREAM_INTS_ENABLE();
}


// Returns the most recent timestamp and relative sample offset from the start of the buffer (must disable interrupts)
void DataStreamTimestamp(datastream_t *dataStream, unsigned long *timestamp, unsigned short *timeFractional, unsigned int *timeStampOffset)
{
	DATA_STREAM_IPL_shadow_t IPLshadow;
	DATA_STREAM_INTS_DISABLE();

    *timestamp = dataStream->lastDateTime;

    // Return the fractional seconds (1/65536 s)
    if (timeFractional != NULL)
    {
        *timeFractional = dataStream->lastTimeFractional;
    }

    // Calculate the sample index that that the timestamp is for (the FIFO length)
    if (timeStampOffset != NULL)
    {
        *timeStampOffset = FifoLength(&dataStream->fifo); 
    }

	DATA_STREAM_INTS_ENABLE();
    
    return;
}

