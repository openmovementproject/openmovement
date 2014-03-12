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

// Bluetooth physical layer hardware abstraction layer
// Karim Ladha, 2013-2014

#ifndef _PHY_HAL_H_
#define _PHY_HAL_H_

// DEFINITIONS
// EHCILL specific defines
#define HCILL_GO_TO_SLEEP_IND 	0x30
#define HCILL_GO_TO_SLEEP_ACK 	0x31
#define HCILL_WAKE_UP_IND 		0x32
#define HCILL_WAKE_UP_ACK 		0x33 
#define HCILL_CTS_SIGNAL		0x34

// BLOCK RECEIVED TYPEDS
#define HCI_CMD_BLOCK 			0x01
#define HCI_ACL_BLOCK 			0x02
#define HCI_EVENT_BLOCK 		0x04
#define HCI_HCILL_BLOCK 		0x30

// GLOBALS - NONE OF THESE ARE USER VARIABLES
extern unsigned char eventBuffer[]; 		// Incoming
extern unsigned char aclInDataBuffer[];		// Incoming

// Exposed tx/rx state for higher layers
extern volatile unsigned char 	phyClockNeededDuringSleep;
extern volatile unsigned char 	blockReceived;
extern volatile unsigned char 	blockSent;

// PROTOTYPES - NON USER
// Physical interface api
unsigned char	PhyPowerUp(void);		// Turn on and init (reset & init script)
void 			PhyPowerOff(void);		// Turn off
unsigned char 	PhySupportsLe(void);	// Return LE support	
void 			PhySendBlock(const unsigned char* data, unsigned short len, unsigned char header);
void 			PhySendWait(void);		// Wait for send to complete
void			PhyReceiveBlock(void);	// Enable recieve of a block (only call after last receive completes)
void 			EventTrigger(void);		// Trigger 
void 			PhyInstallEventHandlerCB(void(*CallBack)(void)); // Called on transfer events (and if triggered)

// UART specific
void 			UartPause(void);				// Safely pause uart comms 	
void 			UartPlay(void);					// Resume after pause
void 			UartRxIdle(void);				// Dump any incomming traffic 
void 			UartSafeSleep(void);			// System cpu to low power state depending on 
unsigned char 	UartSetBaud(unsigned long baud);// Specific timing required

// For debugging sleep states
// eHCILL states
typedef enum{
	RADIO_ASLEEP = 0,		// Asleep
	RADIO_AWAKE = 1,		// Awake

	CTS_WOKEN = 2,			// Woken by controller
	WAKE_IND_RXED = 3,		// Woken by controller
	WAKE_ACK_SENT = 1,		// Awake

	WAKE_IND_SENDING = 4,	// Woken by host - pic
	WAKE_IND_SENT = 5,		// Woken by host
	WAKE_ACK_RXED = 1,		// Awake

	GOTO_SLEEP_RXED = 6,	// Controller set sleep
	GOTO_SLEEP_ACK_SENT = 0 // Asleep
}echill_t;
extern echill_t ehcill_state; // Debug
#endif
// EOF
