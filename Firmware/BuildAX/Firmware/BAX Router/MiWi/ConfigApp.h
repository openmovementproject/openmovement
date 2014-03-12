/*********************************************************************
 *                                                                    
 * Software License Agreement                                         
 *                                                                    
 * Copyright © 2007-2008 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, modify, copy and distribute 
 * Software only when embedded on a Microchip microcontroller or digital 
 * signal controller and used with a Microchip radio frequency transceiver, 
 * which are integrated into your product or third party product (pursuant 
 * to the terms in the accompanying license agreement).   
 *
 * You should refer to the license agreement accompanying this Software for 
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A 
 * PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE 
 * LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, 
 * CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY 
 * DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO 
 * ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
 * LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, 
 * TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT 
 * NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.             
 *                                                                    
 *********************************************************************/
#ifndef __CONFIG_APP_H_
#define __CONFIG_APP_H_

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"


/*********************************************************************/
// following codes defines the platforms as well as the hardware 
// configuration
/*********************************************************************/

/*********************************************************************/
// ENABLE_NETWORK_FREEZER enables the network freezer feature, which
// stores critical network information into non-volatile memory, so
// that the protocol stack can recover from power loss gracefully.
// Network freezer feature needs definition of NVM kind to be 
// used, which is specified in HardwareProfile.h
/*********************************************************************/
#define ENABLE_NETWORK_FREEZER

/*********************************************************************/
// HARDWARE_SPI enables the hardware SPI implementation on MCU
// silicon. If HARDWARE_SPI is not defined, digital I/O pins will
// be used to bit-bang the RF transceiver
/*********************************************************************/
#define HARDWARE_SPI

#ifdef ROUTER
	/*********************************************************************/
	// PROTOCOL_MIWI_PRO enables the application to use MiWi PRO stack. 
	// This definition cannot be defined with PROTOCOL_P2P or PROTOCOL_MIWI.
	/*********************************************************************/
	#define PROTOCOL_MIWI_PRO

	/*********************************************************************/
	// NWK_ROLE_COORDINATOR is not valid if PROTOCOL_P2P is defined. It
	// specified that the node has the capability to be coordinator or PAN 
	// coordinator. This definition cannot be defined with 
	// NWK_ROLE_END_DEVICE.
	/*********************************************************************/
	#define NWK_ROLE_COORDINATOR
#elif defined(SENSOR)
	/*********************************************************************/
	// PROTOCOL_MIWI_PRO enables the application to use MiWi PRO stack. 
	// This definition cannot be defined with PROTOCOL_P2P or PROTOCOL_MIWI.
	/*********************************************************************/
	#define PROTOCOL_MIWI

	/*********************************************************************/
	// NWK_ROLE_COORDINATOR is not valid if PROTOCOL_P2P is defined. It
	// specified that the node has the capability to be coordinator or PAN 
	// coordinator. This definition cannot be defined with 
	// NWK_ROLE_END_DEVICE.
	/*********************************************************************/
	#define NWK_ROLE_END_DEVICE
#endif

/*********************************************************************/
// Definition of MRF24J40 enables the application to use Microchip
// MRF24J40 2.4GHz IEEE 802.15.4 compliant RF transceiver. Only one
// RF transceiver can be defined.
/*********************************************************************/
#define MRF24J40

/*********************************************************************/
// Definition of MRF49XA enables the application to use Microchip
// MRF49XA subGHz proprietary RF transceiver. Only one RF transceiver
// can be defined.
/*********************************************************************/
//#define MRF49XA

/*********************************************************************/
// Definition of MRF89XA enables the application to use Microchip
// MRF89XA subGHz proprietary RF transceiver
/*********************************************************************/
//#define MRF89XA


/*********************************************************************/
// MY_ADDRESS_LENGTH defines the size of wireless node permanent 
// address in byte. This definition is not valid for IEEE 802.15.4
// compliant RF transceivers.
/*********************************************************************/
#define MY_ADDRESS_LENGTH       8

