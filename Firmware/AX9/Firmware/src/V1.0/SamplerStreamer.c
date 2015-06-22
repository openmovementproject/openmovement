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
#include "Peripherals/SysTime.h"
#include "Utils/Fifo.h"
#include "Utils/Util.h"
#include "SamplerStreamer.h"
#include "hci.h"		// To know connection type
#include "bt-app.h"		// For event trigger
#include "bt-serial.h"				
#include "btconfig.h"	// Settings
#include "btprofile.h"	// Device attributes

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

// Interrupt fires at high priority and returns quickly - triggers below isr, caches trigger time
void SamplerTrigger(void)
{
	sampleCount++;
	sampleTicks = SysTimeTicks();
	SAMPLER_INTERRUPT_IF = 1;
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
	sampleTicks = SysTimeTicks();
	SysTimeAddRateCB(SamplerTrigger,settings.sampleRate);
	// The sensors will begin filling the fifos if status.streaming is set
}

void SamplerInitOff(void)
{
	// Chained interrupt off
	SAMPLER_INTERRUPT_IE = 1;
	// Block sampler from getting new data
	status.streaming = 0;
	// Empty fifo so streamer tasks finds no outgoing data
	FifoInit(&streamerFifo, 1, STREAM_OUT_BUFFER_SIZE, streamerOutBuffer);
	// Turn off the timebase
	SysTimeAddRateCB(SamplerTrigger,0);
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
		unsigned char conType;
		#ifdef SAMPLE_LED
			SAMPLE_LED=LED_TURN_ON;
		#endif

		// Grab a sample with cached sample time
		currentSample.sampleCount = sampleCount;
		currentSample.sampleTicks = sampleTicks;

		// Read sensor values
		MultiSingleSample(&currentSample.sensor);

		// For BR connections -> put over serial link
		conType = HciConType(bluetoothConnection);
		if(conType == HCI_CONN_TYPE_BR)
		{
			// Convert to an outgoing data packet 
			unsigned short num = MakeDataPacket(&currentSample, settings.dataMode, (void*)&start);
			
			// Ensure it was successful
			if ((num == 0) || (start == NULL)) return;
		
			// Try to copy to out buffer - if not enough room then skip this sample	
			if (FifoFree(&streamerFifo)>=num)
			{	
				FifoPush(&streamerFifo, start, num);
				status.newXdata = 0; // Indicate new extended data was sent
			}
		}
		#ifndef DISABLE_LE_MODES
		// For BLE connections -> indicate if turned on, overides notify
		else if ((conType == HCI_CONN_TYPE_LE) && (dataOutHandle.dataFlag != 1) && (dataOutHandle.attCfg & ATT_CFG_INDICATE))
		{
			// Convert to an outgoing data packet 
			unsigned short num = MakeDataPacket(&currentSample, 2, (void*)&start); // 20 byte binary packet

			// Ensure it was successful
			if ((num == 0) || (start == NULL)) return;

			dataOutHandle.dataFlag = 1;
			dataOutHandle.attLen = num;
			dataOutHandle.attData = (unsigned char*)start;
			EventTrigger();
		}
		// For BLE connections -> notify if space and not already indicating
		else if ((conType == HCI_CONN_TYPE_LE) && (dataOutHandle.dataFlag != 1) && (dataOutHandle.attCfg & ATT_CFG_NOTIFY))
		{
			// Convert to an outgoing data packet 
			unsigned short num = MakeDataPacket(&currentSample, 2, (void*)&start); // 20 byte binary packet

			// Ensure it was successful
			if ((num == 0) || (start == NULL)) return;

			dataOutHandle.dataFlag = 1;
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
	static unsigned short space, contiguous = 0;
#ifdef MIN_STREAM_TX_LEN
	static unsigned short lastTxTime = 0;
	unsigned short total = FifoLength(&streamerFifo);
#endif
	contiguous = FifoContiguousEntries(&streamerFifo,  &source);
	// Check there is any data
	if(contiguous==0) return;
#ifdef MIN_STREAM_TX_LEN
	// Early out until there is enough data to reach tx len threshold
	if((total < settings.txLenThreshold)&&(lastTxTime == rtcTicksSeconds)) return;
	lastTxTime = rtcTicksSeconds;
#endif
	// Truncate large chunks
	space = BluetoothTxMtu();
	if(space == 0) return;
	if(contiguous > space) 
		contiguous = space;
	// If they are sent then remove from the buffer
	// This means they may be over written while sending if the sample rate is high
	contiguous = BluetoothTxPacket(source, contiguous);
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
		if ((mode & 0x80)||(status.newXdata==1))
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
		if (status.newXdata==1) 	*p++ = PACKET_V2;
		else 						*p++ = PACKET_V1;

		// KL: to keep compatibility with original format 
		// (TODO, migrate to better format with sample at end aswell for  better framing)
		p += slip_encode(p, &sample->sampleCount, sizeof(unsigned short));	
		p += slip_encode(p, &sample->sampleTicks, sizeof(unsigned long));
		p += slip_encode(p, &sample->sensor, sizeof(sensor_t));

		if (status.newXdata == 1) 	
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

