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

// Telnet server
// Karim Ladha, 2013-2014
// Based on code from Microchip's telnet example by Howard Schlunder 9/12/06

#include "Settings.h"
#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/TcpIpTasks.h"
#include "TCPIP Stack/TelnetLocal.h"
#include "Peripherals/Rtc.h"
#include "Utils/Util.h"
#include "Data.h"

//#define DEBUG_ON
#include "Common/debug.h"

#if defined(STACK_USE_TELNET_SERVER)

// Const strings
const char startString[]		=	  	"\r\nBAX TELNET SERVER";
// Security
const char usernameString[]		=		"\r\nLogin:";
const char passwordString[]		= 		"\r\nPassword:";	
// Access denied message
const char accessDeniedString[]	= 		"\r\nAccess denied!\r\n";
// Successful authentication message
const char authenticatedString[]	= 	"\r\nWelcome!\r\n";
// Demo disconnection message
const char exitString[]			= 		"\r\nGoodbye!\r\n";

// Typedefs
typedef enum
{
	SM_WAITING = 0,
	SM_CONNECT,
	SM_LISTEN,
	SM_PRINT_START,
	SM_PRINT_LOGIN,
	SM_GET_LOGIN,
	SM_GET_PASSWORD,
	SM_GET_PASSWORD_BAD_LOGIN,
	SM_AUTHENTICATED,
	SM_DISCONNECTING
} TelnetState_t;

// Connections
typedef struct {
	TCP_SOCKET socket;
	TelnetState_t state;
	unsigned short lastRxLen;
} TelnetClient;

static TelnetClient telnetConnections[MAX_TELNET_CONNECTIONS];

void TelnetReset(TelnetClient* client)
{
//	TCPFlush(client->socket);
	TCPDiscard(client->socket);
	client->state = SM_LISTEN;
	client->lastRxLen = 0;
}	

static char initialized = FALSE;
void TelnetInit(void)
{
	if(initialized)return;
	
	// Initialise all variables/states
	unsigned short clientIndex;
	for(clientIndex = 0; clientIndex < MAX_TELNET_CONNECTIONS; clientIndex++)
	{
		TelnetReset(&telnetConnections[clientIndex]);
		telnetConnections[clientIndex].socket = INVALID_SOCKET;
		telnetConnections[clientIndex].state = SM_WAITING;
	}
	status.telnet_state = OFF;
	status.telnet_mask = 0;
}

void TelnetDeinit(void)
{
	initialized = FALSE;
	
	// Initialise all variables/states
	unsigned short clientIndex;
	for(clientIndex = 0; clientIndex < MAX_TELNET_CONNECTIONS; clientIndex++)
	{
		TelnetReset(&telnetConnections[clientIndex]);
		if(telnetConnections[clientIndex].socket != INVALID_SOCKET)
		{
			TCPClose(telnetConnections[clientIndex].socket);
			telnetConnections[clientIndex].socket = INVALID_SOCKET;
			telnetConnections[clientIndex].state = SM_WAITING;
		}
	}
	status.telnet_state = OFF;
	status.telnet_mask = 0;
}


