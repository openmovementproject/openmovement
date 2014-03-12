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
// Karim Ladha, Dan Jackson, 2013-2014

// This code is tasked with converting samples into the ascii or binary format in the ram buffer.
// The second part of the code attempts to send the data wirelessly.

#include <Compiler.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "Peripherals/MultiSensor.h"
#include "Peripherals/Timer1.h"
#include "Utils/Fifo.h"
#include "Utils/Util.h"
#include "SamplerStreamer.h"
#include "hci + le.h"				// To know connection type
#include "bt app.h"					// For event trigger				
#include "Profile.h"				// To send notifications/indications
#include "bt config.h"

// Fifo
unsigned char streamerOutBuffer[STREAM_OUT_BUFFER_SIZE];
fifo_t streamerFifo;
unsigned short sampleCount = 0;
unsigned long sampleTicks = 0;
// Global sample holder
sampleData_t currentSample;

// Fires at lower priority and performs sampling
void __attribute__((interrupt, shadow, auto_psv)) SAMPLER_VECTOR(void)
{
	SAMPLER_INTERRUPT_IF = 0;
	SamplerTasks();	
}


void SamplerInitOn(void)
{
	// Fifos
	FifoInit(&streamerFifo, 1, STREAM_OUT_BUFFER_SIZE, streamerOutBuffer);
	// Turn on sensors
	MultiStartup(&settings);
	// Setup chained interrupts
	SAMPLER_INTERRUPT_IP = SAMPLER_INT_PRIORITY;
	SAMPLER_INTERRUPT_IF = 0;
	SAMPLER_INTERRUPT_IE = 1;
	// Turn on time base
	sampleCount = 0;
	sampleTicks = TimerTicks();
	TimerInterruptOn(settings.sampleRate);
	// The sensors will begin filling the fifos if status.streaming is set
}

void SamplerInitOff(void)
{
	// Chained interrupt off
	SAMPLER_INTERRUPT_IE = 1;
	// Block sampler from getting new data
	status.streaming = FALSE;
	// Empty fifo so streamer tasks finds no outgoing data
	FifoInit(&streamerFifo, 1, STREAM_OUT_BUFFER_SIZE, streamerOutBuffer);
	// Turn off the timebase
	TimerInterruptOn(MINIMUM_T1_RATE);
	sampleCount = 0;
	// sampleTicks = 0; // Left running
	// Power off sensors
	MultiStandby();
}

// As called from above
void SamplerTasks(void)
{
	if ((status.streaming) && (settings.sampleRate != 0))
	{
		static char* start = NULL;
		#ifdef SAMPLE_LED
			SAMPLE_LED=1;
		#endif

		// Grab a sample with cached sample time
		TIMER_IPL_shadow_t IPLshadow;
		TIMER_INTS_DISABLE();
		currentSample.sampleCount = sampleCount;
		currentSample.sampleTicks = sampleTicks;
		TIMER_INTS_ENABLE()

		// Read sensor values
		MultiSingleSample(&currentSample.sensor);

		// For RFCOMM connections -> put over serial link
		if(hci_connection.connectionType == HCI_CONN_TYPE_BR)
		{
			// Convert to an outgoing data packet 
			unsigned short num = MakeDataPacket(&currentSample, settings.dataMode, (void*)&start);
			
			// Ensure it was successful
			if ((num == 0) || (start == NULL)) return;
		
			// Try to copy to out buffer - if not enough room then skip this sample	
			if (FifoFree(&streamerFifo)>=num)
			{	
				FifoPush(&streamerFifo, start, num);
				status.newXdata = FALSE; // Indicate new extended data was sent
			}
		}
		#ifdef ENABLE_LE_MODE
		// For BLE connections -> indicate if turned on, overides notify
		else if ((hci_connection.connectionType == HCI_CONN_TYPE_LE) && (dataOutHandle.dataFlag != TRUE) && (dataOutHandle.attCfg & ATT_CFG_INDICATE))
		{
			// Convert to an outgoing data packet 
			unsigned short num = MakeDataPacket(&currentSample, 2, (void*)&start); // 20 byte binary packet

			// Ensure it was successful
			if ((num == 0) || (start == NULL)) return;

			dataOutHandle.dataFlag = TRUE;
			dataOutHandle.attLen = num;
			dataOutHandle.attData = (unsigned char*)start;
			EventTrigger();
		}
		// For BLE connections -> notify if space and not already indicating
		else if ((hci_connection.connectionType == HCI_CONN_TYPE_LE) && (dataOutHandle.dataFlag != TRUE) && (dataOutHandle.attCfg & ATT_CFG_NOTIFY))
		{
			// Convert to an outgoing data packet 
			unsigned short num = MakeDataPacket(&currentSample, 2, (void*)&start); // 20 byte binary packet

			// Ensure it was successful
			if ((num == 0) || (start == NULL)) return;

			dataOutHandle.dataFlag = TRUE;
			dataOutHandle.attLen = num;
			dataOutHandle.attData = (unsigned char*)start;
			EventTrigger();
		}
		#endif//ENABLE_LE_MODE
		// On returning, if there is another sample triggered - this will be called again 	
	}
}

