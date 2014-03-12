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

// Bluetooth HCI Layer Definitions + LE
// Karim Ladha, 2013-2014

#ifndef _HCI_H_
#define _HCI_H_

// Types
typedef struct {
	signed char 	connectionMode; // Off(-1),BR(0),LE(1) or BR+LE(2)
	unsigned char 	isInitialised;
	unsigned char 	isConfigured;
	unsigned char 	isConnected;
	unsigned char 	connectionType;
	unsigned char 	remoteADDR[6];
	unsigned short 	connHandle;
	unsigned short 	packetsToAck;
	unsigned short 	hostAclBufferSize;
	unsigned short 	hostNumAclBuffers;
	unsigned short 	controllerLeBufferSize;
	unsigned short 	controllerNumLeBuffers;
	unsigned short	controllerAclBufferSize;
	unsigned short	controllerNumAclBuffers;
	unsigned short 	connectionTimeout;
} hci_connection_t;

#define HCI_CONN_TYPE_BR	1
#define HCI_CONN_TYPE_LE	2

// Globals
extern hci_connection_t hci_connection;

// Prototypes

// Public - from upper levels
unsigned char 	HciInit					(void);
void  			HciInstallL2CAP 		(void(*CallBack)(const unsigned char * data, unsigned short len));
void 			HciDisconnect			(void);
unsigned short 	HciGetAclMtu			(void);
unsigned char 	HciAclCanSend			(void);
unsigned char* 	HciGetAclBuffer 		(void);
unsigned char   HciSendAcl				(const unsigned char* data, unsigned short len);
void 			HciTimeoutHandler		(void );

// DEFINITIONS

// Packet lengths
#define HCI_EVENT_HDR_LEN 	2
#define HCI_ACL_HDR_LEN 	4
#define HCI_CMD_HDR_LEN 	3

// ACL packet flags
#define ACL_PACKET_FLAGS_COMPLETE		0x0000
#define ACL_PACKET_FLAGS_START 			0x1000
#define ACL_PACKET_FLAGS_END			0x2000
#define ACL_PACKET_FLAGS_CONTINUATION	0x3000
#define ACL_PACKET_FLAGS_NOBROADCAST	0x0000

// Event codes
#define HCI_CONNECTION_COMPLETE 		0x03
#define HCI_CONNECTION_REQUEST 			0x04
#define HCI_DISCONNECTION_COMPLETE 		0x05
#define HCI_COMMAND_COMPLETE 			0x0E
#define HCI_COMMAND_STATUS 				0x0F
#define HCI_NBR_OF_COMPLETED_PACKETS 	0x13
#define HCI_RETURN_LINK_KEYS 			0x15
#define HCI_PIN_CODE_REQUEST 			0x16
#define HCI_LINK_KEY_REQUEST 			0x17
#define HCI_LINK_KEY_NOTIFICATION 		0x18
#define HCI_DATA_BUFFER_OVERFLOW		0x1A 
#define HCI_HW_ERROR_EVENT				0x10
#define HCI_HW_MODE_CHANGE_EVENT		0x20
#define HCI_HW_NEW_MAX_SLOTS_EVENT		0x1B
#define HCI_HW_ENCRYPTION_CHANGED		0x08

// BLE events
#define HCI_LE_META											0x3E
#define HCI_SUBEVENT_LE_CONNECTION_COMPLETE					0x01
#define HCI_SUBEVENT_LE_ADVERTISING_REPORT					0x02
#define HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE			0x03
#define HCI_SUBEVENT_LE_READ_REMOTE_USED_FEATURES_COMPLETE	0x04
#define HCI_SUBEVENT_LE_LONG_TERM_KEY_REQUEST				0x05

// Pre-shifter OGFs
#define HCI_LINK_CTRL_OGF 	((unsigned short)0x01<<10)	/*0x0400*/
#define HCI_LINK_POLICY_OGF ((unsigned short)0x02<<10)	/*0x0800*/
#define HCI_HC_BB_OGF 		((unsigned short)0x03<<10)	/*0x0C00*/
#define HCI_INFO_PARAM_OGF 	((unsigned short)0x04<<10)	/*0x1000*/	
#define HCI_LE_OGF			((unsigned short)0x08<<10) 	/*0x2000*/

