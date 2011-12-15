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

// Data Capture
// Dan Jackson, 2011

// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "Data.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Rtc.h"
#include "Settings.h"


// Buffer
#define DATA_BUFFER_MASK (DATA_BUFFER_CAPACITY - 1)
static DataType dataBuffer[DATA_BUFFER_CAPACITY];
static unsigned short dataBufferHead = 0;
static unsigned short dataBufferTail = 0;
static unsigned char dataBufferEvents = 0;
static unsigned long dataBufferLastDateTime = 0;
static unsigned short dataBufferLastTimer = 0x0000;

// Data interrupt
//static unsigned short dataIntStack = 0;
//#define DATA_INT_PUSH() { dataIntStack = (dataIntStack << 1) | ACCEL_INT1_IE; dataIntStack = (dataIntStack << 1) | ACCEL_INT2_IE; }
//#define DATA_INT_POP() { ACCEL_INT2_IE = dataIntStack & 1; dataIntStack >>= 1; ACCEL_INT1_IE = dataIntStack & 1; dataIntStack >>= 1; }
#define DATA_INT_DISABLE() { ACCEL_INT1_IE = 0; ACCEL_INT2_IE = 0; }
#define DATA_INT_ENABLE() { ACCEL_INT1_IE = 1; ACCEL_INT2_IE = 1; }


/*
// Circular buffer operations (b = buffer, h = head, t = tail, i = index / number of elements, c = capacity)
static unsigned int _buffer_temp;
#define BUFFER_CLEAR(_h, _t) { _h = 0; _t = 0; }
#define BUFFER_INCREMENT(_i, _c) (((_i) + 1) % (_c))
#define BUFFER_ADD(_b, _h, _t, _c, _v) { if (BUFFER_INCREMENT(_t, _c) != (_h)) { (_b)[_t] = (_v); _t = BUFFER_INCREMENT(_t, _c); } }
#define BUFFER_REMOVE(_b, _h, _t, _c) (((_h) == (_t)) ? 0 : (_buffer_temp = (_b)[_h], _h = BUFFER_INCREMENT(_h, _c), _buffer_temp))
#define BUFFER_LENGTH(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h) + (_t)))
#define BUFFER_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) + (_h) - 1) : ((_h) - (_t) - 1))
#define BUFFER_CONTIGUOUS_ENTRIES(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h)))
#define BUFFER_ENTRY_POINTER(_b, _h) ((_b) + (_h))
#define BUFFER_CONTIGUOUS_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) - ((_h) == 0 ? 1 : 0)) : ((_h) - (_t) - 1))
#define BUFFER_FREE_POINTER(_b, _t) ((_b) + (_t))
#define BUFFER_EXTERNALLY_ADDED(_t, _c, _i) ((_t) = (((_t) + (_i)) % (_c)))
#define BUFFER_EXTERNALLY_REMOVED(_h, _c, _i) ((_h) = (((_h) + (_i)) % (_c)))
*/


// Clear data buffer
void DataClear(void)
{
    dataBufferHead = 0;
    dataBufferTail = 0;
    dataBufferEvents = 0;
    dataBufferLastDateTime = 0;
    dataBufferLastTimer = 0x0000;
}


