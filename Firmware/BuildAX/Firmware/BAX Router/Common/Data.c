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

// Data output handling functions
// Karim Ladha, 2013-2014

/*
General funtionality:
1) 	Fifo of generic packet types with pointers, flags and types. 
	The pointers can use a dynamic allocation scheme if needed.
2) 	Multiple output streams use the said fifo.
	Each stream clears its flag when it consumes the data.
3)	Each stream consumes the data using a common reader.
	The reader known how to interpret the data including decryption. 
4) 	If the fifo is full before all the data sinks have read the oldest
	data then that data is removed as new data arrives.
5)	Fifo is only used to establish the ordering to decide which 
	element is discarded first on overflow.
6)	We can't have individual heads to a fifo since each would need a tail
	and this makes it difficult to be sure when we can remove an item as 
	the head and tails span the end of the fifo.
*/

// Headers
#include <stdarg.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "Data.h"
#include "Peripherals/Rtc.h"

// Stream input/output specific headers
#include "WirelessProtocols/MCHP_API.h"
#include "Network.h"
#include "MiWiRx.h"
#include "Peripherals/Si443x.h"
#include "USB/USB_CDC_MSD.h"
#ifdef USE_FAT_FS
#include "FatFs/FatFsIo.h"
#else
#include "MDD File System/FSIO.h"
#endif
#include "TCPIPConfig.h"
#ifdef ENABLE_UDP_SERVICES
#include "TCPIP Stack/UDP services local.h"
#endif
#ifdef	STACK_USE_TELNET_SERVER
#include "TCPIP Stack/TelnetLocal.h"
#endif


// Debug setting
//	#define DEBUG_ON
#include "Debug.h"

// SLIP-encoded packet -- write SLIP_END bytes before and after the packet: usb_putchar(SLIP_END);
#define SLIP_END     0xC0                   // End of packet indicator
#define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
#define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
#define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte

// Globals

// Globals - private
volatile unsigned short nextSpace;
dataElement_t dataElementBuffer[NUMBER_OF_ELEMENTS_IN_FIFO];
#ifndef DYNAMIC_ALLOCATION
unsigned char dataElementHeap[NUMBER_OF_ELEMENTS_IN_FIFO * MAX_ELEMENT_SIZE];
#endif

// Prototypes
// Call at startup
void InitDataList(void)
{
	// Assumes zero is NULL - erase all elements
	DATA_IPL_shadow_t IPLshadow;
	DATA_INTS_DISABLE();
	memset(dataElementBuffer, 0, (NUMBER_OF_ELEMENTS_IN_FIFO * sizeof(dataElement_t)));
	nextSpace = 0;	// Location of next element item
	DATA_INTS_ENABLE();
}

// Useful when using dynamic allocation
void ClearDataList(void)
{
	unsigned short i;
	DATA_IPL_shadow_t IPLshadow;
	DATA_INTS_DISABLE();
	for(i=0;i<NUMBER_OF_ELEMENTS_IN_FIFO;i++)
	{
		dataElementBuffer[i].dataType = 0;
		dataElementBuffer[i].dataLen = 0;
		dataElementBuffer[i].flags = 0;
		#ifdef DYNAMIC_ALLOCATION
			if(dataElementBuffer[i].data != NULL)
				free(dataElementBuffer[i].data);
		#endif
		dataElementBuffer[i].data = NULL;		
	}
	nextSpace = 0;	// Reset
	DATA_INTS_ENABLE();
}

// Call externally to add a data element
void AddDataElement(unsigned char type, unsigned char address, unsigned char len, unsigned short flags, void* data)
{
	// Check flags
	if(flags == 0) return; // No valid streams, discard

	DATA_IPL_shadow_t IPLshadow;
	DATA_INTS_DISABLE();

	// Make an element to push into the fifo
	dataElement_t element = {
		.flags = flags,
		.timeStamp = RtcNow(), // Time stamp it
		.dataType = type,
		.address = address,
		.dataLen = len
	};

	// Get a ptr to put the data at
	#ifdef DYNAMIC_ALLOCATION
		// Free pointer on this location if not null
		if(dataElementBuffer[nextSpace].data != NULL)
			free(dataElementBuffer[nextSpace].data);
		// Dynamic: Allocate new space
		element.data = malloc(len);
		if(element.data == NULL)
		{
			DBG_ERROR("Data.c: Insufficient memory");
			DATA_INTS_ENABLE();
			return; // FAIL
		}
	#else
		// Static: We already know there are bytes alotted to the element
		element.data = &dataElementHeap[(MAX_ELEMENT_SIZE*nextSpace)]; 
	#endif

	// Location holding raw data (clamp size)
	if(len > MAX_ELEMENT_SIZE) len = MAX_ELEMENT_SIZE;
	memcpy(element.data,data,len);
		
	// Element is list holding pointer and other info
	memcpy(&dataElementBuffer[nextSpace], &element, sizeof(dataElement_t));

	// Increment index + wrap
	nextSpace = (nextSpace+1)%NUMBER_OF_ELEMENTS_IN_FIFO;

	DATA_INTS_ENABLE();
	return;
}

