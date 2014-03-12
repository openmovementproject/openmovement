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

// Simple interface to Si44 BAX receiver
// Karim Ladha, 2013-2014

#ifndef _BAX_RX_H_
#define _BAX_RX_H_

// Definitions
#define AES_KEY_PKT_TYPE	0		/* Packet type for encryption packets */
#define DECODED_BAX_PKT		1		/* Packet was successfully decoded */
#define RAW_PKT_TYPE		0xff	/* Packet was not in decode list */

// Types
typedef union {
	struct{
		unsigned long address;
		unsigned long keyL[4]; 
	};
	unsigned char b[20];
}BaxKey_t;

typedef union {
	struct {
		/* Locally added */
		unsigned long address;
		unsigned char rssi;
		/* RAW PACKET */
		unsigned char 	pktType;
		unsigned char 	pktId; // b[6] 
		signed char 	xmitPwrdBm;
		unsigned short 	battmv;
		unsigned short	humidSat;
		signed short 	tempCx10;
		unsigned short 	lightLux;
		unsigned short	pirCounts;
		unsigned short	pirEnergy;
		unsigned short	swCountStat;
	};
	unsigned char b[5+AES_BLOCK_SIZE+1]; //+5 for headers +1 for type (22 bytes)
} BaxPacket_t;

// Globals
extern BaxKey_t baxKeys[MAX_LOCAL_BAX_KEYS];

// Includes

// Prototypes
void BaxRxInit(void);
void BaxRxTasks(void);
void BaxReadPacket(void);
void BaxSetDiscoveryCB(void(*CallBack)(BaxPacket_t* pkt));
unsigned short BaxAddKeys(BaxKey_t* keys, unsigned short num);
#endif
//EOF
