// Minor changes by Karim Ladha, Copyright (C)2014 Newcastle University.

/*********************************************************************
*
*	Dynamic Host Configuration Protocol (DHCP) Client
*	Module for Microchip TCP/IP Stack
*	 -Provides automatic IP address, subnet mask, gateway address, 
*	  DNS server address, and other configuration parameters on DHCP 
*	  enabled networks.
*	 -Reference: RFC 2131, 2132
*
*********************************************************************
* FileName:		DHCP.c
* Dependencies:	UDP
* Processor:	PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
* Compiler:		Microchip C32 v1.05 or higher
*				Microchip C30 v3.12 or higher
*				Microchip C18 v3.30 or higher
*				HI-TECH PICC-18 PRO 9.63PL2 or higher
* Company:		Microchip Technology, Inc.
*
* Software License Agreement
*
* Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
* reserved.
*
* Microchip licenses to you the right to use, modify, copy, and
* distribute:
* (i)  the Software when embedded on a Microchip microcontroller or
*      digital signal controller product ("Device") which is
*      integrated into Licensee's product; or
* (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
*		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
*		used in conjunction with a Microchip ethernet controller for
*		the sole purpose of interfacing with the ethernet controller.
*
* You should refer to the license agreement accompanying this
* Software for additional information regarding your rights and
* obligations.
*
* THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
* WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
* LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
* MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
* CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
* PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
* BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
* THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
* SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
* (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
*
*
* Author               Date    Comment
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Nilesh Rajbharti     3/21/01  Original        (Rev 1.0)
* Nilesh Rajbharti     7/10/02  Explicitly initialized tempIPAddress
*                                               (Rev 2.11)
* Nilesh Rajbharti     5/16/03 Increased DHCP_TIMEOUT to 2 seconds.
* Nilesh Rajbharti     5/16/03 Fixed SM_DHCP_BROADCAST logic
*                              where UDPPut was called before setting
*                              active socket.
* Robert Sloan         5/29/03 Improved DHCP State machine to handle
*                              NAK and renew existing IP address.
* Nilesh Rajbharti     8/15/03 Modified _DHCPRecieve() to check for
*                              chaddr field before accpting the packet.
*                              Fixed DHCPTask() where it would not
*                              reply to first OFFER.
* Nilesh Rajbharti     3/1/04  Used tickDiff in DHCPTask() "bind"
*                              state to adjust for irregular TICK_SECOND
*                              Without this logic, actual lease time count
*                              down may be incorrect.
* Howard Schlunder		5/11/06	Fixed tickDiff usage, reducing 
*								accumulated timing error.  Fixed DHCP 
*								state machine requesting IP 0.0.0.0 
*								after lease expiration.
* Howard Schlunder		6/01/06	Added DHCPFlags.bits.bOfferReceived flag to 
*								allow operation on networks with multiple
*								DHCP servers offering multiple addresses
* Howard Schlunder		8/01/06 Added DNS server option to DHCP request, 
*								untested Host Name option to DHCP request
* Howard Schlunder		1/09/06	Fixed a DHCP renewal not renewing lease time bug
* Howard Schlunder		3/16/07 Rewrote DHCP state machine
*                       6/14/13 Increased DHCP_TIMEOUT to random exponential back-off.
********************************************************************/
#define __DHCP_C

#include "TCPIPConfig.h"

#if defined(STACK_USE_DHCP_CLIENT)

#include "TCPIP Stack/TCPIP.h"

// Defines how long to wait before a DHCP request times out
#define DHCP_BASE_TIMEOUT                (2ul)

// Unique variables per interface
typedef struct
{
	UDP_SOCKET			hDHCPSocket;	// Handle to DHCP client socket
	SM_DHCP				smState;		// DHCP client state machine variable
	union
	{
	    struct
	    {
	        unsigned char bIsBound : 1;				// Whether or not DHCP is currently bound
	        unsigned char bEvent : 1;				// Indicates to an external module that the DHCP client has been reset, has obtained new parameters via the DHCP client, or has refreshed a lease on existing ones
	        unsigned char bOfferReceived : 1;		// Whether or not an offer has been received
			unsigned char bDHCPServerDetected : 1;	// Indicates if a DCHP server has been detected
			unsigned char bUseUnicastMode : 1;		// Indicates if the 
	    } bits;
	    BYTE val;
	} flags;
	DWORD 				dwTimer;		// Tick timer value used for triggering future events after a certain wait period.
	DWORD                dwBaseTime;        // Base timer for timeouts in seconds
	DWORD				dwLeaseTime;	// DHCP lease time remaining, in seconds
	DWORD				dwServerID;		// DHCP Server ID cache
	IP_ADDR				tempIPAddress;	// Temporary IP address to use when no DHCP lease
	IP_ADDR				tempGateway;	// Temporary gateway to use when no DHCP lease
	IP_ADDR				tempMask;		// Temporary mask to use when no DHCP lease
	#if defined(STACK_USE_DNS)
	IP_ADDR				tempDNS;		// Temporary primary DNS server
	IP_ADDR				tempDNS2;		// Temporary secondary DNS server
	#endif	
	// Indicates which DHCP values are currently valid
	union
	{
		struct
		{
			char IPAddress:1;	// Leased IP address is valid
			char Gateway:1;		// Gateway address is valid
			char Mask:1;		// Subnet mask is valid
			char DNS:1;			// Primary DNS is valid
			char DNS2:1;		// Secondary DNS is valid
			char HostName:1;	// Host name is valid (not implemented)
		} bits;
		BYTE val;
	} validValues;
} DHCP_CLIENT_VARS;

