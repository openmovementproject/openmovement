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

// PC side bax receiver code
// Karim Ladha, 2013-2014

// Includes
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BaxUtils.h"
#include "BaxRx.h"
#include "aes.h"


// Globals
#ifndef MAX_BAX_INFO_ENTRIES
	#warning "Device will not decrypt packets!"
	#define MAX_BAX_INFO_ENTRIES 	0
	#define MAX_BAX_SAVED_PACKETS 	0
#endif

// Debug setting
#undef DEBUG_LEVEL
#define DEBUG_LEVEL	0
#define DBG_FILE dbg_file
#if (DEBUG_LEVEL > 0)||(GLOBAL_DEBUG_LEVEL > 0)
static const char* dbg_file = "baxrx.c";
#endif
#include "Debug.h"

BaxDeviceInfo_t baxDeviceInfo[MAX_BAX_INFO_ENTRIES];				/*Device info struct*/
BaxEntry_t baxEntries[MAX_BAX_INFO_ENTRIES * MAX_BAX_SAVED_PACKETS];/*Device last packets*/
static unsigned long pairedDevices = 0;								/*Count of paired devices*/
static void(*BaxDevicePairedCB)(unsigned long pairedCount) = NULL;			/*Called on paired device change*/

// Private prototypes
static void BaxEraseEntry(BaxEntry_t *entry);
static void BaxEraseInfo(BaxInfo_t* info);
static void BaxEraseDeviceInfo(BaxDeviceInfo_t* device);
static void BaxInitDeviceInfo(void);
static void BaxAddNewInfo(BaxInfo_t* newInfo);
static void BaxAddEntry(BaxDeviceInfo_t* device, BaxPacket_t* pkt);
static BaxDeviceInfo_t* BaxSearchInfo(unsigned long address);
static unsigned char BaxDecodePkt(BaxPacket_t* pkt);
static unsigned char BaxAddInfoToFile (FSFILE* file, BaxInfo_t* info);
static unsigned char BaxLoadInfoFromFile (FSFILE* file, BaxInfo_t* read);

// Call first
void BaxRxInit(void)
{
	// Clear callback
	BaxDevicePairedCB = NULL;
	pairedDevices = 0;
	// Initialise device info struct
	BaxInitDeviceInfo();
	// Load the device info file
	BaxLoadInfoFile();
}

// Called from the Rx isr to read the new packet
BaxPacket_t* BaxReadPacket(BaxPacket_t* pkt)
{
	switch(pkt->pktType){
		case DECODED_BAX_PKT : 
		case DECODED_BAX_PKT_PIR : 
		case DECODED_BAX_PKT_SW : {
			// Already decoded
			DBG_INFO("\r\nBAX SENSOR PKT");
			return pkt;
			break;
		}
		case -(DECODED_BAX_PKT) : 
		case -(DECODED_BAX_PKT_PIR) : 
		case -(DECODED_BAX_PKT_SW) : {
			// Try decode undecoded sensor packets
			if(BaxDecodePkt(pkt))
			{	
				// Make data element for recevied and decoded packets
				DBG_INFO("\r\nBAX SENSOR PKT");
				// Make pktType positive again
				pkt->pktType = -pkt->pktType;
				return pkt;
			}
			else
			{
				// Dump undecoded data
				;
			}
			break;
		}
		case AES_KEY_PKT_TYPE : 
		case BAX_NAME_PKT : {
			// Save info to info struct
			DBG_INFO("\r\nBAX INFO PKT");
			BaxInfoPktDetected(pkt);
			break;
		}
		default : {
			// Other packets
			DBG_INFO("\r\nBAX PKT UNKNOWN.");
			break;
		}
	} // Switch scope	
	return NULL;
}


// Decode an encrypted packet
static unsigned char BaxDecodePkt(BaxPacket_t* pkt)
{
	// Decrypt (requires temp buffer)
	unsigned char temp[16];
	// Search for an info entry
	BaxDeviceInfo_t* device;
	device = BaxSearchInfo(pkt->address);
	// Check it found one
	if(device == NULL) return FALSE;
	// Decrypt
	aes_decrypt_128(pkt->data,pkt->data,device->info.key,temp);
	// Update last packet list
	BaxAddEntry(device, pkt);
	// Done
	return TRUE;
}

