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

// Network Code

#ifndef NETWORK_H
#define NETWORK_H

extern char radioPresent;		// In local patched version of MRF24J40.c

void PrintMenu(void);
void ProcessMenu(void);
BOOL Reconnect(void);
void RadioOn(void);
void RadioOff(void);

void NetworkDumpMessage(void);

extern BYTE myLongAddress[MY_ADDRESS_LENGTH];


#define SLIP_END     0xC0                   // End of packet indicator
#define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
#define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
#define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte
void usb_putchar(unsigned char c);
unsigned char usb_slip_encoded(void *buffer, unsigned char length);


#define DATA_TEDDI_REPORT_ID        0x54    // ASCII 'T'
#define DATA_TEDDI_PAYLOAD_VERSION  0x04    // 4


#if (DATA_TEDDI_PAYLOAD_VERSION == 0x04)

    #include "Utils/BitPack.h"

    #define DATA_MAX_INTERVAL 24
    #define DEFAULT_DATA_INTERVAL 24        // (24 = 6 seconds)
    #define DATA_INTERVAL (settings.dataInterval)
    
    #define DATA_OFFSET 18
    #define ADDITIONAL_OFFSET(interval) (DATA_OFFSET + BITPACK10_SIZEOF(interval * 2))
    #define ADDITIONAL_LENGTH 4

    // 'V4' TEDDI Data payload -- all WORD/DWORD stored as little-endian (LSB first)
    typedef struct
    {
        unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
        unsigned char  reportId;            // @ 1  [1] Report identifier (0x54, ASCII 'T')
        unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
        unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
        unsigned char  sampleCount;         // @ 5  [1] Sample count (default config is a sample unit of 250 msec interval with an equal number of PIR and audio samples; 24 = 6 seconds)
        unsigned short sequence;            // @ 6  [2] Sequence number (16-bit)
        unsigned short unsent;              // @ 8  [2] Number of unsent samples on device (default config is a sample unit of 250 msec)
        unsigned short temp;                // @10  [2] Temperature reading (if top bit set, humidicon sensor, otherwise old sensor)
        unsigned short light;               // @12  [2] Light reading
        unsigned short battery;             // @14  [2] Battery reading
        unsigned short humidity;            // @16  [2] Humidity reading
        unsigned char  data[BITPACK10_SIZEOF(DATA_MAX_INTERVAL * 2)];   // @18 [50] PIR and audio energy (4 Hz, 20x 2x 10-bit samples)
        unsigned short parentAddress;		// @ADDITIONAL_OFFSET+0  [2] (optional) Parent address
        unsigned short parentAltAddress;	// @ADDITIONAL_OFFSET+2  [2] (optional) Parent alt. address
    } __attribute__ ((packed, aligned(1))) dataPacket_t;
    
    // Bit-pack 4x 10-bit samples into 5-byte groups (stored little-endian):
    // AAAAAAAA BBBBBBAA CCCCBBBB DDCCCCCC DDDDDDDD
    // 76543210 54321098 32109876 10987654 98765432

#endif


#define MAX_PACKETS 12
#define DEFAULT_TRANSMIT_THRESHOLD 1                // Number of packets buffered before start of transmission
#define TRANSMIT_THRESHOLD (settings.transmitThreshold)                // Number of packets buffered before start of transmission


// TODO: Move this -- not in the right palce at all
typedef struct
{
	unsigned short temp;
	unsigned short light;
	unsigned short battery;
	unsigned short humidity;
	unsigned short pir[DATA_MAX_INTERVAL];
	unsigned short audio[DATA_MAX_INTERVAL];
} reading_t;
extern reading_t reading;


void DumpPacket(dataPacket_t *dp);


#endif