/*********************************************************************/
// EUI_x defines the xth byte of permanent address for the wireless
// node
/*********************************************************************/
#define EUI_7 0x7E
#define EUI_6 0xD1
#define EUI_5 0x7E
#define EUI_4 0xD1
#define EUI_3 0x7E
#define EUI_2 0xD1
#define EUI_1 (0xff)
#define EUI_0 (0xff)

/*********************************************************************/
// TX_BUFFER_SIZE defines the maximum size of application payload
// which is to be transmitted
/*********************************************************************/
#define TX_BUFFER_SIZE 88

/*********************************************************************/
// RX_BUFFER_SIZE defines the maximum size of application payload
// which is to be received
/*********************************************************************/
#define RX_BUFFER_SIZE 88

/*********************************************************************/
// MY_PAN_ID defines the PAN identifier. Use 0xFFFF if prefer a 
// random PAN ID.
/*********************************************************************/
#define MY_PAN_ID                       0x7ED1

/*********************************************************************/
// ADDITIONAL_NODE_ID_SIZE defines the size of additional payload
// will be attached to the P2P Connection Request. Additional payload 
// is the information that the devices what to share with their peers
// on the P2P connection. The additional payload will be defined by 
// the application and defined in main.c
/*********************************************************************/
#define ADDITIONAL_NODE_ID_SIZE   1

/*********************************************************************/
// P2P_CONNECTION_SIZE defines the maximum P2P connections that this 
// device allowes at the same time. 
/*********************************************************************/
#define CONNECTION_SIZE             48    // 16


/*********************************************************************/
// TARGET_SMALL will remove the support of inter PAN communication
// and other minor features to save programming space
/*********************************************************************/
//#define TARGET_SMALL

/*********************************************************************/
// ENABLE_PA_LNA enable the external power amplifier and low noise
// amplifier on the RF board to achieve longer radio communication 
// range. To enable PA/LNA on RF board without power amplifier and
// low noise amplifier may be harmful to the transceiver.
/*********************************************************************/
//#define ENABLE_PA_LNA


/*********************************************************************/
// ENABLE_HAND_SHAKE enables the protocol stack to hand-shake before 
// communicating with each other. Without a handshake process, RF
// transceivers can only broadcast, or hardcoded the destination address
// to perform unicast.
/*********************************************************************/
#ifdef ROUTER
	#define ENABLE_HAND_SHAKE
#endif


/*********************************************************************/
// ENABLE_SLEEP will enable the device to go to sleep and wake up 
// from the sleep
/*********************************************************************/
#ifdef SENSOR
	#define ENABLE_SLEEP
#endif


/*********************************************************************/
// ENABLE_ED_SCAN will enable the device to do an energy detection scan
// to find out the channel with least noise and operate on that channel
/*********************************************************************/
//#define ENABLE_ED_SCAN


/*********************************************************************/
// ENABLE_ACTIVE_SCAN will enable the device to do an active scan to 
// to detect current existing connection. 
/*********************************************************************/
#define ENABLE_ACTIVE_SCAN


/*********************************************************************/
// ENABLE_SECURITY will enable the device to encrypt and decrypt
// information transferred
/*********************************************************************/
//#define ENABLE_SECURITY

/*********************************************************************/
// ENABLE_INDIRECT_MESSAGE will enable the device to store the packets
// for the sleeping devices temporily until they wake up and ask for
// the messages
/*********************************************************************/
#ifdef ROUTER
	#define ENABLE_INDIRECT_MESSAGE
#endif


/*********************************************************************/
// ENABLE_BROADCAST will enable the device to broadcast messages for
// the sleeping devices until they wake up and ask for the messages
/*********************************************************************/
//#define ENABLE_BROADCAST


