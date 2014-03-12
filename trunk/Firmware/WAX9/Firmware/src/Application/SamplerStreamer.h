/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// Sample Streamer
// Karim Ladha, 2013-2014

// This code is tasked with converting samples into the ascii or binary format in the ram buffer.
// The second part of the code attempts to send the data wirelessly.

#ifndef _SAMPLERSTREAMER_H_
#define _SAMPLERSTREAMER_H_

#include "Peripherals/MultiSensor.h"
#include "Utils/Fifo.h"

typedef struct {
	unsigned short sampleCount;	// Short 2 bytes
	sensor_t sensor; 			// 9 axis 18 bytes
	unsigned long sampleTicks;	// Long 4 bytes
}sampleData_t; // 24 Bytes

extern sampleData_t currentSample;
extern unsigned short sampleCount;
extern unsigned long sampleTicks;
extern unsigned char streamerOutBuffer[STREAM_OUT_BUFFER_SIZE]; 

// Use extra intrrupt chaining to allow call at lower priority
#define SAMPLE_LED				LED_B
#define SAMPLER_VECTOR			_IC1Interrupt	
#define SAMPLER_INTERRUPT_IE	IEC0bits.IC1IE
#define SAMPLER_INTERRUPT_IF	IFS0bits.IC1IF
#define SAMPLER_INTERRUPT_IP	IPC0bits.IC1IP

// SLIP-encoded packet -- write SLIP_END bytes before and after the packet: usb_putchar(SLIP_END);
#define SLIP_END     0xC0                   // End of packet indicator
#define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
#define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
#define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte

// Defines
#define NINE_AXIS_PACKET	'9'
#define PACKET_V1			0x01
#define PACKET_V2			0x02

// Interrupt fires at high priority and returns quickly - triggers below isr, caches trigger time
#define SamplerTrigger()	{sampleCount++;\
							sampleTicks += ((unsigned long)(PR1+1)<<1);\
							SAMPLER_INTERRUPT_IF = 1;}


extern fifo_t streamerFifo;

void SamplerInitOn(void);

void SamplerInitOff(void);

void SamplerTasks(void);

void StreamerTasks(void);

// Makes single sample packets ready to send 
unsigned short MakeDataPacket(sampleData_t* sample, unsigned char mode, void** start);

size_t slip_encode(void *outBuffer, const void *inBuffer, size_t length);
#endif
