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
#include "Compiler.h"
#if defined(__C30__) || defined (__C32__)
#include "HardwareProfile.h"
#elif defined(__arm__)
#include "Config.h"
#endif


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
void FifoInit(fifo_t *fifo, size_t elementSize, unsigned int capacity, void FIFO_EDS *buffer)
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
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();
    fifo->head = 0;
    fifo->tail = 0;
	FIFO_INTS_ENABLE();
}


// See how many contiguous free spaces there are
unsigned int FifoContiguousSpaces(fifo_t *fifo, void **buffer)
{
    unsigned int contiguous;
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();

    // Find the buffer pointer
    if (buffer != NULL)
    {
        // TODO: Save the multiply by keeping track of the byte pointer to the tail?
// WARNING: Not EDS-compatible
        *buffer = (void *)((unsigned char *)fifo->buffer + fifo->tail * fifo->elementSize);
    }

    // See how many entries we can contiguously write to
    if (fifo->capacity == 0) { contiguous = 0; }
    else if (fifo->tail >= fifo->head) { contiguous = fifo->capacity - fifo->tail - ((fifo->head == 0) ? 1 : 0); }
    else { contiguous = fifo->head - fifo->tail - 1; }

	FIFO_INTS_ENABLE();

    return contiguous;
}


// See how many contiguous free spaces there are (alternative)
void FIFO_EDS *FifoContiguousSpaces2(fifo_t *fifo,  unsigned int *contiguous)
{
	void FIFO_EDS *retVal;
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();

    // TODO: Save the multiply by keeping track of the byte pointer to the tail?
    retVal = (void FIFO_EDS *)((unsigned char FIFO_EDS *)fifo->buffer + fifo->tail * fifo->elementSize);

    // See how many entries we can contiguously write to
	if (contiguous != NULL)
	{
	    if (fifo->capacity == 0) { *contiguous = 0; }
    	else if (fifo->tail >= fifo->head) { *contiguous = fifo->capacity - fifo->tail - ((fifo->head == 0) ? 1 : 0); }
    	else { *contiguous = fifo->head - fifo->tail - 1; }
	}

	FIFO_INTS_ENABLE();

	return retVal;
}


// See how many contiguous entries there are
unsigned int FifoContiguousEntries(fifo_t *fifo, void **buffer)
{
    unsigned int contiguous;
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();

    // Find the buffer pointer
    if (buffer != NULL)
    {
        // TODO: Save the multiply by keeping track of the byte pointer to the head?
// WARNING: Not EDS-compatible
        *buffer = (void *)((unsigned char *)fifo->buffer + fifo->head * fifo->elementSize);
    }

    // See how many entries we can contiguously read from
    if (fifo->tail >= fifo->head) { contiguous = fifo->tail - fifo->head; }
    else { contiguous = fifo->capacity - fifo->head; }

	FIFO_INTS_ENABLE();

    return contiguous;
}


// See how many contiguous entries there are (alternative)
void FIFO_EDS *FifoContiguousEntries2(fifo_t *fifo,  unsigned int *contiguous)
{
	void FIFO_EDS *retVal;
	unsigned int offset;

	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();

    // Find the buffer pointer
    // TODO: Save the multiply by keeping track of the byte pointer to the head?
	offset = fifo->head * fifo->elementSize;
    retVal = (void FIFO_EDS *)((unsigned char FIFO_EDS *)fifo->buffer + offset);

    // See how many entries we can contiguously read from
    if (contiguous != NULL)
	{
	    if (fifo->tail >= fifo->head) { *contiguous = fifo->tail - fifo->head; }
	    else { *contiguous = fifo->capacity - fifo->head; }
	}

	FIFO_INTS_ENABLE();

    return retVal;
}


// Data has been directly added to the FIFO
void FifoExternallyAdded(fifo_t *fifo, unsigned int count)
{
    // TODO: Check if count > freeLength
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();
	if (fifo->capacity != 0)
	{
	    fifo->tail = fifo->mask ? ((fifo->tail + count) & fifo->mask) : ((fifo->tail + count) % fifo->capacity);
	}    
	FIFO_INTS_ENABLE();
}