BOOL DHCPClientInitializedOnce = FALSE;

static BYTE _DHCPReceive(void);
static void _DHCPSend(BYTE messageType, BOOL bRenewing);

#if defined (WF_CS_IO)
extern void SignalDHCPSuccessful(void);
extern void SetDhcpProgressState(void);
#endif

/*****************************************************************************
  Function:
	static void LoadState(BYTE vInterface)

  Summary:
	Saves the DHCPClient state information structure to the appropriate 
	location and loads DHCPClient with the state information for the specified 
	interface.

  Description:
	Saves the DHCPClient state information structure to the appropriate 
	location and loads DHCPClient with the state information for the specified 
	interface.

  Precondition:
	None

  Parameters:
	None

  Returns:
	None

  Remarks:
  	This function does nothing when you only have one physical interface.
***************************************************************************/
#if NETWORK_INTERFACES > 1

static DHCP_CLIENT_VARS	DHCPClients[NETWORK_INTERFACES];
static DHCP_CLIENT_VARS	*SelectedDHCPClient;
#define DHCPClient		(*SelectedDHCPClient)
#define LoadState(v)	do(SelectedDHCPClient = &DHCPClients[v])while(0)

#else

static DHCP_CLIENT_VARS DHCPClient;

#define LoadState(v)

#endif


/*****************************************************************************
  Function:
	void DHCPInit(BYTE vInterface)

  Summary:
	Resets the DHCP client module for the specified interface.

  Description:
	Resets the DHCP client module, giving up any current lease, knowledge of 
	DHCP servers, etc. for the specified interface.

  Precondition:
	None

  Parameters:
	vInterface - Interface number to initialize DHCP client state variables 
		for.   If you only have one interface, specify 0x00.

  Returns:
	None

  Remarks:
	This function may be called multiple times throughout the life of the 
	application, if desired.  
***************************************************************************/
void DHCPInit(BYTE vInterface)
{
	BYTE i;
	
	// Upon the first call after POR, we must reset all handles to invalid so 
	// that we don't inadvertently close someone else's handle.
	if(!DHCPClientInitializedOnce)
	{
		DHCPClientInitializedOnce = TRUE;
		for(i = 0; i < NETWORK_INTERFACES; i++)
		{
			LoadState(i);
			DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
		}		
	}
	
	
	LoadState(vInterface);
	
	if(DHCPClient.hDHCPSocket != INVALID_UDP_SOCKET)
	{
		UDPClose(DHCPClient.hDHCPSocket);
		DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
	}

	// Reset state machine and flags to default values
    DHCPClient.dwBaseTime = DHCP_BASE_TIMEOUT;
	DHCPClient.smState = SM_DHCP_GET_SOCKET;
	DHCPClient.flags.val = 0;
	DHCPClient.flags.bits.bUseUnicastMode = TRUE;	// This flag toggles before use, so this statement actually means to start out using broadcast mode.
	DHCPClient.flags.bits.bEvent = TRUE;
}


/*****************************************************************************
  Function:
	void DHCPDisable(BYTE vInterface)

  Summary:
	Disables the DHCP Client for the specified interface.

  Description:
	Disables the DHCP client for the specified interface by sending the state 
	machine to "SM_DHCP_DISABLED".  If the interface was previously configured 
	by DHCP, the configuration will continue to be used but the module will no 
	longer preform any renewals.

  Precondition:
	None

  Parameters:
	vInterface - Interface number to disable the DHCP client on.   If you only 
		have one interface, specify 0x00.

  Returns:
	None

  Remarks:
	Since the interface continues using its old configuration, it is possible 
	that the lease may expire and the DHCP server provide the IP to another
	client.  The application should replace the current IP address and other
	configuration with static information following a call to this function.
***************************************************************************/
void DHCPDisable(BYTE vInterface)
{
	LoadState(vInterface);

	if(DHCPClient.hDHCPSocket != INVALID_UDP_SOCKET)
	{
		UDPClose(DHCPClient.hDHCPSocket);
		DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
	}

	DHCPClient.smState = SM_DHCP_DISABLED;
}


/*****************************************************************************
  Function:
	void DHCPEnable(BYTE vInterface)

  Summary:
	Enables the DHCP client for the specified interface.

  Description:
	Enables the DHCP client for the specified interface, if it is disabled.  
	If it is already enabled, nothing is done.

  Precondition:
	None

  Parameters:
	vInterface - Interface number to enable the DHCP client on.   If you only 
		have one interface, specify 0x00.

  Returns:
	None
***************************************************************************/
void DHCPEnable(BYTE vInterface)
{
	LoadState(vInterface);

	if(DHCPClient.smState == SM_DHCP_DISABLED)
	{
        DHCPClient.dwBaseTime = DHCP_BASE_TIMEOUT;
		DHCPClient.smState = SM_DHCP_GET_SOCKET;
		DHCPClient.flags.bits.bIsBound = FALSE;
	}
}

