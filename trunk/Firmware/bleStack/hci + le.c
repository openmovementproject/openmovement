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

#include <string.h>
#include "hci phy.h"	// Send, install
#include "hci + le.h"	// Definitions
#include "bt nvm.h"		// Device name etc.
#include "bt config.h"	// Settings
#include "debug.h"

#if DEBUG_HCI
	static const char* file = "hci+le.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "hci+le.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

// PRIVATE
// Private - from lower levels
static void HciPutEvent(const unsigned char *event, unsigned short len);
static void HciPutAcl(const unsigned char *data, unsigned short len);


// GLOBALS
static void(*DataCallBack)(const unsigned char * data, unsigned short len) = NULL;

// HCI connection data
hci_connection_t hci_connection;

// Code
unsigned char HciInit(void)
{
	// Setup mode, Off(-1),BR(0),LE(1) or BR+LE(2)
	hci_connection.connectionMode = BT_HOST_MODE;
	// Force LE mode off unless supported
	if(!HciSupportsLe()) hci_connection.connectionMode = 0;
	hci_connection.isConnected	= FALSE_;
	hci_connection.isConfigured	= FALSE_;
	hci_connection.connectionType = 0;
	hci_connection.connHandle 	= 0;
	hci_connection.packetsToAck = 0;
	hci_connection.controllerAclBufferSize = 0;
	hci_connection.controllerNumAclBuffers = 0;
	hci_connection.controllerLeBufferSize = 0;
	hci_connection.controllerNumLeBuffers = 0;
	hci_connection.hostAclBufferSize = MAX_IN_ACL_LENGTH; // Our incomming buffer size
	hci_connection.hostNumAclBuffers = 1; 
	hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT;

	// Install lower level callbacks
	HciInstallAclCB	(HciPutAcl);
	HciInstallEventCB (HciPutEvent);

	// Return 
	hci_connection.isInitialised = TRUE_;	
    return TRUE_;
}

void HciInstallL2CAP (void(*CallBack)(const unsigned char * data, unsigned short len))
{
	// To forware ACL packets to L2CAP
	DataCallBack = CallBack;
}

static void HciPutAcl(const unsigned char * data, unsigned short len)
{
	// We are not concerned about ACL packets in the HCI layer - check, strip and forward
	unsigned short header = BT_READLE16(data);
    unsigned short headerflags = 0x3000 & header;
	unsigned short handle = 0x0fff & header;

	if(	(headerflags != ACL_PACKET_FLAGS_COMPLETE) &&
		(headerflags != ACL_PACKET_FLAGS_END))
    {
        DBG_ERROR("l2cap cont. pkt!\n");
		return;
    }

	// Are we connected?
	if	(hci_connection.isConnected != TRUE_)
	{
		// Establish connection from the l2cap info to use in response
		DBG_ERROR("acl to null conn.\n");
		return;
	}

	// Does the handle match?
	if (handle != hci_connection.connHandle)
	{
		// Probably an LE query during an RFCOMM session - do not respond
		DBG_ERROR("l2cap handle mismatch\n");
		return;
	}

	if(DataCallBack!=NULL)DataCallBack(data+HCI_ACL_HDR_LEN,len-HCI_ACL_HDR_LEN); // Forward without acl header
}

static void HciSendCmd(const unsigned char* data, unsigned short len, unsigned short command)
{
	unsigned char* sendBuffer = HciPhyGetCmdBuffer();		// Always returns valid buffer
	BT_WRITELE16(sendBuffer,command);		// OCF|OGF
	sendBuffer[2] = len - HCI_CMD_HDR_LEN;	// Command length

	ASSERT(len <= (MAX_CMD_PACKET_LENGTH));

	if(len > HCI_CMD_HDR_LEN)				// Copy rest of command in 
	{
		ASSERT(data!=NULL);
		memcpy(&sendBuffer[3], data, len-HCI_CMD_HDR_LEN); 
	}
	
	HciPhyWriteCmd(sendBuffer, len); 		// Send
}

