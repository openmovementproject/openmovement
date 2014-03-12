/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// Stream
// Karim Ladha, Dan Jackson, 2012

// Written to allow generic data to use WAX2.1 visualiser

#include "Utils/Fifo.h"

// Data payload -- all WORD/DWORD stored as little-endian (LSB first)
typedef struct __attribute__((aligned(2))) DataPacket_t
{
	unsigned char  reportType __attribute__ ((packed));		// [1] = 0x12 (USER_REPORT_TYPE)
	unsigned char  reportId __attribute__ ((packed));	    // [1] = 0x78 (ASCII 'x')
	unsigned short deviceId __attribute__ ((packed));		// [2] = Device identifier (16-bit)
	unsigned char  status __attribute__ ((packed));			// [1] = Device status (bit 0 is battery warning, top 7 bits reserved)
	unsigned short sample __attribute__ ((packed));			// [2] = Analogue sample (top 6 bits indicate measurement information, lower 10 bits are the value)
	unsigned char  format __attribute__ ((packed));			// [1] = Accelerometer data format 0xE9 (+/- 2g, 2-bytes, 100Hz); Top two bits is range, next two bits is format [0=3x10+2-bit, 2=signed 16-bit, 3=3x10+2 & 3*16-gyro] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
	unsigned short sequenceId __attribute__ ((packed));		// [2] = Sequence number of first accelerometer reading in this packet (16-bit sample index -- will wrap or be reset if device sleeps and resumes)
	unsigned char  outstanding __attribute__ ((packed));		// [1] = Number of samples remaining on device after this packet (0xff: >= 255)
	unsigned char  sampleCount __attribute__ ((packed));		// [1] = Number of samples in this packet. 0x0c = 13 samples (number of accelerometer samples)
	//Sample sampleData[BUFFER_PACKET_MAX_SAMPLES];	// bytes: [sampleCount * numAxes * bytesPerSample = 78 -- would be 102 with 17 samples] (sample data)
} DataPacket;

void SendData(unsigned short device_id, unsigned short sequence_id,  void* data, unsigned short len);

void SendAvailableData(fifo_t* fifo);

// USB write SLIP (RFC 1055) encoded data
unsigned char usb_slip_encoded(void *buffer, unsigned char length);
unsigned char bluetooth_slip_encoded(DataPacket* packetHeader, unsigned short headerSize, void *data, unsigned short dataSize);




// If is not critical, delay sending whie any enabled streams are busy
char StreamerIsBusy(StateStream stream);

// Output sector from a filestream to a Bluetooth or USB stream
char StreamerOutputData(void *buffer, StateStream stream, short streamFormat, unsigned short len, const char *prefix);

//EOF
