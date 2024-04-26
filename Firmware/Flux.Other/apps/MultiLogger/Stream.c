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

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "USB/USB_CDC_MSD.h"
#include "Utils/Fifo.h"
#include "Utils/FileStream.h"
#include "Bluetooth/Bluetooth.h"
#include "Apps/MultiLogger/Recorder.h"
#include "Apps/MultiLogger/Settings.h"
#include "Apps/MultiLogger/Stream.h"
#include "Utils/Util.h"

// SLIP-encoded packet -- write SLIP_END bytes before and after the packet: usb_putchar(SLIP_END);
#define SLIP_END     0xC0                   // End of packet indicator
#define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
#define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
#define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte

// [dgj] On DP, should be: 1068+6      // KL: must be less than max bt send size +6 extra for ascii debug version
#ifndef BT_MAX_RFCOMM_SEND_SIZE
#error "BT_MAX_RFCOMM_SEND_SIZE not defined"
#endif
#define STREAM_BUFFER_MAX BT_MAX_RFCOMM_SEND_SIZE

// Bluetooth write SLIP (RFC 1055) encoded data - 512-byte sector, *2 in case all are escaped, +1 for start SLIP_END, +1 for stop SLIP_END, +40 for prefix metadata, +2 more as an invalid {SLIP_ESC,\n} pair is added to signify line-based parsing should begin again
unsigned char __attribute__((aligned(2))) slipBuffer[STREAM_BUFFER_MAX];

// Encode SLIP (RFC 1055) data
size_t slip_encode(void *outBuffer, const void *inBuffer, size_t length, size_t outBufferSize, unsigned short *consumed)
{
    const unsigned char *sp = (const unsigned char *)inBuffer;
    unsigned char *dp = outBuffer;
    while (length--)
    {
        if (*sp == SLIP_END)
		{
			if (outBufferSize < 2) { break; }
			*dp++ = SLIP_ESC;
			*dp++ = SLIP_ESC_END;
			outBufferSize -= 2;
		}
        else if (*sp == SLIP_ESC)
		{
			if (outBufferSize < 2) { break; }
			*dp++ = SLIP_ESC;
			*dp++ = SLIP_ESC_ESC;
			outBufferSize -= 2;
		}
        else
		{
			if (outBufferSize < 1) { break; }
			*dp++ = *sp;
			outBufferSize--;
		}
        ++sp;
    }
	if (consumed != NULL) { *consumed = (unsigned short)((void *)sp - inBuffer); }
    return (size_t)((void *)dp - (void *)outBuffer);
}

unsigned short ASCIIPacket(void *buffer, const void *data, unsigned short len, unsigned short offset, unsigned short maxLen, unsigned short *consumed)
{
	char *ptr;
    const char *label = NULL;
    unsigned short sampleCount = (((const unsigned char *)data)[22]) | ((unsigned short)(((const unsigned char *)data)[23]) << 8);
    typedef struct { short x, y, z; } threeaxis_t;
    typedef struct { short ax, ay, az, gx, gy, gz, mx, my, mz; } nineaxis_t;
    int i;
	unsigned short written = 0;

    // Start packet
    ptr = (char *)buffer;

	if (((const unsigned char *)data)[1] == 'x')
	{
	    nineaxis_t *values = (nineaxis_t *)(data + 24);    // Data contents (@24 offset in sector buffer, 480 bytes)

		//"2000/01/01 00:00:00.000,ax,ay,az,gx,gy,gz"
	
	    // Up to 29 ASCII chars/sample, so 1024 byte buffer up to ~35 samples
		*consumed = 0;
	    for (i = offset < 24 ? 0 : ((offset - 24) / sizeof(nineaxis_t)); i < sampleCount; i++)
	    {
	        //written += sprintf(ptr + written, "%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", values[i].ax, values[i].ay, values[i].az, values[i].gx, values[i].gy, values[i].gz, values[i].mx, values[i].my, values[i].mz);
			written += sprintf(ptr + written, "%s,%d,%d,%d,%d,%d,%d\r\n", (const char*)"$X",values[i].ax, values[i].ay, values[i].az, values[i].gx, values[i].gy, values[i].gz);
			if (written + (125) > maxLen) { break; }	// Can't fit any more
			*consumed += sizeof(nineaxis_t);
	    }
	
		if (i >= sampleCount)
		{
			// Read to end of packet
			*consumed = len - offset;
		}
	}
	else
	{
	    threeaxis_t *values = (threeaxis_t *)(data + 24);    // Data contents (@24 offset in sector buffer, 480 bytes)

	    // Determine device type
	    switch (((const unsigned char *)data)[1])
	    {
	        case 'a': label = "$ACCEL"; break;
	        case 'g': label = "$GYRO"; break;
	        case 'm': label = "$MAG"; break;
	    }
	    if (label == NULL) { return 0; }
	
	    // Up to 29 ASCII chars/sample, so 1024 byte buffer up to ~35 samples
		*consumed = 0;
	    for (i = offset < 24 ? 0 : ((offset - 24) / sizeof(threeaxis_t)); i < sampleCount; i++)    // 80 = 1 sample, 40 = 2 samples, 20 = 4 samples, 10 = 8 samples, 5 = 16 samples, 3 = 26 samples
	    {
	        written += sprintf(ptr + written, "%s=%d,%d,%d\r\n", label, values[i].x, values[i].y, values[i].z);
			if (written + (40 + 5) > maxLen) { break; }	// Can't fit any more
			*consumed += sizeof(threeaxis_t);
	    }
	
		if (i >= sampleCount)
		{
			// Read to end of packet
			*consumed = len - offset;
		}
	}

	return written;
}


