// Minor changes by Karim Ladha, Copyright (C)2014 Newcastle University.

/*********************************************************************
 *
 *	Microchip TCP/IP Stack Demo Application Configuration Header
 *
 *********************************************************************
 * FileName:        TCPIPConfig.h
 * Dependencies:    Microchip TCP/IP Stack
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.10 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.34 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
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
 * WARRANTY OF ANY KIND, EITHER EXPR*====ESS OR IMPLIED, INCLUDING WITHOUT
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		10/04/2006	Original
 * Ken Hesky            07/01/2008  Added ZG2100-specific features
 * SG                   03/25/2009  Added ZGLinkMgrII specific features
 ********************************************************************/
#ifndef __TCPIPCONFIG_H
#define __TCPIPCONFIG_H

#include "GenericTypeDefs.h"
#include "Compiler.h"


#define STACK_USE_TCP					// Include tcp.h
#define STACK_USE_DHCP_CLIENT			// Basic requirement
#define STACK_USE_ICMP_SERVER			// Ping query and response capability
#define STACK_USE_ICMP_CLIENT			// Ping transmission capability
#define STACK_USE_NBNS					// Sends netbios name out
#define STACK_USE_TELNET_SERVER			// Telnet server
#define STACK_USE_TINYHTTP_SERVER		// HTTP server
//#define STACK_USE_SSL_SERVER


#define MAC_ACCEPT_MULTICAST			// Enable multicast receive
#define ENABLE_UDP_SERVICES				// See c file
#define DHCP_REQ_NAME					// Add name request to dhcp request KL

// =======================================================================
//   Network Addressing Options
// =======================================================================

/* Default Network Configuration
 *   These settings are only used if data is not found in EEPROM.
 *   To clear EEPROM, hold BUTTON0, reset the board, and continue
 *   holding until the LEDs flash.  Release, and reset again.
 */
#define MY_DEFAULT_HOST_NAME			"BAXROUTER"

#define MY_DEFAULT_MAC_BYTE1            (0x02)	// Use the default of 00-04-A3-00-00-00 KL_ set local admin bit
#define MY_DEFAULT_MAC_BYTE2            (0x04)	// if using an ENCX24J600, MRF24WB0M, or
#define MY_DEFAULT_MAC_BYTE3            (0xA3)	// PIC32MX6XX/7XX internal Ethernet 
#define MY_DEFAULT_MAC_BYTE4            (0x43)	// C 0x43 controller and wish to use the 
#define MY_DEFAULT_MAC_BYTE5            (0x4C)	// L 0x4C internal factory programmed MAC
#define MY_DEFAULT_MAC_BYTE6            (0x32)	// 1 0x31 address instead.

#define MY_DEFAULT_IP_ADDR_BYTE1        (169ul)
#define MY_DEFAULT_IP_ADDR_BYTE2        (254ul)
#define MY_DEFAULT_IP_ADDR_BYTE3        (1ul)
#define MY_DEFAULT_IP_ADDR_BYTE4        (1ul)

#define MY_DEFAULT_MASK_BYTE1           (255ul)
#define MY_DEFAULT_MASK_BYTE2           (255ul)
#define MY_DEFAULT_MASK_BYTE3           (0ul)
#define MY_DEFAULT_MASK_BYTE4           (0ul)

#define MY_DEFAULT_GATE_BYTE1           (169ul)
#define MY_DEFAULT_GATE_BYTE2           (254ul)
#define MY_DEFAULT_GATE_BYTE3           (1ul)
#define MY_DEFAULT_GATE_BYTE4           (1ul)

#define MY_DEFAULT_PRIMARY_DNS_BYTE1	(169ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE2	(254ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE3	(1ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE4	(1ul)