/*****************************************************************************
  Function:
	BOOL DHCPIsEnabled(BYTE vInterface)

  Summary:
	Determins if the DHCP client is enabled on the specified interface.

  Description:
	Determins if the DHCP client is enabled on the specified interface.

  Precondition:
	None

  Parameters:
	vInterface - Interface number to query.   If you only have one interface, 
		specify 0x00.

  Returns:
	None
***************************************************************************/
BOOL DHCPIsEnabled(BYTE vInterface)
{
	LoadState(vInterface);
	return DHCPClient.smState != SM_DHCP_DISABLED;
}


/*****************************************************************************
  Function:
	BOOL DHCPIsBound(BYTE vInterface)

  Summary:
	Determins if the DHCP client has an IP address lease on the specified 
	interface.

  Description:
	Determins if the DHCP client has an IP address lease on the specified 
	interface.

  Precondition:
	None

  Parameters:
	vInterface - Interface number to query.   If you only have one interface, 
		specify 0x00.

  Returns:
	TRUE - DHCP client has obtained an IP address lease (and likely other 
		parameters) and these values are currently being used.
	FALSE - No IP address is currently leased
***************************************************************************/
BOOL DHCPIsBound(BYTE vInterface)
{
	LoadState(vInterface);
	return DHCPClient.flags.bits.bIsBound;
}

/*****************************************************************************
  Function:
	BOOL DHCPStateChanged(BYTE vInterface)

  Summary:
	Determins if the DHCP client on the specified interface has changed states 
	or refreshed its IP address lease.

  Description:
	Determins if the DHCP client on the specified interface has changed states 
	or refreshed its IP address lease.  This function can be used to determine
	when to update an LCD or other display whenever the DHCP assigned IP 
	address has potentially changed.
	
  Precondition:
	None

  Parameters:
	vInterface - Interface number to query.   If you only have one interface, 
		specify 0x00.

  Returns:
	TRUE - The IP address lease have been reliquished (due to reinitilization), 
		obtained (first event), or renewed since the last call to 
		DHCPStateChanged().
	FALSE - The DHCP client has not detected any changes since the last call to 
		DHCPStateChanged().
***************************************************************************/
BOOL DHCPStateChanged(BYTE vInterface)
{
	LoadState(vInterface);
	if(DHCPClient.flags.bits.bEvent)
	{
		DHCPClient.flags.bits.bEvent = 0;
		return TRUE;
	}
	return FALSE;
}


/*****************************************************************************
  Function:
	BOOL DHCPIsServerDetected(BYTE vInterface)

  Summary:
	Determins if the DHCP client on the specified interface has seen a DHCP 
	server.

  Description:
	Determins if the DHCP client on the specified interface has seen a DHCP 
	server.
	
  Precondition:
	None

  Parameters:
	vInterface - Interface number to query.   If you only have one interface, 
		specify 0x00.

  Returns:
	TRUE - At least one DHCP server is attached to the specified network 
		interface.
	FALSE - No DHCP servers are currently detected on the specified network 
		interface.
***************************************************************************/
BOOL DHCPIsServerDetected(BYTE vInterface)
{
	LoadState(vInterface);
	return DHCPClient.flags.bits.bDHCPServerDetected;
}

/*****************************************************************************
  Function:
	void DHCPTempIPAddr(void)

  Summary:
	Copy DHCPClient.tempIPAddress into AppConfig.MyIPAddr.  SOFTAP_ZEROCONF_SUPPORT.

  Description:
  	Copy DHCPClient.tempIPAddress into AppConfig.MyIPAddr. 
	
  Precondition:
	None

  Parameters:
	None

  Returns:
	None
***************************************************************************/
void DHCPTempIPAddr(void)
{
	AppConfig.MyIPAddr = DHCPClient.tempIPAddress;  
}