unsigned short SlipEncodeStream(void *buffer, const void *data, unsigned short len, unsigned short offset, unsigned short maxLen, unsigned short *consumed)
{
	unsigned char *ptr = buffer;
    unsigned short sampleCount;
	unsigned short localOffset = offset;
	unsigned short dataSize;

	sampleCount = (((const unsigned char *)data)[22]) | ((unsigned short)(((const unsigned char *)data)[23]) << 8);
	//if (sampleCount > 5) { sampleCount = 5; }	// Limit number of samples

	// Calculate size of data (in a filthy way)
    switch (((const unsigned char *)data)[1])
    {
        case 'a': dataSize = sampleCount * 6; break;
        case 'g': dataSize = sampleCount * 6; break;
        case 'm': dataSize = sampleCount * 6; break;
        case 'x': dataSize = sampleCount * 9; break;
		default: return 0;	// Unsupported stream
    }

	// Send header first
	if (offset == 0)
	{
	    #define MAX_DEVICES 4
	    static unsigned short sequence_id[MAX_DEVICES] = {0};
	    unsigned short device_id = 0xffff;
	    static DataPacket header;
		unsigned char format;

	    // Start SLIP packet
	    ptr = (unsigned char *)buffer;
		*ptr++ = SLIP_END;
	
	    // Determine 'device_id'
		format = 0xE9;
	    switch (((const unsigned char *)data)[1])
	    {
	        case 'a': device_id = 0; break;
	        case 'g': device_id = 1; break;
	        case 'm': device_id = 2; break;
	        case 'x': device_id = 3; format = 0x29; break;
	    }
	    if (device_id < 0 || device_id >= MAX_DEVICES) { return 0; }
	
	    header.reportType = 0x12;
	    header.reportId = 'x';
	    header.deviceId = device_id;
	    header.status = 0;
	    header.sample = 0x03ff;
	    header.format = format;
	    header.sequenceId = sequence_id[device_id];
	    header.outstanding = 0;
	    header.sampleCount = (((const unsigned char *)data)[22]) | ((unsigned short)(((const unsigned char *)data)[23]) << 8);
	
	    // Update sequence id
	    sequence_id[device_id] += header.sampleCount;
	
	    // Buffer
	    ptr += slip_encode(ptr, &header, sizeof(DataPacket), maxLen - 1, consumed);

		if (*consumed < sizeof(DataPacket)) { return 0; }	// Error: cannot even send first packet

		localOffset = 24;
	}

    // Data contents (@24 offset in sector buffer, 480 bytes)
	{
		unsigned short slipConsumed = 0;
		unsigned short numToWrite = dataSize - (localOffset - 24);
		ptr += slip_encode(ptr, data + localOffset, numToWrite, maxLen - (unsigned short)((void *)ptr - buffer) - 3, &slipConsumed);	// Keep 3 bytes spare for end stream
		localOffset += slipConsumed;
	}

	if (localOffset - 24 >= dataSize)
	{
	    // End SLIP packet
		*ptr++ = SLIP_END;
	
	    // Add an invalid escape sequence {SLIP_ESC,\n} to signify line-based parsing should resume
		*ptr++ = SLIP_ESC;
		*ptr++ = '\n';

		// Read to end of packet
		localOffset = len;
	}

	// Bytes we consumed
	*consumed = localOffset - offset;

	return (unsigned short)((void *)ptr - buffer) - offset;
}

// Takes:
//   buffer to write to
//   data as a source OMX sector
//   len input data length (512 bytes)
//   offset within the data to write out
//   maxLen size of the output storage buffer
//   consumed number of input bytes
// Returns bytes written to buffer
unsigned short SlipEncodeSector(void *buffer, const void *data, unsigned short len, unsigned short offset, unsigned short maxLen, unsigned short *consumed)
{
	unsigned char *ptr = (unsigned char *)buffer;
	unsigned short written = 0;

    // If at the start, delimit this SLIP packet
	if (offset == 0)
	{
		ptr[written++] = SLIP_END;
	}

    // Encode whole sector 
    written += slip_encode(ptr + written, data + offset, len - offset, maxLen - written - 3, consumed);	// Reserve 3 bytes for end case

	// End of SLIP packet
	if (offset + *consumed >= len)
	{
	    // Delimit this SLIP packet
		ptr[written++] = SLIP_END;
	
	    // Add an invalid escape sequence {SLIP_ESC,\n} to signify line-based parsing should resume
		ptr[written++] = SLIP_ESC;
		ptr[written++] = '\n';
	}

	return written;
}


