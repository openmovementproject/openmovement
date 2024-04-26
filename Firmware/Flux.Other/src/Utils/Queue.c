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

// Queue circular buffer (replaces 'Fifo' - doesn't have common interrupt blocking)
// Dan Jackson, 2016

// NOTE: Caller must mask interrupts if required.


/*
// Circular buffer operations (b = buffer, h = head, t = tail, i = index / number of elements, c = capacity)
static unsigned int _queue_temp;
#define QUEUE_CLEAR(_h, _t) { _h = 0; _t = 0; }
#define QUEUE_INCREMENT(_i, _c) (((_i) + 1) % (_c))
#define QUEUE_ADD(_b, _h, _t, _c, _v) { if (QUEUE_INCREMENT(_t, _c) != (_h)) { (_b)[_t] = (_v); _t = QUEUE_INCREMENT(_t, _c); } }
#define QUEUE_REMOVE(_b, _h, _t, _c) (((_h) == (_t)) ? 0 : (_queue_temp = (_b)[_h], _h = QUEUE_INCREMENT(_h, _c), _queue_temp))
#define QUEUE_LENGTH(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h) + (_t)))
#define QUEUE_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) + (_h) - 1) : ((_h) - (_t) - 1))
#define QUEUE_CONTIGUOUS_ENTRIES(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h)))
#define QUEUE_ENTRY_POINTER(_b, _h) ((_b) + (_h))
#define QUEUE_CONTIGUOUS_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) - ((_h) == 0 ? 1 : 0)) : ((_h) - (_t) - 1))
#define QUEUE_FREE_POINTER(_b, _t) ((_b) + (_t))
#define QUEUE_EXTERNALLY_ADDED(_t, _c, _i) ((_t) = (((_t) + (_i)) % (_c)))
#define QUEUE_EXTERNALLY_REMOVED(_h, _c, _i) ((_h) = (((_h) + (_i)) % (_c)))
*/


// Include
#include <string.h>
#include <stdint.h>

#include "Utils/Queue.h"


// Initialize Queue data structure
void QueueInit(queue_t *queue, size_t elementSize, unsigned int capacity, void *buffer)
{
	memset(queue, 0, sizeof(queue_t));

    queue->head = 0;
    queue->tail = 0;
    queue->elementSize = elementSize;
    queue->capacity = capacity;
    queue->mask = 0;
    queue->buffer = buffer;

    // Set the mask if the capacity is a power of two
    if ((capacity & (capacity - 1)) == 0)
    {
        queue->mask = capacity - 1;
    }
}


// Clear Queue
void QueueClear(queue_t *queue)
{
	queue->head = queue->tail;	
}


// Add values to the Queue
unsigned int QueuePush(queue_t *queue, const void *values, unsigned int count)
{
    unsigned int remaining;
    int pass = 0;

    // Up to two passes (queue wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned int contiguous, n;

        // See how many entries to process in this pass
        contiguous = QueueContiguousSpaces(queue, &bufferPointer);
        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
        if (n <= 0) { break; }              // No more

        // Copy n * queue->elementSize bytes
		if (values != NULL)
		{
			size_t length = n * queue->elementSize;
			memcpy(bufferPointer, values, length);
			values = (const unsigned char *)values + length;
		}

        // Update tail pointer
        QueueExternallyAdded(queue, n);

        // Decrease number remaining
        remaining -= n;
        if (remaining <= 0) { break; }      // Processed all
    }

    // Return number of entries processed
    return (count - remaining);
}


// Empty values from the Queue (returns number popped)
unsigned int QueuePop(queue_t *queue, void *destination, unsigned int count)
{
    unsigned int remaining;
    int pass = 0;

    // Up to two passes (queue wraps inside buffer)
    remaining = count;
    for (pass = 0; pass < 2; pass++)
    {
        void *bufferPointer = NULL;
        unsigned int contiguous, n;
        
        // See how many entries to process in this pass
        contiguous = QueueContiguousEntries(queue, &bufferPointer);
        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
        if (n <= 0) { break; }              // No more

		// Copy n * queue->elementSize bytes
		if (destination != NULL)
		{
			size_t length = n * queue->elementSize;
			memcpy(destination, bufferPointer, length);
			destination = (unsigned char *)destination + length;
		}
        
        // Update head pointer
        QueueExternallyRemoved(queue, n);
        
        // Decrease number remaining
        remaining -= n;
        if (remaining <= 0) { break; }      // Processed all
    }
    
    // Return number of entries processed
    return (count - remaining);
}


// Returns the current length of the Queue
unsigned int QueueLength(const queue_t *queue)
{
    unsigned int head = *(volatile unsigned int *)&queue->head;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int tail = *(volatile unsigned int *)&queue->tail;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int length;
    // Calculate the length
    if (tail >= head) { length = tail - head; }
    else { length = (queue->capacity - head) + tail; }
    return length;
}


// Returns the free space left in the Queue
unsigned int QueueFree(const queue_t *queue)
{
    unsigned int head = *(volatile unsigned int *)&queue->head;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int tail = *(volatile unsigned int *)&queue->tail;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int numFree;
    // Calculate the number free
    if (queue->capacity == 0) { numFree = 0; }
    else if (tail >= head) { numFree = queue->capacity - tail + head - 1; }
    else { numFree = head - tail - 1; }
    return numFree;
}


// See how many contiguous entries there are
unsigned int QueueContiguousEntries(const queue_t *queue, void **buffer)
{
    unsigned int head = *(volatile unsigned int *)&queue->head;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int tail = *(volatile unsigned int *)&queue->tail;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int contiguous;

    // Find the buffer pointer
    if (buffer != NULL)
    {
        *buffer = (void *)((unsigned char *)queue->buffer + head * queue->elementSize);
    }

    // See how many entries we can contiguously read from
    if (tail >= head) { contiguous = tail - head; }
    else { contiguous = queue->capacity - head; }

    return contiguous;
}


// See how many contiguous free entries there are
unsigned int QueueContiguousSpaces(const queue_t *queue, void **buffer)
{
    unsigned int head = *(volatile unsigned int *)&queue->head;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int tail = *(volatile unsigned int *)&queue->tail;	// Ensure we read once to get a consistent view (in case it is modified in an interrupt)
    unsigned int contiguous;

    // Find the buffer pointer
    if (buffer != NULL)
    {
        *buffer = (void *)((unsigned char *)queue->buffer + tail * queue->elementSize);
    }

    // See how many entries we can contiguously write to
    if (queue->capacity == 0) { contiguous = 0; }
    else if (tail >= head) { contiguous = queue->capacity - tail - ((head == 0) ? 1 : 0); }
    else { contiguous = head - tail - 1; }

    return contiguous;
}


// Data has been directly removed from the Queue
void QueueExternallyRemoved(queue_t *queue, unsigned int count)
{
	if (queue->capacity != 0)
	{
		// (caller checks to prevent underflow)
	    queue->head = queue->mask ? ((queue->head + count) & queue->mask) : ((queue->head + count) % queue->capacity);
	}    
}


// Data has been directly added to the Queue
void QueueExternallyAdded(queue_t *queue, unsigned int count)
{
	if (queue->capacity != 0)
	{
		// (caller checks to prevent overflow)
	    queue->tail = queue->mask ? ((queue->tail + count) & queue->mask) : ((queue->tail + count) % queue->capacity);
	}    
}

