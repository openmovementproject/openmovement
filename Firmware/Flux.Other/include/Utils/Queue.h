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


// A circular queue of fixed-size elements. 
// This implementation tracks the head and tail indexes (leaving the length to be derived),
// rather than choose to track the head (or tail) and the length (leaving the other 
// end to be derived).  To avoid the full/empty ambiguity when the head and tail are the 
// same index, the maximum number of elements is 'capacity'-1.  However, the advantage 
// of this approach is that many producer/consumer applications (even those that 
// run in different embedded contexts) may not require a mutex as the length is never 
// directly written, and neither the head nor tail are derived from the other end.


#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>				// for size_t

// SampleQueue state
typedef struct
{
	unsigned int head;			// Remove from the head of the queue
	unsigned int tail;			// Additions to the tail of the queue
	size_t elementSize;			// Size of each element
	unsigned int capacity;		// Maximum number of elements in the queue (+1)
	unsigned int mask;			// Bitwise mask (if capacity is a power of two)
	void *buffer;				// Data buffer (user must provide correct alignment)
} queue_t;


/*
// (For caller -- Interrupt mask protection functions - used to block fifo interrupts (may result in timestamp jitter))
#ifdef __C30__
	typedef unsigned char FIFO_IPL_shadow_t;
	#define FIFO_INTS_DISABLE()	{IPLshadow = SRbits.IPL; if(SRbits.IPL<FIFO_INTERRUPT_PRIORITY)SRbits.IPL=FIFO_INTERRUPT_PRIORITY;}
	#define FIFO_INTS_ENABLE()	{SRbits.IPL = IPLshadow;}
#elif defined (__C32__)
	typedef unsigned long FIFO_IPL_shadow_t;
	#define FIFO_INTS_DISABLE()	{IPLshadow = _CP0_BCS_STATUS((_CP0_STATUS_IPL_MASK&((~FIFO_INTERRUPT_PRIORITY)<<_CP0_STATUS_IPL_POSITION)),(_CP0_STATUS_IPL_MASK&(FIFO_INTERRUPT_PRIORITY<<_CP0_STATUS_IPL_POSITION)));} // Assign IPL equal to FIFO_INTERRUPT_PRIORITY
	#define FIFO_INTS_ENABLE()	{_CP0_BCS_STATUS((_CP0_STATUS_IPL_MASK&(~IPLshadow)),(_CP0_STATUS_IPL_MASK&IPLshadow));}
#elif defined (__arm__)
	// KL: Added standard ARM cmsis interrupt masking method
	typedef unsigned char FIFO_IPL_shadow_t;
#if (__CORTEX_M >= 0x03)
	#define FIFO_BASE_PRI_NUM	(FIFO_INTERRUPT_PRIORITY << (8 - __NVIC_PRIO_BITS))
	#define FIFO_INTS_DISABLE()	{IPLshadow = __get_BASEPRI(); if(IPLshadow>FIFO_BASE_PRI_NUM){__set_BASEPRI(FIFO_BASE_PRI_NUM);}}
	#define FIFO_INTS_ENABLE()	{__set_BASEPRI(IPLshadow);}
#else
	// M0 processor doesn't support masking
	#define FIFO_INTS_DISABLE()	{__disable_irq();}
	#define FIFO_INTS_ENABLE()	{__enable_irq();} 
#endif
#endif
*/


// Initialize Queue data structure
void QueueInit(queue_t *queue, size_t elementSize, unsigned int capacity, void *buffer);

// Clear Queue
void QueueClear(queue_t *queue);

// Add values to the Queue (returns number pushed)
unsigned int QueuePush(queue_t *queue, const void *values, unsigned int count);

// Empty values from the Queue (returns number popped)
unsigned int QueuePop(queue_t *queue, void *destination, unsigned int count);

// Returns the current length of the Queue
unsigned int QueueLength(const queue_t *queue);

// Returns the free space left in the Queue
unsigned int QueueFree(const queue_t *queue);

// See how many contiguous entries there are
unsigned int QueueContiguousEntries(const queue_t *queue,  void **buffer);

// See how many contiguous free entries there are
unsigned int QueueContiguousSpaces(const queue_t *queue, void **buffer);

// Data has been directly removed from the Queue
void QueueExternallyRemoved(queue_t *queue, unsigned int count);

// Data has been directly added to the Queue
void QueueExternallyAdded(queue_t *queue, unsigned int count);


#endif