// If is not critical, delay sending whie any enabled streams are busy
char StreamerIsBusy(StateStream stream)
{
    char ret = 0;

    // If streaming over wireless, return whether it is busy
    if ((stream == STREAM_WIRELESS || stream == STREAM_BOTH) && BluetoothTxBusy()) { ret |= 1; }

    // If streaming over USB, return whether it is busy
    if ((stream == STREAM_USB || stream == STREAM_BOTH) && USBCDCBusy()) { ret |= 2; }

    return ret;
}

// Output sector from a filestream to a Bluetooth or USB stream
char StreamerOutputData(void *buffer, StateStream stream, short streamFormat, unsigned short len, const char *prefix)
{
	int maxLen = STREAM_BUFFER_MAX;
    char ret = 0;

#if 1
    if (stream != STREAM_NONE && ((unsigned char *)buffer)[1] == 'x' && streamFormat == STREAM_MODE_TEXT)
	{
	    unsigned short sampleCount = (((const unsigned char *)buffer)[22]) | ((unsigned short)(((const unsigned char *)buffer)[23]) << 8);
	    typedef struct { short ax, ay, az, gx, gy, gz, mx, my, mz; } nineaxis_t;
	    nineaxis_t *values = (nineaxis_t *)(buffer + 24);    // Data contents (@24 offset in sector buffer, 480 bytes)
		int i;

	    for (i = 0; i < sampleCount; i++)
	    {
			//ax,ay,az,gx,gy,gz"
			static char streambuffer[44];
			char *p = streambuffer;
			//sprintf(buffer, "%d,%d,%d,%d,%d,%d\r\n", values[i].ax, values[i].ay, values[i].az, values[i].gx, values[i].gy, values[i].gz);
			p += my_nitoa(p, values[i].ax); *p++ = ','; 
			p += my_nitoa(p, values[i].ay); *p++ = ','; 
			p += my_nitoa(p, values[i].az); *p++ = ','; 
			p += my_nitoa(p, values[i].gx); *p++ = ','; 
			p += my_nitoa(p, values[i].gy); *p++ = ','; 
			p += my_nitoa(p, values[i].gz); *p++ = '\r'; *p++ = '\n'; *p = '\0'; 
			//writeHandler(streambuffer, (unsigned short)(p - streambuffer));
			write_handler_t oldWriteHandler = writeHandler;
			if (stream & STREAM_USB) 		{ writeHandler = usb_write;			printf("%s",streambuffer); }
			if (stream & STREAM_WIRELESS) 	{ writeHandler = Bluetooth_write; 	printf("%s",streambuffer);}	
			writeHandler = oldWriteHandler;

	    }
		return 1;
	}

// TODO: This shouldn't be needed
if (stream != STREAM_NONE && ((unsigned char *)buffer)[1] == 'l' && streamFormat == STREAM_MODE_TEXT)
{
return 1;
}

#endif


    if (stream != STREAM_NONE)
    {
        unsigned short prefixLen = 0;
        unsigned char *ptr = slipBuffer;
		unsigned short offset = 0;

		// Add a prefix the first time into the main loop
        if (prefix != NULL && prefix[0] != '\0')
        {
            prefixLen = strlen(prefix);
			if (prefixLen > maxLen) { prefixLen = maxLen; }	// Prefix doesn't fit (will be truncated)
            memcpy(ptr, prefix, prefixLen);
            ptr += prefixLen;
        }

		// Main loop to send chunks of data
		for (;;)
		{
	        unsigned short outLen = 0;
			unsigned short consumed = 0;

	        if      (streamFormat == STREAM_MODE_WAX_SLIP)  { outLen = SlipEncodeStream(ptr, buffer, len, offset, maxLen - prefixLen, &consumed); }
	        else if (streamFormat == STREAM_MODE_OMX_SLIP)  { outLen = SlipEncodeSector(ptr, buffer, len, offset, maxLen - prefixLen, &consumed); }
	        else if (streamFormat == STREAM_MODE_TEXT) 		{ outLen =      ASCIIPacket(ptr, buffer, len, offset, maxLen - prefixLen, &consumed); }
	
			// No more data to send
	        if (outLen + prefixLen  <= 0) { break; }

			// Send the data
	        {
	            // Streaming over wireless
	            if ((stream == STREAM_WIRELESS || stream == STREAM_BOTH))
	            {
	                // Send out over BT port
					if (BluetoothTxBusy())
					{
						BluetoothSerialWait();
					}
                	BluetoothSendData(slipBuffer, prefixLen + outLen);
	                ret |= 1;
	            }
	
	            // Streaming over USB
	            if ((stream == STREAM_USB || stream == STREAM_BOTH))
	            {
	                // Send out over USB
	                usb_write(slipBuffer, prefixLen + outLen);
	                ret |= 2;
	            }
	        }

			// Reset pointer to start of buffer (no prefix after the first loop iteration)
			ptr = slipBuffer;
			prefixLen = 0;

			// Update offset
			offset += consumed;
			if (outLen == 0 || consumed == 0) { break; }		// If we're not making progress consuming the stream
		}


    }

    return ret;
}


//EOF
