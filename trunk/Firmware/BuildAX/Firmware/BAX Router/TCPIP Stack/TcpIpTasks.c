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

// TCP/IP Tasks
// Karim Ladha, 2013-2014

// Headers
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/TelnetLocal.h"
#include "TCPIP Stack/TinyHTTP.h"
#include "TCPIP Stack/UDP services local.h"
#include "TcpIpTasks.h"

// Prototypes
void PingDemo(void);

// Globals
APP_CONFIG AppConfig;

// Source
void InitAppConfig(void)
{

		#ifdef RAND_MAC
		if(settings.mac[6] != 1)
		{
			unsigned char i;
			srand(settings.start_time);
			for(i=0;i<6;i++){settings.mac[i]=rand();}
			settings.mac[0] = 0x02;	// Locally administered bit set
			settings.mac[1] = 0x04; // OUI = Microchip Inc.
			settings.mac[2] = 0xA3;
			settings.mac[6] = 0x01; // set configured flag
			WriteNvm(SETTINGS_ADDRESS + ((void*)settings.mac - (void*)&settings),settings.mac,7);
			SettingsSave();
		}
		#else
		settings.mac[0] = MY_DEFAULT_MAC_BYTE1;settings.mac[1] = MY_DEFAULT_MAC_BYTE2;settings.mac[2] = MY_DEFAULT_MAC_BYTE3;
		settings.mac[3] = MY_DEFAULT_MAC_BYTE4;settings.mac[4] = MY_DEFAULT_MAC_BYTE5;settings.mac[5] = MY_DEFAULT_MAC_BYTE6;
		#endif
		
		APP_CONFIG AppConfigLocal = {
		.MyMACAddr.v = {settings.mac[0], settings.mac[1], settings.mac[2], settings.mac[3], settings.mac[4], settings.mac[5]},
		.Flags.bIsDHCPEnabled = TRUE,
		.Flags.bInConfigMode = TRUE,
		.MyIPAddr.v = {MY_DEFAULT_IP_ADDR_BYTE1 , MY_DEFAULT_IP_ADDR_BYTE2, MY_DEFAULT_IP_ADDR_BYTE3 , MY_DEFAULT_IP_ADDR_BYTE4},
		.DefaultIPAddr.v = {MY_DEFAULT_IP_ADDR_BYTE1 , MY_DEFAULT_IP_ADDR_BYTE2, MY_DEFAULT_IP_ADDR_BYTE3 , MY_DEFAULT_IP_ADDR_BYTE4},
		.MyMask.v = {MY_DEFAULT_MASK_BYTE1 , MY_DEFAULT_MASK_BYTE2, MY_DEFAULT_MASK_BYTE3 , MY_DEFAULT_MASK_BYTE4},
		.MyGateway.v = {MY_DEFAULT_GATE_BYTE1 , MY_DEFAULT_GATE_BYTE2, MY_DEFAULT_GATE_BYTE3 , MY_DEFAULT_GATE_BYTE4},
		.PrimaryDNSServer.v = {MY_DEFAULT_PRIMARY_DNS_BYTE1 , MY_DEFAULT_PRIMARY_DNS_BYTE2 , MY_DEFAULT_PRIMARY_DNS_BYTE3  , MY_DEFAULT_PRIMARY_DNS_BYTE4},
		.NetBIOSName = MY_DEFAULT_HOST_NAME
		};

	// Make net bios name space padded to 15 chars
	memcpy(AppConfigLocal.NetBIOSName,settings.device_name,strlen(settings.device_name));
	FormatNetBIOSName(AppConfigLocal.NetBIOSName);	

	// Initialise global
	memcpy(&AppConfig, &AppConfigLocal, sizeof(APP_CONFIG));
	
	return;
}

void TcipipTasks(void)
{

	if(status.ethernet_state < INITIALISED)
	{
		// Setup the local application variables, default ip, MAC address etc.
		InitAppConfig();
		// Init Telnet to all disconnected before resetting HW
#if defined(STACK_USE_TELNET_SERVER)		
		TelnetInit();
#endif 
#ifdef STACK_USE_TINYHTTP_SERVER
		// Init server
		HTTPInit();
#endif
#ifdef ENABLE_UDP_SERVICES
	UdpInit();
#endif
// Other modules initied in stackinit()
		// Initialize core stack layers (MAC, ARP, TCP, UDP) and
		// application modules (HTTP, SNMP, etc.)
		StackInit();
		status.ethernet_state = INITIALISED;
	}
	
	// This task performs normal stack task including checking
	// for incoming packet, type of packet and calling
	// appropriate stack entity to process it.
	StackTask();

	#if defined(STACK_USE_TELNET_SERVER)
	TelnetTask();
	#endif
	
	#ifdef STACK_USE_TINYHTTP_SERVER
	HTTPServer();
	#endif

	#if defined(STACK_USE_NBNS)
	NBNSTask();
	#endif

	#if defined(ENABLE_UDP_SERVICES)
	UdpRemoteDiscovery();
	#endif

	{	
		// Low freq tasks
		static DWORD last_time = 0;
		DWORD now = TickGet(); 
		if((now - last_time)> TICK_SECOND)
		{
			last_time = now;
	
			// Set global status variable
			if((status.ethernet_state == INITIALISED) && (!AppConfig.Flags.bInConfigMode))
				status.ethernet_state = ACTIVE;
			else if((status.ethernet_state == ACTIVE) && (AppConfig.Flags.bInConfigMode))
				status.ethernet_state = INITIALISED;

			#if defined(ENABLE_UDP_SERVICES)
			{
				static DWORD lastBcast = 0;
				if(	((now - lastBcast) > (TICK_MINUTE)) &&
					(settings.udp_stream == STATE_ON))
				{
					lastBcast = now;
		    		UdpBroadcastService();
				}
			}
			#endif
			// Add other tasks here...
		}
	}
	
	
	#if defined(STACK_USE_ICMP_CLIENT)
	PingDemo();
	#endif

	return;
}
//EOF
