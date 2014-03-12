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

// Bluetooth L2CAP layer
// Karim Ladha, 2013-2014

#ifndef _L2CAP_H_
#define _L2CAP_H_

// Types
// Basic state
typedef enum {
CLOSED = 0,
CONFIGURING,
OPEN,						
WAIT_CLOSING
} l2cap_state_t;
// Configuration state
typedef enum {
CONFIG_NO_STATE	= 0,
CONFIG_REQ_TXED	= 1,
CONFIG_REQ_RXED = 2,
CONFIG_RESP_RXED = 3,	
CONFIG_RESP_TXED = 4
} config_state_t;
// Channel data - multiple
typedef struct 
{
	l2cap_state_t state;		// State sumary
	config_state_t configLocal; // Local to remote device
	config_state_t configRemote;// Remote to local
	unsigned short psm;
	unsigned short remoteCID;
	unsigned short remoteMTU;
	unsigned short localCID;
	unsigned char lastId;
} l2cap_channel_t;
// Connection data - one pre remote device
typedef struct {
	unsigned char 	isInitialised;
	unsigned char	lastSignalingId;
	unsigned short  connectionTimeout;
} l2cap_connection_t;

// Globals


// Prototypes
// Public
unsigned char 	L2capInit					(void);
void  			L2capInstallRfcomm 			(unsigned char (*CallBack)(const unsigned char * data, unsigned short len));
void  			L2capInstallSdp 			(unsigned char (*CallBack)(const unsigned char * data, unsigned short len));
void  			L2capInstallAtt 			(unsigned char (*CallBack)(const unsigned char * data, unsigned short len));

unsigned char 	L2capDisconnect				(void); 				// Diconnects all channels
unsigned short  L2capGetMtuForPsm			(unsigned short psm); 	// Get max payload size
unsigned char*	L2capGetSendBuffer			(void); 				// Assemble the packet in situ
unsigned char 	L2capSendCid 				(unsigned char* buffer, unsigned short len, unsigned short cid);
unsigned char 	L2capSendPsm 				(unsigned char* buffer, unsigned short len, unsigned char psm);
void			L2capTimeoutHandler			(void);

// Private
void 			L2capPacketHandler			(const unsigned char * data, unsigned short len);
void 			L2capHandleSignallingPacket	(const unsigned char* payload, unsigned short length);


// Definitions
#define L2CAP_HEADER_LEN		4
#define L2CAP_SIG_HEADER_LEN	4

// Channels
#define L2CAP_SIGNALLING_CID	0x01
#define L2CAP_LE_ATT_CID		0x04
#define L2CAP_LE_SIG_CID		0x05
#define L2CAP_LE_SMP_CID		0x06

// PSM values supported - other blocked
#define L2CAP_PSM_RFCOMM		0x0003
#define L2CAP_PSM_SDP			0x0001
#define L2CAP_PSM_ATT			0x001f // TODO: Create sdp entry for this
#define L2CAP_PSM_END_LIST		0xffff

#define L2CAP_PSM_SUPPORT_LIST	{L2CAP_PSM_SDP,L2CAP_PSM_RFCOMM,L2CAP_PSM_END_LIST}


		
// L2cap codes
#define L2CAP_ERROR 		0x01
#define L2CAP_CONN_REQ 		0x02
#define L2CAP_CONN_RSP 		0x03
#define L2CAP_CFG_REQ 		0x04
#define L2CAP_CFG_RSP 		0x05
#define L2CAP_DISCONN_REQ 	0x06
#define L2CAP_DISCONN_RSP 	0x07
#define L2CAP_ECHO_REQ		0x08
#define L2CAP_ECHO_RSP		0x09
#define L2CAP_INFO_REQ 		0x0A
#define L2CAP_INFO_RSP 		0x0B


#endif