static void CreateAdvertisingDataPacket(unsigned char* cmd) // Creates packet in situ
{
	// Adds data to set advert packet and response packet
	memset(cmd,0, (HCI_LE_SET_ADVERT_DATA_LEN - HCI_CMD_HDR_LEN));// Extra bytes are zeroed	
	unsigned short remaining = (HCI_LE_SET_ADVERT_DATA_LEN - HCI_CMD_HDR_LEN - 1) ; 
	// There are always exactly 31 bytes in an advert cmd + 1 byte for the SIGNIFICANT length field 
	cmd[1] = 0x02;	remaining--; // Len of parameter - 2 bytes
	cmd[2] = 0x01;	remaining--; // Support flags descriptor
	cmd[3] = 0x05;	remaining--; // Support flags val (limited discoverable (b0), br not supported )
	// Try and copy in name (remaining = 28)
	unsigned short nameLen = strlen(GetBTName());	
	if ((remaining-2) >= nameLen)
	{
		cmd[4] = nameLen + 1;
		cmd[5] = 0x09; 			// Complete name tag
		memcpy(&cmd[6],GetBTName(),nameLen);
		cmd[0] = nameLen + 2 + 3;// SIGNIFICANT part of packet len
	}
	else
	{ 
		cmd[4] = remaining - 1;
		cmd[5] = 0x08;			// Shortened name tag
		memcpy(&cmd[6],GetBTName(),remaining-2);
		cmd[0] = 31;			// SIGNIFICANT part of packet len
	}
	// This line causes android to discover att services using SDP instead of LE mode
	// to allow propper dual mode support we must add an SDP entry for BR-ATT mode (PSM=31)
	//if(hci_connection.connectionMode == 2)cmd[3] = 0x01; // If want to be a dual mode device
	// Return leaving data in buffer
	return;
}

