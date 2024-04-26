// KL 26-11-2011
// Created to locate all BT code in one place
// Contains c file run_loop_embedded.c

// Includes

#include <btstack/hci_cmds.h>
#include <btstack/run_loop.h>
#include <btstack/sdp_util.h>
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "hci.h"
#include "l2cap.h"
#include "btstack_memory.h"
#include "remote_device_db.h"
#include "rfcomm.h"
#include "sdp.h"
#include "config.h"
#include "Bluetooth/bt_control_cc256x.h"
#include "Bluetooth/Bluetooth.h"
#include "Bluetooth/BTtasks.h"
#include "Utils/fifo.h"

/* Globals */
// RFCOMM 
// Fifos
fifo_t bt_out_fifo = {0};
fifo_t bt_in_fifo = {0};
static unsigned char BtInBuffer[BT_IN_BUFFER_SIZE] = {0};		// Incoming
static unsigned char BtOutBuffer[BT_OUT_BUFFER_SIZE] = {0}; 	// To send

// Incoming data, called on receive from BTtasks 
static unsigned short GetDataIn(void* data ,unsigned short len);	// Called internally to add data to the receive fifo

// Outgoing data pointers used by RF comm
static unsigned short SendDataOut(void);							// called internally to send data whenever possible from BTTasks
static enum {NONE,PACKET,FIFO} last_out_source = NONE;
// Currently sending pointers - do not use
static volatile unsigned char* out_data_buffer	= NULL;
static volatile unsigned short out_data_buffer_len = 0;
// Queued outgoing data
volatile unsigned char* out_data_buffer_queued	= NULL;
volatile unsigned short out_data_buffer_len_queued = 0;

//#include "..\..\..\..\btstack\src\run_loop_embedded.c"
#include "..\..\btstack\src\run_loop_embedded.c"


/*Following is for BT state machine init and packet handler*/
// Set the following to provide different link keys
char* bt_link_key = BT_DEFAULT_LINK_KEY;
char* btDeviceName = BT_DEFAULT_NAME;

// Non-user variables
uint8_t   rfcomm_channel_nr = 1;
uint16_t  rfcomm_channel_id = 0;
// Cast to service_record_item_t in sdp.c - leave below variable, it contains all the rfcomm data (len>98 bytes)
uint8_t   __attribute__((aligned(2))) spp_service_buffer[100]; 
// Prototypes
extern void hal_uart_dma_init_off(void);
static void my_packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void BtDataOutTry(void);


#ifdef BT_FAST_RECEIVE
static char fastMode = 0;
extern void BtFastReceiveHandler(unsigned char *buffer, int size);
char BtFastReceive(unsigned char *packet, int size)
{
	#define BT_FAST_RECEIVE_BUFFER_SIZE 1024
	#define SLIP_END     0xC0                   // End of packet indicator
	#define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
	#define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
	#define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte
	static char buffer[BT_FAST_RECEIVE_BUFFER_SIZE];
	static char escaped = 0;
	static int offset = 0;
	int i;
	
	if (fastMode == 0)
	{
		for (i = 0; i < size; i++)
		{
			if (packet[i] == SLIP_END)
			{
				packet += i;
				size -= i;
				fastMode = 1;
				offset = 0;
				break;
			}
		}
		if (fastMode == 0) { return 0; }
	}
	
//LED_R = !LED_R;	
	
	for (i = 0; i < size; i++)
	{	
		if (packet[i] == SLIP_END) 
		{ 
			if (offset > 0)
			{
				BtFastReceiveHandler(buffer, offset);
			}	
			offset = 0; escaped = 0; 
		}
		else if (packet[i] == SLIP_ESC) { escaped = 1; }
		else if (escaped && packet[i] == SLIP_ESC_END && offset < BT_FAST_RECEIVE_BUFFER_SIZE) { buffer[offset++] = SLIP_END; escaped = 0; }
		else if (escaped && packet[i] == SLIP_ESC_ESC && offset < BT_FAST_RECEIVE_BUFFER_SIZE) { buffer[offset++] = SLIP_ESC; escaped = 0; }
		else if (offset < BT_FAST_RECEIVE_BUFFER_SIZE) { buffer[offset++] = packet[i]; escaped = 0; }
	}
	
	return 1;
}
#endif


