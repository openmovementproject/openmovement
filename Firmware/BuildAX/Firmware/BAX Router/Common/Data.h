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

#ifndef __DATA_H__
#define __DATA_H__
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
#include "Peripherals/Rtc.h"

// Definitions
#define NUMBER_OF_ELEMENTS_IN_FIFO	32
#define MAX_ELEMENT_SIZE			128 	/* for non-dynmic there are multiple instances, teddi1.11 pkt = 82 bytes)*/
#define MAX_ELEMENT_TEXT_LEN		(MAX_ELEMENT_SIZE)			/*Text in/output buffer (reader)*/
#define MAX_ELEMENT_BIN_LEN 		(2*MAX_ELEMENT_SIZE + 2)	/*Allow for slip encoding too*/
#define DYNAMIC_ALLOCATION

// Data type definitions 
#define TYPE_TEXT_ELEMENT	0 	// Raw text string type
#define TYPE_BAX_PKT		1	// Binary BAX packet
#define TYPE_MIWI_PKT		2	// Binary Teddi 1.11 packet

// For socketed connections using source id
// use this to indicate all end points
#define DATA_DEST_ALL	0xff

// Types

// Data element flags 
#define TXT_FILE_FLAG	(0x0001)
#define BIN_FILE_FLAG	(0x0002)
#define TXT_CDC_FLAG	(0x0004)
#define BIN_CDC_FLAG	(0x0008)
#define TXT_UDP_FLAG	(0x0010)
#define BIN_UDP_FLAG	(0x0020)
#define TXT_GSM_FLAG	(0x0040)
#define BIN_GSM_FLAG	(0x0080)
#define TXT_TEL_FLAG	(0x0100)
#define BIN_TEL_FLAG	(0x0200)
#define TXT_ERR_FLAG	(0x0400)
#define _UNUSEDFLAG_	(0x0800)
// CMD type flags
#define CMD_BATCH_FLAG	(0x1000)
#define CMD_CDC_FLAG	(0x2000)
#define CMD_UDP_FLAG	(0x4000)
#define CMD_TEL_FLAG	(0x8000)
// Maintain these to match flags
#define CMD_ERR_STREAMS 	(0xf400)
#define TXT_STREAMS_MASK 	(0xf555)
#define BIN_STREAMS_MASK 	(0x02AA)


typedef struct {
	unsigned short 	flags;
	DateTime		timeStamp;
	unsigned char 	dataType;
	unsigned char 	address;
	unsigned char 	dataLen;
	unsigned char* 	data;
} dataElement_t;

// Globals

// Globals - private, debug
extern dataElement_t dataElementBuffer[NUMBER_OF_ELEMENTS_IN_FIFO];
#ifndef DYNAMIC_ALLOCATION
extern unsigned char dataElementHeap[NUMBER_OF_ELEMENTS_IN_FIFO * MAX_ELEMENT_SIZE];
#endif

// Prototypes
// Call at startup
void InitDataList(void);
// Useful when using dynamic allocation
void ClearDataList(void);
// Add data element to fifo
void AddDataElement(unsigned char type, unsigned char address, unsigned char len, unsigned short flags, void* data);
// Call to return a suitable flag variable in agreement with the currently open streams
unsigned short GetOpenStreams(void);
// Read upto all of the stored elements to upto all streams
void ReadDataElements(unsigned short flags);

// Other
// Simple function to dump raw ascii, capitalised hex to a buffer, no spaces, with a null. Can increment back too
unsigned short DumpHexToChar(char* dest, void* source, unsigned short len, unsigned char littleEndian);

// Private prototypes
// Private, reads elements
char* ReadElementTxt(unsigned short* len, dataElement_t* element, unsigned char mode);
// Private, reads elements
unsigned char* ReadElementBin(unsigned short* len, dataElement_t* element, unsigned char mode);
// Private, binary slip encoder
size_t slip_encode(void *outBuffer, const void *inBuffer, size_t length, size_t outBufferSize);

// Interrupts
typedef unsigned char DATA_IPL_shadow_t;
#define DATA_INTS_DISABLE()	{IPLshadow = SRbits.IPL; if(SRbits.IPL<DATA_ELEMENT_PRIORITY)SRbits.IPL=DATA_ELEMENT_PRIORITY;}
#define DATA_INTS_ENABLE()	{SRbits.IPL = IPLshadow;}

// Other packet types 
// Types


#endif