// Collect any new data -- typically called from an interrupt
inline void DataTasks(void)
{
    // Service either interrupt (but only actually configured to use INT1)
    if (ACCEL_INT1_IF || ACCEL_INT2_IF)
    {
        unsigned char source;

        // Clear interrupt flag
        ACCEL_INT1_IF = 0;
        ACCEL_INT2_IF = 0;

        // Read interrupt source
        source = AccelReadIntSource();

        // Check for watermark
        if (source & ACCEL_INT_SOURCE_WATERMARK)
        {
            unsigned short contiguous, num, passes;

            // Update timestamp for current FIFO length
            dataBufferLastDateTime = RtcNowFractional(&dataBufferLastTimer);

            // Empty hardware FIFO - up to two passes as first read may be up against the end of the circular buffer
            for (passes = 2; passes != 0; --passes)
            {
                // See how many samples we can read contiguously
                if (dataBufferTail >= dataBufferHead) { contiguous = DATA_BUFFER_CAPACITY - dataBufferTail - ((dataBufferHead == 0) ? 1 : 0); }
                else { contiguous = dataBufferHead - dataBufferTail - 1; }

                // If we aren't able to fit any in, we've over-run our software buffer
                if (contiguous == 0)
                {
                    dataBufferEvents |= DATA_EVENT_BUFFER_OVERFLOW;         // Flag a software FIFO over-run error
                    AccelReadFIFO(NULL, ACCEL_MAX_FIFO_SAMPLES);    // Dump hardware FIFO contents to prevent continuous watermark/over-run interrupts
                    break;
                }

                // Reads the ADXL hardware FIFO (bytes = ADXL_BYTES_PER_SAMPLE * entries)
                num = AccelReadFIFO((short *)&dataBuffer[dataBufferTail], contiguous);

                if (num == 0) { break; }                // No more entries to read

                // Increment and wrap tail pointer
                dataBufferTail = (dataBufferTail + num) & DATA_BUFFER_MASK;
            }
        }

        // Check for over-run
        if (source & ACCEL_INT_SOURCE_OVERRUN)
        {
            dataBufferEvents |= DATA_EVENT_FIFO_OVERFLOW;         // Flag a hardware FIFO over-run error
        }

        // Check for single-tap
        if (source & ACCEL_INT_SOURCE_SINGLE_TAP)
        {
            status.events |= DATA_EVENT_SINGLE_TAP;
        }

        // Check for double-tap
        if (source & ACCEL_INT_SOURCE_DOUBLE_TAP)
        {
            status.events |= DATA_EVENT_DOUBLE_TAP;
            status.debugFlashCount = 3;
        }
    }
    return;
}


// Returns the current length of the buffer
unsigned short DataLength(void)
{
    unsigned short length;

    // Disable interrupts so we get a consistent view of the tail pointer
    DATA_INT_DISABLE();

    // Calculate the length
    if (dataBufferTail >= dataBufferHead) { length = dataBufferTail - dataBufferHead; }
    else { length = (DATA_BUFFER_CAPACITY - dataBufferHead) + dataBufferTail; }

    // Restore interruptes
    DATA_INT_ENABLE();

    return length;
}


// Returns the most recent timestamp and relative sample offset from the start of the buffer
void DataTimestamp(unsigned long *timestamp, short *relativeOffset)
{
    short relative;

    // Disable interrupts so we get a consistent view of the timestamp and tail variables
    DATA_INT_DISABLE();

    // Calculate the sample index that that the timestamp is for (the FIFO length)
    if (dataBufferTail >= dataBufferHead) { relative = (short)(dataBufferTail - dataBufferHead); }
    else { relative = (short)((DATA_BUFFER_CAPACITY - dataBufferHead) + dataBufferTail); }
    
    // Take into account how many whole samples the fractional part of timestamp accounts for
    relative -= (short)(((unsigned long)dataBufferLastTimer * AccelFrequency()) >> 16);

    *timestamp = dataBufferLastDateTime;

    // Output relative offset
    *relativeOffset = relative;

    // Restore interruptes
    DATA_INT_ENABLE();

    return;
}


// Empties a value from the buffer
char DataPop(DataType *value, unsigned short count)
{
    unsigned short n = 0;

    // Disable interrupts to ensure consistency
    DATA_INT_DISABLE();

    while (n < count)
    {
        if (dataBufferTail == dataBufferHead) { break; }
        *value = dataBuffer[dataBufferHead];
        dataBufferHead = (dataBufferHead + 1) & DATA_BUFFER_MASK;
        n++;
    }

    // Restore interruptes
    DATA_INT_ENABLE();

    return n;
}


// Return (and clear) the events status flag
unsigned char DataEvents(void)
{
    unsigned char ret = dataBufferEvents;
    dataBufferEvents = 0;
    return ret;
}

