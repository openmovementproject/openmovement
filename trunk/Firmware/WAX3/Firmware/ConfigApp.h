/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// ConfigApp.h - Application configuration header.
// Based on Microchip Technology file 'ConfigApp.h' from Microchip's MiWi example

#ifndef __CONFIG_APP_H_
#define __CONFIG_APP_H_

#include "HardwareProfile.h"

// ENABLE_CONSOLE will enable the print out on the hyper terminal this definition is very helpful in the debugging process
//#define ENABLE_CONSOLE

// ENABLE_NETWORK_FREEZER enables the network freezer feature, which stores critical network information into non-volatile memory, so
// that the protocol stack can recover from power loss gracefully. Network freezer feature needs definition of NVM kind to be 
// used, which is specified in HardwareProfile.h
//#ifndef SIMPLE_EXAMPLE
//    #define ENABLE_NETWORK_FREEZER
//#endif

// HARDWARE_SPI enables the hardware SPI implementation on MCU silicon. If HARDWARE_SPI is not defined, digital I/O pins will be used to bit-bang the RF transceiver
#define HARDWARE_SPI


// Definition of Protocol Stack. ONLY ONE PROTOCOL STACK CAN BE CHOSEN

// PROTOCOL_P2P enables the application to use MiWi P2P stack. This definition cannot be defined with PROTOCOL_MIWI.
#define PROTOCOL_P2P

// PROTOCOL_MIWI enables the application to use MiWi mesh networking stack. This definition cannot be defined with PROTOCOL_P2P.
//#define PROTOCOL_MIWI


// NWK_ROLE_COORDINATOR is not valid if PROTOCOL_P2P is defined. It specified that the node has the capability to be coordinator or PAN 
// coordinator. This definition cannot be defined with NWK_ROLE_END_DEVICE.
#if (DEVICE_TYPE == 0)    // FFD - Full-function device
    #define NWK_ROLE_COORDINATOR
#elif (DEVICE_TYPE == 1 || DEVICE_TYPE == 2 || DEVICE_TYPE == 3)      // RFD - Reduced-function device
    #define NWK_ROLE_END_DEVICE
#else
    #error DEVICE_TYPE not valid.
#endif


// Definition of RF Transceiver. ONLY ONE TRANSCEIVER CAN BE CHOSEN

// Definition of MRF24J40 enables the application to use Microchip MRF24J40 2.4GHz IEEE 802.15.4 compliant RF transceiver. Only one RF transceiver can be defined.
#define MRF24J40

// Definition of MRF49XA enables the application to use Microchip MRF49XA subGHz proprietary RF transceiver. Only one RF transceiver can be defined.
//#define MRF49XA

// Definition of MRF89XA enables the application to use Microchip MRF89XA subGHz proprietary RF transceiver
//#define MRF89XA



// MY_ADDRESS_LENGTH defines the size of wireless node permanent address in byte. This definition is not valid for IEEE 802.15.4 compliant RF transceivers.
#define MY_ADDRESS_LENGTH       8		// 4


#define P2P_EXTERNAL_ADDRESS

// EUI_x defines the xth byte of permanent address for the wireless node
#define EUI_7 0x00
#define EUI_6 0x04
#define EUI_5 0xA3
#define EUI_4 0x00
#define EUI_3 0x00
#define EUI_2 0x00
#define EUI_1 ((DEVICE_ID >> 8) & 0xff)
#define EUI_0 ((DEVICE_ID     ) & 0xff)

// TX_BUFFER_SIZE defines the maximum size of application payload which is to be transmitted
#define TX_BUFFER_SIZE 127	// 40

// RX_BUFFER_SIZE defines the maximum size of application payload which is to be received
#define RX_BUFFER_SIZE 127	// 40

// MY_PAN_ID defines the PAN identifier
#define MY_PAN_ID (DEFAULT_PAN)

// ADDITIONAL_NODE_ID_SIZE defines the size of additional payload will be attached to the P2P Connection Request. Additional payload 
// is the information that the devices what to share with their peers on the P2P connection. The additional payload will be defined by 
// the application and defined in main.c
#define ADDITIONAL_NODE_ID_SIZE   1

// P2P_CONNECTION_SIZE defines the maximum P2P connections that this device allows at the same time. 
#define CONNECTION_SIZE             10

// TARGET_SMALL will remove the support of inter PAN communication and other minor features to save programming space
//#define TARGET_SMALL

