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

#include <stdlib.h>
#include "l2cap + le.h"
#include "hci + le.h"
#include "bt config.h"
#include "debug.h"

// DEBUG
#if  DEBUG_L2CAP
	static const char* file = "l2cap+le.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "l2cap+le.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

// GLOBALS

// Buffer checked out flags - for debugging 
unsigned char l2capSendBufferCheckedOut = FALSE_;

l2cap_channel_t l2cap_channels[MAX_L2CAP_CONNECTIONS];
l2cap_connection_t l2cap_connection;

unsigned char (*sdpCallBack)		(const unsigned char * data, unsigned short len) = NULL;
unsigned char (*rfcommCallBack)		(const unsigned char * data, unsigned short len) = NULL;
unsigned char (*attCallBack)		(const unsigned char * data, unsigned short len) = NULL;
unsigned char (*smpCallBack)		(const unsigned char * data, unsigned short len) = NULL;

// Supported psm list
const unsigned short psmSupportList[] = L2CAP_PSM_SUPPORT_LIST;

// CODE
unsigned char L2capInit(void)
{
	unsigned char i;
	// Delete channels
	for (i=0;i<MAX_L2CAP_CONNECTIONS;i++)
	{
		l2cap_channels[i].state = CLOSED;
		l2cap_channels[i].configLocal = CONFIG_NO_STATE;
		l2cap_channels[i].configRemote = CONFIG_NO_STATE;
		l2cap_channels[i].psm = 0;
		l2cap_channels[i].remoteCID = 0;
		l2cap_channels[i].localCID = 0;
		l2cap_channels[i].lastId = 0;
	}
	// Init states
	l2cap_connection.isInitialised = TRUE_;
	l2cap_connection.lastSignalingId = 0;
	l2cap_connection.connectionTimeout = L2CAP_CONNECTION_TIMEOUT;

	// Install handler
	HciInstallL2CAP (L2capPacketHandler);
	// Done
	return TRUE_;
}

void L2capInstallRfcomm(unsigned char (*CallBack)(const unsigned char * data, unsigned short len))
{
	rfcommCallBack = CallBack;
}
void L2capInstallSdp(unsigned char (*CallBack)(const unsigned char * data, unsigned short len))
{
	sdpCallBack = CallBack;
}
void L2capInstallAtt(unsigned char (*CallBack)(const unsigned char * data, unsigned short len))
{
	attCallBack = CallBack;
}
void L2capInstallSmp(unsigned char (*CallBack)(const unsigned char * data, unsigned short len))
{
	smpCallBack = CallBack;
}

l2cap_channel_t* L2capChannelForPsm (unsigned short psm)
{
	unsigned short i;
	// Find cid for psm
	for(i=0;i<MAX_L2CAP_CONNECTIONS;i++)
	{
		if(l2cap_channels[i].psm == psm) // Find channel
		{
			if(l2cap_channels[i].state != CLOSED)
			return &l2cap_channels[i];
		}
	}
	return NULL;	
}

l2cap_channel_t* L2capChannelForLocalCid (unsigned short cid)
{
	unsigned short i;
	// Find cid for psm
	for(i=0;i<MAX_L2CAP_CONNECTIONS;i++)
	{
		if(l2cap_channels[i].localCID == cid) // Find channel
		{
			if(l2cap_channels[i].state != CLOSED)
			return &l2cap_channels[i];
		}
	}
	return NULL;	
}

l2cap_channel_t* L2capGetEmptyChannel (void)
{
	unsigned short i;
	// Find cid for psm
	for(i=0;i<MAX_L2CAP_CONNECTIONS;i++)
	{
		if(l2cap_channels[i].state == CLOSED) // Find channel
		{
			// Re initialise it
			l2cap_channels[i].configLocal = CONFIG_NO_STATE;
			l2cap_channels[i].configRemote = CONFIG_NO_STATE;
			l2cap_channels[i].psm = 0;
			l2cap_channels[i].remoteCID = 0;
			l2cap_channels[i].localCID = 0;
			l2cap_channels[i].lastId = 0;
			return &l2cap_channels[i];
		}
	}
	return NULL;	
}