// Call to return the flags currently set to accept data by the settings AND are operational
unsigned short GetOpenStreams(void)
{
	// NOTE: The command and error flags are always enabled 
	// USE: (CMD_UDP_FLAG | CMD_CDC_FLAG | TXT_CMD_BATCH | TXT_ERR_FLAG)
	unsigned short ret = 0;

	if((settings.usb_stream == STATE_ON)&&(status.usb_state == ACTIVE))
	{
		if(settings.usb_stream_mode == 0) 		// Text
			ret |= TXT_CDC_FLAG;
		else if (settings.usb_stream_mode == 1) // Binary
			ret |= BIN_CDC_FLAG;
	}  
	if((settings.file_stream == STATE_ON)&&(status.file_state == ACTIVE))
	{
		if(settings.file_stream_mode == 0) 		// Text
			ret |= TXT_FILE_FLAG;
		else if (settings.file_stream_mode == 1) // Binary
			ret |= BIN_FILE_FLAG;
	}  
	if((settings.udp_stream == STATE_ON)&&(status.udp_state == ACTIVE))
	{
		if(settings.udp_stream_mode == 0) 		// Text
			ret |= TXT_UDP_FLAG;
		else if (settings.udp_stream_mode == 1) // Binary
			ret |= BIN_UDP_FLAG;
	}  
	if((settings.telnet_stream == STATE_ON)&&(status.telnet_state == ACTIVE))
	{
		if(settings.telnet_stream_mode == 0) 		// Text
			ret |= TXT_TEL_FLAG;
		else if (settings.telnet_stream_mode == 1) // Binary
			ret |= BIN_TEL_FLAG;
	}  
	if((settings.gsm_stream == STATE_ON)&&(status.gsm_state == ACTIVE))
	{
		if(settings.gsm_stream_mode == 0) 		// Text
			ret |= TXT_GSM_FLAG;
		else if (settings.gsm_stream_mode == 1) // Binary
			ret |= BIN_GSM_FLAG;
	}
	return ret;  
}

