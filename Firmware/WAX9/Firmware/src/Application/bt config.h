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

// BT Configuration and definitions
// Karim Ladha, 2013-2014

// Create locally

#ifndef _BT_CONFIG_
#define _BT_CONFIG_

// DEBUG
//#define BT_DEBUG_ON 	// Turns on functions in Debug.c and sets up debug for selected files
//#define BT_USB_DEBUG	// Enables USB CDC specific debug channel requirements
//extern volatile unsigned char outBuffer[];// To peek into cdc debug out on break
//extern volatile unsigned short outHead, outTail;

// Chipset support
#define BT_CHIPSET_CC2564					//add support for this chipset
#define INIT_SCRIPT_PACKETS_2564	143 	//for cc2564_init_script_v2.10.c 18,321 bytes
#define BT_CHIPSET_CC2560					//add support for this chipset
#define INIT_SCRIPT_PACKETS_2560	300 	//for cc2560_init_script_v1.0.c 39,172 bytes

// Setup controller mode, 
#define BT_HOST_MODE 2						// Off(-1),BR(0),LE(1) or BR+LE(2)
#define ENABLE_LE_MODE						// If chipset reports no LE, device defaults to BR only
#define BT_SIMULTANEOUS_LE_BR_DISABLED 		// Turns off LE when BR connection made (if not connected)

// INIT SCRIPT
#define INIT_SCRIPT_IN_PROG_SPACE 		// For linker placed script OR mapped eeprom
//#define INIT_SCRIPT_IN_EXT_EEPROM		// For linker in unmapped eeprom (remove EEPROM_START_ADDRESS below)

// NVM SETTINGS
#ifndef EEPROM_START_ADDRESS	// Must match with HW profile too
#define EEPROM_START_ADDRESS 	0x0100000UL
#endif
// SETTINGS ADDRESS IS (EEPROM_START_ADDRESS + 0)				// Upto 256
#define DEVICE_NAME_NVM			(EEPROM_START_ADDRESS + 0x100)	// Upto 128
#define DEVICE_PAIRING_CODE		(EEPROM_START_ADDRESS + 0x180)	// Upto 64 
#define DEVICE_MAC_ADDRESS		(EEPROM_START_ADDRESS + 0x1C0) 	// Upto 64
#define LINK_KEY_ADDRESS 		(EEPROM_START_ADDRESS + 0x200)	// Upto 256 (8 keys)
#define LE_BONDING_DATA			(EEPROM_START_ADDRESS + 0x300)	// Upto 256 /*TODO*/
// SELF TEST RESULTS IS (EEPROM_START_ADDRESS + 400)			// Upto 1024
#define INIT_SCRIPT_CC2564 		(EEPROM_START_ADDRESS + 0x800) 	// Start of cc2564 init script
#define INIT_SCRIPT_CC2560 		(EEPROM_START_ADDRESS + 0x5000) // Start of cc2560 init script
#define INIT_SCRIPT_NULL		(EEPROM_START_ADDRESS + 0xF000) // Start of empty init script

// NUMBER OF LINK KEYS TO STORE 
#define NUMBER_OF_SAVED_KEYS 	3 // Link keys are 32 bytes

// DEVICE DISCOVERY
#define BT_DEFAULT_NAME				"WAX9-"
#define MAX_BT_NAME_LEN				32
#define BT_DEFAULT_PIN 				"0000"
#define BT_DEFAULT_COD 				0x080704ul
#define USE_T1_FOR_TICK

// HAL
#define EHCILL_SETTING 		1 				// 1 or 0 for on or off (always on for low power)

// HCI
#define MAX_CMD_PACKET_LENGTH	35			// Outgoing
#define MAX_EVENT_PACKET_LENGTH	35			// Incoming
#define MAX_OUT_ACL_LENGTH		(512+13) 	// Outgoing RFCOMM_PACKET + 5B rfcomm + 4B l2cap + 4B acl
#define MAX_IN_ACL_LENGTH 		(512+13)	// Incomming
#define HCI_CONNECTION_TIMEOUT	60			// Seconds, the radio should inform us it has lost contact before this

// ATT
#define ATT_IND_TIMEOUT		10 		// In seconds, time before we give up on waiting for indication resp
#define ATT_USE_BLE_SERIAL			// Creates rfcomm like serial port
#define LE_ADD_CR_TO_CMDS			// Each packet to serial port input handle is CR teminated

// L2CAP
#define MAX_L2CAP_CONNECTIONS	2	// 2 min if SDP and RFCOMM active together
#define L2CAP_PRE_EMPT_CONFIG_RESP	// Send our config request first
#define L2CAP_CONNECTION_TIMEOUT 30 // In seconds, max inactivity before checking connection is ok using echo request

// RFCOMM
#define RFCOMM_NUM_CHANNELS 		2 	// Should be 2
#define RFCOMM_CH_MUX 0x00			// Multiplexer/Control channel - hard coded
#define RFCOMM_CH_DATA 0x01			// Data channel number - hard coded


// USEFUL MACROS
#define BT_WRITELE16(ptr, val)		{*(unsigned char*)ptr = (unsigned char)val; *(unsigned char*)(ptr+1) = ((unsigned short)val>>8);}
#define BT_READLE16(ptr)			((((unsigned short)(*(unsigned char*)(ptr+1)))<<8) + *(unsigned char*)ptr)

#define BT_WRITEBE16(ptr, val)		{*(unsigned char*)ptr = ((unsigned short)val>>8); *(unsigned char*)(ptr+1) = (unsigned char)val;}
#define BT_READBE16(ptr)			((((unsigned short)(*(unsigned char*)ptr))<<8) + *(unsigned char*)(ptr+1))

#define BT_READBE32(ptr)			((((unsigned long)(*(unsigned char*)ptr))<<24)+\
									(((unsigned long)(*(unsigned char*)(ptr+1)))<<16)+\
									(((unsigned long)(*(unsigned char*)(ptr+2)))<<8)+\
									(((unsigned long)(*(unsigned char*)(ptr+3)))))
#define BT_WRITEBE32(ptr,val)		{*(unsigned char*)ptr = ((unsigned char)((unsigned long)val>>24));\
									*(unsigned char*)(ptr+1) = ((unsigned char)((unsigned long)val>>16));\
									*(unsigned char*)(ptr+2) = ((unsigned char)((unsigned long)val>>8));\
									*(unsigned char*)(ptr+3) = ((unsigned char)((unsigned long)val));}

// TRUE/FALSE definition excluding generic type
#ifndef FALSE_
	#define FALSE_ 0
#endif
#ifndef TRUE_
	#define TRUE_ 1
#endif

// Signalling from all layers to app
extern void BTEvent(unsigned short event);
// List of valid events
#define BT_EVENT_TURN_OFF			1
#define BT_EVENT_STARTING			2
#define BT_EVENT_FAILED				3
#define BT_EVENT_INITIALISED		4	
#define BT_EVENT_CONNECTED			5
#define BT_EVENT_DISCONNECTED		6
#define BT_EVENT_CONNECTION_REQ		7
#define BT_EVENT_COMM_OPEN			8
#define BT_EVENT_COMM_CLOSED		9
#define BT_EVENT_RADIO_ASLEEP		10
#define BT_EVENT_RADIO_AWAKE		11

#endif