// Data has been directly removed from the FIFO
void FifoExternallyRemoved(fifo_t *fifo, unsigned int count)
{
    // TODO: Check if count > length
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();
	if (fifo->capacity != 0)
	{
	    fifo->head = fifo->mask ? ((fifo->head + count) & fifo->mask) : ((fifo->head + count) % fifo->capacity);
	}    
	FIFO_INTS_ENABLE();
}


// Returns the current length of the FIFO
unsigned int FifoLength(fifo_t *fifo)
{
    unsigned int length;
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();
    // Calculate the length
    if (fifo->tail >= fifo->head) { length = fifo->tail - fifo->head; }
    else { length = (fifo->capacity - fifo->head) + fifo->tail; }
	FIFO_INTS_ENABLE();
    return length;
}


// Returns the free space left in the FIFO
unsigned int FifoFree(fifo_t *fifo)
{
    unsigned int free;
	FIFO_IPL_shadow_t IPLshadow;
	FIFO_INTS_DISABLE();
    // Calculate the free
    if (fifo->capacity == 0) { free = 0; }
    else if (fifo->tail >= fifo->head) { free = fifo->capacity - fifo->tail + fifo->head - 1; }
    else { free = fifo->head - fifo->tail - 1; }
	FIFO_INTS_ENABLE();

    return free;
}


// Empty values from the FIFO
unsigned int FifoPop(fifo_t *fifo, void *destination, unsigned int count)
{
    unsigned int remaining;
    char pass = 0;

    // Up to two passes (FIFO wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned int contiguous, n = 0;
        
        // See how many entries to process in this pass
// WARNING: This will not work with EDS memory
        bufferPointer = (void *)FifoContiguousEntries2(fifo, &contiguous);
        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
        if (n <= 0) { break; }              // No more

		// Copy n * fifo->elementSize bytes
		if (destination != NULL)
		{
			size_t length = n * fifo->elementSize;
			memcpy(destination, bufferPointer, length);
			destination = (unsigned char *)destination + length;
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

// Empty values from the FIFO - modified to return valid pointer to the data IF it is contiguous
unsigned int FifoPop2(fifo_t *fifo, void *destination, unsigned int count, void** dataSource)
{
    unsigned int remaining;
    char pass = 0;

    // Up to two passes (FIFO wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned int contiguous, n = 0;
        
        // See how many entries to process in this pass
// WARNING: This will not work with EDS memory
        bufferPointer = (void *)FifoContiguousEntries2(fifo, &contiguous);
        if (remaining <= contiguous) 
		{ 	// Continuous block of data is present
			n = remaining; 	
			if(pass==0)	// First pass indicates that the chunk is at least as big as pop request size
				*dataSource = bufferPointer; // return valid pointer
		} 
		else 
		{ 	// Partial chunk of data 
			n = contiguous; 
			*dataSource = NULL;				// return null pointer
		}
        
		if (n <= 0) { break; }              // No more

		// Copy n * fifo->elementSize bytes
		if (destination != NULL)
		{
			size_t length = n * fifo->elementSize;
			memcpy(destination, bufferPointer, length);
			destination = (unsigned char *)destination + length;
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
unsigned int FifoPush(fifo_t *fifo, void *values, unsigned int count)
{
    unsigned int remaining;
    char pass = 0;

    // Up to two passes (FIFO wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned int contiguous, n = 0;

        // See how many entries to process in this pass
// WARNING: This will not work with EDS memory
        bufferPointer = (void *)FifoContiguousSpaces2(fifo, &contiguous);
        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
        if (n <= 0) { break; }              // No more

        // Copy n * fifo->elementSize bytes
		if (values != NULL)
		{
			size_t length = n * fifo->elementSize;
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


// Directly set tail (specialist use only)
void FifoSetTail(fifo_t *fifo, unsigned int tail)
{
//	FIFO_IPL_shadow_t IPLshadow;
//	FIFO_INTS_DISABLE();
	fifo->tail = tail;
//	FIFO_INTS_ENABLE();
}