/*Command OCF*/
#define HCI_DISCONNECT 					(0x06|HCI_LINK_CTRL_OGF)
#define HCI_ACCEPT_CONN_REQ 			(0x09|HCI_LINK_CTRL_OGF)
#define HCI_REJECT_CONN_REQ 			(0x0A|HCI_LINK_CTRL_OGF)
#define HCI_PIN_CODE_REQ_REP 			(0x0D|HCI_LINK_CTRL_OGF)
#define HCI_LINK_KEY_REQ_REP 			(0x0B|HCI_LINK_CTRL_OGF)
#define HCI_LINK_KEY_REQ_REP_NEGATIVE 	(0x0C|HCI_LINK_CTRL_OGF)
#define HCI_RESET 						(0x03|HCI_HC_BB_OGF)
#define HCI_W_SCAN_EN 					(0x1A|HCI_HC_BB_OGF)
#define HCI_W_COD 						(0x24|HCI_HC_BB_OGF)
#define HCI_H_BUF_SIZE 					(0x33|HCI_HC_BB_OGF)
#define HCI_W_LOCAL_NAME 				(0x13|HCI_HC_BB_OGF)
#define HCI_SET_EVT_MASK				(0x01|HCI_HC_BB_OGF)
#define HCI_R_BUF_SIZE 					(0x05|HCI_INFO_PARAM_OGF)
#define HCI_R_BD_ADDR 					(0x09|HCI_INFO_PARAM_OGF)
#define HCI_SET_PAGE_TIMEOUT			(0x18|HCI_HC_BB_OGF)


#define HCI_DISCONNECT_LEN				6	// Handle2,reason 0x13
#define HCI_ACCEPT_CONN_REQ_LEN 		10	// BD add6, role
#define HCI_REJECT_CONN_REQ_LEN			10 	// BD add6, reason 0x0D for limited resources
#define HCI_RESET_LEN 					3
#define HCI_W_SCAN_EN_LEN 				4	// 1 or 0
#define HCI_W_COD_LEN 					6	// COD [3] returned
#define HCI_H_BUF_SIZE_LEN 				10 	// Host ACL_LEN2,HCI_LEN1,ACL_NUM2,HCI_NUM2
#define HCI_R_BUF_SIZE_LEN 				3	// Device ACL_LEN2,HCI_LEN1,ACL_NUM2,HCI_NUM2
#define HCI_R_BD_ADDR_LEN 				3
#define HCI_W_LOCAL_NAME_LEN 			err!// Header2,cmdlen1,string
#define HCI_SET_PAGE_TIMEOUT_LEN		5	// Time x0.625ms
#define HCI_PIN_CODE_REQ_REP_LEN 		14	// BD add6, PinLen1, pinN=4 - assumes 4 chars!	
#define HCI_LINK_KEY_REQ_REP_LEN 		25	// BD add6, lk 16 
#define HCI_LINK_KEY_REQ_REP_NEGATIVE_LEN 9	// BD add6
#define HCI_SET_EVT_MASK_LEN			11

// BLE controller support 
#define HCI_WRITE_LE_HOST_SUPPORTED 		(0x6d|HCI_HC_BB_OGF)
#define HCI_WRITE_LE_HOST_SUPPORTED_LEN 	(HCI_CMD_HDR_LEN + 2)  	// 1,1 for ble on AND simultaneous 
#define HCI_READ_LE_HOST_SUPPORTED			(0x6c|HCI_HC_BB_OGF)		
#define HCI_READ_LE_HOST_SUPPORTED_LEN		(HCI_CMD_HDR_LEN + 2)	// ret: status and 1,1 for ble on AND simultaneous 					

