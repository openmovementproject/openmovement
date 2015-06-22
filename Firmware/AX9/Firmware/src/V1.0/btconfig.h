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

// INCLUDE
#include "bt-utils.h"	// Required throughout

// DEBUG
#define BT_DEBUG_ON 	// Turns on functions in Debug.c and sets up debug for selected files
// DEFINITIONS
#ifdef BT_DEBUG_ON
#define BT_USE_SLOW_UART	// Useful for finding race conditions
	#define DEBUG_PROFILE		0	/* Debug device specific module */
	#define DEBUG_APP 			3	/* Debug application */
	#define DEBUG_RFCOMM 		0	/* Debug rfcomm */
	#define DEBUG_SDP 			4	/* Debug service discovery */
	#define DEBUG_L2CAP 		0	/* Debug l2cap */
	#define DEBUG_HCI 			3	/* Debug host controller interface */
	#define DEBUG_CONNECT		3	/* Debug device discovery/connection */
	#define DEBUG_BLE			0	/* Debug low energy layers */
	#define DEBUG_PHY 			3	/* Debug physical layer, required >=3 for below sub sets */
	#define DEBUG_PHY_EHCILL	0	/* Debug ehcill signals */
	#define DEBUG_PHY_DUMP 		1	/* Debug dump all phy coms */
	#define BT_USB_DEBUG			// Enables USB CDC specific debug channel requirements
	extern volatile unsigned char outBuffer[];	// To peek into cdc debug out on break (useful to see unprinted debug msgs)
	extern volatile unsigned short outHead, outTail;
	#define BT_HOST_MODE_DEBUG	HCI_MODE_OFF	// Over ride the host mode in debug
#endif

// Chipset support
#define BT_CHIPSET_CC2564					//add support for this chipset
#define INIT_SCRIPT_PACKETS_2564	143 	//for cc2564_init_script_v2.10.c 18,321 bytes
//#define BT_CHIPSET_CC2560					//add support for this chipset
//#define INIT_SCRIPT_PACKETS_2560	300 	//for cc2560_init_script_v1.0.c 39,172 bytes

// Setup controller initial mode 
#define BT_HOST_MODE 	HCI_MODE_BRLE_DISC	// See enum in hci.h - used at startup
#define BT_SIMULTANEOUS_LE_BR_DISABLED 		// Makes BR/EDR and LE controllers look like two devices (TODO: Allow co-existence)
//#define DISABLE_LE_MODES					// If chipset reports no LE, device defaults to BR only, this stops code generation

// INIT SCRIPT
#define INIT_SCRIPT_IN_PROG_SPACE 		// For linker placed script OR mapped eeprom
//#define INIT_SCRIPT_IN_EXT_EEPROM		// For linker in unmapped eeprom (remove BLUETOOTH_NVM_START below)

// NVM SETTINGS/DATA
#define DEVICE_NAME_NVM			(BLUETOOTH_NVM_START)			// Upto 128
#define DEVICE_PAIRING_CODE		(BLUETOOTH_NVM_START + 0x080)	// Upto 64 
#define DEVICE_MAC_ADDRESS		(BLUETOOTH_NVM_START + 0x0C0) 	// Upto 64
#define LINK_KEY_ADDRESS 		(BLUETOOTH_NVM_START + 0x100)	// Upto 256 (upto 8 keys)
#define LE_BONDING_DATA			(BLUETOOTH_NVM_START + 0x200)	// Upto 512 /*TODO*/
#define INIT_SCRIPT_CC2564 		(BLUETOOTH_NVM_START + 0x400) 	// Start of cc2564 init script (18,321 bytes ~ 18*1024=0x4800)
#define INIT_SCRIPT_NULL		(BLUETOOTH_NVM_START + 0x4BF8) 	// Start of empty init script (4 bytes)
// TOTAL NVM = 0x4C00 (19.5kB)

