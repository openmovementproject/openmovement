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

// EEPROM variable location
// Karim Ladha, 2013-2014

// Specific to the WAX9

#include <string.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "bt nvm.h"
#include "bt config.h"

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
	char *pin = GetBTPin();
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
	
	// Append id/mac
	// Get remaining bytes in name using id and mac
	ReadNvm(DEVICE_MAC_ADDRESS,mac, 7);
	unsigned short len = strlen(name); 
	if(settings.deviceId == 0)
	{
		;// An id of zero prevents appending of if to name
	}
	else if (settings.deviceId == 0xffff)
	{
		// For an id of 0xffff we use the mac address bytes
		if ((MAX_BT_NAME_LEN-len) > 4) // If theres space to append 4 chars to the name
		{
			char *ptr = &mac[1];
			*(name+len++) = (((*ptr>>4)&0x000f)<0x0A)?('0'+((*ptr>>4)&0x000f)):('A'-0xA+((*ptr>>4)&0x000f));
			*(name+len++) = (((*ptr>>0)&0x000f)<0x0A)?('0'+((*ptr>>0)&0x000f)):('A'-0xA+((*ptr>>0)&0x000f));
			ptr = &mac[0];
			*(name+len++) = (((*ptr>>4)&0x000f)<0x0A)?('0'+((*ptr>>4)&0x000f)):('A'-0xA+((*ptr>>4)&0x000f));
			*(name+len++) = (((*ptr>>0)&0x000f)<0x0A)?('0'+((*ptr>>0)&0x000f)):('A'-0xA+((*ptr>>0)&0x000f));
			*(name+len++) = '\0';
		}	
	}
	else 
	{
		// For any other device id, write the decimal out (if space)
		if ((MAX_BT_NAME_LEN-len) > 5) // If theres space to append a number to the name
		{
			*(name+len++) = '0' + (settings.deviceId/10000)%10;
			*(name+len++) = '0' + (settings.deviceId/1000)%10;
			*(name+len++) = '0' + (settings.deviceId/100)%10;
			*(name+len++) = '0' + (settings.deviceId/10)%10;
			*(name+len++) = '0' +  settings.deviceId%10;
			*(name+len++) = '\0';
		}	
	}


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
		WriteNvm(DEVICE_MAC_ADDRESS, device_MAC_local, 7);
	}
}


// Device name
__prog__ const char __attribute__((space(prog),address((DEVICE_NAME_NVM)))) EepromDeviceName[] = 
	"WAX9-";

// Pairing code
__prog__ const char __attribute__((space(prog),address((DEVICE_PAIRING_CODE)))) EepromPairingCode[] = 
	"0000";

// Mac address
__prog__ const char  __attribute__((space(prog),address((DEVICE_MAC_ADDRESS)))) EepromMacAddress[] = 
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// Link keys
#define MAX_KEY_SIZE 32 // Link keys are 26 bytes
__prog__ const char  __attribute__((space(prog),address((LINK_KEY_ADDRESS)))) EepromLinkKeyDB[NUMBER_OF_SAVED_KEYS][MAX_KEY_SIZE] =
	{{0}};

// Init scripts
#if defined(BT_CHIPSET_CC2564)
__prog__ const char __attribute__((space(prog),address((INIT_SCRIPT_CC2564))))
cc2564_init_script[] =
{

#error "For licensing reasons, the TI CC2564 initialization script is not included"

};

const unsigned long cc2564_init_script_size = sizeof(cc2564_init_script);
#endif

#ifdef BT_CHIPSET_CC2560

#error "For licensing reasons, the TI CC2560 initialization script (part 1) is not included"

};

__prog__ const char __attribute__((space(prog),address((INIT_SCRIPT_CC2560)+sizeof(cc2560_init_script))))
cc2560_init_script_2[] =
{

#error "For licensing reasons, the TI CC2560 initialization script (part 2) is not included"

};

const unsigned long cc2560_init_script_size = sizeof(cc2560_init_script) + sizeof(cc2560_init_script_2);
#endif

#if !defined(BT_CHIPSET_CC2560) && !defined(BT_CHIPSET_CC2564)
	#warning "There is an invalid linker script being created in the output .hex file!"
	__prog__ const char __attribute__((space(prog),address((INIT_SCRIPT_NULL))))
	null_init_script[] = {0x01,0x03,0x0c,0x00}; // Just to see how much space we actually are using for prog (its a reset cmd)
	const unsigned long null_init_script_size = sizeof(null_init_script);
#endif