// Code
void BTInit(void)
{
	#ifdef ENABLE_LOG_INFO
	char buffer[32];
	#endif

	int ipl_shadow = SRbits.IPL;
	SRbits.IPL= 7; 					// User interrupts off
	#ifdef ENABLE_LOG_INFO
	printLine("Init BTstack...\n\r");	
	#endif

	// Init send receive fifos
	FifoInit(&bt_out_fifo, 1, 	BT_OUT_BUFFER_SIZE, BtOutBuffer);
	FifoInit(&bt_in_fifo, 	1, 	BT_IN_BUFFER_SIZE, 	BtInBuffer);

	// Out buffer pointers
	last_out_source = NONE;
	out_data_buffer_len = 0;
	out_data_buffer = NULL;
	out_data_buffer_len_queued = 0;
	out_data_buffer_queued	= NULL;

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
	#ifdef HAVE_EHCILL
    	bt_control_cc256x_enable_ehcill(1);
	#else
    	bt_control_cc256x_enable_ehcill(0);
	#endif


    // init L2CAP
    l2cap_init();
    l2cap_register_packet_handler(my_packet_handler);
    
    // init RFCOMM
    rfcomm_init();
    rfcomm_register_packet_handler(my_packet_handler);
    //rfcomm_register_service_internal(NULL, rfcomm_channel_nr, 100);  // reserved channel, mtu=100
#ifdef BT_FAST_RECEIVE
	rfcomm_register_service_with_initial_credits_internal(NULL, rfcomm_channel_nr, 64, 10);  // reserved channel, mtu=100, 10 credits
#else
	rfcomm_register_service_with_initial_credits_internal(NULL, rfcomm_channel_nr, 100, 1);  // reserved channel, mtu=100, 1 credit
#endif

    // init SDP, create record for SPP and register with SDP
    sdp_init();
	memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    service_record_item_t * service_record_item = (service_record_item_t *) spp_service_buffer;
    sdp_create_spp_service( (uint8_t*) &service_record_item->service_record, 1, "SPP"); 

	#ifdef ENABLE_LOG_INFO
	sprintf(buffer,"SDP buff size:%u\n\r", (uint16_t) (sizeof(service_record_item_t) + de_get_len((uint8_t*) &service_record_item->service_record)));
	printLine(buffer);
	#endif

    sdp_register_service_internal(NULL, service_record_item);
    
    // ready - enable irq used in h4 task
	SRbits.IPL = ipl_shadow;
    
 	// turn on!
	hci_power_control(HCI_POWER_ON);

    // go!
	bluetoothState = BT_STARTING;	

	return;		
}

void BTOff(void)
{
	// State information lost - will need re-initialising
	hal_uart_dma_init_off();	// Just power off module
	bluetoothState = BT_OFF;	// Set state
	return;
}

void BTClearDataBuffers(void)
{
	// Reset send vars - this indicates we are able to send data if we want
	last_out_source = NONE;
	out_data_buffer_len = 0;
	out_data_buffer = NULL;
	out_data_buffer_len_queued = 0;
	out_data_buffer_queued	= NULL;
	// Init send+receive fifos 
	FifoInit(&bt_out_fifo, 1, 	BT_OUT_BUFFER_SIZE, BtOutBuffer);
	FifoInit(&bt_in_fifo, 	1, 	BT_IN_BUFFER_SIZE, 	BtInBuffer);
}

// Bluetooth logic
static void my_packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	#ifdef ENABLE_LOG_INFO
	  	char buffer[32];
	#endif
	bd_addr_t event_addr;
	uint8_t   rfcomm_channel_nr;
	uint16_t  mtu;
