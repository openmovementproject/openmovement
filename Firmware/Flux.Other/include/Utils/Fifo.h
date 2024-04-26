/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *	 this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *	 this list of conditions and the following disclaimer in the documentation 
 *	 and/or other materials provided with the distribution.
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

#if defined(__C30__) || defined (__C32__)
#include "Compiler.h"
#include "HardwareProfile.h"
#elif defined(__arm__)
#include <stdlib.h>
#include "Config.h"
#endif

#if defined(__PIC24FJ256DA206__) && !defined(USE_EDS)
#warning "EDS Expected"
#endif

#ifdef USE_EDS
#warning "Using extended data space (24bit) pointers"
#define FIFO_EDS __eds__
#else
#define FIFO_EDS 
#endif

// FIFO
typedef struct
{
	unsigned int head;
	unsigned int tail;
	size_t elementSize;
	unsigned int capacity;
	unsigned int mask;
	void FIFO_EDS *buffer;
} fifo_t;

// Interrupt mask protection functions - used to block fifo interrupts (may result in timestamp jitter)
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

// Initialize FIFO data structure
void FifoInit(fifo_t *fifo, size_t elementSize, unsigned int capacity, void FIFO_EDS *buffer);

// Clear FIFO
void FifoClear(fifo_t *fifo);

// Add values to the FIFO - NEVER double push
unsigned int FifoPush(fifo_t *fifo, void *values, unsigned int count);

// Empty values from the FIFO - Will not overwite itself, returns number written successfully
unsigned int FifoPop(fifo_t *fifo, void *destination, unsigned int count);
unsigned int FifoPop2(fifo_t *fifo, void *destination, unsigned int count, void** dataSource); // KL, allows streaming of data out of fifos using the vdma

// Returns the current length of the FIFO
unsigned int FifoLength(fifo_t *fifo);

// Returns the free space left in the FIFO
unsigned int FifoFree(fifo_t *fifo);

// See how many contiguous entries there are
unsigned int FifoContiguousEntries(fifo_t *fifo,  void **buffer);
void FIFO_EDS *FifoContiguousEntries2(fifo_t *fifo,  unsigned int *contiguous);

// See how many contiguous free entries there are
unsigned int FifoContiguousSpaces(fifo_t *fifo, void **buffer);
void FIFO_EDS *FifoContiguousSpaces2(fifo_t *fifo,  unsigned int *contiguous);

// Data has been directly removed from the FIFO
void FifoExternallyRemoved(fifo_t *fifo, unsigned int count);

// Data has been directly added to the FIFO
void FifoExternallyAdded(fifo_t *fifo, unsigned int count);

// Directly set tail (specialist use only)
void FifoSetTail(fifo_t *fifo, unsigned int tail);

#endif