// Call externally to put all pending data sources over the flagged channels
void ReadDataElements(unsigned short flags)
{
	// Read (output) data elements with matching flags to those set
	DATA_IPL_shadow_t IPLshadow;
	unsigned short i;
	unsigned short index;
	dataElement_t *element;
	unsigned short activeFlags;
	unsigned char elementType, elementAddress; 
	unsigned short firstIndexRead;

	// No streams being read - exit
	if(flags == 0) return; 	

	// firstIndexRead must be an impossible index to start with
	firstIndexRead = NUMBER_OF_ELEMENTS_IN_FIFO; //(outside array)	
	for(i=0;i<NUMBER_OF_ELEMENTS_IN_FIFO;i++) 	
	{
		unsigned char *sourceBin;
		char* sourceTxt;
		unsigned short lenTxt, lenBin;

		// We can not allow elements to be overwritten whilst reading
		DATA_INTS_DISABLE();

		// Set start point, always start at the oldest item in the list
		if(firstIndexRead == NUMBER_OF_ELEMENTS_IN_FIFO) 
		{
			firstIndexRead = nextSpace;			// Initialise on first call only
		}
		else if (firstIndexRead != nextSpace) 	// Extra elements added during loops
		{
			// If the new nextSpace <= next index -> continue
			// If the new nextSpace > next index, skip to nextSpace
			unsigned short added;
			if(nextSpace >= firstIndexRead) added = nextSpace - firstIndexRead;
			else added = NUMBER_OF_ELEMENTS_IN_FIFO + nextSpace - firstIndexRead;
			if(added > i)  // Written over the next space to be read with newer data
			{
				i = added; // The overwritten slots are lost now
				// Break if all the items have been replaced
				// Only items in the list on first call are read 
				if(i >=NUMBER_OF_ELEMENTS_IN_FIFO) break;
			}
		}	

		// Find the next index	
		index = (firstIndexRead+i)%NUMBER_OF_ELEMENTS_IN_FIFO;

		// Get next element from this index
		element = &dataElementBuffer[index];

		// See if it has any of the flags we are looking to read
		activeFlags = (element->flags & flags);

		if(activeFlags == 0) // Not reading element on this call
		{
			#ifdef DYNAMIC_ALLOCATION
			// Free pointer on this location if not null and no streams
			if((element->data != NULL)&&(element->flags == 0))
			{
				free(element->data);
				element->data = NULL;
			}
			#endif
			// If not, continue
			DATA_INTS_ENABLE();
			continue; // No, we will skip it
		}

		// Now read the elements + latch type
		elementType = element->dataType;
		elementAddress = element->address;

		// Check if there are any text streams consuming this element
		if(activeFlags & TXT_STREAMS_MASK)
		{
			sourceTxt = ReadElementTxt(&lenTxt,element,0);
		}
		// Check if there are any binary streams consuming this element
		if(activeFlags & BIN_STREAMS_MASK)
		{
			sourceBin = ReadElementBin(&lenBin,element,0);
		}
		// Clear the flags for the active streams
		element->flags &= ~activeFlags;
		// Now we have read the elements data - re-enable ints
		DATA_INTS_ENABLE();


		// Write out data, text and binary streams
		if((activeFlags & TXT_FILE_FLAG) && (status.output_text_file))
		{
			// Write to text file
			FILE_LED = !FILE_LED; // Flicker LED
			if (FSfwrite(sourceTxt, 1, lenTxt, status.output_text_file) != lenTxt)
			{
				status.file_state = ERROR;
			}
		}
		if(activeFlags & TXT_CDC_FLAG)
		{
			// Send over CDC
			CDC_LED = !CDC_LED; // Flicker LED
			usb_write(sourceTxt,lenTxt);
		}
		if(activeFlags & TXT_ERR_FLAG)
		{
			// STDERR out
			//fprintf(stderr,"%s",sourceTxt);
			
			// Write to error log file file
			if(status.error_file != NULL)
			{
				FILE_LED = !FILE_LED; // Flicker LED
				// Timestamp
				const char* ptr = RtcToString(RtcNow());
				unsigned char len = strlen(ptr);
				FSfwrite(ptr, 1, len, status.error_file);			// Time stamp
				FSfwrite(",", 1, 1, status.error_file);				// Comma
				FSfwrite(sourceTxt, 1, lenTxt, status.error_file);	// Text error
		        FSfclose(status.error_file);						// Save
				status.error_file = FSfopen(ERROR_FILE, "a");		// Re-open
			}
		}

#ifdef	STACK_USE_TELNET_SERVER
		if(activeFlags & TXT_TEL_FLAG)
		{
			TEL_LED = !TEL_LED; // Flicker LED
			unsigned short clientMask = status.telnet_mask; // Default is all authenticated clients for non text
			if(elementAddress != DATA_DEST_ALL) 			// Specific client(s) being spoken too
				clientMask = (1ul<<elementAddress);	 		// Data is sent to corrent recipient client
			TelnetWrite(sourceTxt,lenTxt,clientMask);		// Send elements over the appropriate stream
		}
#endif

#ifdef ENABLE_UDP_SERVICES
		if(activeFlags & TXT_UDP_FLAG)
		{
			TCP_LED = !TCP_LED; // LED flickers 
			unsigned short clientMask = status.udp_mask; 	// Default is all authenticated clients for non text
			if(elementAddress != DATA_DEST_ALL) 			// Specific client(s) being spoken too
				clientMask = (1ul<<elementAddress);	 		// Data is sent to corrent recipient client
			UdpWritePacket(sourceTxt,lenTxt,clientMask);	// Send elements over the appropriate stream
		}
#endif

// TODO
		if(activeFlags & TXT_GSM_FLAG)
		{
		}	

		if((activeFlags & BIN_FILE_FLAG) && (status.output_bin_file))
		{
			// Write to text file
			FILE_LED = !FILE_LED; // Flicker LED
			if (FSfwrite(sourceBin, 1, lenBin, status.output_bin_file) != lenBin)
			{
				status.file_state = ERROR;
			}
		}
		if(activeFlags & BIN_CDC_FLAG)
		{
			// Send over CDC
			CDC_LED = !CDC_LED; // Flicker LED
			usb_write(sourceBin,lenBin);
		}
#ifdef	STACK_USE_TELNET_SERVER
		if(activeFlags & BIN_TEL_FLAG)
		{
			TEL_LED = !TEL_LED; // Flicker LED
			unsigned short clientMask = status.telnet_mask; // Default is all authenticated clients for non text
			if(elementAddress != DATA_DEST_ALL) 			// Specific client(s) being spoken too
				clientMask = (1ul<<elementAddress);	 		// Data is sent to corrent recipient client
			TelnetWrite(sourceBin,lenBin,clientMask);		// Send elements over the appropriate stream
		}
#endif

#ifdef ENABLE_UDP_SERVICES
		if(activeFlags & BIN_UDP_FLAG)
		{
			TCP_LED = !TCP_LED; // LED flickers 
			unsigned short clientMask = status.udp_mask; 	// Default is all authenticated clients for non text
			if(elementAddress != DATA_DEST_ALL) 			// Specific client(s) being spoken too
				clientMask = (1ul<<elementAddress);	 		// Data is sent to corrent recipient client
			UdpWritePacket(sourceBin,lenBin,clientMask);	// Send elements over the appropriate stream
		}
#endif

//TODO
		if(activeFlags & BIN_GSM_FLAG)
		{
		}	


		// Commands
		if(activeFlags & CMD_CDC_FLAG)
		{
			CDC_LED = !CDC_LED; // Flicker LED
			SettingsCommand(sourceTxt, SETTINGS_CDC, 0);
		}
		if(activeFlags & CMD_BATCH_FLAG)
		{
			SettingsCommand(sourceTxt, SETTINGS_BATCH, 0);
		}
		if(activeFlags & CMD_TEL_FLAG)
		{
			// Telnet commands have a source address as well
			SettingsCommand(sourceTxt, SETTINGS_TELNET, elementAddress);
		}
// TODO
		if(activeFlags & CMD_UDP_FLAG)
		{
			// Udp commands have a source address as well
			SettingsCommand(sourceTxt, SETTINGS_UDP, elementAddress);
		}

	}// For loop for all elements

	return;
}