static void HciCommandCompleteEvent(const unsigned char* event, unsigned char* cmdDataBuffer)
{
	unsigned short len = 0;
	unsigned short opCode = 0;
	unsigned short eventCode;
	unsigned char status;

	eventCode = BT_READLE16(&event[3]);
	status = event[5];
	
	if(status != 0)
	{
    	DBG_ERROR( "cmd fail:%04X->",eventCode);		
    	DBG_DUMP((event+2),event[1]);	
	}

    switch (eventCode)
    {
		// Init interface
		case HCI_RESET:	{
			DBG_INFO( "reset\n");
			if(hci_connection.connectionMode == -1)
			{
				len	= 0;	
				opCode = 0;
				BTEvent(BT_EVENT_INITIALISED);
				break; // Further configuration not required - mode is off
			}
			BT_WRITELE16(&cmdDataBuffer[0],hci_connection.hostAclBufferSize);
			cmdDataBuffer[2] = 0; 									// Max synchronous data packet len, 0
			BT_WRITELE16(&cmdDataBuffer[3],hci_connection.hostNumAclBuffers); 
			BT_WRITELE16(&cmdDataBuffer[5],0);  					// Max num synvhronous packets, 0
			len	= HCI_H_BUF_SIZE_LEN;	
			opCode = HCI_H_BUF_SIZE;
			break;
			}
		case HCI_H_BUF_SIZE: {
			DBG_INFO( "buff set\n");
			len	= HCI_R_BUF_SIZE_LEN;	
			opCode =  HCI_R_BUF_SIZE;	
			break;
			}	
		case HCI_R_BUF_SIZE:{
			DBG_INFO( "buff read\n");
			hci_connection.controllerAclBufferSize = BT_READLE16(&event[6]);
			hci_connection.controllerNumAclBuffers = BT_READLE16(&event[9]);	
			DBG_INFO("br buff %u x %u\n",hci_connection.controllerAclBufferSize,hci_connection.controllerNumAclBuffers);
			len	= HCI_R_BD_ADDR_LEN;	
			opCode =  HCI_R_BD_ADDR;	
			break;
			}	
		// Init BR settings
		case HCI_R_BD_ADDR:	{
			// Save our controllers address
			DBG_INFO( "mac\n");
			memcpy(GetBTMAC(),(event+6),6);
			SaveBTMAC();
			BT_WRITELE16(&cmdDataBuffer[0],0x1FA0); // Default 5s
			len	= HCI_SET_PAGE_TIMEOUT_LEN;	
			opCode =  HCI_SET_PAGE_TIMEOUT;
			break;
			}
		case HCI_SET_PAGE_TIMEOUT:	{
			DBG_INFO( "timeout set\n");
#if !defined(BT_CHIPSET_CC2560) && !defined(BT_CHIPSET_CC2564)
	#warning "Your controller *may* expect a 248 char name packet (BT4.0 spec)!"
#endif
			len = 1+strlen((char*)GetBTName());// +1 for null
			ASSERT(len <= MAX_CMD_PACKET_LENGTH-HCI_CMD_HDR_LEN);
			memcpy(&cmdDataBuffer[0],GetBTName(),(len));
			len += HCI_CMD_HDR_LEN; 
			// Write device name
			opCode =  HCI_W_LOCAL_NAME;
			break;
			}
		case HCI_W_LOCAL_NAME:{
			DBG_INFO( "name set\n");	
			cmdDataBuffer[0] = GetBTCOD();
			cmdDataBuffer[1] = GetBTCOD()>>8;
			cmdDataBuffer[2] = GetBTCOD()>>16;
			// Write device COD	
			len	= HCI_W_COD_LEN;	
			opCode =  HCI_W_COD;
			break;
			}	
		// Init LE settings
		case HCI_W_COD:{
			DBG_INFO( "cod set\n");
			cmdDataBuffer[0] = cmdDataBuffer[1] = 0;				// Inform controller of required LE support
			if (hci_connection.connectionMode > 0)									
				cmdDataBuffer[0] = 0x01; // LE support on
			if (hci_connection.connectionMode == 2)
				cmdDataBuffer[1] = 0x01; // Dual support on
			len	=  HCI_WRITE_LE_HOST_SUPPORTED_LEN;	
			opCode =  HCI_WRITE_LE_HOST_SUPPORTED;
			break;
			}	
		case HCI_WRITE_LE_HOST_SUPPORTED:{
			DBG_INFO( "le host set\n");
			if (hci_connection.connectionMode == 0) 				// No LE initialisation, skip next section
			{
				cmdDataBuffer[0] = 3;	// Write & Page scan on	
				// Turn on radio, causes state to skip to end
				len	=  HCI_W_SCAN_EN_LEN;	
				opCode =  HCI_W_SCAN_EN;			
			}
			else 													// If we are initing the LE 
			{
				memcpy(&cmdDataBuffer[0],"\xff\xff\xff\xff\xff\x1f\x00\x20",8);	// Set hci event mask LE event flag
				len	=  HCI_SET_EVT_MASK_LEN;	
				opCode =  HCI_SET_EVT_MASK;
			}
			break;
			}	
		case HCI_SET_EVT_MASK : {
				memset(&cmdDataBuffer[0],0xff,8);						// Set LE event mask
				len	=  HCI_LE_SET_EVENT_MASK_LEN;	
				opCode =  HCI_LE_SET_EVENT_MASK;
				break;
			}	

		case HCI_LE_SET_EVENT_MASK : {
			DBG_INFO( "le evt set\n");						// Read LE controller buffer sizes
			len	=  HCI_LE_READ_BUFFER_SIZE_LEN;	
			opCode =  HCI_LE_READ_BUFFER_SIZE;
			break;
			}	

		case HCI_LE_READ_BUFFER_SIZE : {
			DBG_INFO( "le buff read\n");
			hci_connection.controllerLeBufferSize = BT_READLE16(&event[6]) - 4; // LE buffer size minus the L2CAP header
			hci_connection.controllerNumLeBuffers = event[8];					// Num of LE buffers
			DBG_INFO("le buff %u x %u\n",hci_connection.controllerLeBufferSize, hci_connection.controllerNumLeBuffers);
			len	=  HCI_LE_READ_SUPPORTED_FEATURES_LEN;	
			opCode =  HCI_LE_READ_SUPPORTED_FEATURES;
			break;
			}	
		case HCI_LE_READ_SUPPORTED_FEATURES:{	
			DBG_INFO( "le sup. read\n");
			// encryption available = b0 of event[6]				// Set advert params	
			// Advertising data 
			BT_WRITELE16(&cmdDataBuffer[0], 0x0800ul);
			BT_WRITELE16(&cmdDataBuffer[2], 0x0800ul);
			cmdDataBuffer[4] = 0;
			cmdDataBuffer[5] = 0;
			cmdDataBuffer[6] = 0;
			memcpy(&cmdDataBuffer[7],GetBTMAC(),6);
			cmdDataBuffer[13] = 0x07;
			cmdDataBuffer[14] = 0;
			len	=  HCI_LE_SET_ADVERTISING_PARAMS_LEN;	
			opCode =  HCI_LE_SET_ADVERTISING_PARAMS;
			break;
			}		
		case HCI_LE_SET_ADVERTISING_PARAMS:	{						// Set advert data
			DBG_INFO( "le ad param\n");
			CreateAdvertisingDataPacket(cmdDataBuffer);					// Packet made in place
			len	=  HCI_LE_SET_ADVERT_DATA_LEN;	
			opCode =  HCI_LE_SET_ADVERT_DATA;
			break;
			}	
		case HCI_LE_SET_ADVERT_DATA:{								// Set scan response data
			DBG_INFO( "le ad data\n");
			CreateAdvertisingDataPacket(cmdDataBuffer);
			len	=  HCI_LE_SET_SCAN_RESP_DATA_LEN;	
			opCode =  HCI_LE_SET_SCAN_RESP_DATA;
			break;
			}
		case HCI_LE_SET_SCAN_RESP_DATA:	{							// LE discoverable on
			DBG_INFO( "le ad set\n");	
			cmdDataBuffer[0] = 1; 		// On
			len	=  HCI_LE_SET_ADVERT_ENABLE_LEN;	
			opCode =  HCI_LE_SET_ADVERT_ENABLE;
			break;	
			}	
		case HCI_LE_SET_ADVERT_ENABLE:	{
			DBG_INFO( "le ad on\n");	
			if(hci_connection.connectionMode == 2)				// If dual mode support desired
			{	
				cmdDataBuffer[0] = 3;	// Write & Page scan on	
				len	=  HCI_W_SCAN_EN_LEN;	
				opCode =  HCI_W_SCAN_EN;
			}
			else
			{
				hci_connection.isConfigured = TRUE_;					// Setup complete, ready
				BTEvent(BT_EVENT_INITIALISED);
			}		
			break;	
			}
		case HCI_W_SCAN_EN:	{
				DBG_INFO( "br disc. on\n");
				hci_connection.isConfigured = TRUE_;					// Setup complete, ready
				BTEvent(BT_EVENT_INITIALISED);
			break;		
			}
		default: 
			break;
	}

	// If we are not configured, send the next prepared command
	// If there was no command (len = 0), relinquish buffer
	if((hci_connection.isConfigured	== FALSE_)&&(len > 0))	// If we are not yet configured
	{
		HciSendCmd(cmdDataBuffer, len, opCode);					// Send next command
	}

}