/*********************************************************************/
// RFD_WAKEUP_INTERVAL defines the wake up interval for RFDs in second.
// This definition is for the FFD devices to calculated various
// timeout. RFD depends on the setting of the watchdog timer to wake 
// up, thus this definition is not used.
/*********************************************************************/
#define RFD_WAKEUP_INTERVAL     8


/*********************************************************************/
// ENABLE_FREQUENCY_AGILITY will enable the device to change operating
// channel to bypass the sudden change of noise
/*********************************************************************/
#define ENABLE_FREQUENCY_AGILITY


// Constants Validation
    
#if !defined(MRF24J40) && !defined(MRF49XA) && !defined(MRF89XA)
    #error "One transceiver must be defined for the wireless application"
#endif

#if (defined(MRF24J40) && defined(MRF49XA)) || (defined(MRF24J40) && defined(MRF89XA)) || (defined(MRF49XA) && defined(MRF89XA))
    #error "Only one transceiver can be defined for the wireless application"
#endif

#if !defined(PROTOCOL_P2P) && !defined(PROTOCOL_MIWI) && !defined(PROTOCOL_MIWI_PRO)
    #error "One Microchip proprietary protocol must be defined for the wireless application."
#endif

#if defined(ENABLE_FREQUENCY_AGILITY)
    #define ENABLE_ED_SCAN
#endif

#if MY_ADDRESS_LENGTH > 8
    #error "Maximum address length is 8"
#endif

#if MY_ADDRESS_LENGTH < 2
    #error "Minimum address length is 2"
#endif

#if defined(MRF24J40)
    #define IEEE_802_15_4
    #ifdef MY_ADDRESS_LENGTH
        #if MY_ADDRESS_LENGTH != 8
        	#warning MY_ADDRESS_LENGTH must be 8
        #endif
    #endif
    #undef MY_ADDRESS_LENGTH
    #define MY_ADDRESS_LENGTH 8
#endif

#if defined(ENABLE_NETWORK_FREEZER)
    //#define ENABLE_NVM
    //#define far /*fix for NVM freezer*/
    //#define USE_PROGRAMMING_SPACE
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



#if defined(PROTOCOL_MIWI_PRO)
    #include "ConfigMiWiPRO.h"
#elif defined(PROTOCOL_MIWI)
    #include "ConfigMiWi.h"
#endif

// Missing prototypes for MyNVM.c
extern BOOL NVMInit(void);
extern void nvmGetMyPANID(unsigned char *v);
extern void nvmPutMyPANID(unsigned char *v);
extern void nvmGetCurrentChannel(unsigned char *v);
extern void nvmPutCurrentChannel(unsigned char *v);
extern void nvmGetConnMode(unsigned char *v);
extern void nvmPutConnMode(unsigned char *v);
extern void nvmGetConnectionTable(void *v);
extern void nvmPutConnectionTable(void *v);
extern void nvmPutConnectionTableIndex(void *v, unsigned short index);
extern void nvmGetOutFrameCounter(unsigned char *v);
extern void nvmPutOutFrameCounter(unsigned char *v);
extern void nvmGetMyShortAddress(unsigned char *v);
extern void nvmPutMyShortAddress(unsigned char *v);
extern void nvmGetMyParent(unsigned char *v);
extern void nvmPutMyParent(unsigned char *v);
extern void nvmGetRoutingTable(void *v);
extern void nvmPutRoutingTable(void *v);
extern void nvmGetKnownCoordinators(unsigned char *v);
extern void nvmPutKnownCoordinators(unsigned char *v);
extern void nvmGetRoutingTable(void *v);
extern void nvmPutRoutingTable(void *v);
extern void nvmGetNeighborRoutingTable(void *v);
extern void nvmPutNeighborRoutingTable(void *v);
extern void nvmGetFamilyTree(void *v);
extern void nvmPutFamilyTree(void *v);
extern void nvmGetRole(unsigned char *v);
extern void nvmPutRole(unsigned char *v);

#endif