/*LE OCF and Lengths*/
#define HCI_LE_SET_EVENT_MASK		(0x01|HCI_LE_OGF)		// param: set event mask 
#define HCI_LE_SET_EVENT_MASK_LEN	(HCI_CMD_HDR_LEN + 8) 	// ret: stat

#define HCI_LE_READ_BUFFER_SIZE		(0x02|HCI_LE_OGF)		// param:
#define HCI_LE_READ_BUFFER_SIZE_LEN	HCI_CMD_HDR_LEN 		// ret: stat, acl pkt len 2, num pkts 1

#define HCI_LE_READ_SUPPORTED_FEATURES	(0x03|HCI_LE_OGF)	// param:
#define HCI_LE_READ_SUPPORTED_FEATURES_LEN	HCI_CMD_HDR_LEN // ret: features 8, b0 0/1 Encryption available 

// OCF 0x04 missing from spec

#define HCI_LE_SET_RANDOM_ADDRESS	(0x05|HCI_LE_OGF)		// param: address (6)
#define HCI_LE_SET_RANDOM_ADDRESS_LEN (HCI_CMD_HDR_LEN + 6) // ret: stat

#define HCI_LE_SET_ADVERTISING_PARAMS (0x06|HCI_LE_OGF)				// param: see below
#define HCI_LE_SET_ADVERTISING_PARAMS_LEN (HCI_CMD_HDR_LEN + 15)   	// ret: stat
/* params:
Advertising_Interval_Min,2,Nx0.625ms (default 0x0800)
Advertising_Interval_Max,2,Nx0.625ms (default 0x0800)
Advertising_Type,1,		(ADV_IND 0)(ADV_DIRECT_IND 1)(ADV_SCAN_IND 2)(ADV_NONCONN_IND 3)
Own_Address_Type,1, 	0x00 Public Device Address (default),0x01 Random Device Address
Direct_Address_Type,1, 	0x00 Public Device Address (default),0x01 Random Device Address
Direct_Address,6,		Address
Advertising_Channel_Map,1, 0x7 = all advert channels 
Advertising_Filter_Policy,1,0x00 Allow Scan Request from Any, Allow Connect Request from Any (default).
							0x01 Allow Scan Request from White List Only, Allow Connect Request from Any.
							0x02 Allow Scan Request from Any, Allow Connect Request from White List Only.
							0x03 Allow Scan Request
*/

#define HCI_LE_READ_ADVERTISING_TX_PWR	(0x07|HCI_LE_OGF)		// param:
#define HCI_LE_READ_ADVERTISING_TX_PWR_LEN (HCI_CMD_HDR_LEN) 	// ret: stat, (1) power db signed 
	
#define HCI_LE_SET_ADVERT_DATA			(0x08|HCI_LE_OGF)		// param: len(1), data(31) - see spec for details
#define HCI_LE_SET_ADVERT_DATA_LEN		(HCI_CMD_HDR_LEN+32)	// ret: stat

#define HCI_LE_SET_SCAN_RESP_DATA		(0x09|HCI_LE_OGF)		// param: len(1), data(31) - see spec for details
#define HCI_LE_SET_SCAN_RESP_DATA_LEN	(HCI_CMD_HDR_LEN+32)	// ret: stat

#define HCI_LE_SET_ADVERT_ENABLE		(0x0A|HCI_LE_OGF)		// param: on or off (1)
#define HCI_LE_SET_ADVERT_ENABLE_LEN 	(HCI_CMD_HDR_LEN+1)		// ret: stat

#define HCI_LE_READ_SUPPORTED_STATES	(0x1C|HCI_LE_OGF)		//param:
#define HCI_LE_READ_SUPPORTED_STATES_LEN (HCI_CMD_HDR_LEN) 		//ret: stat, states(8) mask for - state combinations supported

#define HCI_LE_SET_SCAN_PARAMS	(0x0B|HCI_LE_OGF)				// param: see below
#define HCI_LE_SET_SCAN_PARAMS_LEN		(HCI_CMD_HDR_LEN+7)		//ret: stat
/*
LE_Scan_Type,1, passive or active(requests sent)
LE_Scan_Interval,2,scan interval x0.625ms, default 0x0010 10ms
LE_Scan_Window,2,scan window x0.625ms, default 0x0010 10ms
Own_Address_Type,1,own address 0 = public
Scanning_Filter_Policy,1,accept all adverts = 0
*/