unsigned short L2capGetMtuForPsm(unsigned short psm)
{
	unsigned short mtu = 0xffff;
	// Get the channel
	l2cap_channel_t* channel = L2capChannelForPsm(psm);
	if(channel == NULL) return 0;
	// Find limiting factor (their mtu or ours)
	if(channel->remoteMTU < mtu)mtu = channel->remoteMTU;
	if(HciGetAclMtu()< mtu) 	mtu = HciGetAclMtu();
	// Subtract l2cap header len
	ASSERT(mtu > L2CAP_HEADER_LEN);// No connection?
	return (mtu - L2CAP_HEADER_LEN);
}

unsigned char* L2capGetSendBuffer (void)
{
	if(l2capSendBufferCheckedOut == TRUE_)
		{DBG_ERROR("buff error!\n");}
	l2capSendBufferCheckedOut = TRUE_;
	return HciGetAclBuffer() + L2CAP_HEADER_LEN; // Leave room for L2CAP header - added later
}

unsigned char L2capSendPsm (unsigned char* buffer, unsigned short len, unsigned char psm)
{
	// Relinquish buffer
	l2capSendBufferCheckedOut = FALSE_;
	// Find cid for psm
	l2cap_channel_t* channel = L2capChannelForPsm(psm);

	if((channel != NULL) && (channel->state == OPEN))	
		return L2capSendCid (buffer,len, channel->remoteCID); // Send to remote channel

	DBG_ERROR("invalid psm\n");
	return FALSE_;	
}

unsigned char L2capSendCid (unsigned char* buffer, unsigned short len, unsigned short cid)
{
	// Relinquish buffer
	l2capSendBufferCheckedOut = FALSE_;

	// Set len to zero to reliquish buffer
	if (len == 0) return TRUE_;

	// Add L2CAP header
	BT_WRITELE16((buffer-L2CAP_HEADER_LEN), len); 	// Write len
	BT_WRITELE16((buffer-2), cid);					// Write cid

	DBG_INFO("L2CAP out:");
	DBG_DUMP(buffer-L2CAP_HEADER_LEN, len+L2CAP_HEADER_LEN);

	// Send packet
	return HciSendAcl (buffer - L2CAP_HEADER_LEN, len + L2CAP_HEADER_LEN);
}