/*****************************************************************************
  Function:
	void DHCPTask(void)

  Summary:
	Performs periodic DHCP tasks for all interfaces.

  Description:
	This function performs any periodic tasks requied by the DHCP module, 
	such as sending and receiving messages involved with obtaining and
	maintaining a lease.

  Precondition:
	None

  Parameters:
	None

  Returns:
	None
***************************************************************************/
void DHCPTask(void)
{
	BYTE i;
	
	for(i = 0; i < NETWORK_INTERFACES; i++)
	{
		LoadState(i);
		switch(DHCPClient.smState)
		{
			case SM_DHCP_DISABLED:
				// When the module is disabled, do absolutely nothing
				break;
			
			case SM_DHCP_GET_SOCKET:
				// Open a socket to send and receive broadcast messages on
				//DHCPClient.hDHCPSocket = UDPOpen(DHCP_CLIENT_PORT, NULL, DHCP_SERVER_PORT);
				
				//putrsUART("\r\nDHCPTask: SM_DHCP_GET_SOCKET\r\n");	 
				
				DHCPClient.hDHCPSocket = UDPOpenEx(0,UDP_OPEN_SERVER,DHCP_CLIENT_PORT, DHCP_SERVER_PORT);
				if(DHCPClient.hDHCPSocket == INVALID_UDP_SOCKET)
					break;
	
				DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
				// No break
	
			case SM_DHCP_SEND_DISCOVERY:
				// Assume default IP Lease time of 60 seconds.
				// This should be minimum possible to make sure that if the
				// server did not specify lease time, we try again after this 
				// minimum time.
				DHCPClient.dwLeaseTime = 60;
				DHCPClient.validValues.val = 0x00;
				DHCPClient.flags.bits.bIsBound = FALSE;	
				DHCPClient.flags.bits.bOfferReceived = FALSE;
	
				// putrsUART("DHCPTask: SM_DHCP_SEND_DISCOVERY\r\n");     

				// No point in wasting time transmitting a discovery if we are 
				// unlinked.  No one will see it.  
				if(!MACIsLinked())
					break;
	
				// Ensure transmitter is ready to accept data
				if(UDPIsPutReady(DHCPClient.hDHCPSocket) < 300u)
					break;

				// Toggle the BOOTP Broadcast flag to ensure compatibility with 
				// bad DHCP servers that don't know how to handle broadcast 
				// responses.  This results in the next discovery attempt to be 
				// made using the opposite mode.
				DHCPClient.flags.bits.bUseUnicastMode ^= 1;
	
				// Ensure that we transmit to the broadcast IP and MAC addresses
				// The UDP Socket remembers who it was last talking to
				memset((void*)&UDPSocketInfo[DHCPClient.hDHCPSocket].remote.remoteNode, 0xFF, sizeof(UDPSocketInfo[0].remote.remoteNode));
	
				// Send the DHCP Discover broadcast
				_DHCPSend(DHCP_DISCOVER_MESSAGE, FALSE);
	
				// Start a timer and begin looking for a response
                DHCPClient.dwTimer = TickGet() + ((DHCPClient.dwBaseTime * TICK_SECOND) + (LFSRRand() % TICK_SECOND));
				DHCPClient.smState = SM_DHCP_GET_OFFER;
				break;
	
			case SM_DHCP_GET_OFFER:
				// Check to see if a packet has arrived
				if(UDPIsGetReady(DHCPClient.hDHCPSocket) < 250u)
				{
					// Go back and transmit a new discovery if we didn't get an offer after the timeout
                    if((long)(TickGet() - DHCPClient.dwTimer) > 0)
                    {
                        // Double the backoff time
                        if (DHCPClient.dwBaseTime < 64ul)
                        {
                            DHCPClient.dwBaseTime <<= 1;
                        }
                        DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                    }
					break;
				}
	
				// Let the DHCP server module know that there is a DHCP server 
				// on this network
				DHCPClient.flags.bits.bDHCPServerDetected = TRUE;
	
				// Check to see if we received an offer
				if(_DHCPReceive() != DHCP_OFFER_MESSAGE)
					break;
	
				// putrsUART("DHCPTask: SM_DHCP_GET_OFFER: Receive offer. Go to SM_DHCP_SEND_REQUEST \r\n");     
	
				DHCPClient.smState = SM_DHCP_SEND_REQUEST;
				// No break
	
			case SM_DHCP_SEND_REQUEST:
				if(UDPIsPutReady(DHCPClient.hDHCPSocket) < 258u)
					break;

				// Ensure that we transmit to the broadcast IP and MAC addresses
				// The UDP Socket remembers who it was last talking to, so 
				// we must set this back to the broadcast address since the 
				// current socket values are the unicast addresses of the DHCP 
				// server.
				memset((void*)&UDPSocketInfo[DHCPClient.hDHCPSocket].remote.remoteNode, 0xFF, sizeof(UDPSocketInfo[0].remote.remoteNode));
	
				// Send the DHCP request message
				_DHCPSend(DHCP_REQUEST_MESSAGE, FALSE);
	
				// putrsUART("DHCPTask: SM_DHCP_SEND_REQUEST \r\n");	 
				
				// Start a timer and begin looking for a response
                DHCPClient.dwTimer = TickGet() + ((DHCPClient.dwBaseTime * TICK_SECOND) + (LFSRRand() % TICK_SECOND));;
				DHCPClient.smState = SM_DHCP_GET_REQUEST_ACK;
				break;
	
			case SM_DHCP_GET_REQUEST_ACK:
				// Check to see if a packet has arrived
				if(UDPIsGetReady(DHCPClient.hDHCPSocket) < 250u)
				{
					// Go back and transmit a new discovery if we didn't get an ACK after the timeout
					if((long)(TickGet() - DHCPClient.dwTimer) > 0)
                    {
                        if (DHCPClient.dwBaseTime < 64)
                        {
                            DHCPClient.dwBaseTime <<= 1;
                        }
                        DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                    }
					break;
				}
	
				// Check to see if we received an offer
				switch(_DHCPReceive())
				{
					case DHCP_ACK_MESSAGE:
						UDPClose(DHCPClient.hDHCPSocket);
						DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
						DHCPClient.dwTimer = TickGet();
						DHCPClient.smState = SM_DHCP_BOUND;
						DHCPClient.flags.bits.bEvent = 1;
						DHCPClient.flags.bits.bIsBound = TRUE;	

						// putrsUART("DHCPTask: SM_DHCP_GET_REQUEST_ACK: Receive DHCP_ACK_MESSAGE \r\n");	 

						if(DHCPClient.validValues.bits.IPAddress)
						{
							AppConfig.MyIPAddr = DHCPClient.tempIPAddress;
							
							#if defined(WF_CS_IO) 
							    #if defined(STACK_USE_UART )
							        putrsUART("DHCP client successful\r\n");
							    #endif
    							SignalDHCPSuccessful();
							#endif
							
						}	
						if(DHCPClient.validValues.bits.Mask)
							AppConfig.MyMask = DHCPClient.tempMask;
						if(DHCPClient.validValues.bits.Gateway)
							AppConfig.MyGateway = DHCPClient.tempGateway;
						#if defined(STACK_USE_DNS)
							if(DHCPClient.validValues.bits.DNS)
								AppConfig.PrimaryDNSServer.Val = DHCPClient.tempDNS.Val;
							AppConfig.SecondaryDNSServer.Val = 0x00000000ul;
							if(DHCPClient.validValues.bits.DNS2)
								AppConfig.SecondaryDNSServer.Val = DHCPClient.tempDNS2.Val;
						#endif
						//if(DHCPClient.validValues.bits.HostName)
						//	memcpy(AppConfig.NetBIOSName, (void*)DHCPClient.tempHostName, sizeof(AppConfig.NetBIOSName));
	
						break;
	
					case DHCP_NAK_MESSAGE:
						// putrsUART("DHCPTask: SM_DHCP_GET_REQUEST_ACK: Receive DHCP_NAK_MESSAGE \r\n");	 
						DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
						break;
				}
				break;
	
			case SM_DHCP_BOUND:
				if(TickGet() - DHCPClient.dwTimer < TICK_SECOND)
					break;
	
				// Check to see if our lease is still valid, if so, decrement lease 
				// time
                if(DHCPClient.dwLeaseTime > (DHCPClient.dwBaseTime * 3))
				{
					DHCPClient.dwTimer += TICK_SECOND;
					DHCPClient.dwLeaseTime--;
					break;
				}
	
				// Open a socket to send and receive DHCP messages on
				//DHCPClient.hDHCPSocket = UDPOpen(DHCP_CLIENT_PORT, NULL, DHCP_SERVER_PORT);
				
				DHCPClient.hDHCPSocket = UDPOpenEx(0,UDP_OPEN_SERVER,DHCP_CLIENT_PORT, DHCP_SERVER_PORT);
				if(DHCPClient.hDHCPSocket == INVALID_UDP_SOCKET)
					break;
	
				// putrsUART("DHCPTask: SM_DHCP_BOUND -> SM_DHCP_SEND_RENEW  \r\n");	 
	
				DHCPClient.smState = SM_DHCP_SEND_RENEW;
				
				// No break
	
			case SM_DHCP_SEND_RENEW:
			case SM_DHCP_SEND_RENEW2:
			case SM_DHCP_SEND_RENEW3:
				if(UDPIsPutReady(DHCPClient.hDHCPSocket) < 258u)
					break;

                                #if defined(WF_CS_IO)
                                    SetDhcpProgressState();
                                #endif
				
				// Send the DHCP request message
				_DHCPSend(DHCP_REQUEST_MESSAGE, TRUE);
				DHCPClient.flags.bits.bOfferReceived = FALSE;
	
				// Start a timer and begin looking for a response
                DHCPClient.dwTimer = TickGet() + ((DHCPClient.dwBaseTime * TICK_SECOND) + (LFSRRand() % TICK_SECOND));
				DHCPClient.smState++;
				break;
	
			case SM_DHCP_GET_RENEW_ACK:
			case SM_DHCP_GET_RENEW_ACK2:
			case SM_DHCP_GET_RENEW_ACK3:
				// Check to see if a packet has arrived
				if(UDPIsGetReady(DHCPClient.hDHCPSocket) < 250u)
				{
                    // Go back and transmit a new discovery if we didn't get an ACK after the timeout
                    if((long)(TickGet() - DHCPClient.dwTimer) > 0)
					{
						if(++DHCPClient.smState > SM_DHCP_GET_RENEW_ACK3)
						{
							DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
							//putrsUART("DHCPTask: SM_DHCP_GET_RENEW_ACK3 timeout -> SM_DHCP_SEND_DISCOVERY	\r\n");  
						}
					}
					break;
				}
	
				// Check to see if we received an offer
				switch(_DHCPReceive())
				{
					case DHCP_ACK_MESSAGE:
						UDPClose(DHCPClient.hDHCPSocket);
						DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
						DHCPClient.dwTimer = TickGet();
						DHCPClient.smState = SM_DHCP_BOUND;
						DHCPClient.flags.bits.bEvent = 1;
						//putrsUART("DHCPTask: SM_DHCP_GET_RENEW_ACK2/3: Receive DHCP_ACK_MESSAGE \r\n");  
						break;
		
					case DHCP_NAK_MESSAGE:
						//putrsUART("DHCPTask: SM_DHCP_GET_RENEW_ACK2/3: Receive DHCP_NAK_MESSAGE \r\n");  
						DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
						break;
				}
				break;
		}
	}
}



