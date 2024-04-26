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

// Data Logger
// Dan Jackson, 2010-2012


#ifndef LOGGER_H
#define LOGGER_H

// Includes
#include "Utils/Fifo.h"

// Data type
typedef struct { unsigned short x, y, z; } DataType;        // Data type

// TODO: Make a larger buffer (something breaks at the moment if it is larger)
#define DATA_BUFFER_CAPACITY 256   // Buffer size -- must be a power of 2. Memory requirement is * sizeof(DataType) -- e.g. 256 entries takes 1.5 kB

// Data buffer
typedef struct
{
    fifo_t fifo;
    unsigned long lastDateTime;
    unsigned short lastTimeFractional;
    DataType buffer[DATA_BUFFER_CAPACITY];
} DataStream;

// Initialize data collection buffer
void DataInit(DataStream *dataStream);

// Clear data collection buffer
void DataClear(DataStream *dataStream);

// Update the timestamp (for the end of the FIFO)
void DataUpdateTimestamp(DataStream *dataStream);

// Returns the most recent timestamp
void DataTimestamp(DataStream *dataStream, unsigned long *timestamp, unsigned short *timeFractional, unsigned short *fifoLength);

// Data events
#define DATA_EVENT_NONE                0x00
#define DATA_EVENT_RESUME              0x01
#define DATA_EVENT_SINGLE_TAP          0x02
#define DATA_EVENT_DOUBLE_TAP          0x04
//#define DATA_EVENT_EVENT               0x08
#define DATA_EVENT_FIFO_OVERFLOW       0x10
#define DATA_EVENT_BUFFER_OVERFLOW     0x20
//#define DATA_EVENT_UNHANDLED_INTERRUPT 0x40
//#define DATA_EVENT_CHECKSUM_FAIL       0x80	// Not used internally on CWA





// Collect any new data (usually called from interrupt)
inline void LoggerAccelTasks(void);

#ifdef USE_GYRO
// Collect any new data -- typically called from an interrupt
inline void LoggerGyroTasks(void);
#endif


// Clear the data buffers
void LoggerClear(void);

// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename);

// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename, char debug);


// Start logging
char LoggerStart(const char *filename);

// Write a logging sector
short LoggerWrite(void);

// Stop logging
void LoggerStop(void);


#endif
