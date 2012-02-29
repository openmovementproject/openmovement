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
 *
 * KL 26-11-2011
 * Created to locate all BT code in one place
 * Contains c file run_loop_embedded.c

*/

//Includes
#include <stddef.h> // NULL
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <btstack/hci_cmds.h>
#include <btstack/run_loop.h>
#include <btstack/sdp_util.h>
#include "Compiler.h"
#include "bt_control_cc256x.h"
#include "hci.h"
#include "l2cap.h"
#include "btstack_memory.h"
#include "remote_device_db.h"
#include "rfcomm.h"
#include "sdp.h"
#include "config.h"
#include "BTtasks.h"
#include "util.h"
#include "DisplayBasicFont.h"

/*
Compile the following .c files from the stack in the project:
btstack_memory.c
hci.c
hci_cmds.c
hci_transport_h4_dma.c
l2cap.c
l2cap_signaling.c
linked_list.c
memory_pool.c
remote_device_db_memory.c
rfcomm.c
run_loop.c
sdp.c
sdp_util.c
utils.c (this has its own hexdump so re-name any others)
*/
#define HEARTBEAT_PERIOD_MS 500

#include "btstack\src\run_loop_embedded.c"
#include "DisplayBasicFont.h"

/*Following is for BT state machine init and packet handler*/
enum bt_state bt_state_local = BT_OFF;
uint8_t   rfcomm_channel_nr = 1;
uint16_t  rfcomm_channel_id;
static uint8_t   rfcomm_send_credit = 0;
uint8_t   __attribute__((aligned)) spp_service_buffer[100]; // Cast to service_record_item_t in sdp.c

// Dummy data source
uint8_t data_buffer[14];

static void  heartbeat_handler(struct timer *ts);
static void prepare_data_packet(void); 
static void my_packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

// Code
void BTInit(void)
{
	int ipl_shadow = SRbits.IPL;
	SRbits.IPL= 7; 					// User interrupts off
	DisplayPrintLine("Init BTstack...\n\r",NEXT_LINE,1);
	
	/// GET STARTED ///
	btstack_memory_init();
    run_loop_init(RUN_LOOP_EMBEDDED);
	
    // init HCI
	hci_transport_t    * transport = hci_transport_h4_dma_instance();
	bt_control_t       * control   = bt_control_cc256x_instance();
    hci_uart_config_t  * config    = hci_uart_config_cc256x_instance();
    remote_device_db_t * remote_db = (remote_device_db_t *) &remote_device_db_memory;
	hci_init(transport, config, control, remote_db);
	
    // use eHCILL
    //bt_control_cc256x_enable_ehcill(1);

    // init L2CAP
    l2cap_init();
    l2cap_register_packet_handler(my_packet_handler);
    
    // init RFCOMM
    rfcomm_init();
    rfcomm_register_packet_handler(my_packet_handler);
    rfcomm_register_service_internal(NULL, rfcomm_channel_nr, 100);  // reserved channel, mtu=100

    // init SDP, create record for SPP and register with SDP
    sdp_init();
	memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    service_record_item_t * service_record_item = (service_record_item_t *) spp_service_buffer;
    sdp_create_spp_service( (uint8_t*) &service_record_item->service_record, 1, "SPP Accel");
    printf("SDP service buffer size: %u\n\r", (uint16_t) (sizeof(service_record_item_t) + de_get_len((uint8_t*) &service_record_item->service_record)));
    sdp_register_service_internal(NULL, service_record_item);
    
    // set one-shot timer
    timer_source_t heartbeat;
    heartbeat.process = &heartbeat_handler;
    run_loop_set_timer(&heartbeat, HEARTBEAT_PERIOD_MS);
    run_loop_add_timer(&heartbeat);

    // ready - enable irq used in h4 task
	SRbits.IPL = ipl_shadow;
    
 	// turn on!
	hci_power_control(HCI_POWER_ON);

    // go!
    //run_loop_execute();
	bt_state_local = BT_STARTING;	

	return;		
}

