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

// UDP Services
// Karim Ladha, 2013-2014
// Based on code by Microchip Inc. and Howard Schlunder     10/7/04

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Settings.h"
#include "Data.h"
#include "Peripherals/Rtc.h"
#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/UDP services local.h"

#include "Utils/Util.h"
#define DEBUG_ON
#include "Common/debug.h"

#ifdef ENABLE_UDP_SERVICES
#ifndef STACK_CLIENT_MODE
#error	"Stack not configured properly to reply"
#endif

typedef enum {
	UDP_BROADCAST,
	UDP_NEW_CONNECTION,
	UDP_CONNECTION_TIMEOUT,
	UDP_CONNECTION_CLOSED
} UdpEventPacketTypes;

typedef enum {
	UDP_GET_SOCKET,
	UDP_LISTEN,
	UDP_RESPOND,
	UDP_OPEN,
	UDP_RESET,
	UDP_OFF,
} UDP_State_t;

typedef struct {
	UDP_State_t state;
	UDP_SOCKET	socket;
	unsigned long timer;
} udp_connection_t;

// Globals
static udp_connection_t udpClients[MAX_UDP_DATA_SOCKETS];

// Prototypes
static unsigned short UdpMakeEventPacket(unsigned char type, UDP_SOCKET socket);

// Init
void UdpInit(void)
{
	// Once!
	static unsigned char i = 0;
	for(;i<MAX_UDP_DATA_SOCKETS;i++)
	{
		DBG_INFO("UDP INIT\r\n");
		udpClients[i].state = UDP_GET_SOCKET;
		udpClients[i].socket = INVALID_UDP_SOCKET;
		udpClients[i].timer = 0xffffffff;
		status.udp_mask = 0;
	}
}

// Used for announcing only - see below
void AnnouncementSend(unsigned char type, UDP_SOCKET_INFO* sktInfo);

unsigned char UdpWritePacket(void* data, unsigned short len, unsigned short clientMask)
{
	udp_connection_t* client;
	//UDP_SOCKET	socket;
	UDP_SOCKET_INFO* sktInfo;
	unsigned short timeout, maskLocal, space = 0;
	unsigned char clientIndex;

	// Empty mask, zero len, null data
	if((clientMask == 0) || (len == 0) || (data == NULL))
		return 0;

	DBG_INFO("UDP WRITE\r\n");

	// Check is connected
	if(!MACIsLinked())  
		return 0;

	for(clientIndex = 0; clientIndex < MAX_UDP_DATA_SOCKETS; clientIndex++)
	{
		maskLocal = 0x0001 << clientIndex;
		client = &udpClients[clientIndex];
		if((clientMask & maskLocal) && (client->state == UDP_OPEN))
		{
			// Check it is open
			if(!UDPIsOpened(client->socket))
			{
				DBG_INFO("UDP CLIENT %u RESET\r\n",clientIndex);
				client->state = UDP_RESET;
				continue;
			}
			// Get socket info
			sktInfo = &UDPSocketInfo[client->socket];
			// Open udp socket to the clients ip and socket
			//socket = UDPOpenEx(sktInfo->remote.remoteNode.IPAddr.Val, UDP_OPEN_IP_ADDRESS, 0, sktInfo->remotePort);
			//socket = UDPOpenEx(sktInfo->remote.remoteNode,UDP_OPEN_NODE_INFO, 0,sktInfo->remotePort);
			// Check for success
//			if(socket == INVALID_UDP_SOCKET)
//			{
//				DBG_INFO("UDP CLIENT %u FAIL OPEN\r\n",clientIndex);
//				continue;
//			}
			timeout = 1000;// 10ms			// May need to wait for current send
			while(timeout-- && space<len)	// 1500bytes @ 10mbps = 1.2ms + latency (10ms?)
			{
				space = UDPIsPutReady(client->socket); // Physically polls the device
				Delay10us(1); 
			}
			if(space<len) 
			{
				DBG_INFO("UDP CLIENT %u TIMEOUT\r\n",clientIndex);
				continue;
			}
			// Send the packet
			UDPPutArray((void*)data, len);
			UDPFlush();
			UDPClose(client->socket);
			// Clear flag
			clientMask &= ~maskLocal;
			DBG_INFO("UDP CLIENT %u TXED\r\n",clientIndex);
		}
	} // Next client
	// Return if all flagged channels sent
	return (clientMask==0);
}