void StreamerTasks(void)
{
	// Number of contiguous bytes
	static void* source = NULL;
	static unsigned short contiguous = 0;
	contiguous = FifoContiguousEntries(&streamerFifo,  &source);
	// Check there is any data
	if(contiguous==0) return;
	// Truncate large chunks
	if(contiguous > RfcommMaxPayload()) 
		contiguous = RfcommMaxPayload();
	// If they are sent then remove from the buffer
	// This means they may be over written while sending if the sample rate is high
	if (RfcommSendData(source, contiguous)){}
		FifoExternallyRemoved(&streamerFifo, contiguous);
	// Else, Failed to send them, bluetooth busy or not connected
	return;
}

unsigned short MakeDataPacket(sampleData_t* sample , unsigned char mode, void** start)
{
	// Get sample
	static char output[100]; // Ensure you dont overrun this buffer
	char *p = output;
	unsigned short num = 0;

	// Ascii mode
	if ((mode == 0)||(mode == 128))
	{
		// ascii mode

		// Print string
		p += my_nitoa(p, sample->sampleCount); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.accel.x); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.accel.y); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.accel.z); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.gyro.x); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.gyro.y); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.gyro.z); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.mag.x); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.mag.y); *p++ = ','; 
		p += my_nitoa(p, sample->sensor.mag.z);  

		// If top bit set then append extra data
		if ((mode & 0x80)||(status.newXdata==TRUE))
		{
			*p++ = ',';
			p += my_nitoa(p, status.battmv); *p++ = ','; 
			p += my_nitoa(p, status.temperature); *p++ = ','; 
			{
				char * ptr = (char*)my_ultoa(status.pressure); 
				unsigned char len = strlen(ptr);
				memcpy(p,ptr,len);
				p+=len;
				*p++ = ',';
			}
			p += my_nitoa(p, status.inactivity);
		}

		*p++ = '\r';*p++ = '\n'; *p = '\0'; 
		// Count bytes
		num = p - output;
	}

	// Binary slip mode 
	if (mode == 1)
	{
		// binary mode 1
		*p++ = SLIP_END;
		*p++ = NINE_AXIS_PACKET;
		if (status.newXdata==TRUE) 	*p++ = PACKET_V2;
		else 						*p++ = PACKET_V1;

		// KL: to keep compatibility with original format 
		// (TODO, migrate to better format with sample at end aswell for  better framing)
		p += slip_encode(p, &sample->sampleCount, sizeof(unsigned short));	
		p += slip_encode(p, &sample->sampleTicks, sizeof(unsigned long));
		p += slip_encode(p, &sample->sensor, sizeof(sensor_t));

		if (status.newXdata==TRUE) 	
		{	
			p += slip_encode(p, &status.battmv, 		sizeof(unsigned short));
			p += slip_encode(p, &status.temperature, 	sizeof(short));
			p += slip_encode(p, &status.pressure, 		sizeof(unsigned long));
		}	

		*p++ = SLIP_END;	
		// Count bytes
		num = p - output;	
	}

	// 20 byte special mode for ble attribute notifications
	if (mode == 2)
	{
		// 20 Byte payload for BLE applications
		// Copy sample to static
		memcpy(&output[0],sample,(sizeof(sampleData_t)-4)); 	// 20
		// Set number of bytes
		num = 20;
	}

	// Set pointer to data
	*start = output;
	return num;
}

// Encode SLIP (RFC 1055) data
size_t slip_encode(void *outBuffer, const void *inBuffer, size_t length)
{
    const unsigned char *sp = (const unsigned char *)inBuffer;
    unsigned char *dp = outBuffer;
    while (length--)
    {
        if (*sp == SLIP_END)
		{
			*dp++ = SLIP_ESC;
			*dp++ = SLIP_ESC_END;
		}
        else if (*sp == SLIP_ESC)
		{
			*dp++ = SLIP_ESC;
			*dp++ = SLIP_ESC_ESC;
		}
        else
		{
			*dp++ = *sp;
		}
        ++sp;
    }
    return (size_t)((void *)dp - (void *)outBuffer);
}