// Bluetooth logic
static void my_packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    bd_addr_t event_addr;
    uint8_t   rfcomm_channel_nr;
    uint16_t  mtu;
    int err;
    
	switch (packet_type) {
		case HCI_EVENT_PACKET:
			switch (packet[0]) {
					
				case BTSTACK_EVENT_STATE:
					// bt stack activated, get started - set local name
					if (packet[2] == HCI_STATE_WORKING) {
                        hci_send_cmd(&hci_write_local_name, "CWA3 device");
					}
					break;
				
				case HCI_EVENT_COMMAND_COMPLETE:
					if (COMMAND_COMPLETE_EVENT(packet, hci_read_bd_addr)){
						//char temp[36]; // On the stack!
                        bt_flip_addr(event_addr, &packet[6]);
                        //sprintf(temp,"BD-ADDR: %s\n\r", bd_addr_to_str(event_addr));
						DisplayPrintLine("BT init success",NEXT_LINE,1);
						bt_state_local = BT_INITIALISED; // By now we are fully initialised
                        break;
                    }
					if (COMMAND_COMPLETE_EVENT(packet, hci_write_local_name)){
                        hci_discoverable_control(1);
                        break;
                    }
                    break;

				case HCI_EVENT_LINK_KEY_REQUEST:
					// deny link key request
                    DisplayPrintLine("Link key request",NEXT_LINE,1);
                    bt_flip_addr(event_addr, &packet[2]);
					hci_send_cmd(&hci_link_key_request_negative_reply, &event_addr);
					break;
					
				case HCI_EVENT_PIN_CODE_REQUEST:
					// inform about pin code request
					DisplayPrintLine( "PIN = 0000",NEXT_LINE,1);
                    //printLine("Pin code request - using '0000'\n\r");
                    bt_flip_addr(event_addr, &packet[2]);
					hci_send_cmd(&hci_pin_code_request_reply, &event_addr, 4, "0000");
					break;
                
                case RFCOMM_EVENT_INCOMING_CONNECTION:
					// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
					bt_flip_addr(event_addr, &packet[2]); 
					rfcomm_channel_nr = packet[8];
					rfcomm_channel_id = READ_BT_16(packet, 9);
					//printf("RFCOMM channel %u requested for %s\n\r", rfcomm_channel_nr, bd_addr_to_str(event_addr));
                    rfcomm_accept_connection_internal(rfcomm_channel_id);
					break;
					
				case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
					// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
    				if (packet[2]) {
						//printf("RFCOMM channel open failed, status %u\n\r", packet[2]);
                        DisplayPrintLine("Connection failed :(",NEXT_LINE,1);
					} else {
						rfcomm_channel_id = READ_BT_16(packet, 12);
						mtu = READ_BT_16(packet, 14);
						//printf("\n\rRFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n\r", rfcomm_channel_id, mtu);
						DisplayPrintLine("Connection success :)",NEXT_LINE,1);
						bt_state_local = BT_CONNECTED;
					}
					break;
                    
                case DAEMON_EVENT_HCI_PACKET_SENT:
                case RFCOMM_EVENT_CREDITS:
                    if (!rfcomm_channel_id) break;
                    // try send
                    err = rfcomm_send_internal(rfcomm_channel_id, (uint8_t *)data_buffer, sizeof(data_buffer));
                    switch (err){
                        case 0:
                            prepare_data_packet();
                            break;
                        case BTSTACK_ACL_BUFFERS_FULL:
                            break;
                        default:
                           //printf("rfcomm_send_internal() -> err %d\n\r", err);
                        break;
                    }
                    break;
                    
                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    rfcomm_channel_id = 0;
                    break;
                
                default:
					{
						static unsigned char i,buffer[100];
						for(i=0;i<100;i++)
						{
							buffer[i] = *packet++;
						}
						Nop();
						Nop();
						Nop();
						Nop();
					}
                    break;
			}
            break;
		// Incoming data
        case RFCOMM_DATA_PACKET:
            // hack: truncate data (we know that the packet is at least on byte bigger
            packet[size] = 0;
            DisplayPrintLine((char*)packet,7,1);
            rfcomm_send_credit = 1;                
        default:
            break;
	}
}

static void  heartbeat_handler(struct timer *ts){
    if (rfcomm_send_credit){
        rfcomm_grant_credits(rfcomm_channel_id, 1);
        rfcomm_send_credit = 0;
    }
    run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
    run_loop_add_timer(ts);
}

// KL 2011 - task function form of runloop()
void BTtasks( void )
{
static data_source_t *ds;
static data_source_t *next;
static timer_source_t *ts;

    // process data sources
    for (ds = (data_source_t *) data_sources; ds != NULL ; ds = next){
        next = (data_source_t *) ds->item.next; // cache pointer to next data_source to allow data source to remove itself
        ds->process(ds);
    }
        
	#ifdef HAVE_TICK
    // process timers
    while (timers) {
        ts = (timer_source_t *) timers;
        if (ts->timeout > system_ticks) break;
        run_loop_remove_timer(ts);
        ts->process(ts);
    }
	#endif

    // disable IRQs and check if run loop iteration has been requested. if not, go to sleep
    hal_cpu_disable_irqs();
    if (trigger_event_received){
        hal_cpu_enable_irqs_and_sleep();
        return;
    }
    hal_cpu_enable_irqs();
        
	return;
}

void  prepare_data_packet(void)
{
	static unsigned long count=0;
	sprintf((char*)data_buffer,"Test%lu\r\n",count++);
	return;
} 
//EOF
