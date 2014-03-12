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

#ifndef FIFO_H
#define FIFO_H


// FIFO
typedef struct
{
    unsigned short head;
    unsigned short tail;
    unsigned short elementSize;
    unsigned short capacity;
    unsigned short mask;
    void *buffer;
} fifo_t;


// Initialize FIFO data structure
void FifoInit(fifo_t *fifo, unsigned short elementSize, unsigned short capacity, void *buffer);

// Clear FIFO
void FifoClear(fifo_t *fifo);

// Add values to the FIFO
unsigned short FifoPush(fifo_t *fifo, void *values, unsigned short count);

// Empty values from the FIFO
unsigned short FifoPop(fifo_t *fifo, void *values, unsigned short count);

// Returns the current length of the FIFO
unsigned short FifoLength(fifo_t *fifo);

// Returns the free space left in the FIFO
unsigned short FifoFree(fifo_t *fifo);

// See how many contiguous entries there are
unsigned short FifoContiguousEntries(fifo_t *fifo, void **buffer);

// See how many contiguous free entries there are
unsigned short FifoContiguousSpaces(fifo_t *fifo, void **buffer);

// Data has been directly removed from the FIFO
void FifoExternallyRemoved(fifo_t *fifo, unsigned short count);

// Data has been directly added to the FIFO
void FifoExternallyAdded(fifo_t *fifo, unsigned short count);

#endif