//	int err;

	switch (packet_type) {
		case HCI_EVENT_PACKET:
			switch (packet[0]) {
					
				case BTSTACK_EVENT_STATE:
					// bt stack activated, get started - set local name
					if (packet[2] == HCI_STATE_WORKING) {
                        hci_send_cmd(&hci_write_local_name, btDeviceName);
					}
					break;
				
				case HCI_EVENT_COMMAND_COMPLETE:
					if (COMMAND_COMPLETE_EVENT(packet, hci_read_bd_addr)){
                        bt_flip_addr(event_addr, &packet[6]);
						#ifdef ENABLE_LOG_INFO
						printLine("BT init over\r\n");
                        sprintf(buffer,"I:%s\n\r", bd_addr_to_str(event_addr));
						printLine(buffer);
						#endif
                        break;
                    }
					if (COMMAND_COMPLETE_EVENT(packet, hci_write_local_name)){
                        hci_discoverable_control(1);
						// Providing the init completed without failure, now we are fully initialised and discoverable
						if (bluetoothState != BT_FAIL) bluetoothState = BT_INITIALISED; 
                        break;
                    }
                    break;

				case HCI_EVENT_LINK_KEY_REQUEST:
					// link key request
					#ifdef ENABLE_LOG_INFO
                    printLine("Link key request\r\n");
					#endif
                    bt_flip_addr(event_addr, &packet[2]);
					hci_send_cmd(&hci_link_key_request_negative_reply, &event_addr);
					break;
					
				case HCI_EVENT_PIN_CODE_REQUEST:
					// inform about pin code request
					#ifdef ENABLE_LOG_INFO
                    printLine("Pin req\n\r");
					#endif
                    bt_flip_addr(event_addr, &packet[2]);
					hci_send_cmd(&hci_pin_code_request_reply, &event_addr, 4, bt_link_key);
					break;
                
                case RFCOMM_EVENT_INCOMING_CONNECTION:
					// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
					bt_flip_addr(event_addr, &packet[2]); 
					rfcomm_channel_nr = packet[8];
					rfcomm_channel_id = READ_BT_16(packet, 9);
					#ifdef ENABLE_LOG_INFO
					sprintf(buffer,"RFCOMM ch %u\n\r", rfcomm_channel_nr );
 					printLine(buffer);
					sprintf(buffer,"U:%s\n\r",bd_addr_to_str(event_addr));
 					printLine(buffer);
					#endif
	                rfcomm_accept_connection_internal(rfcomm_channel_id);
					break;
					
				case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
					// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
#ifdef BT_FAST_RECEIVE
	fastMode = 0;
#endif					
    				if (packet[2]) {
						#ifdef ENABLE_LOG_INFO
						sprintf(buffer,"RFCOMM fail %u\n\r", packet[2]);
						printLine(buffer);
						#endif
					} else {
						rfcomm_channel_id = READ_BT_16(packet, 12);
						mtu = READ_BT_16(packet, 14);
						#ifdef ENABLE_LOG_INFO
						sprintf(buffer,"RFCOMM %u open\n\r", rfcomm_channel_id);
						printLine(buffer);
						sprintf(buffer,"Max frame size %u\n\r", mtu);
						printLine(buffer);
						#endif
						bluetoothState = BT_CONNECTED;
						//BtDataOutTry(); // Check for outgoing data
					}
					break;
                    
                case DAEMON_EVENT_HCI_PACKET_SENT:
                case RFCOMM_EVENT_CREDITS:
					// An outgoing packet sent successfully or new outgoing credits were granted 
					// This new function will block if re-entrant call is attemted from ISR scope
					BtDataOutTry();
					break;
                    
                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    rfcomm_channel_id = 0;
					#ifdef ENABLE_LOG_INFO
					printLine("RFCOMM closed\n\r");
					#endif
					if (bluetoothState >= BT_CONNECTED)
						bluetoothState = BT_INITIALISED;
#ifdef BT_FAST_RECEIVE
	fastMode = 0;
#endif
                    break;
                default:
					{
						#if defined(BT_DUMP_PACKET_HANDLER)
							// All unhandled packets
							unsigned int i = (*(packet+1))+2;
							#ifdef ENABLE_LOG_ERROR
							printLine("ERROR: Unhandled packet: ");
							for(;i>0;i--)
							{
								printf("%02X:",*packet++);
							}
							printLine("\r\n");
							#endif
						#endif
					}
                    break;
			}
            break;
		// Incoming data
        case RFCOMM_DATA_PACKET:
#ifdef BT_FAST_RECEIVE
	if (!BtFastReceive((unsigned char *)packet, (int)size))
	{
		GetDataIn((void*)packet,size);
	}	
	rfcomm_grant_credits(rfcomm_channel_id, 1);	// Grant new creditis to sender
#else
			if (GetDataIn((void*)packet,size))				// If the buffers are not over full
				rfcomm_grant_credits(rfcomm_channel_id, 1);	// Grant new credits to sender
#endif
       default:
            break;
	}
	{
		// To dump ALL packets!
		#if defined(BT_DUMP_PACKET_HANDLER)
			unsigned int i = (*(packet+1))+2;
			printLine("Packet: ");
			for(;i>0;i--)
			{
				printf("%02X:",*packet++);
			}
			printLine("\r\n");
		#endif
	}
}