char* ReadElementTxt(unsigned short* len, dataElement_t* element, unsigned char mode)
{
	static char output[MAX_ELEMENT_TEXT_LEN + 1];
	char *ptr=output;
	unsigned short length, maxLen;
	// Timestamp
	maxLen = &output[MAX_ELEMENT_TEXT_LEN-1] - ptr;
	switch (element->dataType) {
		// Raw text mode
		case TYPE_TEXT_ELEMENT :{
			length = element->dataLen;
			if(length>maxLen)length = maxLen;
			memcpy(ptr, element->data, length);
			ptr += length;
			break;
		}
		// Si44 BAX radio data packet
		case TYPE_BAX_PKT :{
			// Timestamp
			ptr+=sprintf(ptr,"BAX,%s,",RtcToString(element->timeStamp));
			// Device ID
			ptr+=DumpHexToChar(ptr,&element->data[0],4,TRUE); // Little endian, data[3]=='B', '42XXXXXX'
			// RSSI
			ptr+=sprintf(ptr,",%ddBm,",(short)RssiTodBm(element->data[4]));
			// Payload
			if(element->data[5] != 1) 	// Encryption packet
			{
				ptr+=DumpHexToChar(ptr,&element->data[5],(element->dataLen-5),FALSE);
				*ptr++ = '\r';*ptr++ = '\n';
			}
			else 						// Data packet
			{
				ptr+=sprintf(ptr,"%u,%u,%d,%u,%u.%02u,",
					(element->data[5]), 				// pktType 1
					(element->data[6]), 				// pktId
					((signed short)element->data[7]), 	// xmitPwr dbm
					LE_READ_16(&element->data[8]), 		// battmv
					(element->data[11]&0xff), 			// humidSat MSB
					(((signed short)39*(element->data[10]&0xff))/100));// humidSat LSB
				ptr+=sprintf(ptr,"%d,%u,%u,%u,%u\r\n",
					(signed short)LE_READ_16(&element->data[12]),// tempCx10
					LE_READ_16(&element->data[14]), 	// lightLux
					LE_READ_16(&element->data[16]),		// pirCounts
					LE_READ_16(&element->data[18]),		// pirEnergy
					LE_READ_16(&element->data[20])); 	// swCountStat
			}
			break;
			}	
		case TYPE_MIWI_PKT :{
			// Back to MiWi type packet pointer type
			RECEIVED_MESSAGE* pkt = (RECEIVED_MESSAGE*)(element->data);
			// Cast to TEDDI type packet
			dataPacket_t*	payload = (dataPacket_t*)pkt->Payload;
			// Timestamp
			ptr+=sprintf(ptr,"TED,%s,",RtcToString(element->timeStamp));
			// Device Id
			ptr+=sprintf(ptr,"%u,",payload->deviceId);
			// RSSI
			ptr+=sprintf(ptr,"%ddBm,",MrfRssiTodBm(pkt->PacketRSSI));
			// Payload dump all
			ptr+=DumpHexToChar(ptr,payload,sizeof(dataPacket_t),FALSE);
			*ptr++ = '\r';*ptr++ = '\n';
			break;
		}
		// TODO
		// MRF packets
		// Data annotation packets
		// Error reports
		default : break;
	}

	*len = ptr - output;	// Set the return len field
	*(char*)ptr = '\0'; 	// Always null terminate string
	return 	output;	
}