// Listen for broadbast and reply with unicast
void UdpRemoteDiscovery(void)
{
	unsigned char clientIndex;
	udp_connection_t* client;
	unsigned short size, maskLocal;
	UDP_SOCKET_INFO* sktInfo;

	// Must be connected to a network
	if(!MACIsLinked())
		return;

	for(clientIndex = 0; clientIndex < MAX_UDP_DATA_SOCKETS; clientIndex++)
	{
		maskLocal = 0x0001 << clientIndex;
		client = &udpClients[clientIndex];

		if(!UDPIsOpened(client->socket))
		{
			client->state = UDP_RESET;
			// Continue to open port
		}

		// Simple state machine
		switch(client->state)
		{
			case UDP_RESET:{
				DBG_INFO("UDP RESET %u\r\n",clientIndex);
				// Inform udp stack if it was open
				if(client->socket != INVALID_UDP_SOCKET)
					UDPClose(client->socket);
				// Inform status
				status.udp_mask &= ~maskLocal;
				// Get another socket
				client->state = UDP_GET_SOCKET;
				// Fall through
			}
			case UDP_GET_SOCKET:{
				// Open a server socket for remote client
				client->socket = UDPOpenEx(NULL,UDP_OPEN_SERVER,UDP_SERVICE_PORT,0);
				// Check
				if(client->socket == INVALID_UDP_SOCKET)
					break;
				// Now listen to it
				DBG_INFO("UDP LISTEN %u\r\n",clientIndex);
				client->state = UDP_LISTEN;
				// Fall through
			}	
			case UDP_LISTEN:{
				// Do nothing if no data is waiting
				size = UDPIsGetReady(client->socket);
				if(!size)break;
				
				// Debug
				char buffer[64];

				sktInfo = &UDPSocketInfo[client->socket];
				DBG_INFO("UDP RX: client:<%u>, on port:<%u>, length: <%u>\r\nDATA:<\r\n",
					clientIndex, 
					sktInfo->remotePort,
					size);
				if(size > 64) size = 64; 
				// Basic check for authentication
				if (UDPGetArray((void*)buffer,size))
				{
					char* ptr = buffer;
					while(((ptr-buffer)<64) && (*ptr!='p'))ptr++;
					if(strnicmp(ptr, "password",8)==0) 
						client->state = UDP_RESPOND;
				}
				DBG_WRITE(buffer,size);
				DBG_INFO(">\r\n");

				// Discard
				UDPDiscard(); 

				// Spammed
				if(client->state != UDP_RESPOND)
					break;
				
				// Now try response
				DBG_INFO("UDP TXING %u\r\n",clientIndex);;
				// Fall through
				client->timer = TickGet(); // Time from here for timeout
			}	
			case UDP_RESPOND: {
				// Try response
				size = UDPIsPutReady(client->socket);
				if(!size)
				{
					// If we cant respond in time, assume error
					if((TickGet() - client->timer)>TICK_SECOND)
					{	
						DBG_INFO("UDP RESP TIMEOUT %u\r\n",clientIndex);
						client->state = UDP_RESET; // socket is reset
					}
					break;
				}

				// Respond back to socket that contacted us
				sktInfo = &UDPSocketInfo[client->socket];
				if(UdpMakeEventPacket(UDP_NEW_CONNECTION, client->socket) > 0)
					UDPFlush();

				DBG_INFO("UDP OPEN %u\r\n",clientIndex);

				// Inform status
				status.udp_mask |= maskLocal;
				
				// Create connection notification event
				{
					char buffer[64];
					int index = 0;
					index += sprintf(buffer,"New UDP connection from %u.%u.%u.%u\r\n",
					sktInfo->remote.remoteNode.IPAddr.v[0],
					sktInfo->remote.remoteNode.IPAddr.v[1],
					sktInfo->remote.remoteNode.IPAddr.v[2],
					sktInfo->remote.remoteNode.IPAddr.v[3]);
					AddDataElement(TYPE_TEXT_ELEMENT,DATA_DEST_ALL, index, GetOpenStreams(), buffer);
				}
				// New state
				client->timer = TickGet(); // Time from here for timeout
				client->state = UDP_OPEN;
				// Fall through
			}
			case UDP_OPEN: {
				// We wait in this state listening
				size = UDPIsGetReady(client->socket);
				if(size) 
				{
					client->timer = TickGet(); 	// Restart timer
					UDPDiscard(); 				// Discard new data
				}

				// Check lease time
				if(((TickGet()) - client->timer) > (UDP_DATA_CONNECTION_LEASE_TIME))
				{
					DBG_INFO("UDP LEASE TIMEOUT %u\r\n",clientIndex);
					if(UdpMakeEventPacket(UDP_CONNECTION_TIMEOUT, client->socket)>0)
						UDPFlush();
					client->state = UDP_RESET; // socket is reset
				}
				// The client disconnect will reset the socket 
				break;
			}

			default: {
				client->state = UDP_RESET;
				break;
			}
		}// Switch
	}// For all clients
	
	// If there are any connections open and/or enabled update status
	if((status.udp_mask != 0) && (settings.udp_stream == STATE_ON))
		status.udp_state = ACTIVE;
	else if ((status.udp_mask != 0) || (settings.udp_stream == STATE_ON))
		status.udp_state = INITIALISED;
	else
		status.udp_state = OFF;

	// Done
	return;
}