// Call this to keep transmitting data
static void BtDataOutTry(void)
{
	static volatile unsigned char alreadycalled = FALSE; // Internal context saving and blocking from ISR
#ifdef ENABLE_LOG_ERROR
  	char buffer[32];
#endif
	int err;

	// Prevent ISR calls to this function from interrupting the flow control
	if (alreadycalled == TRUE) return;
	// After this instruction there are potential re-entrant problems - so it blocks
	alreadycalled = TRUE;

	// Not sure if this does anything - same as next line really
	if (bluetoothState != BT_CONNECTED)
		{alreadycalled = FALSE;	return;}

	// Function to try and send data
	if (!rfcomm_channel_id ) 
		{alreadycalled = FALSE; return;}// If no channel id has been set then break here

	// Check for new data ONLY IF there is none queued, if there is none then break
	if ((out_data_buffer_len == 0) || (out_data_buffer==NULL)) // If there is no currently pending data, check for new data
	{
		if (!SendDataOut()) 			// Check for new outgoing data
		{
			alreadycalled = FALSE; 
			return; 				
		}
	} 	

	// Try and send pending outgoing data, checking result
	err = rfcomm_send_internal(rfcomm_channel_id, (uint8_t *)out_data_buffer, (uint16_t )out_data_buffer_len);

	// Check for errors
	switch (err){
		case 0:							// The data sent (or was queued for transmission
			if (SendDataOut())			// See if there is new data (removed old from fifo too)
				embedded_trigger();		// New data - trigger iteration of run loop
			break;
		case BTSTACK_ACL_BUFFERS_FULL:		// We are waiting for the radio to send
		case RFCOMM_NO_OUTGOING_CREDITS:	// We are waiting for the host to ack last data
		// The current buffer is still being sent
			break;
		default:
		#ifdef ENABLE_LOG_ERROR
		sprintf(buffer,"RFCOM err:%d\n\r", err);
		printLine(buffer);
		#endif
		break;
	}

	alreadycalled = FALSE;
	return;
}

// Send data out either queued packet or serialIO - calls to this funct = last data has been sent
unsigned short SendDataOut(void)
{
	// What was the last thing to send
	if (last_out_source == FIFO)
	{
		// FIFO out data was processed last - remove from fifo
		FifoExternallyRemoved(&bt_out_fifo, out_data_buffer_len);
	}
	if (last_out_source == PACKET)
	{
		// Packet out data was processed last
		// No need to do anything
	}

	// See if there is outgoing data in the serial buffers - sets out_data_buffer pointer
	out_data_buffer_len = FifoContiguousEntries(&bt_out_fifo, (void*)(&out_data_buffer));	
	if (out_data_buffer_len)
	{
		if (out_data_buffer_len > BT_MAX_RFCOMM_SEND_SIZE)	// If there is too much data
			out_data_buffer_len = BT_MAX_RFCOMM_SEND_SIZE;  // Clamp to max
		last_out_source = FIFO;
		return out_data_buffer_len; // Allow it to send - must call BTTasks to send
	}

	// See if there is a queued packet
	if ((out_data_buffer_len_queued) && (out_data_buffer_queued != NULL))
	{
		// Set pointers to new data
		out_data_buffer_len = out_data_buffer_len_queued;
		out_data_buffer = out_data_buffer_queued;
		// Clear set of queued data pointers
		out_data_buffer_len_queued = 0; 
		out_data_buffer_queued = NULL;
		
		if (out_data_buffer_len > BT_MAX_RFCOMM_SEND_SIZE)	// If there is too much data
			out_data_buffer_len = BT_MAX_RFCOMM_SEND_SIZE;  // Clamp to max
		last_out_source = PACKET;
		return out_data_buffer_len; // Allow it to send - must call BTTasks to send
	}

	// Reset send vars - this indicates we are able to send data if we want
	last_out_source = NONE;
	out_data_buffer_len = 0;
	out_data_buffer = NULL;
	out_data_buffer_len_queued = 0;
	out_data_buffer_queued	= NULL;
	return (0); 					// No data to send
} 

// Get data in - always serialIO processed, data FROM external connection
unsigned short GetDataIn(void* data ,unsigned short len)
{
	// Just stick incoming chars to the in fifo
	return (FifoPush(&bt_in_fifo, data, len));
} 

// KL 2011 - task function form of runloop()
// Does at least one interration of the loop each time its called
unsigned int BTtasks( void )
{
	data_source_t *ds;
	data_source_t *next;
	timer_source_t *ts;
	unsigned char repeat = FALSE;

	// Load up outgoing buffers if possible, following loop will send/recieve data
	BtDataOutTry();				// Allow single call to restart stack

	do {
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
	
	    // Check if run loop interation required
	    hal_cpu_disable_irqs(); 	// Guard trigger read (it is set in interrupts)
		repeat = trigger_event_received;
		trigger_event_received = 0;	// Clear the trigger flag
		hal_cpu_enable_irqs();		// Re-enable ints
	
	}while(repeat);	// Repeat until no more interations are required

	// In the main application we should implement the sleep function
	// The sleep function should call the tasks again if it wakes on:
	// UART RX
	// UART TX
	// TIMER1 (BT timer source)
	// UART CTS EDGE (eHCILL trigger)
	// All these call embedded trigger so we can use this

	return bluetoothState;
}

//EOF
