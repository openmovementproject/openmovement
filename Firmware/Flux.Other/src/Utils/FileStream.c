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

#include <stdio.h>
#include <stdlib.h>

#include "HardwareProfile.h"

#include "Utils/Fifo.h"
#include "Utils/FileStream.h"
#include "Utils/Util.h"

#include "Utils/filesystem.h"

#define SECTOR_SIZE 512


// Initialize the specified file stream (structure initialized by caller)
#ifndef FILE_STREAM_USE_GLOBALS
void FileStreamInit(filestream_t *fileStream, void *sectorBuffer)
{
    fileStream->sequenceId = 0;
    fileStream->scratchBuffer = sectorBuffer;
}
#else
void FileStreamInit(filestream_t *fileStream, void *sectorBuffer)
{
    fileStream->sequenceId = 0;
}
#endif

// Increment the sequence id - causes parser to NOT interpolate between frames
void FileStreamIncrementId(filestream_t *fileStream)
{
    fileStream->sequenceId++;
}


// Prepare a stream buffer, returning a pointer to the data portion (clears unused parts of the data buffer)
void *FileStreamPrepareData(filestream_t *fileStream, unsigned long timestamp, unsigned short fractionalTime, signed short timestampOffset, unsigned short sampleCount)
{
    filestream_data_t *packet;
#ifndef FILE_STREAM_USE_GLOBALS
    packet = (filestream_data_t *)fileStream->scratchBuffer;
#else
    packet = (filestream_data_t *)scratchBuffer;
#endif
    
	packet->packetType         = 'd';
	packet->streamId           = fileStream->streamId;
	packet->payloadLength      = 0x1fc;
	packet->sequenceId         = fileStream->sequenceId;
	packet->timestamp          = timestamp;
	packet->fractionalTime     = fractionalTime;
	packet->timestampOffset    = timestampOffset;
    packet->sampleRate         = fileStream->sampleRate;
	packet->sampleRateModifier = fileStream->sampleRateModifier;
    packet->dataType           = fileStream->dataType;
	packet->dataConversion     = fileStream->dataConversion;
	packet->channelPacking     = fileStream->channelPacking;
	packet->sampleCount        = sampleCount;
    
#if 0
    // Clear unused data parts
    {
        size_t elementSize = 0;
        size_t used = 0;
        
        // TODO: Calculate element size if known types
        elementSize = ?;
        
        used = elementSize * sampleCount;
        if (used < 480)
        {
            memset(fileStream->scratchBuffer + 24, 0, 480 - used);
        }
    }
#endif
   
    // Clear tail of the packet
	packet->aux[0]             = 0;
	packet->aux[1]             = 0;
	packet->aux[2]             = 0;
	packet->checksum           = 0;
    
    // Increment sequence id (unique for each prepared header)
    fileStream->sequenceId++;

	return ((unsigned char *)packet + 24);
}


// Output a stream buffer (calculates the checksum before output if needed)
char FileStreamOutputData(filestream_t *fileStream, char useEcc, char useChecksum)
{
    filestream_data_t *packet;
    FILE *fp;
#ifndef FILE_STREAM_USE_GLOBALS
    packet = (filestream_data_t *)fileStream->scratchBuffer;
    fp = (FILE *)fileStream->fileHandle;
#else
    packet = (filestream_data_t *)scratchBuffer;
	#ifdef FILE_STREAM_IGNORE_FILE_PTR
		//KL - if we are not writing to a file then we dont care if its void
		fp = (void*)!NULL;
	#else
	    fp = (FILE *)logFile;
	#endif
#endif

    // Calculate packet checksum
    if (useChecksum)
    {
        packet->checksum = checksum(packet, 510);
    }
    
    // Check we have an open handle
    if (fp != NULL)
    {	   
	    // Output the sector
#if defined(fwriteSector)
	    if (fwriteSector(packet, fp, useEcc))
	    {
	        return 1;
	    }
#else
		if (fwrite(packet, 1, SECTOR_SIZE, fp) == SECTOR_SIZE)
		{
			return 1;
		}
#endif

	} 
    
    return 0;
}