// Adds the current packet to the last entries list
static void BaxAddEntry(BaxDeviceInfo_t* device, BaxPacket_t* pkt)
{
	BaxEntry_t* temp;
	// Early out for no device entries
	if(MAX_BAX_SAVED_PACKETS <= 0) return;
	// Get pointer to oldest in list
	temp = device->entry[MAX_BAX_SAVED_PACKETS-1];
	// Shift the list of *pointers* to remove last entry
	memmove(&device->entry[1],&device->entry[0], ((MAX_BAX_SAVED_PACKETS-1) * sizeof(BaxEntry_t*)));
	// Overwrite the older entry and set time
	temp->time = RtcNow();
	temp->rssi = pkt->rssi;
	temp->pktType = pkt->pktType;
	memcpy(temp->data,pkt->data,sizeof(BaxDataPacket_t));
	// Set first entry to old pointer (pointers rotate and are preserved)
	device->entry[0] = temp;
	// Done
	return;
}

// Set this to the callback fptr to enable device discovery. Call at main scope.
void BaxInfoPktDetected (BaxPacket_t* pkt)
{
	// Info structure and pointer stacked
	BaxInfo_t* infoToSave = NULL;
	BaxInfo_t  tempInfo;
	// Check
	if(pkt==NULL)return;
	// Init local
	BaxEraseInfo(&tempInfo);
	// Check packet type
	if(pkt->pktType == AES_KEY_PKT_TYPE)
	{
		// Copy in parts
		tempInfo.address = pkt->address;
		memcpy(tempInfo.key,pkt->data,AES_BLOCK_SIZE);
		// Indicate we have a new entry
		infoToSave = &tempInfo;
		// Add it to ram
		BaxAddNewInfo(infoToSave);
		DBG_INFO("\r\nNew bax info added.");
	}
	else if (pkt->pktType == BAX_NAME_PKT)
	{
		// Search for a pre-existing info entry for this device
		BaxDeviceInfo_t* device;
		unsigned short i;
		device = BaxSearchInfo(pkt->address);
		// Can't name devices we don't know
		if(device == NULL) return;
		// Format and copy name field
		for(i=0;i<(BAX_NAME_LEN-1);i++)
		{
			// Read each char
			char c = pkt->data[i];
			// Force to alpha numeric plus space/dash
			if(	(c < '0' && c != ' ' && c != '-' && c != '\0') || /* Allow space, dash, null */
				(c > '9' && c < 'A') || /* Remove this ascii range */
				(c > 'Z' && c < 'a') || /* Remove this ascii range */
				(c > 'z') )  			/* Remove end of ascii range */
			c = '_'; 					/* Replace illegal chars with '_' */
			device->info.name[i] = c;
			// Early out on null
			if (c == '\0') break;
		}
		// Null terminate
		device->info.name[BAX_NAME_LEN-1] = '\0';
		DBG_INFO("\r\nNew device name: %s",device->info.name);
		// Save to file too
		infoToSave = &device->info;	
	}

	// If we have a new info entry to save
	if(infoToSave != NULL)
	{
		// Add to file as well
		FSFILE* config_file = FSfopen(BAX_DEVICE_INFO_FILE,"a");
		if(config_file)
		{
			// Save to file
			if(BaxAddInfoToFile(config_file, infoToSave))
			{
				DBG_INFO("\r\nNew bax info saved to file.");
			}
			// Close
			FSfclose(config_file);
		}
	}
	return;
}

// Device discovery callback set
void BaxSetDevicePairedCB(void(*CallBack)(unsigned long pairedCount))
{
	// Set callback
	BaxDevicePairedCB = CallBack;
	// Call it if not null
	if(BaxDevicePairedCB)BaxDevicePairedCB(pairedDevices);
}

// Erase a device entry
static void BaxEraseEntry(BaxEntry_t *entry)
{
	if(entry == NULL) return;
	entry->time = DATETIME_INVALID;
	//memset(entry->pkt,0,sizeof(BaxDataPacket_t)); Not needed
}

