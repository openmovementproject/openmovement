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

// Simple interface to MRF24J40 TEDDI 1.11 MiWi receiver
// Karim Ladha, Dan Jackson, 2013-2014

// Headers
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "Peripherals/Rtc.h"
#include "Data.h"

#include "Transceivers/Transceivers.h"
#include "WirelessProtocols/MCHP_API.h"
#include "Console.h"
#include "Network.h"
#include "MiWiRx.h"

// Debug setting
//	#define DEBUG_ON
#include "Debug.h"

//Globals
unsigned char seen[NUM_COORDINATOR] = {0};

// Code
void MiWiRxInit(void)
{
	// Check mask
	if(((long)CHANNEL_MASK) == 0ul)
	{
		status.mrf_state = OFF;
		return;
	}
#ifdef MRF_REMAP // If we are sharing the SPI module
	Si44_IRQ_OFF();
#endif

	// MiWi
	ConsoleInit(); 
	NetworkInit();
	MiApp_ProtocolInit(FALSE);
	if(!radioPresent)
	{
		status.mrf_state = ERROR;
	}
	else
	{
		status.mrf_state = ACTIVE;
		StartNetwork();
	}
#ifdef MRF_REMAP // If we are sharing the SPI module
	Si44_IRQ_EN();
#endif
}

// Call this at 10s interval
void MiWiRxTasks(void)
{
	if(status.mrf_state == ERROR || status.mrf_state == OFF) return; // Must call init first

#ifdef MRF_REMAP // If we are sharing the SPI module
	Si44_IRQ_OFF();
#endif
	// MiWi coodinator sends keep alive every 10 secs
	unsigned short i;
	MiApp_FlushTx();
	MiApp_WriteData(0x12);
	MiApp_WriteData('B');
	MiApp_WriteData((unsigned char)settings.deviceId);
	MiApp_WriteData((unsigned char)(settings.deviceId >> 8));
	
	// Bitmap of 'seen-within-the-last-two-minutes' coordinators (so they know they are visible)
	for (i = 0; i < NUM_COORDINATOR/8; i++)
	{
		unsigned char c = 0x00, b;
		for (b = 0; b < 8; b++)
		{
			if (seen[8 * i + b])
			{
				seen[8 * i + b]--;
				c |= (1 << b);
			}
		}
		MiApp_WriteData(c);
	}
	// Try send 
	if(!MiApp_BroadcastPacket(FALSE))
	{
		if(!MiApp_BroadcastPacket(FALSE)) 	// Retry once
			status.mrf_state = INITIALISED; // May not be working
	}

	// If not operational
	if(status.mrf_state == INITIALISED)		// Needs a network forming
	{
		DBG_ERROR("MiWiRx.c: MRF chip network down, re-forming\r\n");
		MiWiRxInit(); 						// Re-init MiWi stack	
		if(status.mrf_state != ACTIVE) 		// If still not operational
		{
			status.mrf_state = ERROR;		// Error
			DBG_ERROR("MiWiRx.c: MRF hardware fail\r\n");
		}
	}
#ifdef MRF_REMAP // If we are sharing the SPI module
	Si44_IRQ_EN();
#endif
}

void MiWiRxReadPacket(void) // Called from ISR only (no spi remap needed)
{
	// MiWi Rx, asynchronous
	if (MiApp_MessageAvailable())
	{
		//lastReceived = rtcTicksSeconds;
		if (rxMessage.PayloadSize >= 10 && rxMessage.Payload[0] == 0x12 && rxMessage.Payload[1] == 'S')
		{
			unsigned short shortAddress = rxMessage.Payload[8] | ((unsigned short)rxMessage.Payload[9] << 8);
			int index = (shortAddress >> 8);
			if (index < NUM_COORDINATOR)
			{
				seen[index] = SEEN_COUNTER_RESET;
			}
		}
		else if (rxMessage.PayloadSize >= 5 && rxMessage.Payload[0] == 0x12 && rxMessage.Payload[1] == DATA_TEDDI_REPORT_ID)
		{
			AddDataElement(TYPE_MIWI_PKT,DATA_DEST_ALL, sizeof(RECEIVED_MESSAGE), GetOpenStreams(), &rxMessage);	// Add element
			SI44_LED = 0;																			// Blink led
		}
		else
		{
			#ifdef ENABLE_CONSOLE
			NetworkDumpMessage(); // Console only
			#endif
		}	    
		MiApp_DiscardMessage();
	}
}


//EOF