void L2capPacketHandler(const unsigned char * data, unsigned short len)
{
	unsigned short length;
	unsigned short cid;

	// Sanity checks
	if((data == NULL) || (len<=(L2CAP_HEADER_LEN)))
	{
		DBG_INFO("L2CAP:Invalid ACL packet ignored!\n");
		return;
	}

	// Get parameters
	length = BT_READLE16(data);
	cid = BT_READLE16(&data[2]);

	if(cid == 0)
	{
		DBG_ERROR("cid zero!\n");
		return;			
	}

    DBG_INFO("L2CAP  in:");
    DBG_DUMP(data, length+4);

	// Switch packet to correct handler
	switch(cid){
		// The signalling channel is always allowed
		case (L2CAP_SIGNALLING_CID):
			l2cap_connection.connectionTimeout = L2CAP_CONNECTION_TIMEOUT;	// Clear connection timeout
			L2capHandleSignallingPacket(data+L2CAP_HEADER_LEN,length); 		// Strip L2CAP headers
			break;

		// The next LE channels are only notified of packets if connected to an LE client
		#ifdef ENABLE_LE_MODE
		case (L2CAP_LE_ATT_CID):
			if(hci_connection.connectionType != HCI_CONN_TYPE_LE) 		// Only forward if currently connected to LE
				{DBG_INFO("att!\n");break;}
			DBG_INFO("att->\n");
			if(attCallBack != NULL)
				attCallBack(data+L2CAP_HEADER_LEN,length); 				// Strip L2CAP headers
			break;
		case (L2CAP_LE_SMP_CID):
			if(hci_connection.connectionType != HCI_CONN_TYPE_LE) 		// Only forward if currently connected to LE
				{DBG_INFO("smp!\n");break;}
			DBG_ERROR("smp->!\n");
			if(smpCallBack!=NULL)
				smpCallBack(data+L2CAP_HEADER_LEN,length); 				// Strip L2CAP headers 
			break;
		case (L2CAP_LE_SIG_CID):
			if(hci_connection.connectionType != HCI_CONN_TYPE_LE) 		// Only forward if currently connected to LE
				{DBG_INFO("le sig!\n");break;}
			DBG_ERROR("le sig->!!\n");
			break;
		#endif

		// Normal L2CAP connections each have a channel, PSM and state
		default:
			{
				unsigned char i;
				for(i=0;i<MAX_L2CAP_CONNECTIONS;i++)
				{
					if(l2cap_channels[i].localCID == cid) 			// Find channel
					{
						if(l2cap_channels[i].state != OPEN)			// Check its connected
						{
							DBG_ERROR("channel closed\n");
							break;
						}
						switch(l2cap_channels[i].psm){				// Send to correct psm
							case(L2CAP_PSM_RFCOMM):
								DBG_INFO("rfcomm->\n");
								if(rfcommCallBack != NULL)
									rfcommCallBack(data+L2CAP_HEADER_LEN,length);  	// Strip L2CAP headers
								break;
							case(L2CAP_PSM_SDP):
								DBG_INFO("sdp->\n");
								if(sdpCallBack != NULL)
									sdpCallBack(data+L2CAP_HEADER_LEN,length);  	// Strip L2CAP headers
								break;
							case(L2CAP_PSM_ATT):
								DBG_ERROR("Att psm pkt!\n");
								break;
							default:
								DBG_ERROR("Wrong psm!\n");
								break;
						}
						return; 									// Handled
					}
				}
			}
	}
	return;	
}

unsigned char L2capDisconnectChannel (l2cap_channel_t* channel)
{
	unsigned char* buffer;

	// Check it exists
	if(channel == NULL)
	{
		DBG_ERROR("disc null!\n");
		return FALSE_;
	}

	if(channel->state == CLOSED)
	{
		DBG_INFO("re-close!\n");
		return FALSE_;
	}

	// Make request
	buffer =	L2capGetSendBuffer();
	buffer[0] = L2CAP_DISCONN_REQ;
	buffer[1] = ++channel->lastId;
	BT_WRITELE16(&buffer[2], 4); // Four bytes in disc request
	BT_WRITELE16(&buffer[4], channel->remoteCID);
	BT_WRITELE16(&buffer[6], channel->localCID);

	// Send
	if(!L2capSendCid (buffer, 8, L2CAP_SIGNALLING_CID)) // 8 bytes total
	{
		DBG_ERROR("disc fail:%u\n",channel->psm);
		return	FALSE_;
	}
	DBG_INFO("disc:%u\n",channel->psm);
	channel->state = WAIT_CLOSING;
	return TRUE_;
}

unsigned char L2capDisconnect(void)
{
	unsigned short i;
	// Send disconnect reqest to every channel
	for(i=0;i<MAX_L2CAP_CONNECTIONS;i++)
	{
		if(l2cap_channels[i].state != CLOSED) // Find channel
		{
			L2capDisconnectChannel(&l2cap_channels[i]);
		}
	}
	// Erase channel data
	L2capInit();	
	return TRUE_;
}

