/* Based on "ConfigMiWiPRO.h" by Microchip, copyright as below, changes are:
 * Copyright (c) 2012, Newcastle University, UK.
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

/********************************************************************
* FileName:		ConfigMiWiPRO.h
* Dependencies: 
* Processor:	PIC18, PIC24F, PIC24H, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Hardware:		PICDEM Z, Explorer 16
* Complier:     Microchip C18 v3.04 or higher
*				Microchip C30 v2.03 or higher	
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice
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
*********************************************************************
* File Description:
*
*  This file provides configuration settings for P2P protocol stack
*
* Change History:
*  Rev   Date         Author        Description
*  4.1   6/3/2011     yfy           MAL v2011-06
********************************************************************/

#ifndef __CONFIGURE_MIWI_PRO_H

    #define __CONFIGURE_MIWI_PRO_H
    
    #include "ConfigApp.h"

    #if defined(PROTOCOL_MIWI_PRO)
        #include "WirelessProtocols/SymbolTime.h"
    
        /*********************************************************************/
        // ENABLE_DUMP will enable the stack to be able to print out the 
        // content of the P2P connection entry. It is useful in the debugging
        // process
        /*********************************************************************/
        #define ENABLE_DUMP

        /*********************************************************************/
        // ENABLE_MIWI_PRO_ACKNOWLEDGEMENT enables the MiWi PRO stack to
        // send back an acknowledgement packet when a MiWi PRO data packet
        // for application is received. This process will be handled 
        // automatically in the stack without application layer involvement.
        // MiWi PRO acknowledgement is ack in network layer. It may be sent 
        // across multiple hops and should be differiented with MAC ack.
        // When message is sent to a sleeping device, the MiWi PRO 
        // acknowledgemnt will be received later and notified by call 
        // back function MiApp_CB_RFDAcknowledgement. Otherwise, whether
        // receive MiWi PRO acknowledgement will be used to notify application
        // layer if transmission is successful.
        /*********************************************************************/
        //#define ENABLE_MIWI_PRO_ACKNOWLEDGEMENT
        
        /*********************************************************************/
        // NUM_COORDINATOR defines the maximum number of Coordinators that
        // the network can support. The possible numbers are 8, 16, 32 and 64.
        // NUM_COORDINAOTR definition is closedly associated with RAM and NVM
        // resources
        /*********************************************************************/
        #define NUM_COORDINATOR             64
        
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
        // MIWI_PRO_ACK_TIMEOUT is the timeout period in symbols for a node to
        // receive a MiWi PRO network layer acknowledgement. This parameter is 
        // for MiWi PRO network layer, not for MAC layer. MAC layer acknowledgement
        // timeout is handled in MiMAC layer.
        /*********************************************************************/
        #define MIWI_PRO_ACK_TIMEOUT            (ONE_SECOND)

        /*********************************************************************/
        // ENABLE_BROADCAST_TO_SLEEP_DEVICE enables messages broadcast to a 
        // sleeping device.
        /*********************************************************************/
        #define ENABLE_BROADCAST_TO_SLEEP_DEVICE
        
        /*********************************************************************/
        // When broadcasting to a sleeping device is enabled, it is hard for 
        // a parent node to track which end device has received the broadcast
        // message. However, if no tracking is provided, the end device may 
        // receive the same broadcast multiple times. MiWi PRO solves this problem
        // by tracking the broadcast message on sleeping device side. 
        // INDIRECT_MESSAGE_TIMEOUT_CYCLE defines the total number of messages
        // receives before the broadcast record times out. It is hard for a 
        // sleeping node to track timing, so tracking the number of message
        // received is a simpler way. 
        /*********************************************************************/
        #define INDIRECT_MESSAGE_TIMEOUT_CYCLE 5
        
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
        // RESYNC_TIMES defines the maximum number of times to try resynchronization
        // in all available channels before hand over the control to the application
        // layer
        /*********************************************************************/
        #define RESYNC_TIMES                0x03

        /*********************************************************************/
        // ENABLE_ROUTING_UPDATE enables the Coordinator capble device to 
        // periodically send out Routing Table Report to update the routing 
        // table. The interval of sending out routing table is defined as 
        // ROUTING_UPDATE_INTERVAL
        /*********************************************************************/
        #define ENABLE_ROUTING_UPDATE
        
        /*********************************************************************/
        // ROUTING_UPDATE_INTERVAL defines the interval in symbols that the
        // Coordinator capable device to send out Routing Table Report. This 
        // definition is only effective if ENABLE_ROUTING_UPDATE is defined.
        /*********************************************************************/
        #define ROUTING_UPDATE_INTERVAL  (ONE_HOUR)
        
        
        /*********************************************************************/
        // ROUTING_UPDATE_EXPIRATION defines the valid link expired after 
        // times of ROUTING_UPDATE_INTERVAL without receiving Routing Table 
        // Report. If this parameter is defined as 3, a link will becomes 
        // invalid after (3 * ROUTING_UPDATE_INTERVAL) symbols if no Routing 
        // Table Report is received from the other side of the link.
        /*********************************************************************/
        #define ROUTING_UPDATE_EXPIRATION   3
        
        
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
        
        /*********************************************************************/
        // COMM_INTERVAL defines communication interval in symbols of 
        // protocol services communications. An incomplete list of protocol
        // services that use COMM_INTERVAL are FAMILY_TREE_REPORT, 
        // ROUTING_TABLE_REPORT and CHANNLE_HOPPING_REQUEST.
        /*********************************************************************/
        #define COMM_INTERVAL               ONE_SECOND
        
        /*********************************************************************/
        // FAMILY_TREE_BROADCAST defines the number of broadcasts for Family 
        // Tree Report protocol service after a Coordinator joins the network. 
        // To ensure delivery of the Family Tree table, it is recommended to 
        // set this value higher than 1.
        /*********************************************************************/
        #define FAMILY_TREE_BROADCAST   3
        
        /*********************************************************************/
        // ROUTING_TABLE_BROADCAST defines the number of broadcasts for 
        // Routing Table Report. To ensure routing table accuracy on all
        // coordinators, it is recommended to set this value higher than 1.
        /*********************************************************************/
        #define ROUTING_TABLE_BROADCAST 3
        
        
        /*********************************************************************/
        // COMM_RSSI_THRESHOLD defines the minimum signal strength that is 
        // acceptable for a Coordinator to route messages through. A neighbor 
        // Coordinator that has signal strength that is higher than this 
        // threshold can be list as neighbor to be able to route message.
        // This setting usually is related to the signal strength 
        // interpretation for the specific RF transceiver. This value is 
        // represented by BYTE with 8-bit data width, thus should be less 
        // than 256.
        /*********************************************************************/
        #define COMM_RSSI_THRESHOLD     0x01
        
        /*********************************************************************/
        // RANDOM_DELAY_RANGE defines random delay range in milliseconds.
        // When rebroadcast a message, it is recommended that a Coordinator 
        // should introduce a random delay to avoid multiple Coordinators 
        // rebroadcast and collide the messages at the same time. The actual 
        // random delay will be randomly selected between 0 and 
        // RANDOM_DELAY_RANGE. This setting is recommended to be set 
        // higher than 20. The value is represented by BYTE with 8-bit data 
        //  width, thus should be less than 256.
        /*********************************************************************/
        #define RANDOM_DELAY_RANGE      200
        
        /*********************************************************************/
        // PACKET_RECORD_SIZE is the parameter that specifies the maximum
        // number of packet record available. Packet record is used to
        // track the broadcast messages so that the wireless node knows if
        // the same packet has been received before.
        /*********************************************************************/
        #define PACKET_RECORD_SIZE   5
        
        /*********************************************************************/
        // PACKET_RECORD_TIMEOUT defines the timeout in symbols for a
        // node to expire its packet record. The packet record is used
        // to track the received broadcast message and to prevent receiving
        // duplicate broadcast message. This definition is only valid for
        // a non-sleeping device.
        /*********************************************************************/
        #define PACKET_RECORD_TIMEOUT    (ONE_SECOND/2)


        /*********************************************************************/
        // FA_BROADCAST_TIME defines the total number of times to broadcast
        // the channel hopping message to the rest of PAN, before the 
        // Frequency Agility initiator jump to the new channel
        /*********************************************************************/
        #define FA_BROADCAST_TIME           0x03
        
        
        /*********************************************************************/
        // FA_MAX_NOISE_THRESHOLD defines the maximum noise level that a 
        // Coordinator accepts for a channel to avoid sending out Frequency 
        // Agility Against Channel protocol service when a new channel is 
        // proposed by PAN Coordinator to hop to during frequency agility 
        // operation. This setting usually is related to the signal strength 
        // interpretation for the specific RF transceiver. This value is 
        // represented by BYTE with 8-bit data width, thus should be less 
        // than 256.
        /*********************************************************************/        
        #define FA_MAX_NOISE_THRESHOLD   0x80
        
        /*********************************************************************/
        // FA_WAIT_TIMEOUT defines the timeouts in symbols during the 
        // frequency agility process. Those timeouts include timeout for all 
        // Coordinators to start energy scan; timeout for the PAN Coordinator 
        // to receive Frequency Agility Against Channel protocol service 
        // after a channel is suggested and timeout for all devices to jump 
        // to the new channel after receiving Frequency Agility Change Channel 
        // protocol service. To leave granularity for the protocol runs 
        // smoothly, it is recommended to set this timeout higher than 1 
        // second.
        /*********************************************************************/
        #define FA_WAIT_TIMEOUT             ((ONE_SECOND) * 2)
        
        /*********************************************************************/
        // FA_COMM_INTERVAL defines the time interval between broadcasting 
        // the frequency agility related message for FA_BROADCAST_TIME times.
        // To leave granularity for the protocol runs smoothly, it is 
        // recommended to set this timeout higher than a half second.
        /*********************************************************************/
        #define FA_COMM_INTERVAL            (ONE_SECOND)
        
    #endif
#endif