// Main telnet state machine
void TelnetTask(void)
{
	unsigned short clientIndex, offset1, offset2;
	const char* CR = "\r";
	
	// Init on first call
	static unsigned char once = FALSE;
	if(!once){once=TRUE;TelnetInit();}

	// Loop through each telnet session
	for(clientIndex = 0; clientIndex < MAX_TELNET_CONNECTIONS; clientIndex++)
	{
		TelnetClient *client = &telnetConnections[clientIndex];
		
		// If the socket is valid, check it is connected
		if(client->socket != INVALID_SOCKET)
		{
			if(TCPWasReset(client->socket))
			{
				DBG_INFO("Telnet: Reset\r\n");
				TelnetReset(client);
				status.telnet_mask &= ~(1ul <<clientIndex);
			}
		}
		
		// Handle session state
		switch(client->state)
		{
			case SM_WAITING:
			{
				DBG_INFO("Telnet: Waiting\r\n");
				
				// Try and establish a connection
				if(client->socket == INVALID_SOCKET)
					client->socket = TCPOpen(0, TCP_OPEN_SERVER, TELNET_PORT, TCP_PURPOSE_TELNET);
				if(client->socket == INVALID_SOCKET)
					break;
	
				// Open an SSL listener if SSL server support is enabled
				#if defined(STACK_USE_SSL_SERVER)
					TCPAddSSLListener(client->socket, TELNETS_PORT);
				#endif
				client->state = SM_CONNECT;
				
			}	//fall through
			case SM_CONNECT:
				#if defined(STACK_USE_SSL_SERVER)
					// Reject unsecured connections if TELNET_REJECT_UNSECURED is defined
					#if defined(TELNET_REJECT_UNSECURED)
						if(!TCPIsSSL(client->socket))
						{
							if(TCPIsConnected(client->socket))
							{
								TCPDisconnect(client->socket);
								TCPDisconnect(client->socket);
								break;
							}	
						}
					#endif
					// Don't attempt to transmit anything if we are still handshaking.
					if(TCPSSLIsHandshaking(client->socket))
						break;
				#endif
			case SM_LISTEN:
			{
				// Has a client connected yet?
				if(!TCPIsConnected(client->socket))
				{
					break;
				}
				DBG_INFO("Telnet: Connected\r\n");
				client->state = SM_PRINT_START;
			}
			case SM_PRINT_START:	
			{	// Make certain the socket can be written to
				if(TCPIsPutReady(client->socket) < (strlen(startString)))
					break;
				
				// Place the application protocol data into the transmit buffer.
				TCPPutString(client->socket, (BYTE*)startString);
	
				#ifdef TELNET_NO_SECURITY
					// Skip login
					client->state = SM_AUTHENTICATED;
				#else
					// Login prompt
					client->state = SM_PRINT_LOGIN;
				#endif

				// Send the packet
				TCPFlush(client->socket);

				break;
			}
			case SM_PRINT_LOGIN:	
			{	// Make certain the socket can be written to
				if(TCPIsPutReady(client->socket) < (strlen(usernameString)))
					break;
				
				// Place the application protocol data into the transmit buffer.
				TCPPutString(client->socket, (BYTE*)usernameString);
	
				client->state = SM_GET_LOGIN;

				// Send the packet
				TCPFlush(client->socket);
			}
			case SM_GET_LOGIN:
			{	
				// Make sure we can put the password prompt
				if(TCPIsPutReady(client->socket) < (strlen(passwordString)))
					break;
	
				// See if the user pressed return
				offset1 = TCPFindArrayEx(client->socket, (BYTE*)CR, 1, 0, 0, FALSE); // CR is defined above as CR[] = "\r"

				if(offset1 == 0xFFFF) // No \r found
				{
					if(TCPGetRxFIFOFree(client->socket) == 0u)
					{
						TCPPutString(client->socket, (BYTE*)"\r\nToo much data.\r\n");
						TCPFlush(client->socket);
						client->state = SM_DISCONNECTING;
					}
					break;
				}

				// \r has been found, find username - case insensitive
				offset2 = TCPFindArrayEx(client->socket, (BYTE*)settings.telnet_username, strlen(settings.telnet_username), 0, 0, TRUE);
			
				if(	(offset2 == 0xffff) ||										// No found
					(offset1 - offset2 != strlen(settings.telnet_username)))	// Extra chars after name 
					/*(can't check for chars before as client may have sent formatting info)*/
				{
					// Did not find the username, but let's pretend we did so we don't leak the user name validity
					client->state = SM_GET_PASSWORD_BAD_LOGIN;	
				}
				else
				{
					client->state = SM_GET_PASSWORD;
				}
	
				// Username verified, throw this line of data away
				TCPDiscard(client->socket);
	
				// Print the password prompt
				TCPPutString(client->socket, (BYTE*)passwordString);
				TCPFlush(client->socket);
				break;
			}
			case SM_GET_PASSWORD:
			case SM_GET_PASSWORD_BAD_LOGIN:
			{
				// Make sure we can put the password prompt
				if(TCPIsPutReady(client->socket) < (strlen(authenticatedString)))
					break;
	
				// See if the user pressed return
				offset1 = TCPFindArrayEx(client->socket, (BYTE*)CR, 1, 0, 0, FALSE); // CR is defined above as CR[] = "\r"

				if(offset1 == 0xFFFF) // No \r found
				{
					if(TCPGetRxFIFOFree(client->socket) == 0u)
					{
						TCPPutString(client->socket, (BYTE*)"\r\nToo much data.\r\n");
						TCPFlush(client->socket);
						client->state = SM_DISCONNECTING;
					}
					break;
				}

				// \r has been found, find username - case sensitive
				offset2 = TCPFindArrayEx(client->socket, (BYTE*)settings.telnet_password, strlen(settings.telnet_password), 0, 0, FALSE);

				// Username verified, throw this line of data away
				TCPDiscard(client->socket);
			
				if(	(offset2 == 0xffff) ||										// No found
					(offset1 - offset2 != strlen(settings.telnet_password)) ||	// Extra chars after name
					(client->state == SM_GET_PASSWORD_BAD_LOGIN))						// Bad user name
					/*(can't check for chars before as client may have sent formatting info)*/
				{
					// Did not find the password
					client->state = SM_PRINT_LOGIN;	
					TCPPutString(client->socket, (BYTE*)accessDeniedString);
					break;
				}
				else
				{
					TCPPutString(client->socket, (BYTE*)authenticatedString);
					TCPFlush(client->socket);
					client->state = SM_AUTHENTICATED;
				}
			}
			case SM_AUTHENTICATED:
			{
				
				// Has a client dis-connected
				if(!TCPIsConnected(client->socket))
				{
					TelnetReset(client);
					status.telnet_mask &= ~(1ul <<clientIndex);
					break;
				}
				
				// Set mask
				status.telnet_mask |= (1ul<<clientIndex);
				// Incomming stream:
				// Check it has space for more
				if(TCPGetRxFIFOFree(client->socket) == 0u)
				{
					TCPPutString(client->socket, (BYTE*)"\r\nToo much data.\r\n");
					TCPFlush(client->socket);
					client->state = SM_DISCONNECTING;
				}
				else
				{
					unsigned short rxLen = TCPIsGetReady(client->socket);
					if(rxLen > client->lastRxLen) { // Search for \r in input data
						client->lastRxLen = rxLen;
						// See if the user pressed return
						rxLen = TCPFindArrayEx(client->socket, (BYTE*)CR, 1, 0, 0, FALSE); // CR is defined above as CR[] = "\r"
						if(rxLen != 0xffff)
						{
							client->lastRxLen = 0;
							// \r found at position rxLen
							if(rxLen > MAX_TELNET_LINE_LEN) 
							{
								// Indicate loss
								TCPPutString(client->socket, (BYTE*)"\r\nLine too long!\r\n");
								DBG_INFO("Telnet: Line too long!\r\n");
							}
							else
							{
								char* temp = malloc(rxLen+1);		// Allocate buffer (1 byte for CR)	
								if(temp != NULL)
								{
									rxLen = TCPGetArray(client->socket, (BYTE*)(temp), rxLen+1);	// Read array out
									temp[rxLen] = '\0';			// Terminate it
									FormatLine((temp));			// Format it
									AddDataElement(TYPE_TEXT_ELEMENT,clientIndex, strlen(temp), CMD_TEL_FLAG , (void*)temp); 	// Add element
									free(temp);						// Free memory
								}
								else
								{
									DBG_ERROR("Telnet: malloc failure");
								}
							}
							TCPDiscard(client->socket);
						}
					}
				}
				break;
			} // Case authenticated
			case SM_DISCONNECTING:
				DBG_INFO("Telnet: Disconnect\r\n");
			default:
			{
				// Clear mask
				status.telnet_mask &= ~(1ul<<clientIndex);
				// Send string
				if(TCPIsPutReady(client->socket) >= strlen(exitString))
				{
					TCPPutString(client->socket, (BYTE*)exitString);
					// Flush
					TCPFlush(client->socket);
				}
				// Disconnect
				TCPDisconnect(client->socket);
				TCPDisconnect(client->socket);
				TelnetReset(client);
				status.telnet_mask &= ~(1ul <<clientIndex);;							

				break;
			}
		}// End main switch
	}// End for loop for connections

	if(status.telnet_mask != 0) // If there are any connections open
	{
		// Update status
		if(settings.telnet_stream == STATE_ON)	status.telnet_state = ACTIVE;
		else 									status.telnet_state = INITIALISED;
	}
 
}