void L2capHandleSignallingPacket(const unsigned char* payload, unsigned short length)
{
	unsigned char* buffer;
	unsigned short 	cid;
	l2cap_channel_t* channel = NULL;

	// Handle packets on L2CAP signalling channel
	do{
		unsigned char 	code = 	payload[0];
		unsigned char 	id = l2cap_connection.lastSignalingId = payload[1];
		unsigned short 	len = 	BT_READLE16(&payload[2]);
		const unsigned char* 	data = &payload[4];

		switch(code){
			case(L2CAP_ERROR):
				DBG_ERROR("err:%u\n",BT_READLE16(data));
				break;
			case(L2CAP_CONN_REQ):
				DBG_INFO("<-con req psm: %u\n",BT_READLE16(data));
				channel = L2capGetEmptyChannel();
				if(channel == NULL)
				{	
					DBG_ERROR("no ch. spare\n");
					break;
				}
				// Setup channel
				channel->lastId = id+1;
				channel->psm = BT_READLE16(data);
				channel->remoteCID = BT_READLE16(&data[2]);
				channel->localCID = channel->remoteCID+1; // Arbitrary
				// Make a conn accept reponse
				buffer =	L2capGetSendBuffer();
				buffer[0] = L2CAP_CONN_RSP;
				buffer[1] = id;
				BT_WRITELE16(&buffer[2], 8); // Eight bytes in accept conn
				BT_WRITELE16(&buffer[4], channel->localCID);	
				BT_WRITELE16(&buffer[6], channel->remoteCID);
				// Check for psm in support list
				{
					const unsigned short* validPsm = psmSupportList; 
					unsigned short response = 0;
					for(;;)	
					{
						if(*validPsm == L2CAP_PSM_END_LIST)	
						{
							// Unsuported psm resp
							DBG_INFO("psm rej.\n");
							response = 0x0002;
							break;
						} 
						if(*validPsm == channel->psm)		{response = 0x0000;break;} // No errors
						validPsm++;	
					}
					BT_WRITELE16(&buffer[8], response); // No errors
				}
				BT_WRITELE16(&buffer[10],0x0000); // Connection success	
				// Send L2CAP_CONN_RSP
				if(!L2capSendCid (buffer, 12, L2CAP_SIGNALLING_CID)) // 12 bytes total
				{
					DBG_ERROR("fail send\n");
				}
				else
				{
					DBG_INFO("->con ack psm: %u\n",channel->psm);
					channel->state = CONFIGURING;
				}

#ifdef L2CAP_PRE_EMPT_CONFIG_RESP
				// Wait for buffer to send - done in phy layer for now
				// Find mtu (much shorter for LE connection types)
				unsigned short mtu = HciGetAclMtu();

				// Now send our config request
				buffer =	L2capGetSendBuffer();
				buffer[0] = L2CAP_CFG_REQ;
				channel->lastId = id+1;
				buffer[1] = channel->lastId;
				BT_WRITELE16(&buffer[2], 8); // Eight bytes in cfg req
				BT_WRITELE16(&buffer[4], channel->remoteCID);
				BT_WRITELE16(&buffer[6], 0x0000);	// Flags 0, no continuation support
				buffer[8] = 0x01;				// MTU option
				buffer[9] = 0x02;				// 2 bytes
				BT_WRITELE16(&buffer[10], mtu); // Set MTU
				// Send - L2CAP_CFG_REQ
				if(!L2capSendCid (buffer, 12, L2CAP_SIGNALLING_CID)) // 12 bytes total
				{
					DBG_ERROR("fail send\n");
				}
				else
				{
					DBG_INFO("->cfg req psm:%u, mtu:%u\n",channel->psm,mtu);
					channel->configRemote = CONFIG_REQ_TXED; 
				}
#endif
				break;
			case(L2CAP_CONN_RSP):
				DBG_ERROR("con resp?!\n");
				// Don't expect these - we never establish the connection
				break;

			case(L2CAP_CFG_REQ):
				cid = BT_READLE16(data);
				channel = L2capChannelForLocalCid(cid);
				if(channel == NULL)
				{	
					DBG_ERROR("cfg to null!\n");
					break;
				}

				// Read remote device's MTU
				if((data[4]==0x01)&&(data[5]==0x02)) // 2 byte MTU configuration
				{
					channel->remoteMTU = BT_READLE16(&data[6]);
					DBG_INFO("<-cfg req mtu: %u\n",channel->remoteMTU);
				}
				else
				{
					DBG_ERROR("unknown mtu\n");
					break;	
				}

				channel->configLocal = CONFIG_REQ_RXED;

				// Prepare a response
				buffer =	L2capGetSendBuffer();
				buffer[0] = L2CAP_CFG_RSP;
				buffer[1] = id;
				BT_WRITELE16(&buffer[2], 6); // Six bytes in cfg resp if we accept their MTU
				BT_WRITELE16(&buffer[4], channel->remoteCID);
				BT_WRITELE16(&buffer[6], 0x0000);	// Flags 0, no continuation support
				BT_WRITELE16(&buffer[8], 0x0000); 	// Status 0, success (we will just accept their mtu size and use it)

				// Send - L2CAP_CFG_RSP
				if(!L2capSendCid (buffer, 10, L2CAP_SIGNALLING_CID)) // 10 bytes total
				{
					DBG_ERROR("send fail\n");
					break;
				}
				else
				{
					DBG_INFO("->cfg rsp psm:%u\n",channel->psm);
					channel->configLocal = CONFIG_RESP_TXED;		// Sent response
				}
				
				if(	(channel->configLocal == CONFIG_RESP_TXED) && 
					(channel->configRemote == CONFIG_RESP_RXED)) 	// Channel configured
				{
					DBG_INFO("open psm:%u\n",channel->psm);
					channel->state = OPEN; 
					break;
				}
				else if (channel->configRemote != CONFIG_REQ_TXED) 	// We haven't sent out cfg req yet
				{
					// Wait for buffer to send - done in phy layer for now
					// Find mtu (much shorter for LE connection types)
					unsigned short mtu = HciGetAclMtu();

					// Now send our config request
					buffer =	L2capGetSendBuffer();
					buffer[0] = L2CAP_CFG_REQ;
					channel->lastId = id+1;
					buffer[1] = channel->lastId;
					BT_WRITELE16(&buffer[2], 8); // Eight bytes in cfg req
					BT_WRITELE16(&buffer[4], channel->remoteCID);
					BT_WRITELE16(&buffer[6], 0x0000);	// Flags 0, no continuation support
					buffer[8] = 0x01;				// MTU option
					buffer[9] = 0x02;				// 2 bytes
					BT_WRITELE16(&buffer[10], mtu); // Set MTU
					// Send - L2CAP_CFG_REQ
					if(!L2capSendCid (buffer, 12, L2CAP_SIGNALLING_CID)) // 12 bytes total
					{
						DBG_ERROR("send fail\n");
					}
					else
					{
						DBG_INFO("->cfg req psm:%u, mtu:%u\n",channel->psm,mtu);
						channel->configRemote = CONFIG_REQ_TXED; 
					}
				}
				break;

			case(L2CAP_CFG_RSP):
				DBG_INFO("<-cfg rsp psm: %u\n",channel->psm);
				cid = BT_READLE16(data);
				channel = L2capChannelForLocalCid (cid);
				if(channel == NULL)
				{	
					DBG_ERROR("cfg res. null\n");
					break;
				}
				if(	(BT_READLE16(&data[2]) != 0x0000) || 	// Check flags
					(BT_READLE16(&data[4]) != 0x0000) )		// Check result
				{
					DBG_ERROR("refuse cfg\n");
					L2capDisconnectChannel(channel);
				}
				channel->lastId = id+1;

				channel->configRemote = CONFIG_RESP_RXED;

				if(	(channel->configLocal == CONFIG_RESP_TXED) && 
					(channel->configRemote == CONFIG_RESP_RXED)) 	// Channel configured
				{
					DBG_INFO("open psm:%u\n",channel->psm);
					channel->state = OPEN; 
				}

				break;

			case(L2CAP_DISCONN_REQ):
				DBG_INFO("<-disc req\n");
				cid = BT_READLE16(data);
				channel = L2capChannelForLocalCid (cid);
				if(channel == NULL)
				{
					DBG_ERROR("null req disc?!\n");
					break; 
				}
				// Prepare response
				buffer =	L2capGetSendBuffer();
				buffer[0] = L2CAP_DISCONN_RSP;
				buffer[1] = id;
				BT_WRITELE16(&buffer[2], 4); // Four bytes in disc resp
				BT_WRITELE16(&buffer[4], channel->localCID);
				BT_WRITELE16(&buffer[6], channel->remoteCID);

				// Send
				if(!L2capSendCid (buffer, 8, L2CAP_SIGNALLING_CID)) // 8 bytes total
				{
					DBG_ERROR("send fail\n");
				}
				else
				{
					DBG_INFO("->disc rsp\n");
				}
				DBG_INFO("closed psm:%u\n",channel->psm);
				channel->state = CLOSED;
				channel->configLocal = CONFIG_NO_STATE;
				channel->configRemote = CONFIG_NO_STATE;
				channel->psm = 0;
				channel->remoteCID = 0;
				channel->localCID = 0;
				channel->lastId = 0;
				break;

			case(L2CAP_DISCONN_RSP):
				DBG_INFO("<-disc rsp\n");
				cid = BT_READLE16(&data[2]);
				channel = L2capChannelForLocalCid (cid);
				// If channel is null then we may have already cleared it
				if(channel == NULL)
					break; 
				// Fully shut down channel
				DBG_INFO("closed psm:%u\n",channel->psm);
				channel->state = CLOSED;
				channel->configLocal = CONFIG_NO_STATE;
				channel->configRemote = CONFIG_NO_STATE;
				channel->psm = 0;
				channel->remoteCID = 0;
				channel->localCID = 0;
				channel->lastId = 0;
				break;

			case(L2CAP_ECHO_REQ):
				DBG_INFO("<-echo req\n");
				// Make response
				buffer =	L2capGetSendBuffer();
				buffer[0] = L2CAP_ECHO_RSP;
				buffer[1] = id;
				BT_WRITELE16(&buffer[2], 0); // Set length to zero as its optional
				// Send
				if(!L2capSendCid (buffer, 4, L2CAP_SIGNALLING_CID)) // 4 bytes total
				{
					DBG_ERROR("send fail\n");
				}
				else
				{
					DBG_INFO("->echo resp\n");
				}
				break;

			case(L2CAP_ECHO_RSP):
				DBG_INFO("<-echo resp\n");
				break;

			case(L2CAP_INFO_REQ):{
				unsigned short 	infoType = BT_READLE16(data);
				DBG_INFO("<-info req %u\n",infoType);
				// Make response
				buffer =	L2capGetSendBuffer();
				buffer[0] = L2CAP_INFO_RSP;
				buffer[1] = id;
				if(infoType == 0x01) // Connectionless mtu
				{
					// Find mtu (much shorter for LE connection types)
					unsigned short mtu = HciGetAclMtu();

					// Make response
					BT_WRITELE16(&buffer[2], 6); 			// Six bytes in response
					BT_WRITELE16(&buffer[4], infoType); 	// Extended features
					BT_WRITELE16(&buffer[6], 0x0000); 		// Success
					BT_WRITELE16(&buffer[8], mtu); 	// MTU size
					// Send
					if(!L2capSendCid (buffer, 10, L2CAP_SIGNALLING_CID)) // 10 bytes total
					{
						DBG_ERROR("send fail\n");
					}
					else
					{
						DBG_INFO("->info resp mtu:%u \n",mtu);
					}
				}
				else if	(infoType == 0x02)	// Extended features or single channel support
				{
					BT_WRITELE16(&buffer[2], 8); 	  // Eight bytes in response
					BT_WRITELE16(&buffer[4], infoType); // Extended features
					BT_WRITELE16(&buffer[6], 0x0000); // Sucess
					#ifndef ENABLE_LE_MODE
					BT_WRITELE16(&buffer[8], 0x0000); // No extended feature flags
					#else
					BT_WRITELE16(&buffer[8], 0x0080); // No extended feature flags set except fixed channels
					#endif
					BT_WRITELE16(&buffer[10], 0x0000);// No extended feature flags set	
					// Send
					if(!L2capSendCid (buffer, 12, L2CAP_SIGNALLING_CID)) // 12 bytes total
					{
						DBG_ERROR("send fail\n");
					}
					else
					{
						DBG_INFO("->info resp xf\n");
					}
				}
				#ifdef ENABLE_LE_MODE
				else if	(infoType == 0x03)	// Fixed channel support mask
				{
					BT_WRITELE16(&buffer[2], 16); 	  	// 16 bytes in response
					BT_WRITELE16(&buffer[4], infoType); // Extended features
					BT_WRITELE16(&buffer[6], 0x0000); 	// Sucess

					BT_WRITELE16(&buffer[8], 0x0072);	// 0 - 15 - Channels 1,4,5,6 supported (LE)
					BT_WRITELE16(&buffer[10], 0x0000); 	// 16 - 31
					BT_WRITELE16(&buffer[12], 0x0000);  // 32 - 47	
					BT_WRITELE16(&buffer[14], 0x0000); 	// 48 - 63

					// Send resp
					if(!L2capSendCid (buffer, 16, L2CAP_SIGNALLING_CID)) // 12 bytes total
					{
						DBG_ERROR("send fail\n");
					}
					else
					{
						DBG_INFO("->info resp ch.map\n");// p1430 BT4.0 Spec
					}
				}
				#endif
				else // unsupported
				{
					BT_WRITELE16(&buffer[2], 8); 	  	// Eight bytes in response
					BT_WRITELE16(&buffer[4], infoType); // Extended features
					BT_WRITELE16(&buffer[6], 0x0001); 	// Unsuported
					// Send
					if(!L2capSendCid (buffer, 8, L2CAP_SIGNALLING_CID)) // 8 bytes total
					{
						DBG_ERROR("send fail\n");
					}
					else
					{
						DBG_INFO("->info resp not.sup\n");
					}
				}
			}
			break;
			case(L2CAP_INFO_RSP):
				DBG_ERROR("inf resp?\n");
			break;
			default:
				DBG_INFO("code!:\n",code);
			break;
		}

		// Check if there are any more codes in the packet
		payload = data + len; 						// Update ptr to end of packet
		length -= (len + L2CAP_SIG_HEADER_LEN);		// Update length
		if(length >=  L2CAP_SIG_HEADER_LEN)			// If more bytes are present than processed
		{
			DBG_INFO("bk2bk sig?!\n");
		}
	}while(length > L2CAP_SIG_HEADER_LEN+4); 		// Must be at least 8 bytes (echo req)
 	return;
}

