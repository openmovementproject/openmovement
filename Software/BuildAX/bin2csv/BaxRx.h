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

// Headers
#include "BaxUtils.h"

// Definitions
#define BAX_DEVICE_INFO_FILE	"BAX_INFO.BIN"

#define AES_KEY_PKT_TYPE	0		/* Packet type for encryption packets */
#define BAX_NAME_PKT		4		/* Name type pkt format */
#define DECODED_BAX_PKT		1		/* BAX2.0 format */
#define DECODED_BAX_PKT_PIR	2		/* BAX2.0 format */
#define DECODED_BAX_PKT_SW	3		/* BAX2.0 format */
#define RAW_PKT_LIMIT		127		/* Added to pkt types not decoded, i.e. still encrypted */

#ifndef AES_BLOCK_SIZE
	#define AES_BLOCK_SIZE		16
#elif (AES_BLOCK_SIZE != 16)
	#error "AES block size must be 16 for this driver"
#endif
#define BAX_PKT_DATA_LEN	AES_BLOCK_SIZE
#define BAX_NAME_LEN		AES_BLOCK_SIZE	/* N.b. Including null (15+1)*/

// Storage of bax keys and historical packets in ram - set externally in hw profile
#define MAX_BAX_INFO_ENTRIES	20 	/*Number of devices we save the keys for, 36 bytes ram each*/
#define MAX_BAX_SAVED_PACKETS	1	/*Number historical packets saved, 20 bytes each multiplied by max keys*/

// Types
// Describes a generalised bax packet, data portion described below
PACK( typedef union {
	struct {
		/* Locally added */
		unsigned long address;			//b[0] address header (4)
		unsigned char rssi;				//b[4] rssi header (1)
		/* Over ridden by decrypter if unknown*/
		signed char 	pktType;		//b[5] encryption data starts here for link pkts (1)
		/* Data */
		unsigned char data[BAX_PKT_DATA_LEN];//b[6] - b[21], may be safely cast to one of following
	};
	unsigned char b[5+AES_BLOCK_SIZE+1]; //+5 for headers +1 for type (22 bytes)
} BaxPacket_t);

// The raw packet data structure (after decryption)
PACK( typedef struct {
		/* Data */
		unsigned char 	pktId; 			//b[0] 
		signed char 	xmitPwrdBm;		//b[1]
		unsigned short 	battmv;			//b[2]
		unsigned short	humidSat;		//b[4]
		signed short 	tempCx10;		//b[6]
		unsigned short 	lightLux;		//b[8]
		unsigned short	pirCounts;		//b[10]
		unsigned short	pirEnergy;		//b[12]
		unsigned short	swCountStat;	//b[14]	
} BaxSensorPacket_t);

// The name packet data structure
typedef struct {
		/* Name */
		char name[BAX_NAME_LEN];
} BaxNamePacket_t;

// The blank/inspecific packet data structure
typedef struct {
		/* Raw bytes */
		char data[BAX_PKT_DATA_LEN];
} BaxDataPacket_t;

// The key packet data structure
typedef struct {
		/* Encryption key */
		unsigned char aesKey[AES_BLOCK_SIZE];
} BaxLinkPacket_t;

// The bax info structure for the decryption key and user friendly name
PACK(typedef struct {				/*36 bytes total, PADDED TO 64 BYTES IN FILES*/
		unsigned long address; 	/*4 bytes*/
		unsigned char key[AES_BLOCK_SIZE];/*16 bytes*/
		char name[BAX_NAME_LEN];/*16 bytes*/
}BaxInfo_t);

// The historical packet type storing device packet history
PACK(typedef struct {		/*22 bytes total*/
	DateTime time;			/*4 bytes timestamp*/
	unsigned char rssi;		/*1 byte rssi (raw)*/
	unsigned char pktType;	/*1 byte packet type*/
	unsigned char data[BAX_PKT_DATA_LEN];/*16 bytes data*/
}BaxEntry_t); 

// The structure holding device info
typedef struct {
	BaxInfo_t info;
	BaxEntry_t *entry[MAX_BAX_SAVED_PACKETS];
}BaxDeviceInfo_t;

// Globals
// Externed for debug only
extern BaxDeviceInfo_t baxDeviceInfo[MAX_BAX_INFO_ENTRIES];
extern BaxEntry_t baxEntries[MAX_BAX_INFO_ENTRIES * MAX_BAX_SAVED_PACKETS];

// RSSI to dBm macro
#define RssiTodBm(_c) ((signed char)-128 + ((unsigned char)_c>>1))

// Includes

// Prototypes
// First
void BaxRxInit(void);
// Intermittently, checks for HW errors
void BaxRxTasks(void);
// Erase saved bax info on disk, replace with ram copy
void BaxSaveInfoFile(void);
// Load bax info from disk
void BaxLoadInfoFile(void);
// Retrieve device info/data
char* BaxGetName(unsigned long address);
BaxEntry_t* BaxGetLast(unsigned long address, unsigned short offset);
// From ISR only
BaxPacket_t* BaxReadPacket(BaxPacket_t* pkt);
// Device discovery setter
extern void(*BaxInfoPacketCB)(BaxPacket_t* pkt);
void BaxSetDevicePairedCB(void(*CallBack)(unsigned long pairedCount));
// Use to inform bax code of new info packet (writes to a file)
void BaxInfoPktDetected (BaxPacket_t* pkt); 
// Add info struct to ram
void BaxAddKey(BaxInfo_t* key);

#endif
//EOF
