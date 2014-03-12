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

// FIFO buffer
// Dan Jackson, 2011-2012

#include <stdlib.h>
#include <string.h>

#include "Utils/Fifo.h"


/*
// Circular buffer operations (b = buffer, h = head, t = tail, i = index / number of elements, c = capacity)
static unsigned int _fifo_temp;
#define FIFO_CLEAR(_h, _t) { _h = 0; _t = 0; }
#define FIFO_INCREMENT(_i, _c) (((_i) + 1) % (_c))
#define FIFO_ADD(_b, _h, _t, _c, _v) { if (FIFO_INCREMENT(_t, _c) != (_h)) { (_b)[_t] = (_v); _t = FIFO_INCREMENT(_t, _c); } }
#define FIFO_REMOVE(_b, _h, _t, _c) (((_h) == (_t)) ? 0 : (_fifo_temp = (_b)[_h], _h = FIFO_INCREMENT(_h, _c), _fifo_temp))
#define FIFO_LENGTH(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h) + (_t)))
#define FIFO_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) + (_h) - 1) : ((_h) - (_t) - 1))
#define FIFO_CONTIGUOUS_ENTRIES(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h)))
#define FIFO_ENTRY_POINTER(_b, _h) ((_b) + (_h))
#define FIFO_CONTIGUOUS_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) - ((_h) == 0 ? 1 : 0)) : ((_h) - (_t) - 1))
#define FIFO_FREE_POINTER(_b, _t) ((_b) + (_t))
#define FIFO_EXTERNALLY_ADDED(_t, _c, _i) ((_t) = (((_t) + (_i)) % (_c)))
#define FIFO_EXTERNALLY_REMOVED(_h, _c, _i) ((_h) = (((_h) + (_i)) % (_c)))
*/


// Initialize FIFO data structure
void FifoInit(fifo_t *fifo, unsigned short elementSize, unsigned short capacity, void *buffer)
{
    fifo->elementSize = elementSize;
    fifo->capacity = capacity;
    fifo->buffer = buffer;
    fifo->head = 0;
    fifo->tail = 0;

    // Check if is power of two
    if ((capacity & (capacity - 1)) == 0)
    {
        fifo->mask = capacity - 1;
    }
    else
    {
        fifo->mask = 0;
    }
}


// Clear FIFO
void FifoClear(fifo_t *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
}


// See how many contiguous free spaces there are
unsigned short FifoContiguousSpaces(fifo_t *fifo, void **buffer)
{
    unsigned short contiguous;

    // Find the buffer pointer
    if (buffer != NULL)
    {
        // TODO: Save the multiply by keeping track of the byte pointer to the tail?
        *buffer = (void *)((unsigned char *)fifo->buffer + fifo->tail * fifo->elementSize);
    }

    // See how many entries we can contiguously write to
    if (fifo->tail >= fifo->head) { contiguous = fifo->capacity - fifo->tail - ((fifo->head == 0) ? 1 : 0); }
    else { contiguous = fifo->head - fifo->tail - 1; }

    return contiguous;
}


// See how many contiguous entries there are
unsigned short FifoContiguousEntries(fifo_t *fifo, void **buffer)
{
    unsigned short contiguous;

    // Find the buffer pointer
    if (buffer != NULL)
    {
        // TODO: Save the multiply by keeping track of the byte pointer to the head?
        *buffer = (void *)((unsigned char *)fifo->buffer + fifo->head * fifo->elementSize);
    }

    // See how many entries we can contiguously read from
    if (fifo->tail >= fifo->head) { contiguous = fifo->tail - fifo->head; }
    else { contiguous = fifo->capacity - fifo->head; }

    return contiguous;
}


// Data has been directly added to the FIFO
void FifoExternallyAdded(fifo_t *fifo, unsigned short count)
{
    // TODO: Check if count > freeLength
    fifo->tail = fifo->mask ? ((fifo->tail + count) & fifo->mask) : ((fifo->tail + count) % fifo->capacity);
}


// Data has been directly removed from the FIFO
void FifoExternallyRemoved(fifo_t *fifo, unsigned short count)
{
    // TODO: Check if count > length
    fifo->head = fifo->mask ? ((fifo->head + count) & fifo->mask) : ((fifo->head + count) % fifo->capacity);
}


// Returns the current length of the FIFO
unsigned short FifoLength(fifo_t *fifo)
{
    unsigned short length;

    // Calculate the length
    if (fifo->tail >= fifo->head) { length = fifo->tail - fifo->head; }
    else { length = (fifo->capacity - fifo->head) + fifo->tail; }

    return length;
}


// Returns the free space left in the FIFO
unsigned short FifoFree(fifo_t *fifo)
{
    unsigned short free;

    // Calculate the free
    if (fifo->tail >= fifo->head) { free = fifo->capacity - fifo->tail + fifo->head - 1; }
    else { free = fifo->head - fifo->tail - 1; }

    return free;
}


// Empty values from the FIFO
unsigned short FifoPop(fifo_t *fifo, void *values, unsigned short count)
{
    unsigned short remaining;
    char pass = 0;

    // Up to two passes (FIFO wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned short contiguous, n = 0;
        
        // See how many entries to process in this pass
        contiguous = FifoContiguousEntries(fifo, &bufferPointer);
        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
        if (n <= 0) { break; }              // No more

		// Copy n * fifo->elementSize bytes
		if (values != NULL)
		{
			unsigned short length = n * fifo->elementSize;
			memcpy(values, bufferPointer, length);
			values = (unsigned char *)values + length;
		}
        
        // Update head pointer
        FifoExternallyRemoved(fifo, n);
        
        // Decrease number remaining
        remaining -= n;
        if (remaining <= 0) { break; }      // Processed all
    }
    
    // Return number of entries processed
    return (count - remaining);
}


// Add values to the FIFO
unsigned short FifoPush(fifo_t *fifo, void *values, unsigned short count)
{
    unsigned short remaining;
    char pass = 0;

    // Up to two passes (FIFO wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned short contiguous, n = 0;

        // See how many entries to process in this pass
        contiguous = FifoContiguousSpaces(fifo, &bufferPointer);
        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
        if (n <= 0) { break; }              // No more

        // Copy n * fifo->elementSize bytes
		if (values != NULL)
		{
			unsigned short length = n * fifo->elementSize;
			memcpy(bufferPointer, values, length);
			values = (unsigned char *)values + length;
		}

        // Update tail pointer
        FifoExternallyAdded(fifo, n);

        // Decrease number remaining
        remaining -= n;
        if (remaining <= 0) { break; }      // Processed all
    }

    // Return number of entries processed
    return (count - remaining);
}