static void HciPutEvent(const unsigned char* event, unsigned short eventLen) // eventLen is unused
{
	unsigned char  cmdDataBuffer[MAX_CMD_PACKET_LENGTH-HCI_CMD_HDR_LEN]; // For responses

	switch (event[0]){
		case HCI_COMMAND_COMPLETE: {
			HciCommandCompleteEvent(event,cmdDataBuffer);
			return; // Handled externally
			}		

		case HCI_COMMAND_STATUS :{
			if(event[2] != 0){
				DBG_INFO("cmd fail:\n");
				DBG_DUMP(event,(event[1]+2));
			}			
			break;
			}		
		case HCI_NBR_OF_COMPLETED_PACKETS : {
			unsigned short num = BT_READLE16(&event[5]);
			DBG_INFO("pkts: %u of %u\n",num,hci_connection.packetsToAck);
			hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT; // Reset timeout
			if(hci_connection.packetsToAck > num)
				hci_connection.packetsToAck-= num;
			else
				hci_connection.packetsToAck = 0;
			break;
			}	
		case HCI_CONNECTION_REQUEST :{
			DBG_INFO("con req\n");
			memcpy(&cmdDataBuffer[0],&event[2],6); 			// Copy bd address to response
			if (hci_connection.isConnected == FALSE_)	// Accept new connection
			{
				BTEvent(BT_EVENT_CONNECTION_REQ);
				DBG_INFO("ack\n");
				cmdDataBuffer[6] = 0x01; 				// Role as slave
				HciSendCmd(cmdDataBuffer, HCI_ACCEPT_CONN_REQ_LEN, HCI_ACCEPT_CONN_REQ);	
			}
			else										// Already connected, reject
			{		
				DBG_INFO("nack\n");		
				cmdDataBuffer[6] = 0x0D; 				// Reason is limited resources		
				HciSendCmd(cmdDataBuffer, HCI_REJECT_CONN_REQ_LEN, HCI_REJECT_CONN_REQ);	
			}		
			break;
			}
		case HCI_CONNECTION_COMPLETE :{
			if(event[2] == 0){
				DBG_INFO("br new con.\n");
				hci_connection.isConnected	= TRUE_;
				hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT;
				hci_connection.connectionType = HCI_CONN_TYPE_BR;
				hci_connection.connHandle 	= BT_READLE16(&event[3]);
				hci_connection.packetsToAck = 0;
				memcpy(hci_connection.remoteADDR,&event[5],6);
				BTEvent(BT_EVENT_CONNECTED);
				// Now remove LE discoverability
				#ifdef BT_SIMULTANEOUS_LE_BR_DISABLED
				if(hci_connection.connectionMode > 0)
				{
					cmdDataBuffer[0] = 0; 		// Off - LE mode off when connected
					HciSendCmd(cmdDataBuffer, HCI_LE_SET_ADVERT_ENABLE_LEN, HCI_LE_SET_ADVERT_ENABLE);
				}
				#endif	
			}
			else 
			{
				DBG_INFO("br con. fail\n");
			}	
			break;
			}	
		case HCI_DISCONNECTION_COMPLETE :{
			DBG_INFO("discon.\n");
			hci_connection.isConnected	= FALSE_;
			hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT;
			hci_connection.connectionType = 0;
			hci_connection.connHandle 	= 0;
			hci_connection.packetsToAck = 0;
			BTEvent(BT_EVENT_DISCONNECTED);
			// Re-enable advertising and discovery
			if(hci_connection.connectionMode == -1)
			{
				// Radio reconfiguration not required - reset
				hci_connection.isConfigured	= FALSE_; 	// Radio will be reset to off
				HciSendCmd(cmdDataBuffer, HCI_RESET_LEN , HCI_RESET);
				
			}
			if(hci_connection.connectionMode > 0)
			{
				hci_connection.isConfigured	= FALSE_; 	// Trigger stack to reconfigure device
				cmdDataBuffer[0] = 1; 					// On - will trigger discoverable cmd too if needed
				HciSendCmd(cmdDataBuffer, HCI_LE_SET_ADVERT_ENABLE_LEN, HCI_LE_SET_ADVERT_ENABLE);
			}
			else
			{
				hci_connection.isConfigured	= FALSE_; // Trigger stack to reconfigure device
				cmdDataBuffer[0] = 3;		// Write & Page scan on	
				HciSendCmd(cmdDataBuffer, HCI_W_SCAN_EN_LEN, HCI_W_SCAN_EN);	
			}
			break;
			}	
		case HCI_PIN_CODE_REQUEST : {
			DBG_INFO("pin req\n");
			memcpy(&cmdDataBuffer[0],&event[2],6); 		// Copy bd address to response
			unsigned char length = strlen(GetBTPin());
			if(length > 16)length=16;					// BT spec 4.0 max pin len clamp
			memcpy(&cmdDataBuffer[7],GetBTPin(),length);// Copy pin to response
			cmdDataBuffer[6] = length;					// Copy pin - no NULL, important	
			length = (HCI_PIN_CODE_REQ_REP_LEN + length) - 4;// Set len fields (if pin > 4 bytes)
			HciSendCmd(cmdDataBuffer, length, HCI_PIN_CODE_REQ_REP);
			break;
			}	
		case HCI_LINK_KEY_REQUEST :{
			DBG_INFO("linkk req\n");
			memcpy(&cmdDataBuffer[0],&event[2],6); 		// Copy bd address to response buffer
			if(FindLinkKeyForBD((linkKey_t*)&cmdDataBuffer[0]))	// Search for the link key , appends to packet
			{
				DBG_INFO("have key\n");
				HciSendCmd(cmdDataBuffer, HCI_LINK_KEY_REQ_REP_LEN, HCI_LINK_KEY_REQ_REP);
			}
			else
			{
				DBG_INFO("no key\n");
				HciSendCmd(cmdDataBuffer, HCI_LINK_KEY_REQ_REP_NEGATIVE_LEN, HCI_LINK_KEY_REQ_REP_NEGATIVE);
			}
			break;
			}	
		case HCI_LINK_KEY_NOTIFICATION :{
			DBG_INFO("new key\n");
			StoreLinkKey((linkKey_t*)&event[2]);				// Expects: bd_add6, link key16
			break;
			}
		case HCI_DATA_BUFFER_OVERFLOW :{
			DBG_ERROR("acl overrun\n");
			break;
			}	
		case HCI_LE_META :{
			switch(event[2]){
				case HCI_SUBEVENT_LE_CONNECTION_COMPLETE :{
					if(event[3] == 0)
					{
						// Already connected, don't change connection type, result is only read
						// write will function and streaming + printfs will remain diverted to
						// the existing connection ().
						if(hci_connection.isConnected == TRUE_) 
						{
							DBG_INFO("le req!\n");
							break;
						}	
						else 
						{
							DBG_INFO("le new con.\n");
							hci_connection.isConnected	= TRUE_;
							hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT;
							hci_connection.connHandle 	= BT_READLE16(&event[4]);
							hci_connection.packetsToAck = 0;
							memcpy(hci_connection.remoteADDR,&event[7],6);
							hci_connection.connectionType = HCI_CONN_TYPE_LE;
							BTEvent(BT_EVENT_CONNECTED);
							#if 0
							DBG_INFO("Status: %u\n",(*(unsigned char*)(event+3)));
							DBG_INFO("Handle: %04X\n",BT_READLE16(&event[4]));
							DBG_INFO("Role: %u\n",(*(unsigned char*)(event+6)));
							DBG_INFO("AddressType: %u, ",(*(unsigned char*)(event+7)));
							DBG_DUMP(&event[8],6);
							DBG_INFO("ConnIntv: %u\n",BT_READLE16(&event[14]));
							DBG_INFO("ConnLtcy: %u\n",BT_READLE16(&event[16]));
							DBG_INFO("ConnTmot: %u\n",BT_READLE16(&event[18]));
							DBG_INFO("ClkAccy: %u\n",(*(unsigned char*)(event+20)));
							#endif
						}
					}
					else
					{
						DBG_INFO("le con. fail\n");
					}
					break;
					}	

				default: {
					DBG_INFO("le.event?:");
					DBG_DUMP(event,event[1]+2);
					break;
					}	
				}
			}
			break;
		case HCI_HW_MODE_CHANGE_EVENT :{
			DBG_INFO("mode sw\n");
			break;
			}	
		case HCI_HW_NEW_MAX_SLOTS_EVENT :{
			DBG_INFO("slots change\n");
			break;	
			}
		case HCI_HW_ENCRYPTION_CHANGED :{
			DBG_INFO("encypt change\n");
			break;
			}	
		case HCI_HW_ERROR_EVENT :{
			DBG_ERROR("err:%u",event[2]);
			break;
			}	
		default : {
			DBG_INFO("event:");
			DBG_DUMP(event,event[1]+2);
			break;
			}	
	}
	return;
}