unsigned char TelnetWrite(void* data, unsigned short len, unsigned short flags)
{
	// Set outgoing flags for message
	unsigned char clientIndex;
	unsigned short remaining;
	TelnetClient* client;
	flags &= 0xFF>>(8-MAX_TELNET_CONNECTIONS);
	for(clientIndex = 0; clientIndex < MAX_TELNET_CONNECTIONS; clientIndex++)
	{
		client = &telnetConnections[clientIndex];
		if(flags & (1ul<<clientIndex) && (client->state == SM_AUTHENTICATED))
		{
			unsigned char retrys;
			remaining = len;
			for(retrys=10;retrys>0;retrys--)
			{
				unsigned short txLen = TCPIsPutReady(client->socket);	// How much can we send	
				if(txLen >= remaining) 									// Set ammount
					txLen = remaining;
				if(txLen>0) 
				{
					txLen = TCPPutArray(client->socket,(BYTE*)data+(len-remaining),txLen); // Send
					TCPFlush(client->socket);
				}
				remaining -= txLen;
				if(remaining == 0)break;
				// TCPIP tasks (includes telnet tasks)
				TcipipTasks();
				if(client->state != SM_AUTHENTICATED)break;
			}
			if(remaining==0)
				flags &= ~(1ul<<clientIndex);
			else {DBG_INFO("Telnet write timeout %lu");}
		}
	}
	// Return true for success
	return (flags == 0);
}