#define HCI_LE_SCAN_ENABLE (0x0C|HCI_LE_OGF)		// param: on or off (1), filter duplicates on or off(1)
#define HCI_LE_SCAN_ENABLE_LEN	(HCI_CMD_HDR_LEN+2)	//ret: stat

#define HCI_LE_CREATE_CONNECTION	(0x0D|HCI_LE_OGF)		// param: see below
#define HCI_LE_CREATE_CONNECTION_LEN (HCI_CMD_HDR_LEN+25)	//ret: ?, probably pending like with normal stack?
/*
LE_Scan_Interval,2,x0.625ms
LE_Scan_Window,2,x0.625ms
Initiator_Filter_Policy,1,filter policy 0 = no white list
Peer_Address_Type,1, peer address type 0 = public
Peer_Address,6,peer address
Own_Address_Type,1,own address type 0 = public
Conn_Interval_Min,2,x0.625ms
Conn_Interval_Max,2,x0.625ms
Conn_Latency,2, latency in connection events of slave
Supervision_Timeout,2,x10ms, 0.1 - 32s
Minimum_CE_Length,2,x0.625ms
Maximum_CE_Length,2,x0.625ms
*/

#define HCI_LE_CREATE_CONNECTION_CANCEL			(0x0E|HCI_LE_OGF)	// param:
#define HCI_LE_CREATE_CONNECTION_CANCEL_LEN		(HCI_CMD_HDR_LEN)	//ret: stat

#define HCI_LE_CONNECTION_UPDATE	(0x13|HCI_LE_OGF)		// param:
#define HCI_LE_CONNECTION_UPDATE_LEN (HCI_CMD_HDR_LEN+14)	//ret: stat
/*
Connection_Handle,2,
Conn_Interval_Min,2,x1.25ms
Conn_Interval_Max,2,x1.25ms
Conn_Latency,2,in connection events
Supervision_Timeout,2,x10ms,0.1-32s
Minimum_CE_Length,2,x0.625ms - min conn time
Maximum_CE_Length,2,x0.625ms - mac conn time
*/

#define HCI_LE_READ_REMOTE_FEATURES		(0x16|HCI_LE_OGF)	// param: conn handle(2)
#define HCI_LE_READ_REMOTE_FEATURES_LEN (HCI_CMD_HDR_LEN+2)	//ret: stat, LE Read Remote Used Features Complete event

#define HCI_LE_ENCRYPT		(0x17|HCI_LE_OGF)				// param: key(16),data(16)
#define HCI_LE_ENCRYPT_LEN	(HCI_CMD_HDR_LEN+16+16)			//ret: stat, encrypted data(16)

#define HCI_LE_RAND			(0x18|HCI_LE_OGF)				// param: 
#define HCI_LE_RAND_LEN		(HCI_CMD_HDR_LEN)				// ret: stat, rand(8)

#define HCI_LE_START_ENCRYPTION		(0x19|HCI_LE_OGF)		// param:
#define HCI_LE_START_ENCRYPTION_LEN	(HCI_CMD_HDR_LEN+28)	// ret: stat
/*
Connection_Handle,2,
Random_Number,8,
Encrypted_Diversifier,2,
Long_Term_Key,16,
*/

#define HCI_LE_LTK_REPLY	(0x1a|HCI_LE_OGF)			// param:connection handle, ltk
#define HCI_LE_LTK_REPLY_LEN (HCI_CMD_HDR_LEN+18)		// ret: stat, connection handle

#define HCI_LE_LTK_NEG_REPLY	(0x1b|HCI_LE_OGF)		// param:connection handle
#define HCI_LE_LTK_NEG_REPLY_LEN (HCI_CMD_HDR_LEN+2)	// ret: stat, connection handle

#endif 

