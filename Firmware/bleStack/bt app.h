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

// Bluetooth Application
// Karim Ladha, 2013-2014

#ifndef _BTAPP_H_
#define _BTAPP_H_

#include "bt config.h"
#include "Utils/fifo.h"

#ifdef ENABLE_LE_MODE
	#include "ble att.h" // For types
	// Low energy serial in/out - must be setup in profile. 
	extern attOutHandle_t bleSerialOut;
	extern unsigned char bleIndPending;
#endif

// Old flag variable now unused
#define btPresent (1)  

/* Types*/
typedef enum {
		BT_OFF,				/*0 BT Module is not powered*/
		BT_STARTING,		/*1 BT Module is powered but not initialised*/
		BT_FAIL,			/*2 BT Could not start*/
		BT_INITIALISED,		/*3 BT Module is initialised and discoverable*/
		BT_CONNECTED,		/*4 BT Module has an active connection*/
		BT_CONNECTION_REQUEST /*5 A device has tried to initiate a connection (service stack quickly)*/
		}bt_state_t;
typedef enum {
		BT_STANDBY,			/*0 Can sleep, BT Module is not powered*/
		BT_SLEEP,			/*1 Can sleep, Stack will allow sleep*/
		BT_IDLE,			/*2 Can idle, stack is using UART*/
		BT_ACTIVE			/*3 Can not idle, time critical tasks are in progress*/
		}bt_power_state_t;

// StateVars
extern unsigned short chipsetId;
extern bt_power_state_t bluetoothPowerState;
extern bt_state_t bluetoothState;
extern volatile unsigned char phyClockNeededDuringSleep; // New transport inspecific name for bleStack

// Serial fifos
extern fifo_t bt_out_fifo; 	// Use fifo api to add to in/out fifos
extern fifo_t bt_in_fifo; 	// Use fifo api to add to in/out fifos
extern volatile unsigned char* out_data_buffer_queued;
extern volatile unsigned short out_data_buffer_len_queued;

unsigned char BTInit(void);
void BTStart(void);
void BTDeinit(void);
char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice);
unsigned char RfcommSendData(const void* source, unsigned short num);// See mtu below
unsigned char RfcommTxBusy(void);
void BTClearDataBuffers(void);
void BTTasks(void); 
void BTEvent(unsigned short event); // Available to all layers, events are in btconfig

// External api prototypes for user
// Nvm
extern char* GetBTName(void);								
extern char* GetBTPin(void);								
extern char* GetBTMAC(void);							
extern unsigned long GetBTCOD(void);					
// Rfcomm
extern unsigned short RfcommMaxPayload(void);
// Physical transport control
extern void HciPhyPause(void);
extern void HciPhyPlay(void);
extern void HciPhyAdaptClock(void);	
extern void HciPhySafeSleep(void);
extern void EventTrigger(void);
#endif /*BTApp*/
