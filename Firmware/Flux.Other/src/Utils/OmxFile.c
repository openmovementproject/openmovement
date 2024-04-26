/* 
 * Copyright (c) 2009-2016, Newcastle University, UK.
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


// OmxFile - Dan Jackson, 2016.

/*
Methods and structures for dealing with OMX file headers and output streams.

'omx_stream' type is an output stream (replaces the old 'FileStream' functionality).
The state of the output stream is its omx_stream_type and the stream sequence number.
Operations on the stream include initialization and preparing a sector for writing.

Other operations are on OMX sectors, and checking the header.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/Util.h"

#include "Utils/OmxFile.h"



// (Internal) OMX sample size for a sector type and channel-packing
int OmxSectorSampleSize(unsigned char packetType, uint8_t channelPacking)
{
	int sampleSize = 0;
	
	if (packetType == OMX_PACKET_TYPE_STREAM)
	{
		// TODO: Add special handling for 3x10-bit -> 32-bit DWORD packing, and 3x12x2-bit
		
		sampleSize = (channelPacking & 0x07);		// Bytes-per-sample for OMX_PACKING_xINT8/OMX_PACKING_xINT16/OMX_PACKING_xINT24/OMX_PACKING_xINT32
		if (sampleSize > 5) { sampleSize -= 2; }	// OMX_PACKING_FLOAT/OMX_PACKING_DOUBLE mapped to OMX_PACKING_xINT32/OMX_PACKING_xINT64
		if (sampleSize == 5) { sampleSize = 8; }	// OMX_PACKING_xINT64
		sampleSize *= (channelPacking >> 4);		// Multiply by number of channels
	}
	else if (packetType == OMX_PACKET_TYPE_EVENT)
	{
		sampleSize = channelPacking;
	}
	else if (packetType == OMX_PACKET_TYPE_STRING)
	{
		sampleSize = 1;
	}
	
	return sampleSize;
}



// Initialize as a OMX_PACKET_TYPE_STREAM
void OmxStreamTypeInitStream(omx_stream_type_t *omxStreamType, unsigned char streamId, unsigned short sampleRate, char sampleRateModifier, unsigned char dataType, char dataConversion, unsigned char channelPacking)
{
	memset(omxStreamType, 0, sizeof(omx_stream_type_t));
	
	omxStreamType->packetType = OMX_PACKET_TYPE_STREAM;
	omxStreamType->streamId = streamId;
	omxStreamType->sampleRate = sampleRate;
	omxStreamType->sampleRateModifier = sampleRateModifier;
	omxStreamType->dataType = dataType;
	omxStreamType->dataConversion = dataConversion;
	omxStreamType->channelPacking = channelPacking;
}


// Initialize as a OMX_PACKET_TYPE_EVENT
void OmxStreamTypeInitEvent(omx_stream_type_t *omxStreamType, unsigned char streamId, unsigned char dataType, unsigned char dataSubType, size_t elementSize)
{
	memset(omxStreamType, 0, sizeof(omx_stream_type_t));
	
	omxStreamType->packetType = OMX_PACKET_TYPE_EVENT;
	omxStreamType->streamId = streamId;
	omxStreamType->sampleRate = 0;
	omxStreamType->sampleRateModifier = 0;
	omxStreamType->dataType = dataType;
	omxStreamType->dataConversion = dataSubType;
	omxStreamType->channelPacking = (unsigned char)elementSize;
}


// Initialize as a OMX_PACKET_TYPE_STRING
void OmxStreamTypeInitString(omx_stream_type_t *omxStreamType)
{
	memset(omxStreamType, 0, sizeof(omx_stream_t));
	
	omxStreamType->packetType = OMX_PACKET_TYPE_EVENT;
	omxStreamType->streamId = '*';
	omxStreamType->sampleRate = 0;
	omxStreamType->sampleRateModifier = 0;
	omxStreamType->dataType = 0;
	omxStreamType->dataConversion = 0;
	omxStreamType->channelPacking = 0;
}


// Size of each sample for the specified stream type
int OmxStreamTypeSampleSize(const omx_stream_type_t *omxStreamType)
{
	return OmxSectorSampleSize(omxStreamType->packetType, omxStreamType->channelPacking);
}




// Initialize an OMX stream based on the given type
void OmxStreamInit(omx_stream_t *omxStream, const omx_stream_type_t *omxStreamType)
{
	int sampleSize;

	memset(omxStream, 0, sizeof(omx_stream_t));

	omxStream->streamType = *omxStreamType;

	sampleSize = OmxStreamTypeSampleSize(&omxStream->streamType);
	if (sampleSize > 0)
	{
		omxStream->maxSamples = OMX_SECTOR_DATA_SIZE / sampleSize;
	}
	else 
	{
		omxStream->maxSamples = 0;
	}
}


// Maximum number of samples per sector for the specified stream
int OmxStreamMaxSamples(const omx_stream_t *omxStream)
{
	return omxStream->maxSamples;
}


// Prepare a buffer (caller must ensure buffer is aligned), returning a pointer to the data portion
void *OmxStreamPrepareSector(const omx_stream_t *omxStream, void *buffer, uint32_t timestamp, uint16_t fractionalTime, int16_t timestampOffset)
{
    const omx_stream_type_t *streamType = &omxStream->streamType;
    omx_data_t *packet = (omx_data_t *)buffer;

	packet->packetType         = streamType->packetType;
	packet->streamId           = streamType->streamId;
	packet->payloadLength      = 0x1fc;
	packet->sequenceId         = omxStream->sequenceId;
	packet->timestamp          = timestamp;
	packet->fractionalTime     = fractionalTime;
	packet->timestampOffset    = timestampOffset;
    packet->sampleRate         = streamType->sampleRate;
	packet->sampleRateModifier = streamType->sampleRateModifier;
    packet->dataType           = streamType->dataType;
	packet->dataConversion     = streamType->dataConversion;
	packet->channelPacking     = streamType->channelPacking;
	packet->sampleCount        = 0;
    
    // Clear tail of the packet
	packet->aux[0]             = 0;
	packet->aux[1]             = 0;
	packet->aux[2]             = 0;
	packet->checksum           = 0;

	return ((unsigned char *)packet + 24);
}


// Increments the sequence number of the stream (once a prepared sector is successfully written)
void OmxStreamIncrementSequence(omx_stream_t *omxStream)
{
	omxStream->sequenceId++;
}


// Returns the number of entries already in a sector
int OmxSectorSampleCount(const void *buffer)
{
    const omx_data_t *packet = (const omx_data_t *)buffer;
	return (int)packet->sampleCount;
}


// Updates a sector buffer's sample count to include samples that has been (externally) added to the buffer
void OmxSectorSamplesAdded(void *buffer, int addedSampleCount)
{
    omx_data_t *packet = (omx_data_t *)buffer;
	packet->sampleCount += addedSampleCount;
	return;
}


// Finalize a sector
void OmxSectorFinalize(void *buffer)
{
    omx_data_t *packet = (omx_data_t *)buffer;
	
	// Blank any unused bytes for known data packets
	int sampleSize = OmxSectorSampleSize(packet->packetType, packet->channelPacking);
	if (sampleSize > 0)
	{
		int length = packet->sampleCount * sampleSize;
		if (length >= 0 && length < OMX_SECTOR_DATA_SIZE)
		{
			memset(packet->data + length, 0, OMX_SECTOR_DATA_SIZE - length);
		}
	}
	
    // Add checksum
    packet->checksum = checksum(packet, 510);
}


// Interpret a sector buffer as an omx_fileheader_t, returns NULL if it is not a valid header
omx_fileheader_t *OmxReadHeader(void *buffer)
{
	omx_fileheader_t *header = (omx_fileheader_t *)buffer;
	if (header == NULL) { return NULL; }
	if (header->packetType != 'H' || header->packetSubType != 'A') { return NULL; }
	//if (checksum(packet, 512) != 0) { return NULL; }
	return header;
}


// Creates an annotation sector with the specified data
void OmxSectorAnnotation(void *buffer, uint32_t timestamp, uint16_t fractionalTime, const unsigned char *annotation, int length)
{
    omx_data_t *packet = (omx_data_t *)buffer;

    // Length limit
    if (annotation == NULL) { length = 0; }
    if (length < 0) { length = strlen((const char *)annotation); }
    if (length > 480) { length = 480; }

    // Clear buffer
    memset(buffer, 0, 512);

    // Set fields
    packet->packetType = 's';       // 's' = string
    packet->streamId = '*';         // '*' = all streams
    packet->payloadLength = 0x1fc;  // 508 bytes + 4 header/length = 512 bytes total
    packet->timestamp = timestamp;
    packet->fractionalTime = fractionalTime;
    packet->timestampOffset = 0;
    packet->sampleCount = length;

    // Add annotation data
    memcpy(packet->data, annotation, length);

    // Add checksum
    OmxSectorFinalize(buffer);
}

