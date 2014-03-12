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

// NVM variable location
// Karim Ladha, 2013-2014

#include <string.h>
#include "HardwareProfile.h"
#include "bt nvm.h"
#include "bt config.h"

#warning "This is an example file for how to implement the api for nvm storage"

// RAM vars - mac can be written by stack
static char device_name_local[MAX_BT_NAME_LEN+1] 	= BT_DEFAULT_NAME; 	// Inited to default name
static char device_pin_code_local[5] 				= BT_DEFAULT_PIN; 	// Inited to default pin	
static char device_MAC_local[7] 					= {0xff,0xff,0xff,0xff,0xff,0xff,0xff}; // Valid flag [6], 1==VALID
unsigned long device_cod_local 						= BT_DEFAULT_COD; 	// Inited to default cod

// NVM functions
char* GetBTName(void) 			{return device_name_local;}
char* GetBTPin(void) 			{return device_pin_code_local;}
char* GetBTMAC(void) 			{return device_MAC_local;}
unsigned long GetBTCOD(void) 	{return device_cod_local;}

// Code
void InitNvm(void) // Called by app
{
	unsigned short i;
	char *name = GetBTName();
	char* pin = GetBTPin();
	char *mac = GetBTMAC();

	// In this C file the local name, pin and COD are already initialised to the defaults in rom

	// Try to recover the nvm name if enabled
	ReadNvm((DEVICE_NAME_NVM), name, MAX_BT_NAME_LEN);
	for(i=0;i< MAX_BT_NAME_LEN;i++)
	{
		if (name[i] >= ' ' && name[i] < 0x7f) ; 	// Valid char
		else if (name[i] == '\0') break; 			// End of string
		else{i=0;break;	}							//Invalid char}	
	}
	if (i==0 || i== MAX_BT_NAME_LEN) 
	{
		memcpy(name,BT_DEFAULT_NAME,MAX_BT_NAME_LEN);		// Invalid entry
		WriteNvm((DEVICE_NAME_NVM), name, MAX_BT_NAME_LEN);	// Write default name
	}
	else ; // Valid name read out of memory
	
	// Try to recover the nvm pin code if nvm enabled
	ReadNvm((DEVICE_PAIRING_CODE), pin, 5);
	for(i=0;i<5;i++)
	{
		if (pin[i] >= '0' && pin[i] <= '9');// Valid char
		else if (pin[i] == '\0') break; 	// End of string
		else{i=0;break;	}					//Invalid char}	
	}
	if (i!=4) 
	{
		memcpy(pin,BT_DEFAULT_PIN,5);							// Invalid entry
		WriteNvm((DEVICE_PAIRING_CODE), pin, 5);// Write default
	}
	else ; // Valid name read out of memory

	// Read MAC
	ReadNvm(DEVICE_MAC_ADDRESS,mac, 7);
}

unsigned char FindLinkKeyForBD(linkKey_t* pktBuff) 	// Returns true and copies link key to end of pkt if found
{
	int i;
	linkKey_t buffer[NUMBER_OF_SAVED_KEYS]; // Stack variable, temporary
	unsigned long nvmAddress = LINK_KEY_ADDRESS;

	// Load link key db to ram from rom
	ReadNvm(nvmAddress, buffer, sizeof(linkKey_t)*NUMBER_OF_SAVED_KEYS);

	// Search for matching address
	for(i=0;i<NUMBER_OF_SAVED_KEYS;i++)
	{
		if ((pktBuff->bd_add[0] ==  buffer[i].bd_add[0])&&
			(pktBuff->bd_add[1] ==  buffer[i].bd_add[1])&&
			(pktBuff->bd_add[2] ==  buffer[i].bd_add[2])&&
			(pktBuff->bd_add[3] ==  buffer[i].bd_add[3])&&
			(pktBuff->bd_add[4] ==  buffer[i].bd_add[4])&&
			(pktBuff->bd_add[5] ==  buffer[i].bd_add[5]))
		{
			// Found it - copy out
			memcpy(pktBuff->link_key, buffer[i].link_key ,BD_LINK_KEY_SIZE);
			return TRUE_;
		}
	}
	// Return
	return FALSE_;
}

unsigned char StoreLinkKey(linkKey_t* newKey)		// Stores key to nvm
{
	linkKey_t buffer[NUMBER_OF_SAVED_KEYS]; // Stack variable, temporary
	unsigned long nvmAddress = LINK_KEY_ADDRESS;

	// Load link key db to ram from rom
	ReadNvm(nvmAddress, buffer, sizeof(linkKey_t)*NUMBER_OF_SAVED_KEYS);

	// Replace a key in the ram buffer
	memmove(&buffer[1], &buffer[0], sizeof(linkKey_t)*(NUMBER_OF_SAVED_KEYS-1));
	memcpy(&buffer[0], newKey, sizeof(linkKey_t));

	// Write back to nvm
	WriteNvm(nvmAddress, buffer, sizeof(linkKey_t)*NUMBER_OF_SAVED_KEYS);

	// Return
	return TRUE_;
}

void SaveBTMAC(void)
{
	if(device_MAC_local[6] != 1)
	{
		device_MAC_local[6] = 1;
		WriteNvm(DEVICE_MAC_ADDRESS, device_MAC_local, 6);
	}
}

const unsigned char cc256x_init_script[] = {0x01,0x03,0x0c,0x00}; // Initialise this region (example is reset command, count of 1)
const unsigned long cc256x_init_script_size = sizeof(cc256x_init_script);