// Erase a device info struct
static void BaxEraseInfo(BaxInfo_t* info)
{
	if(info == NULL) return;
	// Erase the info sections
	memset(info,0,sizeof(BaxInfo_t));
}

// Erase sensor info and data, pointers left alone
static void BaxEraseDeviceInfo(BaxDeviceInfo_t* device)
{
	unsigned short i;
	// Erase device info
	BaxEraseInfo(&device->info);
	// Invalidate device data entries
	for(i=0;i<MAX_BAX_SAVED_PACKETS;i++)
	{
		BaxEraseEntry(device->entry[i]);
	}
}

// Initialise the device info
static void BaxInitDeviceInfo(void)
{
	// Init the device info structure
	unsigned short i = 0, j = 0;
	// Assign pointers to entries, clear entries
	for(i=0;i<MAX_BAX_INFO_ENTRIES;i++)
	{
		// Initialise entry pointers
		for(j=0;j<MAX_BAX_SAVED_PACKETS;j++)
		{
			baxDeviceInfo[i].entry[j] = &baxEntries[(i*MAX_BAX_SAVED_PACKETS)+j];
		}
		// Wipe all info/entries
		BaxEraseDeviceInfo(&baxDeviceInfo[i]);
	}
}

// Add a new info struct to ram
static void BaxAddNewInfo(BaxInfo_t* newInfo)
{
	unsigned long address = newInfo->address;
	unsigned short i;

	if(newInfo == NULL) return;	

	// Erase all duplicate entries for address
	for(i=0;i<MAX_BAX_INFO_ENTRIES;i++)
	{
		if(baxDeviceInfo[i].info.address == address)
		{
			DBG_INFO("\r\nOLD KEY DEL.");
			BaxEraseDeviceInfo(&baxDeviceInfo[i]);
			pairedDevices--;
			if(BaxDevicePairedCB)BaxDevicePairedCB(pairedDevices);
		}
	}
	// Add new entries in first empty slot (if duplicate deleted, this will be that slot)
	for(i=0;i<MAX_BAX_INFO_ENTRIES;i++)
	{
		if(baxDeviceInfo[i].info.address == 0ul)
		{	
			DBG_INFO("\r\nNEW KEY ADD.");
			memcpy(&baxDeviceInfo[i].info,newInfo,sizeof(BaxInfo_t));
			pairedDevices++;
			if(BaxDevicePairedCB)BaxDevicePairedCB(pairedDevices);
			break;
		}
	}
	// Dump newest in list if no space
	#if (MAX_BAX_INFO_ENTRIES > 0)
	if(i >= MAX_BAX_INFO_ENTRIES)
	{
		unsigned short j;
		const unsigned short lastIndex = MAX_BAX_INFO_ENTRIES-1;
		DBG_INFO("\r\nOLD KEY REPLACED");
		// Save pointers (this entangles the entry pointers but avoids moving larger memory chunks)
		for(j=0;j<MAX_BAX_SAVED_PACKETS;j++)
			{baxDeviceInfo[lastIndex].entry[j] = baxDeviceInfo[0].entry[j];}
		// Move the memory device info chunk to replace first entry
		memmove(&baxDeviceInfo[0],&baxDeviceInfo[1],sizeof(BaxDeviceInfo_t)*(lastIndex));
		// Clear device info and entry data
		BaxEraseDeviceInfo(&baxDeviceInfo[lastIndex]);
		// Copy in new info structure
		memcpy(&baxDeviceInfo[lastIndex].info,newInfo,sizeof(BaxInfo_t));
		if(BaxDevicePairedCB)BaxDevicePairedCB(pairedDevices);
	}
	#endif
	return;
}

// Retrieve a pointer to the info structure using current raw packet
static BaxDeviceInfo_t* BaxSearchInfo(unsigned long address)
{
	unsigned short i;
	for(i=0;i<MAX_BAX_INFO_ENTRIES;i++)
	{
		if(baxDeviceInfo[i].info.address == address)
			return &baxDeviceInfo[i];
	}
	return NULL;
}

