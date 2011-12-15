/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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

// Data capture
// Dan Jackson, 2011


// Data type
typedef struct { unsigned short x, y, z; } DataType;

// Buffer size
#define DATA_BUFFER_CAPACITY 256   // Must be a power of 2. Memory requirement is * sizeof(DataType) -- e.g. 256 entries takes 1.5 kB

// Clear data collection buffer
void DataClear(void);

// Collect any new data (usually called from interrupt)
inline void DataTasks(void);

// Returns the current length of the buffer
unsigned short DataLength(void);

// Returns the most recent timestamp and relative sample offset from the start of the buffer
void DataTimestamp(unsigned long *timestamp, short *relativeOffset);

// Empties a value from the buffer
char DataPop(DataType *value, unsigned short count);



#define DATA_EVENT_NONE                0x00
#define DATA_EVENT_RESUME              0x01
#define DATA_EVENT_SINGLE_TAP          0x02
#define DATA_EVENT_DOUBLE_TAP          0x04
//#define DATA_EVENT_EVENT               0x08
#define DATA_EVENT_FIFO_OVERFLOW       0x10
#define DATA_EVENT_BUFFER_OVERFLOW     0x20
//#define DATA_EVENT_UNHANDLED_INTERRUPT 0x40
//#define DATA_EVENT_CHECKSUM_FAIL       0x80	// Not used internally on CWA


// Return (and clear) the events status flag
unsigned char DataEvents(void);