void HciDisconnect(void)
{
	unsigned char  cmdDataBuffer[HCI_DISCONNECT_LEN-HCI_CMD_HDR_LEN]; // To assemble packet in
	BT_WRITELE16(&cmdDataBuffer[0],hci_connection.connHandle);
	cmdDataBuffer[2] = 0x13; // Reason 0x13, terminated by user (not in error)
	HciSendCmd(cmdDataBuffer, HCI_DISCONNECT_LEN, HCI_DISCONNECT);
	// Set new state
	hci_connection.isConnected	= FALSE_;
	hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT;
	hci_connection.connectionType = 0;
	hci_connection.connHandle 	= 0;
	return;
}

unsigned short HciGetAclMtu(void)
{
	// This should be called only by l2cap to establish channel packet restiction
	unsigned short mtu = 0;
	if(hci_connection.connectionType == HCI_CONN_TYPE_BR) 
	{
		mtu = MAX_OUT_ACL_LENGTH;							// Our physical acl buffer size
		if(mtu > hci_connection.controllerAclBufferSize) 	// Our controllers limit
			mtu = hci_connection.controllerAclBufferSize;
	}
	else if (hci_connection.connectionType == HCI_CONN_TYPE_LE) 
	{
		// The le parameter doesn't include acl header, so add it on
		DBG_INFO("hci: le req for mtu?\n")
		mtu = hci_connection.controllerLeBufferSize+HCI_ACL_HDR_LEN; 
		if(mtu>MAX_OUT_ACL_LENGTH)
			mtu = MAX_OUT_ACL_LENGTH;
	}
	return (mtu-HCI_ACL_HDR_LEN);	// Remove the acl header len from physical buffer size
}