// Retrieve a name for an address if present
char* BaxGetName(unsigned long address)
{
	// Search for an info entry
	BaxDeviceInfo_t* device;
	device = BaxSearchInfo(address);
	// Check it found one
	if(device == NULL) return NULL;
	// Return pointer
	return device->info.name;
}

// Retrieve the last packet for an address if present
BaxEntry_t* BaxGetLast(unsigned long address, unsigned short offset)
{
	// Search for an info entry
	BaxDeviceInfo_t* device;
	device = BaxSearchInfo(address);
	// Check it found one
	if(device == NULL) return NULL;
	// Check offset is valid
	if(offset >= MAX_BAX_SAVED_PACKETS) return NULL;
	// Check entry has been written
	if(device->entry[offset]->time == DATETIME_INVALID) return NULL;
	// Return pointer to entry
	return device->entry[offset];
}

// Load an info struct from a file pointer, read upto next entry
static unsigned char BaxLoadInfoFromFile (FSFILE* file, BaxInfo_t* read)
{
	// Align to 64 byte block
	signed long filePos = FSftell(file);
	signed long remaining = FSFileSize(file) - filePos;
 	signed long alignment = filePos % 64ul;
	if(alignment != 0)
	{
		FSfseek(file,-alignment,SEEK_CUR);
		remaining += alignment;
	}
	// If there is enough file remaining
	if(remaining < 64)
	{
		// File ended
		return FALSE;
	}
	// Read new entry to pointer
	if(FSfread(read,1,sizeof(BaxInfo_t),file) != sizeof(BaxInfo_t))
	{
		// Read error
		return FALSE;
	}
	// Seek to start of next entry
	FSfseek(file,(64-sizeof(BaxInfo_t)),SEEK_CUR);
	return TRUE;
}

// Save an info struct from a file pointer 
static unsigned char BaxAddInfoToFile (FSFILE* file, BaxInfo_t* info)
{
	// Aligned to 64 byte block
	unsigned char buffer[64];
	signed long filePos = FSftell(file);
 	signed long alignment = filePos % 64ul;
	if(alignment != 0)
	{
		// Over write misaligned section
		FSfseek(file,-alignment,SEEK_CUR);
	}
	// Make new entry, zero pad
	memset(buffer,0,64);
	memcpy(buffer,info,sizeof(BaxInfo_t));
	// Write new entry to file
	if(FSfwrite(buffer,1,64,file) != 64)
	{
		// Write error
		return FALSE;
	}
	// Done
	return TRUE;
}

// Load device info from file
void BaxLoadInfoFile(void)
{
	// Key
	BaxInfo_t read;
	FSFILE* config_file;
	if(MAX_BAX_INFO_ENTRIES == 0) return;
	// Clear all info, initialise pointers
	BaxEraseInfo(&read);
	// Open file
	config_file = FSfopen(BAX_DEVICE_INFO_FILE,"r");
	if(config_file)
	{
		DBG_INFO("\r\nLoading bax config file.");
		for(;;)
		{
			// Read new entry
			if(BaxLoadInfoFromFile (config_file, &read))
			{
				// Add key
				DBG_INFO("\r\nInfo loaded");
				BaxAddNewInfo(&read);
			}
			else
			{
				// Failed
				break;
			}
		}
		// Close
		FSfclose(config_file);
	}
}

// Erases old info values and replaces them with current list from ram
void BaxSaveInfoFile(void)
{
	// Save to file
	FSFILE* config_file;
	if(MAX_BAX_INFO_ENTRIES == 0) return;
	config_file = FSfopen(BAX_DEVICE_INFO_FILE,"w");
	if(config_file)
	{
		unsigned short i;
		DBG_INFO("\r\nReplacing bax config file.");
		for (i=0;i<MAX_BAX_INFO_ENTRIES;i++)
		{
			// Write to file
			if(BaxAddInfoToFile (config_file, &baxDeviceInfo[i].info))
			{
				DBG_INFO("\r\nInfo saved");
			}
			else
			{
				break;
			}
		}
		// Close file
		FSfclose(config_file);
	}
	return;
}

//EOF