// ENABLE_PA_LNA enable the external power amplifier and low noise amplifier on the RF board to achieve longer radio communication 
// range. To enable PA/LNA on RF board without power amplifier and low noise amplifier may be harmful to the transceiver.
//#define ENABLE_PA_LNA

// ENABLE_HAND_SHAKE enables the protocol stack to hand-shake before communicating with each other. Without a handshake process, RF
// transceivers can only broadcast, or hardcoded the destination address to perform unicast.
#define ENABLE_HAND_SHAKE

// ENABLE_SLEEP will enable the device to go to sleep and wake up from the sleep
#ifndef SIMPLE_EXAMPLE
    #define ENABLE_SLEEP
#endif

// ENABLE_ED_SCAN will enable the device to do an energy detection scan to find out the channel with least noise and operate on that channel
#ifndef SIMPLE_EXAMPLE
    #define ENABLE_ED_SCAN
#endif

// ENABLE_ACTIVE_SCAN will enable the device to do an active scan to to detect current existing connection. 
#ifndef SIMPLE_EXAMPLE
    #define ENABLE_ACTIVE_SCAN
#endif

// ENABLE_SECURITY will enable the device to encrypt and decrypt information transferred
//#define ENABLE_SECURITY

// ENABLE_INDIRECT_MESSAGE will enable the device to store the packets for the sleeping devices temporily until they wake up and ask for the messages
#ifndef SIMPLE_EXAMPLE
//    #define ENABLE_INDIRECT_MESSAGE
#endif

// ENABLE_BROADCAST will enable the device to broadcast messages for the sleeping devices until they wake up and ask for the messages
#ifndef SIMPLE_EXAMPLE
    #define ENABLE_BROADCAST
#endif

// RFD_WAKEUP_INTERVAL defines the wake up interval for RFDs in second. This definition is for the FFD devices to calculated various
// timeout. RFD depends on the setting of the watchdog timer to wake up, thus this definition is not used.
#define RFD_WAKEUP_INTERVAL     8

// ENABLE_FREQUENCY_AGILITY will enable the device to change operating channel to bypass the sudden change of noise
#ifndef SIMPLE_EXAMPLE
    #define ENABLE_FREQUENCY_AGILITY
#endif



// Validate constants
    
#if !defined(MRF24J40) && !defined(MRF49XA) && !defined(MRF89XA)
    #error "One transceiver must be defined for the wireless application"
#endif

#if (defined(MRF24J40) && defined(MRF49XA)) || (defined(MRF24J40) && defined(MRF89XA)) || (defined(MRF49XA) && defined(MRF89XA))
    #error "Only one transceiver can be defined for the wireless application"
#endif

#if !defined(PROTOCOL_P2P) && !defined(PROTOCOL_MIWI)
    #error "One Microchip proprietary protocol must be defined for the wireless application."
#endif

#if MY_ADDRESS_LENGTH > 8
    #error "Maximum address length is 8"
#endif

#if MY_ADDRESS_LENGTH < 2
    #error "Minimum address length is 2"
#endif

#if defined(MRF24J40)
    #define IEEE_802_15_4
    #undef MY_ADDRESS_LENGTH
    #define MY_ADDRESS_LENGTH 8
#endif

#if defined(ENABLE_NETWORK_FREEZER)
    #define ENABLE_NVM
#endif

#if defined(ENABLE_ACTIVE_SCAN) && defined(TARGET_SMALL)
    #error  Target_Small and Enable_Active_Scan cannot be defined together 
#endif

#if defined(ENABLE_INDIRECT_MESSAGE) && !defined(RFD_WAKEUP_INTERVAL)
    #error "RFD Wakeup Interval must be defined if indirect message is enabled"
#endif

#if (RX_BUFFER_SIZE > 127)
    #error RX BUFFER SIZE too large. Must be <= 127.
#endif

#if (TX_BUFFER_SIZE > 127)
    #error TX BUFFER SIZE too large. Must be <= 127.
#endif

#if (RX_BUFFER_SIZE < 10)
    #error RX BUFFER SIZE too small. Must be >= 10.
#endif

#if (TX_BUFFER_SIZE < 10)
    #error TX BUFFER SIZE too small. Must be >= 10.
#endif

#if (NETWORK_TABLE_SIZE > 0xFE)
    #error NETWORK TABLE SIZE too large.  Must be < 0xFF.
#endif


#endif