/*****************************************************************************
Function:
  void _DHCPReceive(void)

Description:
  Receives and parses a DHCP message.

Precondition:
  A DHCP message is waiting in the UDP buffer.

Parameters:
  None

Returns:
  One of the DCHP_TYPE* contants.
***************************************************************************/
static BYTE _DHCPReceive(void)
{
	/*********************************************************************
	DHCP PACKET FORMAT AS PER RFC 1541

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
	+---------------+---------------+---------------+---------------+
	|                            xid (4)                            |
	+-------------------------------+-------------------------------+
	|           secs (2)            |           flags (2)           |
	+-------------------------------+-------------------------------+
	|                          ciaddr  (4)                          |
	+---------------------------------------------------------------+
	|                          yiaddr  (4)                          |
	+---------------------------------------------------------------+
	|                          siaddr  (4)                          |
	+---------------------------------------------------------------+
	|                          giaddr  (4)                          |
	+---------------------------------------------------------------+
	|                                                               |
	|                          chaddr  (16)                         |
	|                                                               |
	|                                                               |
	+---------------------------------------------------------------+
	|                                                               |
	|                          sname   (64)                         |
	+---------------------------------------------------------------+
	|                                                               |
	|                          file    (128)                        |
	+---------------------------------------------------------------+
	|                                                               |
	|                          options (312)                        |
	+---------------------------------------------------------------+

	********************************************************************/
	BYTE v;
	BYTE i, j;
	BYTE type;
	BOOL lbDone;
	DWORD tempServerID;


	// Assume unknown message until proven otherwise.
	type = DHCP_UNKNOWN_MESSAGE;

	UDPGet(&v);                             // op

	// Make sure this is BOOT_REPLY.
	if ( v == BOOT_REPLY )
	{
		// Jump to chaddr field (Client Hardware Address -- our MAC address for 
		// Ethernet and WiFi networks) and verify that this message is directed 
		// to us before doing any other processing.
		UDPSetRxBuffer(28);		// chaddr field is at offset 28 in the UDP packet payload -- see DHCP packet format above
		for ( i = 0; i < 6u; i++ )
		{
			UDPGet(&v);
			if ( v != AppConfig.MyMACAddr.v[i])
				goto UDPInvalid;
		}

		// Check to see if this is the first offer.  If it is, record its 
		// yiaddr value ("Your (client) IP address") so that we can REQUEST to 
		// use it later.
		if(!DHCPClient.flags.bits.bOfferReceived)
		{
			UDPSetRxBuffer(16);
			UDPGetArray((BYTE*)&DHCPClient.tempIPAddress, sizeof(DHCPClient.tempIPAddress));
			DHCPClient.validValues.bits.IPAddress = 1;
		}

		// Jump to DHCP options (ignore htype, hlen, hops, xid, secs, flags, 
		// ciaddr, siaddr, giaddr, padding part of chaddr, sname, file, magic 
		// cookie fields)
		UDPSetRxBuffer(240);

		lbDone = FALSE;
		do
		{
			// Get the Option number
			// Break out eventually in case if this is a malformed 
			// DHCP message, ie: missing DHCP_END_OPTION marker
			if(!UDPGet(&v))
			{
				lbDone = TRUE;
				break;
			}

			switch(v)
			{
				case DHCP_MESSAGE_TYPE:
					UDPGet(&v);                         // Skip len
					// Len must be 1.
					if ( v == 1u )
					{
						UDPGet(&type);                  // Get type

						// Throw away the packet if we know we don't need it (ie: another offer when we already have one)
						if(DHCPClient.flags.bits.bOfferReceived && (type == DHCP_OFFER_MESSAGE))
						{
							goto UDPInvalid;
						}
					}
					else
						goto UDPInvalid;
					break;

				case DHCP_SUBNET_MASK:
					UDPGet(&v);                     // Skip len
					// Len must be 4.
					if ( v == 4u )
					{
						// Check to see if this is the first offer
						if(DHCPClient.flags.bits.bOfferReceived)
						{
							// Discard offered IP mask, we already have an offer
							for ( i = 0; i < 4u; i++ )
								UDPGet(&v);
						}
						else
						{
							UDPGetArray((BYTE*)&DHCPClient.tempMask, sizeof(DHCPClient.tempMask));
							DHCPClient.validValues.bits.Mask = 1;
						}
					}
					else
						goto UDPInvalid;
					break;

				case DHCP_ROUTER:
					UDPGet(&j);
					// Len must be >= 4.
					if ( j >= 4u )
					{
						// Check to see if this is the first offer
						if(DHCPClient.flags.bits.bOfferReceived)
						{
							// Discard offered Gateway address, we already have an offer
							for ( i = 0; i < 4u; i++ )
								UDPGet(&v);
						}
						else
						{
							UDPGetArray((BYTE*)&DHCPClient.tempGateway, sizeof(DHCPClient.tempGateway));
							DHCPClient.validValues.bits.Gateway = 1;
						}
					}
					else
						goto UDPInvalid;

					// Discard any other router addresses.
					j -= 4;
					while(j--)
						UDPGet(&v);
					break;

				#if defined(STACK_USE_DNS)
				case DHCP_DNS:
					UDPGet(&j);
					// Len must be >= 4.
					if(j < 4u)
						goto UDPInvalid;

					// Check to see if this is the first offer
					if(!DHCPClient.flags.bits.bOfferReceived)
					{
						UDPGetArray((BYTE*)&DHCPClient.tempDNS, sizeof(DHCPClient.tempDNS));
						DHCPClient.validValues.bits.DNS = 1;
						j -= 4;
					}

					// Len must be >= 4 for a secondary DNS server address
					if(j >= 4u)
					{
						// Check to see if this is the first offer
						if(!DHCPClient.flags.bits.bOfferReceived)
						{
							UDPGetArray((BYTE*)&DHCPClient.tempDNS2, sizeof(DHCPClient.tempDNS2));
							DHCPClient.validValues.bits.DNS2 = 1;
							j -= 4;
						}
					}

					// Discard any other DNS server addresses
					while(j--)
						UDPGet(&v);
					break;
				#endif

					//            case DHCP_HOST_NAME:
					//                UDPGet(&j);
					//                // Len must be >= 4.
					//                if(j < 1u)
					//					goto UDPInvalid;
					//
					//				// Check to see if this is the first offer
					//				if(DHCPFlags.bits.bOfferReceived)
					//				{
					//			        // Discard offered host name, we already have an offer
					//	                while(j--)
					//	                    UDPGet(&v);
					//				}
					//				else
					//				{
					//					for(i = 0; j, i < sizeof(tempHostName); i++, j--)
					//					{
					//						UDPGet(&tempHostName[i]);
					//					}
					//					while(j--)
					//					{
					//						UDPGet(&v);
					//					}
					//					ValidValues.bits.HostName = 1;
					//				}
					//
					//                break;

				case DHCP_SERVER_IDENTIFIER:
					UDPGet(&v);                         // Get len
					// Len must be 4.
					if ( v == 4u )
					{
						UDPGet(&(((BYTE*)&tempServerID)[3]));   // Get the id
						UDPGet(&(((BYTE*)&tempServerID)[2]));
						UDPGet(&(((BYTE*)&tempServerID)[1]));
						UDPGet(&(((BYTE*)&tempServerID)[0]));
					}
					else
						goto UDPInvalid;
					break;

				case DHCP_END_OPTION:
					lbDone = TRUE;
					break;

				case DHCP_IP_LEASE_TIME:
					UDPGet(&v);                         // Get len
					// Len must be 4.
					if ( v == 4u )
					{
						// Check to see if this is the first offer
						if(DHCPClient.flags.bits.bOfferReceived)
						{
							// Discard offered lease time, we already have an offer
							for ( i = 0; i < 4u; i++ )
								UDPGet(&v);
						}
						else
						{
							UDPGet(&(((BYTE*)(&DHCPClient.dwLeaseTime))[3]));
							UDPGet(&(((BYTE*)(&DHCPClient.dwLeaseTime))[2]));
							UDPGet(&(((BYTE*)(&DHCPClient.dwLeaseTime))[1]));
							UDPGet(&(((BYTE*)(&DHCPClient.dwLeaseTime))[0]));

							// In case if our clock is not as accurate as the remote 
							// DHCP server's clock, let's treat the lease time as only 
							// 96.875% of the value given
							DHCPClient.dwLeaseTime -= DHCPClient.dwLeaseTime>>5;
						}
					}
					else
						goto UDPInvalid;
					break;

				default:
					// Ignore all unsupport tags.
					UDPGet(&j);                     // Get option len
					while( j-- )                    // Ignore option values
						UDPGet(&v);
			}
		} while( !lbDone );
	}

	// If this is an OFFER message, remember current server id.
	if ( type == DHCP_OFFER_MESSAGE )
	{
		DHCPClient.dwServerID = tempServerID;
		DHCPClient.flags.bits.bOfferReceived = TRUE;
	}
	else
	{
		// For other types of messages, make sure that received
		// server id matches with our previous one.
		if ( DHCPClient.dwServerID != tempServerID )
			type = DHCP_UNKNOWN_MESSAGE;
	}

	UDPDiscard();                             // We are done with this packet
	return type;

UDPInvalid:
	UDPDiscard();
	return DHCP_UNKNOWN_MESSAGE;
}