// Simple device info
unsigned short UdpMakeEventPacket(unsigned char type, UDP_SOCKET socket)
{
	// First see if we can send
	UDP_SOCKET_INFO* sktInfo = &UDPSocketInfo[socket];
	unsigned short space = UDPIsPutReady(socket);
	unsigned short size, total = space;
	char buffer[64];

	// Check this first
	if(!space) return 0;
	
	size = sprintf(buffer,"BAX ROUTER: #%lu\r\n",settings.deviceId);
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);
	size = sprintf(buffer,"NETBIOS NAME: %s\r\n",AppConfig.NetBIOSName);
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);
	size = sprintf(buffer,"MY MAC: %02X:%02X:%02X:%02X:%02X:%02X:\r\n",
	AppConfig.MyMACAddr.v[0],AppConfig.MyMACAddr.v[1],AppConfig.MyMACAddr.v[2],
	AppConfig.MyMACAddr.v[3],AppConfig.MyMACAddr.v[4],AppConfig.MyMACAddr.v[5]);
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);
	size = sprintf(buffer,"MY IP: %u.%u.%u.%u\r\n",
	AppConfig.MyIPAddr.v[0],AppConfig.MyIPAddr.v[1],
	AppConfig.MyIPAddr.v[2],AppConfig.MyIPAddr.v[3]);
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);
	size = sprintf(buffer,"REPLYING TO IP: %u.%u.%u.%u\r\n",
	sktInfo->remote.remoteNode.IPAddr.v[0],
	sktInfo->remote.remoteNode.IPAddr.v[1],
	sktInfo->remote.remoteNode.IPAddr.v[2],
	sktInfo->remote.remoteNode.IPAddr.v[3]);
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);
	size = sprintf(buffer,"REPLYING TO MAC: %02X.%02X.%02X.%02X.%02X.%02X\r\n",
	sktInfo->remote.remoteNode.MACAddr.v[0],sktInfo->remote.remoteNode.MACAddr.v[1],
	sktInfo->remote.remoteNode.MACAddr.v[2],sktInfo->remote.remoteNode.MACAddr.v[2],
	sktInfo->remote.remoteNode.MACAddr.v[4],sktInfo->remote.remoteNode.MACAddr.v[5]);
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);
	size = sprintf(buffer,"TIME: %s\r\n",RtcToString(RtcNow()));
	if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
	else return (total - space);

	// Then write to UDP buffer as much as possible of response type
	switch(type){
		case UDP_BROADCAST:
				size = sprintf(buffer,"BROADCAST\r\n");
				if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
				break;
		case UDP_NEW_CONNECTION:
				size = sprintf(buffer,"WELCOME\r\n");
				if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
				break;
		case UDP_CONNECTION_TIMEOUT:
				size = sprintf(buffer,"GOODBYE\r\n");
				if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
				break;
		case UDP_CONNECTION_CLOSED:
				size = sprintf(buffer,"CLOSED\r\n");
				if(space >= size){UDPPutArray((void*)buffer, size);space -= size;}
				break;
		default: break;
	}
	// Done
	return (total - space);
}

// Transmit broadcast event packet
void UdpBroadcastService(void)
{
	UDP_SOCKET	socket;
	UDP_SOCKET_INFO* sktInfo;
	unsigned short timeout;

	// Check is connected
	if(!MACIsLinked()) return;

	// Open udp socket (let stack decide out port)
	socket = UDPOpenEx(0,UDP_OPEN_SERVER,0,UDP_BROADCAST_PORT);
	// Check it opened
	if(socket == INVALID_UDP_SOCKET) return;
	// Set to broadcast packet ip address and mac
	sktInfo = &UDPSocketInfo[socket];
	memset((void*)&sktInfo->remote.remoteNode, 0xff, sizeof(NODE_INFO));
	// Wait briefly if busy, 10ms
	timeout = 1000;// Wait briefly if busy, 10ms
	while(--timeout)
	{
		if(UDPIsPutReady(socket))
			break; 
		Delay10us(1);
	}
	// Check for timeout
	if(!timeout) return;
	// Debug out
	DBG_INFO("UDP BROADCAST\r\n");
	// Send broadcast packet if make packet worked
	if(UdpMakeEventPacket(UDP_BROADCAST, socket) > 0)
		UDPFlush();
	// Close socket
	UDPClose(socket);
	return;
}


#endif // #define ENABLE_UDP_SERVICES	