void TelnetDisconnect(unsigned short flags)
{
	// Initialise all variables/states
	unsigned short clientIndex;
	for(clientIndex = 0; clientIndex < MAX_TELNET_CONNECTIONS; clientIndex++)
	{
		if(flags & (1ul<<clientIndex))
		{
			telnetConnections[clientIndex].state = SM_DISCONNECTING;
		}
	}
	return;
}


// Remove control chars BS, \n etc. Null terminates
char* FormatLine(char* source)
{
	char* ptr = source;
	unsigned short remaining = 1 + strlen(ptr);			// Include Null
	for(;remaining>0;remaining--)
	{
		if (*ptr == '\0') { break; }					// Null
        else if (*ptr == 13)   {*ptr = '\0'; break;}  	// CR
        else if (*ptr == 10)   							// LF (remove, possibly from external client)
		{
			memmove(ptr,ptr+1,remaining);				// Move data back removing the BS char
			continue;
		}  						
        else if (*ptr == 8 || *ptr == 127)        		// BS and DEL
        {
			if(ptr>source)								// Don't delete outside of buffer
			{
				memmove(ptr-1,ptr+1,remaining-1);		// Move data back
				ptr--;									// Go back to previour char
			}
			else
			{
				memmove(ptr,ptr+1,remaining-1);			// Move data back removing the BS char
			}
			continue;
        }
		else {ptr++;}
    }
	return source;
}

#endif	//#if defined(STACK_USE_TELNET_SERVER)