// NUMBER OF LINK KEYS TO STORE 
#define NUMBER_OF_SAVED_KEYS 	3 	// Link keys are 32 bytes, make sure there is space in nvm

// DEVICE DISCOVERY
#define MAX_BT_NAME_LEN			32	// Used by nvm when configuring device name

// CONSTANTS
#define BT_MAC_ADD_LEN			6	// MAC length, for disambiguation only 
#define BR_LINK_KEY_LEN			16	// Part of the BT specification 

// CHIPSET HAL
#define EHCILL_SETTING 			1 	// 1 or 0 for on or off (always on for low power)
#define PHY_EVENT_BUFF_OVERLAP		// Non discovery events are small, save 200b ram but con+disc disallowed
#define PHY_COMMAND_BUFF_OVERLAP	// Most commands are small after initialisation, overlap saves memory

// LINK LAYER
#define BT_NUM_CONNECTIONS		1			// Supports multiple connections

// HCI
#define MAX_CMD_PACKET_LENGTH	35			// Outgoing (largest cmd is extended inq. resp)
#define MAX_EVENT_PACKET_LENGTH	35			// Incoming
#define MAX_OUT_ACL_LENGTH		(512+13) 	// Outgoing RFCOMM_PACKET + 5B rfcomm + 4B l2cap + 4B acl
#define MAX_IN_ACL_LENGTH 		(512+13)	// Incomming
#define HCI_CONNECTION_TIMEOUT	60			// Seconds, the radio should inform us it has lost contact before this
#define DEFAULT_LMP_POLICY 		0x0007		// Allow lmp to use role switch, hold and sniff (no park)
#define BT_BR_EIR_TX_PWR 		10			// Use 10dBm discovery power (max range)
#define ENABLE_HCI_CONNECT					// Enable the connection creation features

// L2CAP
#define	NUM_L2CAP_CONNECTIONS	BT_NUM_CONNECTIONS	// Should be same as physical connection count
#define L2CAP_MAX_NUM_PSM		3			// Number of supplorted protocols (typically 2; rfcomm and sdp)
#define L2CAP_MAX_FIX_CHAN		2			// The number of fixed channel supported (excluding channel 1, signal chan)
#define NUM_L2CAP_CHANNELS		2			// Number of configured channels, a connection uses a channel per connected protocol (sdp, rfcomm etc.)
#define L2CAP_PRE_EMPT_CONFIG_RESP			// Send our channel config request without waiting for the remotes
#define L2CAP_CONNECTION_TIMEOUT 30 		// In seconds, max inactivity before checking connection is ok using echo request

// ATT
#define ATT_NUM_CONNECTIONS		BT_NUM_CONNECTIONS	// Number of simultaneous gatt requests
#define ATT_IND_TIMEOUT			10 	// In seconds, time before we give up on waiting for indication resp

// SMP
#define SMP_NUM_CONNECTIONS		BT_NUM_CONNECTIONS	// Number of simultaneous smp requests

// RFCOMM
#define NUM_RFCOMM_CONNECTIONS	BT_NUM_CONNECTIONS	// Number of simultaneous rfcomm requests
#define RFCOMM_NUM_CHANNELS 	2 					// Should be 1 + num protocol channels, ch.0 is MUX
#define RFCOMM_SERIAL_CHANNEL 	0x01				// Data channel number - hard coded

// SDP
#define NUM_SDP_CONNECTIONS 	BT_NUM_CONNECTIONS	// Number of simultaneous sdp requests

// MAIN APPLICATION
#define MIN_ACL_BUFFS_FOR_DATA_OUT	1	// Prevent data sources filling up all the acl slots and blocking critical processes
#define MAX_NUM_APP_TASKS			4	// Max number of task callbacks, tasks are called each event to check for outgoing data etc.
#define MAX_NUM_APP_TIMEOUTS		4	// Max number of timeout callbacks, timeouts are called each second to check for lockups etc.

#endif