unsigned char* ReadElementBin(unsigned short* len, dataElement_t* element, unsigned char mode)
{
	static unsigned char output[MAX_ELEMENT_BIN_LEN];
	unsigned char *ptr=output, *ptrEnd=&output[MAX_ELEMENT_BIN_LEN-1] ;

	// Start the slip pkt
	*ptr++ = SLIP_END;
	// Add the packet time stamp
	ptr += slip_encode(ptr, &element->timeStamp, sizeof(DateTime), (ptr - ptrEnd));

	switch (element->dataType) {
		// Raw text mode - uses strlen, not element length
		case TYPE_TEXT_ELEMENT :{
			ptr += slip_encode(ptr, element->data, strlen((char*)element->data), (ptr - ptrEnd));
			break;
		}
		// Si44 radio data packet (and probably others)
		case TYPE_BAX_PKT :{
			ptr += slip_encode(ptr, element->data, element->dataLen, (ptr - ptrEnd));
			break;
		}
		// TODO
		// MRF packets
		// Data annotation packets
		// Error reports
		default : break;
	}

	*len = ptr - output;	// Set the return len field
	return 	output;	
}

// Returns bytes written or zero for insufficient space
size_t slip_encode(void *outBuffer, const void *inBuffer, size_t length, size_t outBufferSize)
{
    const unsigned char *sp = (const unsigned char *)inBuffer;
    unsigned char *dp = outBuffer;
    while (length--)
    {
        if (*sp == SLIP_END)
		{
			if (outBufferSize < 2) { return 0; }
			*dp++ = SLIP_ESC;
			*dp++ = SLIP_ESC_END;
			outBufferSize -= 2;
		}
        else if (*sp == SLIP_ESC)
		{
			if (outBufferSize < 2) { return 0; }
			*dp++ = SLIP_ESC;
			*dp++ = SLIP_ESC_ESC;
			outBufferSize -= 2;
		}
        else
		{
			if (outBufferSize < 1) { return 0; }
			*dp++ = *sp;
			outBufferSize--;
		}
        ++sp;
    }
    return (size_t)((void *)dp - (void *)outBuffer);
}

// Simple function to dump raw ascii, capitalised hex to a buffer, non-endian, no spaces, with a null.
unsigned short DumpHexToChar(char* dest, void* source, unsigned short len, unsigned char littleEndian)
{
	unsigned short ret = (len*2);
	unsigned char* ptr = source;

	if(littleEndian) ptr += len-1; // Start at MSB

	char temp;
	for(;len>0;len--)
	{
		temp = 0x30 + (*ptr >> 4);
		if(temp>'9')temp += ('A' - '9' - 1);  
		*dest++ = temp;
		temp = 0x30 + (*ptr & 0xf);
		if(temp>'9')temp += ('A' - '9' - 1); 
		*dest++ = temp;

		if(littleEndian)ptr--;
		else ptr++;
	}
	*dest = '\0';
	return ret;
}

//EOF