#define MY_DEFAULT_SECONDARY_DNS_BYTE1	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE2	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE3	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE4	(0ul)
/* TCP Socket Memory Allocation
 *   TCP needs memory to buffer incoming and outgoing data.  The
 *   amount and medium of storage can be allocated on a per-socket
 *   basis using the example below as a guide.
 */
	// Allocate how much total RAM (in bytes) you want to allocate
	// for use by your TCP TCBs, RX FIFOs, and TX FIFOs.
	#define TCP_ETH_RAM_SIZE					(3900ul)
	#define TCP_PIC_RAM_SIZE					(0ul)
	#define TCP_SPI_RAM_SIZE					(0ul)
	#define TCP_SPI_RAM_BASE_ADDRESS			(0x00)

	// Define names of socket types
	#define TCP_SOCKET_TYPES
		#define TCP_PURPOSE_TELNET 		1
		#define TCP_PURPOSE_HTTP_SERVER 2
	#define END_OF_TCP_SOCKET_TYPES

	#if defined(__TCP_C)
		// Define what types of sockets are needed, how many of
		// each to include, where their TCB, TX FIFO, and RX FIFO
		// should be stored, and how big the RX and TX FIFOs should
		// be.  Making this initializer bigger or smaller defines
		// how many total TCP sockets are available.
		//
		// Each socket requires up to 56 bytes of PIC RAM and
		// 48+(TX FIFO size)+(RX FIFO size) bytes of TCP_*_RAM each.
		//
		// Note: The RX FIFO must be at least 1 byte in order to
		// receive SYN and FIN messages required by TCP.  The TX
		// FIFO can be zero if desired.
		#define TCP_CONFIGURATION
		ROM struct
		{
			BYTE vSocketPurpose;
			BYTE vMemoryMedium;
			WORD wTXBufferSize;
			WORD wRXBufferSize;
		} TCPSocketInitializer[] =
		{
			{TCP_PURPOSE_TELNET, TCP_ETH_RAM, 256, 150},
			{TCP_PURPOSE_TELNET, TCP_ETH_RAM, 256, 150},
			{TCP_PURPOSE_HTTP_SERVER, TCP_ETH_RAM, 2048, 512},
		};
		#define END_OF_TCP_CONFIGURATION
	#endif
	
	// UDP 
	#if !defined(STACK_USE_UDP)
		#define MAX_UDP_SOCKETS 6
	#elif defined (STACK_USE_UDP)
		#warning "Set udp sockets you need + 1"
		#define MAX_UDP_SOCKETS 3+X		
	#else
		#warning "You need at least 1 udp socket for dhcp, nbns, announce"
	#endif
	
	// Maximum numbers of simultaneous HTTP connections allowed.
	// Each connection consumes 2 bytes of RAM and a TCP socket
	#define MAX_HTTP_CONNECTIONS	(1u)

    // Define the listening port for the HTTP server
  	#define HTTP_PORT               (80u)
	
    // Define the listening port for the HTTPS server (if STACK_USE_SSL_SERVER is enabled)
	#define HTTPS_PORT				(443u)
	
 	#define MAX_SSL_CONNECTIONS		(2ul)	// Maximum connections via SSL
	#define MAX_SSL_SESSIONS		(2ul)	// Max # of cached SSL sessions
	#define MAX_SSL_BUFFERS			(4ul)	// Max # of SSL buffers (2 per socket)
	#define MAX_SSL_HASHES			(5ul)	// Max # of SSL hashes  (2 per, plus 1 to avoid deadlock)

	// Bits in SSL RSA key.  This parameter is used for SSL sever
	// connections only.  The only valid value is 512 bits (768 and 1024
	// bits do not work at this time).  Note, however, that SSL client
	// operations do currently work up to 1024 bit RSA key length.
	#define SSL_RSA_KEY_SIZE		(512ul)


// -- Telnet Options -----------------------------------------------------
	// Telnet
	#define TELNET_PAS_NAME_SIZE	32
	//#define TELNET_NO_SECURITY
	#define DEFAULT_TELNET_NAME		"admin"
	#define DEFAULT_TELNET_PASS		"password"
	#define MAX_TELNET_LINE_LEN		128 /*Will accept command lines to this length*/


	// Telnet local uses a byte bit field to represent these! 8 MAX 
	#define MAX_TELNET_CONNECTIONS	(2u)

	// Default local listening port for the Telnet server.  Port 23 is the
	// protocol default.
	#define TELNET_PORT				23

	// Default local listening port for the Telnet server when SSL secured.
	// Port 992 is the telnets protocol default.
	#define TELNETS_PORT			992

#endif