// Following two functions handle outgoing ACL packets
unsigned char* HciGetAclBuffer (void)
{
	return (HciPhyGetAclBuffer() + HCI_ACL_HDR_LEN);
}

unsigned char HciAclCanSend(void)
{
	if(	(hci_connection.connectionType == HCI_CONN_TYPE_BR)&&
		(hci_connection.packetsToAck >= hci_connection.controllerNumAclBuffers))
	{
		return FALSE_;
	}
	else if(	(hci_connection.connectionType == HCI_CONN_TYPE_LE)&&
				(hci_connection.packetsToAck >= hci_connection.controllerNumLeBuffers))
	{
		return FALSE_;
	}
	return TRUE_;
}

unsigned char HciSendAcl (const unsigned char* data, unsigned short len)
{
	// Early out for zero lengths
	if(len == 0) 
	{
		HciPhyWriteAcl(NULL, 0); // Relinquish buffer
		return TRUE_;
	}

	// Check connection status
	if(	(hci_connection.isConnected != TRUE_) ||
		(hci_connection.connHandle == 0))
	{		
		DBG_ERROR("no conn. send\n");
		HciPhyWriteAcl(NULL, 0); // Relinquish buffer
		return FALSE_;
	}

	// Check can send - the controller has limited resources
	if(!HciAclCanSend()) 
	{
		DBG_ERROR("acl full!\n");
		HciPhyWriteAcl(NULL, 0); // Relinquish buffer
		return FALSE_;
	}

	// Check not currently sending
	// Not needed - waits in phy hal layer 

	// Add acl header
	BT_WRITELE16((data - HCI_ACL_HDR_LEN), (hci_connection.connHandle | ACL_PACKET_FLAGS_END | ACL_PACKET_FLAGS_NOBROADCAST));
	BT_WRITELE16((data - HCI_ACL_HDR_LEN + 2), (len));

	// Transmit
	HciPhyWriteAcl((data - HCI_ACL_HDR_LEN), (len + HCI_ACL_HDR_LEN));
	hci_connection.packetsToAck++;
	return TRUE_;
}

void HciTimeoutHandler(void )
{
	// Cant timeout unless waiting and connected
	if((hci_connection.packetsToAck == 0)|| (hci_connection.isConnected != TRUE_))
	{
		hci_connection.connectionTimeout = HCI_CONNECTION_TIMEOUT;
		return;
	}
	// Have we been waiting too long
	if(hci_connection.connectionTimeout && --hci_connection.connectionTimeout==0)
	{
		// Timeout connection - force disconnect
		DBG_ERROR("timeout, disc.\n");
		HciDisconnect();
	}
}

// EOF