void L2capTimeoutHandler(void)
{
	// Static used so if there are multiple open connections, we echo all
	unsigned short i;
	// Count down connection timeout
	if(--l2cap_connection.connectionTimeout > 0) return;
	// Timeout occured, see if there are open channels
	l2cap_connection.connectionTimeout = L2CAP_CONNECTION_TIMEOUT;

	// Check for any open channels
	for(i=0;i<MAX_L2CAP_CONNECTIONS;i++)	
	{
		if(l2cap_channels[i].state == OPEN) 		// There is an open channel
		{
			if(hci_connection.packetsToAck == 0) 	// and there are no outgoing packets
			{
				// Send an echo request to the signallin channel and return
				// Make response
				unsigned char * buffer = L2capGetSendBuffer();
				buffer[0] = L2CAP_ECHO_REQ;
				buffer[1] = ++l2cap_connection.lastSignalingId;					
				BT_WRITELE16(&buffer[2], 0); 	// Set length to zero as its optional
				// Send packet to remote channel id
				if(!L2capSendCid (buffer, 4, L2CAP_SIGNALLING_CID)) // 4 bytes total
				{
					DBG_ERROR("send fail\n");
				}
				else
				{
					DBG_INFO("->echo req\n");
				}
			}
			return;
		}
	}
}


//EOF


