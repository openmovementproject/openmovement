/********************************************************************
* FileName:		ConfigMiWi.h
* Dependencies: 
* Processor:	PIC18, PIC24F, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Complier:     Microchip C18 v3.04 or higher
*				Microchip C30 v2.03 or higher	
*               Microchip C32 v1.02 or higher
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice
*
* Copyright © 2007-2010 Microchip Technology Inc.  All rights reserved.
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
*********************************************************************
* File Description:
*
*  This file provides configuration settings for P2P protocol stack
*
* Change History:
*  Rev   Date         Author        Description
*  2.0   4/15/2009    yfy           MiMAC and MiApp revision
*  3.1   5/28/2010    yfy           MiWi DE 3.1
*  4.1   6/3/2011     yfy           MAL v2011-06
********************************************************************/

#ifndef __CONFIGURE_MIWI_H

    #define __CONFIGURE_MIWI_H
    
    #include "ConfigApp.h"
    
    #if defined(PROTOCOL_MIWI)
        #include "WirelessProtocols/SymbolTime.h"
    
        /*********************************************************************/
        // ENABLE_DUMP will enable the stack to be able to print out the 
        // content of the P2P connection entry. It is useful in the debugging
        // process
        /*********************************************************************/
        #define ENABLE_DUMP
        
        
        /*********************************************************************/
        // RFD_DATA_WAIT is the timeout defined for sleeping device to receive 
        // a message from the associate device after Data Request. After this
        // timeout, the RFD device can continue to operate and then go to 
        // sleep to conserve battery power.
        /*********************************************************************/
        #define RFD_DATA_WAIT                   0x00003FFF
        
        
        /*********************************************************************/
        // CONNECTION_RETRY_TIMES is the maximum time that the wireless node
        // can try to establish a connection. Once the retry times are exhausted
        // control will be return to application layer to decide what to do next
        /*********************************************************************/
        #define CONNECTION_RETRY_TIMES          3
    
    
        /*********************************************************************/
        // OPEN_SOCKET_TIMEOUT is the timeout period in symbols for a node to
        // abandon attempt to establish a socket connection, or in MiApp term,
        // an indrect connection
        /*********************************************************************/
        #define OPEN_SOCKET_TIMEOUT             (ONE_SECOND * 3)
    
    
        /*********************************************************************/
        // For a sleeping device, when establishing an indirect connection 
        // (socket), it may not be desirable to poll the data at the normal 
        // interval, which can be longer than OPEN_SOCKET_TIMEOUT, the 
        // solution is to poll the data at a fast rate, lower than 
        // OPEN_SOCKET_TIMEOUT. OPEN_SOCKET_POLL_INTERVAL is the polling 
        // interval in symbols for a sleeping device to acquire data from its 
        // parent in the process of establishing indirect (socket) connection.
        // This parameter is only valid for sleeping device.
        /*********************************************************************/
        #define OPEN_SOCKET_POLL_INTERVAL       (ONE_SECOND)
        
        
        /*********************************************************************/
        // ENABLE_MIWI_ACKNOWLEDGEMENT enables the MiWi stack to acknowledge
        // the data packet from the application layer.
        /*********************************************************************/
        //#define ENABLE_MIWI_ACKNOWLEDGEMENT


        /*********************************************************************/
        // MIWI_ACK_TIMEOUT is the timeout period in symbols for a node to
        // receive a MiWi network layer acknowledgement. This parameter is 
        // for MiWi network layer, not for MAC layer. MAC layer acknowledgement
        // timeout is handled in MiMAC layer.
        /*********************************************************************/
        #define MIWI_ACK_TIMEOUT                (ONE_SECOND)
    
    
        /*********************************************************************/
        // ENABLE_BROADCAST_TO_SLEEP_DEVICE enables messages broadcast to a 
        // sleeping device.
        /*********************************************************************/
        #define ENABLE_BROADCAST_TO_SLEEP_DEVICE
        
        
        /*********************************************************************/
        // BROADCAST_RECORD_SIZE is the parameter that specifies the maximum
        // number of broadcast record available. Broadcast record is used to
        // track the broadcast messages so that the wireless node knows if
        // the same broadcast has been received before.
        /*********************************************************************/
        #define BROADCAST_RECORD_SIZE   4
        
        
        /*********************************************************************/
        // BROADCAST_RECORD_TIMEOUT defines the timeout in symbols for a
        // node to expire its broadcast record. The broadcast record is used
        // to track the received broadcast message and to prevent receiving
        // duplicate broadcast message. This definition is only valid for
        // a non-sleeping device.
        /*********************************************************************/
        #define BROADCAST_RECORD_TIMEOUT    (ONE_SECOND)
        
        
        /*********************************************************************/
        // When broadcasting to a sleeping device is enabled, it is hard for 
        // a parent node to track which end device has received the broadcast
        // message. However, if no tracking is provided, the end device may 
        // receive the same broadcast multiple times. MiWi solves this problem
        // by tracking the broadcast message on sleeping device side. 
        // INDIRECT_MESSAGE_TIMEOUT_CYCLE defines the total number of messages
        // receives before the broadcast record times out. It is hard for a 
        // sleeping node to track timing, so tracking the number of message
        // received is a simpler way. 
        /*********************************************************************/
        #define INDIRECT_MESSAGE_TIMEOUT_CYCLE  5
        
        
        /*********************************************************************/
        // MAX_ROUTING_FAILURE is the number of failures of routing between
        // coordinators before such route is disabled in the decision of 
        // message route. Proper definition of this parameter helps to update
        // the available routes dynamically. This definition is only valid for
        // a coordinator.
        /*********************************************************************/
        #define MAX_ROUTING_FAILURE 8
        
        
        /*********************************************************************/
        // ACTIVE_SCAN_RESULT_SIZE defines the maximum number of active scan
        // results that can be received and recorded within one active scan.
        /*********************************************************************/
        #define ACTIVE_SCAN_RESULT_SIZE 4
            
            
        /*********************************************************************/
        // INDIRECT_MESSAGE_SIZE defines the maximum number of packets that
        // the device can store for the sleeping device(s)
        /*********************************************************************/
        #define INDIRECT_MESSAGE_SIZE   10
    
    
        /*********************************************************************/
        // INDIRECT_MESSAGE_TIMEOUT defines the timeout interval in seconds
        // for the stored packets for sleeping devices
        /*********************************************************************/
        #define INDIRECT_MESSAGE_TIMEOUT (ONE_SECOND * RFD_WAKEUP_INTERVAL * INDIRECT_MESSAGE_TIMEOUT_CYCLE)
    
    
        /*********************************************************************/
        // FA_BROADCAST_TIME defines the total number of times to broadcast
        // the channel hopping message to the rest of PAN, before the 
        // Frequency Agility initiator jump to the new channel
        /*********************************************************************/
        #define FA_BROADCAST_TIME           0x03
    
    
        /*********************************************************************/
        // RESYNC_TIMES defines the maximum number of times to try resynchronization
        // in all available channels before hand over the control to the application
        // layer
        /*********************************************************************/
        #define RESYNC_TIMES                0x03

        
        /*********************************************************************/
        // ENABLE_ENHANCED_DATA_REQUEST enables the Enhanced Data Request 
        // feature of P2P stack. It combines the message that is send from
        // the sleeping device with Data Request command upon wakeup, to save
        // 20% - 30% active time for sleeping device, thus prolong the battery
        // life.
        /*********************************************************************/
        //#define ENABLE_ENHANCED_DATA_REQUEST
        
        
        /*********************************************************************/
        // ENABLE_TIME_SYNC enables the Time Synchronizaiton feature of P2P
        // stack. It allows the FFD to coordinate the check-in interval of
        // sleeping device, thus allow one FFD to connect to many sleeping
        // device. Once Time Synchronization feature is enabled, following
        // parameters are also required to be defined:
        //      TIME_SYNC_SLOTS
        //      COUNTER_CRYSTAL_FREQ
        /*********************************************************************/
        //#define ENABLE_TIME_SYNC
        
        
        /*********************************************************************/
        // TIME_SYNC_SLOTS defines the total number of time slot available 
        // within one duty cycle. As a rule, the number of time slot must be
        // equal or higher than the total number of sleeping devices that are
        // connected to the FFD, so that each sleeping device can be assigned
        // to a time slot. The time slot period is calcualted by following 
        // formula:
        //      Time Slot Period = RFD_WAKEUP_INTERVAL / TIME_SYNC_SLOTS
        // The length of time slot period depends on the primary oscillator
        // accuracy on the FFD as well as the 32KHz crystal accuracy on sleeping
        // devices. 
        // The definition of TIME_SYNC_SLOTS is only valid if ENABLE_TIME_SYNC
        // is defined
        /*********************************************************************/
        #define TIME_SYNC_SLOTS             10
    
    
        /*********************************************************************/
        // COUNTER_CRYSTAL_FREQ defines the frequency of the crystal that 
        // is connected to the MCU counter to perform timing functionality
        // when MCU is in sleep. 
        /*********************************************************************/
        #define COUNTER_CRYSTAL_FREQ        32768
        
    #endif
#endif

