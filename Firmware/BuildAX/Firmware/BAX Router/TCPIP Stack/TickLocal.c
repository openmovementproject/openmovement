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

// Written against Microchip's TCP/IP stack Tick.h header file - provides more flexible timer usage for hardware that can not use timer1
// Karim Ladha, 2013-2014

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "TCPIP Stack/Tick.h"

static volatile WORD overflowTicks = 0;
static BYTE __attribute__((aligned(2))) vTickReading[6];

static void GetTickCopy(void);

void TickInit(void)
{
	// Note: It seems a 700ms tick may be ok based on use of timer1 (16bits) for pic18!?
	// The header file forces a tick to be 256 cpu clock ticks though. Also uses GetSystemClock()
	// Using a 32 bit timer with a div256 prescaler results in a 6 hourly overflow at 48 MIPs
	TMR2 = 0;		// Clear count
	PR2 = 0xffff;	// Set period
	TMR3 = 0;		// Clear count
	PR3 = 0xffff;	// Set period
	T2CON = 0x0038; // 256 prescaler
	T3CON = 0x0030; // 256 prescaler
	IPC1bits.T2IP = TCIPIP_TICK_PRIORITY; // Int priority
	IFS0bits.T2IF = 0;	// Flag cleared
	IEC0bits.T2IE = 1;	// Enable isr
	T2CONbits.TON = 1;	// Start timer
}

static void GetTickCopy(void)
{
	IEC0bits.T2IE = 0; 					// ISR off
	Nop();
	do{
	*(WORD*)&vTickReading[0] = TMR2;	// Read lower two bytes (this holds TMR3 in 32 bit mode)
	*(WORD*)&vTickReading[2] = TMR3;	// Read middle two bytes
	}while(((WORD*)vTickReading)[0] != TMR2); // Check for overflow of lower word

	if(IFS0bits.T2IF)					// Check for overflow of upper word
	{
		IFS0bits.T2IF = 0;				// Perform ISR tasks here and clear flag
		overflowTicks++;
	}
	*(WORD*)&vTickReading[4] = overflowTicks; // Top two bytes
	IEC0bits.T2IE = 1; 					// ISR on
}

DWORD TickGet(void)
{
	GetTickCopy();
	return *((DWORD*)&vTickReading[0]);
}
DWORD TickGetDiv256(void)
{
	GetTickCopy();
	return *((DWORD*)&vTickReading[2]);;
}
DWORD TickGetDiv64K(void)
{
	GetTickCopy();
	return *((DWORD*)&vTickReading[4]);
}

DWORD TickConvertToMilliseconds(DWORD dwTickValue)
{
	return (dwTickValue+(TICKS_PER_SECOND/2000ul))/((DWORD)(TICKS_PER_SECOND/1000ul));
}
void __attribute__((interrupt,auto_psv)) _T2Interrupt(void)
{
	overflowTicks++;
	IFS0bits.T2IF = 0;
}