/*****************************************************************************
  Function:
	static void _DHCPSend(BYTE messageType, BOOL bRenewing)

  Description:
	Sends a DHCP message.

  Precondition:
	UDP is ready to write a DHCP packet.

  Parameters:
	messageType - One of the DHCP_TYPE constants
	bRenewing - Whether or not this is a renewal request

  Returns:
	None
***************************************************************************/
static void _DHCPSend(BYTE messageType, BOOL bRenewing)
{
	BYTE i;
	IP_ADDR	MyIP;


	UDPPut(BOOT_REQUEST);                       // op
	UDPPut(BOOT_HW_TYPE);                       // htype
	UDPPut(BOOT_LEN_OF_HW_TYPE);                // hlen
	UDPPut(0);                                  // hops
	UDPPut(0x12);                               // xid[0]
	UDPPut(0x23);                               // xid[1]
	UDPPut(0x34);                               // xid[2]
	UDPPut(0x56);                               // xid[3]
	UDPPut(0);                                  // secs[0]
	UDPPut(0);                                  // secs[1]
	UDPPut(DHCPClient.flags.bits.bUseUnicastMode ? 0x00: 0x80);// flags[0] with Broadcast flag clear/set to correspond to bUseUnicastMode
	UDPPut(0);                                  // flags[1]

	// If this is DHCP REQUEST message, use previously allocated IP address.
	if((messageType == DHCP_REQUEST_MESSAGE) && bRenewing)
	{
		UDPPutArray((BYTE*)&DHCPClient.tempIPAddress, sizeof(DHCPClient.tempIPAddress));
	}
	else
	{
		UDPPut(0x00);
		UDPPut(0x00);
		UDPPut(0x00);
		UDPPut(0x00);
	}

	// Set yiaddr, siaddr, giaddr as zeros,
	for ( i = 0; i < 12u; i++ )
		UDPPut(0x00);

	// Load chaddr - Client hardware address.
	UDPPutArray((BYTE*)&AppConfig.MyMACAddr, sizeof(AppConfig.MyMACAddr));

	// Set chaddr[6..15], sname and file as zeros.
	for ( i = 0; i < 202u; i++ )
		UDPPut(0);

	// Load magic cookie as per RFC 1533.
	UDPPut(99);
	UDPPut(130);
	UDPPut(83);
	UDPPut(99);

	// Load message type.
	UDPPut(DHCP_MESSAGE_TYPE);
	UDPPut(DHCP_MESSAGE_TYPE_LEN);
	UDPPut(messageType);

	if(messageType == DHCP_DISCOVER_MESSAGE)
	{
		// Reset offered flag so we know to act upon the next valid offer
		DHCPClient.flags.bits.bOfferReceived = FALSE;
	}


	if((messageType == DHCP_REQUEST_MESSAGE) && !bRenewing)
	{
		// DHCP REQUEST message must include server identifier the first time
		// to identify the server we are talking to.
		// _DHCPReceive() would populate "serverID" when it
		// receives DHCP OFFER message. We will simply use that
		// when we are replying to server.
		// If this is a renwal request, we must not include server id.
		UDPPut(DHCP_SERVER_IDENTIFIER);
		UDPPut(DHCP_SERVER_IDENTIFIER_LEN);
		UDPPut(((BYTE*)(&DHCPClient.dwServerID))[3]);
		UDPPut(((BYTE*)(&DHCPClient.dwServerID))[2]);
		UDPPut(((BYTE*)(&DHCPClient.dwServerID))[1]);
		UDPPut(((BYTE*)(&DHCPClient.dwServerID))[0]);
	}

	// Load our interested parameters
	// This is hardcoded list.  If any new parameters are desired,
	// new lines must be added here.
	UDPPut(DHCP_PARAM_REQUEST_LIST);
	UDPPut(DHCP_PARAM_REQUEST_LIST_LEN);
	UDPPut(DHCP_SUBNET_MASK);
	UDPPut(DHCP_ROUTER);
	UDPPut(DHCP_DNS);
	UDPPut(DHCP_HOST_NAME);

	// Add requested IP address to DHCP Request Message
	if( ((messageType == DHCP_REQUEST_MESSAGE) && !bRenewing) || 
		((messageType == DHCP_DISCOVER_MESSAGE) && DHCPClient.tempIPAddress.Val))
	{
		UDPPut(DHCP_PARAM_REQUEST_IP_ADDRESS);
		UDPPut(DHCP_PARAM_REQUEST_IP_ADDRESS_LEN);
		UDPPutArray((BYTE*)&DHCPClient.tempIPAddress, DHCP_PARAM_REQUEST_IP_ADDRESS_LEN);
	}

	// Add any new paramter request here.

	// Desired netbios name - may not be granted
#ifdef DHCP_REQ_NAME
	UDPPut(DHCP_HOST_NAME);
	UDPPut(strlen((const char*)AppConfig.NetBIOSName));
	for (i = 0; i < strlen((const char*)AppConfig.NetBIOSName); i++) 
		UDPPut(AppConfig.NetBIOSName[i]);
#endif

	// End of Options.
	UDPPut(DHCP_END_OPTION);

	// Add zero padding to ensure compatibility with old BOOTP relays that discard small packets (<300 UDP octets)
	while(UDPTxCount < 300u)
		UDPPut(0); 

	// Make sure we advertise a 0.0.0.0 IP address so all DHCP servers will respond.  If we have a static IP outside the DHCP server's scope, it may simply ignore discover messages.
	MyIP.Val = AppConfig.MyIPAddr.Val;
	if(!bRenewing)
		AppConfig.MyIPAddr.Val = 0x00000000;
	UDPFlush();
	AppConfig.MyIPAddr.Val = MyIP.Val;

}


#endif	//#if defined(STACK_USE_DHCP_CLIENT)
