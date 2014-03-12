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

// Bluetooth Application
// Karim Ladha, 2013-2014

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "TimeDelay.h"
#include "Utils/fifo.h"
#include "hci phy.h"
#include "hci + le.h"
#include "l2cap + le.h"
#include "sdp.h"
#include "rfcomm.h"
#include "ble att.h"
#include "bt app.h"
#include "bt nvm.h"
#include "bt config.h"
#include "debug.h"
// Timer specific variables
#ifdef USE_T1_FOR_TICK
	#include "Peripherals/Timer1.h"
#elif defined(USE_RTC_FOR_TICK)
	#include "Peripherals/Rtc.h" // For timeout
#else
	#warning "Default behaviour"
	#include "Peripherals/Rtc.h" // For timeout
#endif

#ifdef BT_USB_DEBUG
	#include "USB\USB_CDC_MSD.h"
#endif

#ifdef ENABLE_LE_MODE
#include "Profile.h"
#endif

// DEBUG
#if  DEBUG_APP
	static const char* file = "btapp.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "btapp.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

/* Globals */
// Basic state variables from older API
bt_state_t bluetoothState = BT_OFF;
bt_power_state_t bluetoothPowerState = BT_STANDBY;

// Serial Fifos
fifo_t bt_out_fifo = {0};
fifo_t bt_in_fifo = {0};
static unsigned char BtInBuffer[BT_IN_BUFFER_SIZE] = {0};		// Incoming
static unsigned char BtOutBuffer[BT_OUT_BUFFER_SIZE] = {0}; 	// To send
// Currently sending pointers - do not use
static volatile unsigned char* out_data_buffer	= NULL;
static volatile unsigned short out_data_buffer_len = 0;
// Queued outgoing data - user
volatile unsigned char* out_data_buffer_queued	= NULL;
volatile unsigned short out_data_buffer_len_queued = 0;

#ifdef ENABLE_LE_MODE
#ifdef ATT_USE_BLE_SERIAL	
	// Serial output handle (global needed for state)
	attOutHandle_t bleSerialOut = {0};
#endif
unsigned char bleIndPending = FALSE;
#endif

// Prototypes
// Serial data send receive - private, see header for public
static unsigned char RfcommDataReceived(const unsigned char* data, unsigned short len); // Intalled to rfcomm
static unsigned short LastDataHasSent(void);					// Called when last data has been sent, checks for new data
static unsigned short OutputDataTry(void);						// Installed in hci phyical layer
static unsigned short RfcommSendSerialDataOut(void);			// Local, non-user, rfcomm send
static void BTTimeouts(void);									// Installed into hci comms layer

#ifdef ENABLE_LE_MODE
// BLE Att support 
static void BleDataIn 	(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset);
static void BleDataOut (unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset);
static unsigned short BleGetHandleLen (unsigned short handle);
static void BleDataIndSent(void);
#ifdef ATT_USE_BLE_SERIAL
static unsigned short BleSendSerialDataOut(void);
#endif
#endif

// Code 
unsigned char BTInit(void)
{
	// Establish comms, power up, init, early out on fail
	BTEvent(BT_EVENT_STARTING);
	if(HciPhyInitOn() != HCI_PHY_SUCCESS)
	{
		BTEvent(BT_EVENT_FAILED);
		return FALSE;
	}

	HciInit();		// Initialise hci layer
	L2capInit();	// Initialise the L2cap layer
    SDP_create(); 	// Based on smalltooth for now
    RFCOMM_create();// Based on smalltooth for now

	#ifdef ENABLE_LE_MODE
	// Init Ble layers
	BleAttInit();		// Attribute layer
	UserProfileInit();	// User profile
	#ifdef ATT_USE_BLE_SERIAL	
		memset(&bleSerialOut,0,sizeof(attOutHandle_t));// Zero serial out handle
	#endif
	bleIndPending = FALSE;
	// Att integration
	BleSetWriteCallback(BleDataIn);  		// Initiated by client side
	BleSetReadCallback (BleDataOut); 		// Initiated by client side
	BleSetGetExtAttLen(BleGetHandleLen); 	// Ble stack must know lengths of dynamic vars
	BleSetValSentCallBack(BleDataIndSent);	// Install indicator sent call back
	#endif

	// Hci integration
	HciInstallDataTryCB(OutputDataTry);				// Called every event to look for outbound data		
	HciInstallTimeOutCB(BTTimeouts);				// Called every event to provide timeouts

	// Rfcomm integration
	RfCommInstallDataCB(RfcommDataReceived);		// Called when incomming data arrives	

	// Init send & receive fifos
	BTClearDataBuffers();

	// Initialise the name,pin etc.
	InitNvm();
	return TRUE;		
}

void BTStart(void)
{
	// Enable event handler
	BT_EVENT_HANDLER_ARM();
	// Trigger stack to start
	EventTrigger();	
}

void BTDeinit(void)
{
	// Clear states
	if(hci_connection.isConnected)
	{
		RfCommDisconnect(RFCOMM_CH_DATA);// Sends disconnect frame over RFCOMM
		L2capDisconnect(); 		// Sends L2CAP channel disconnect to all open CIDs
	}
	if(hci_connection.isConnected)
	{
		hci_connection.connectionMode = -1; // Do not reinit on disc
		HciDisconnect();		// Disconnect current handles
	}	// Wait for HCI layer to disconnect radio
	unsigned short timeout = 10000; // 100ms
	while(timeout-- && hci_connection.isConnected){Delay10us(1);}

	HciPhyInitOff(); // Power down module
	BTEvent(BT_EVENT_TURN_OFF);
	return;
}

// Helper function
char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice)
{
	if(bluetoothState < BT_INITIALISED)
	{
		// Clock up, faster
		unsigned short cosc = OSCCONbits.COSC;
		CLOCK_PLL();

		// Initialise module 
		if(BTInit()) // Sets up everything - can modify the vars before BTstart()
		{

			// Has the user set the name - takes precedence
			if(deviceName != NULL)
			{
				char *name = GetBTName();
				unsigned short len = strlen(deviceName);
				if(len > MAX_BT_NAME_LEN)len = MAX_BT_NAME_LEN;
				memcpy(name,deviceName,len);
				name[len] = '\0';
			}
	
			// Set COD if non zero - set external variable
			if (classOfDevice) device_cod_local = classOfDevice;

			BTStart();

			unsigned long timeout = 500000ul;// 5 seconds
			while	((bluetoothState < BT_INITIALISED)&&
					(bluetoothState != BT_FAIL)	&&
					(timeout--)) // Wait for initalisation of hci layer
			{
				#ifdef BT_USB_DEBUG
					USBCDCWait();
				#endif
				Delay10us(1);
			}

			// Check it worked
			if (bluetoothState < BT_INITIALISED)
			{
				BTDeinit(); // Power off hardware too
			}
		}		

		// Switch back clock
		if(cosc != 0b001)
		{
			HciPhyPause();
			CLOCK_INTOSC();
			HciPhyAdaptClock(); 
			HciPhyPlay();
		}

	} // If bluetooth is already initialised then this just returns
	return (bluetoothState >= BT_INITIALISED);
}

void BTTasks(void)
{
	EventTrigger();  // Safe to call whenever, will trigger ougoing buffers to be sent
}

// Rfcomm data identifier tag for last sent packet
static enum {NONE,PACKET,FIFO} last_out_source = NONE;
void BTClearDataBuffers(void)
{
	// Reset send vars - this indicates we are able to send data if we want
	last_out_source = NONE;
	out_data_buffer_len = 0;
	out_data_buffer = NULL;
	out_data_buffer_len_queued = 0;
	out_data_buffer_queued	= NULL;
	#if defined(ENABLE_LE_MODE) && defined(ATT_USE_BLE_SERIAL)
	bleSerialOut.attLen = 0;
	bleSerialOut.attData = NULL;
	#endif
	// Init send+receive fifos 
	FifoInit(&bt_out_fifo, 1, 	BT_OUT_BUFFER_SIZE, BtOutBuffer);
	FifoInit(&bt_in_fifo, 	1, 	BT_IN_BUFFER_SIZE, 	BtInBuffer);
}

unsigned char RfcommSendData(const void* source, unsigned short num)
{
	if (out_data_buffer_queued != NULL)
		return FALSE;
	if ((num == 0) || (source == NULL))
		return TRUE;

	// This will be sent using calls to tasks
	out_data_buffer_queued	= (unsigned char*)source;
	out_data_buffer_len_queued = num;
	return TRUE;
}

unsigned char RfcommTxBusy(void) // Note: User data only, not uart data/cmds
{
	if(out_data_buffer_queued != NULL || out_data_buffer != NULL) 
		return TRUE; // Currently sending something
	else
		return FALSE;
}

void BTEvent(unsigned short event)
{
	switch(event){
		case BT_EVENT_TURN_OFF	:	{
			bluetoothPowerState = BT_STANDBY;
			bluetoothState = BT_OFF;
			break;
		}
		case BT_EVENT_STARTING	:	{
			bluetoothPowerState = BT_ACTIVE;
			bluetoothState = BT_STARTING;
			break;
		}
		case BT_EVENT_FAILED	:		{
			bluetoothState = BT_FAIL;
			break;
		}
		case BT_EVENT_INITIALISED : {
			bluetoothState = BT_INITIALISED;
			break;
		}
		case BT_EVENT_CONNECTED :	{
			bluetoothState = BT_INITIALISED;
			break;
		}
		case BT_EVENT_DISCONNECTED:	{
			L2capInit();	// Reset upper layers
			SDP_create(); 	// Reset upper layers
			RFCOMM_create();// Reset upper layers
			BTClearDataBuffers(); // Erase outgoing
			bluetoothState = BT_INITIALISED;
			break;
		}
		case BT_EVENT_CONNECTION_REQ :{
			bluetoothState = BT_CONNECTION_REQUEST;
			bluetoothPowerState = BT_ACTIVE;
			break;
		}
		case BT_EVENT_COMM_OPEN:	{
			bluetoothState = BT_CONNECTED;
			break;
		}
		case BT_EVENT_COMM_CLOSED:	{
			bluetoothState = BT_INITIALISED;
			break;
		}
		case BT_EVENT_RADIO_ASLEEP	:{
			bluetoothPowerState = BT_SLEEP;
			break;
		}
		case BT_EVENT_RADIO_AWAKE:	{
			bluetoothPowerState = BT_IDLE;
			break;
		}
		default : break;
	}
	return;
}
/**********************************************************************************/
/*PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE PRIVATE */
/**********************************************************************************/

// All timeout handlers are modelled on this function and installed in the hci low level
static void BTTimeouts(void) // Send only possible if radioAwake
{
	// Timeout tasks done here - once per second
	static unsigned short lastTime = 0xffff; 
	if(lastTime != rtcTicksSeconds)
	{	
		// Call timeout counters for different layers
		lastTime = rtcTicksSeconds;
		// Attribute indicator timeout
		#ifdef ENABLE_LE_MODE
		BleTimeoutHandler();
		#endif
		// HCI layer timeouts, will disconnect device
		HciTimeoutHandler();
		// L2CAP layer timeouts, uses echos so must be clear to send
		L2capTimeoutHandler();
		// Return value
	}
}

// Called externally
static unsigned char RfcommDataReceived(const unsigned char* data, unsigned short len)
{
	FifoPush(&bt_in_fifo, (void*)data, len);
    return TRUE;
}

// Try sending data - checks for outgoing data, installed in event ISR
static unsigned short OutputDataTry(void)
{
	// Length of total sent bytes
	unsigned short serialLen = 0;

	// If we are not connected then bail. Can't send anyway.
	if (bluetoothState != BT_CONNECTED)	return 0; 				   

	// If we are connected and there is no data queued
	if ((out_data_buffer_len == 0) || (out_data_buffer==NULL))
	{
		// Check for new data
		serialLen = LastDataHasSent(); 
	}
	else 
	{
		serialLen = out_data_buffer_len;
	}

	// Check connection type before trying to send - BR connection = RFCOMM
	if((serialLen!=0)&&(hci_connection.connectionType == HCI_CONN_TYPE_BR) && (HciAclCanSend()))
	{
		serialLen = RfcommSendSerialDataOut();	// RFCOMM send handler
	}
	#ifdef ENABLE_LE_MODE
	else if (hci_connection.connectionType == HCI_CONN_TYPE_LE)
	{
		#ifdef ATT_USE_BLE_SERIAL
		if (serialLen!=0)
		{
			serialLen = BleSendSerialDataOut();	// LE serial port service send handler
		}
		#endif
		// BLE Send user indications and notifications if not indicating
		if(bleIndPending == FALSE)
		{ 
			if (serialLen==0) serialLen = UserSendIndications();
			if (serialLen==0) serialLen = UserSendNotifications();
		}
	}
	#endif

	// Return data count
	return serialLen;
}

static unsigned short RfcommSendSerialDataOut(void)
{
	// By this point, we know there is data available and there is a connection
	ASSERT(out_data_buffer_len <= RfcommMaxPayload());		
	if(RfCommSendPacket((const unsigned char *)out_data_buffer, out_data_buffer_len)) 	// Send it
	{
		DBG_INFO("->rfcomm\n")
		unsigned short sentLen = out_data_buffer_len;
		LastDataHasSent(); // If it sent, call this to indicate it was sent
		return sentLen;
	}
	// Data failed to send
	DBG_INFO("!rfcomm\n")
	return 0;
}

// Only call when the output buffers are clear, i.e. data was sent, returns len of next data
static unsigned short LastDataHasSent(void)
{
	// What was the last thing to send
	if (last_out_source == FIFO)
	{
		// FIFO out data was processed last - remove from fifo
		FifoExternallyRemoved(&bt_out_fifo, out_data_buffer_len);
	}
	else if (last_out_source == PACKET)
	{
		// Packet out data was processed last
		// No need to do anything
	}

	// See if there is outgoing data in the serial buffers - sets out_data_buffer pointer
	out_data_buffer_len = FifoContiguousEntries(&bt_out_fifo, (void*)(&out_data_buffer));

	// Serial i/o takes precedence over data packets
	if(out_data_buffer_len)
	{	
		// Set fifo flag so it is removed on next call
		// out_data_buffer_len and out_data_buffer set in above call
		last_out_source = FIFO;
	}
	// See if there is a queued packet
	else if ((out_data_buffer_len_queued) && (out_data_buffer_queued != NULL))
	{
		// Set pointers to new data
		out_data_buffer_len = out_data_buffer_len_queued;
		out_data_buffer = out_data_buffer_queued;
		// Clear set of queued data pointers
		out_data_buffer_len_queued = 0; 
		out_data_buffer_queued = NULL;
		// Set external buffer flag
		last_out_source = PACKET;
	}
	else
	{
		// Set pointers to new data
		out_data_buffer_len = out_data_buffer_len_queued;
		out_data_buffer = out_data_buffer_queued;
		// Clear set of queued data pointers
		out_data_buffer_len_queued = 0; 
		out_data_buffer_queued = NULL;
		// No outgoing data
		last_out_source = NONE;
		return 0;
	}
	
	// Check connection type before trying to send - BR connection = RFCOMM
	if(hci_connection.connectionType == HCI_CONN_TYPE_BR)
	{
		// For RFCOMM, truncate max packet len here
		if (out_data_buffer_len > RfcommMaxPayload())	// If there is too much data
			out_data_buffer_len = RfcommMaxPayload();  	// Clamp to max
		return out_data_buffer_len; 
	}
	#ifdef ENABLE_LE_MODE
	else if (hci_connection.connectionType == HCI_CONN_TYPE_LE)
	{
		// For LE ATT, packet is fragmented during send automatically
		return out_data_buffer_len; 
	}
	#endif
	else
	{
		// No connection, clear state
		last_out_source = NONE;
		out_data_buffer_len = 0;
		out_data_buffer = NULL;
		out_data_buffer_len_queued = 0;
		out_data_buffer_queued	= NULL;
		// No data can send
		return 0; 					
	}
	return 0;
} 

// LE support after this point
#ifdef ENABLE_LE_MODE

void BleDataIn 	(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset)
{
	DBG_INFO("attwr ha:%04X,ln:%u,of:%u, dt:",handle,len,offset);
	DBG_DUMP(data, len);
	switch(handle){
		#ifdef ATT_USE_BLE_SERIAL
		case ATT_SERIAL_IN_HANDLE : // Set it profile, writes to this append to the usual serial buffer
			// Process the command in a command handler
			FifoPush(&bt_in_fifo, (void*)data, len);
			#ifdef LE_ADD_CR_TO_CMDS
			FifoPush(&bt_in_fifo, "\r" , 1); // Manufacture cr
			#endif
			break;
		#endif
		default :
			UserAttributeWrite((void*)data, len, handle, offset);
			break;
	}
	return;
}

// Called BY the stack to READ DYNAMIC external variables
void BleDataOut (unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset)
{
	DBG_INFO("attrd ha:%04X,ln:%u,os:%u\n",handle,len,offset);
	switch(handle){
		default:
			UserAttributeRead((void*)data, len, handle, offset);
			break;
	}
	return;
}

// Called by the stack to READ HANDLE LENGTHS
unsigned short BleGetHandleLen (unsigned short handle)
{
	switch(handle){
		#ifdef ATT_USE_BLE_SERIAL
		case ATT_SERIAL_IN_HANDLE :
			/*Careful, the stack will error if the incoming data
			  packet is longer than this value and not callback.*/
			return 32; // Max len user can write into fifo at once (write more than once)
			break;
		#endif
		default:
			return UserAttributeReadLen(handle);
			break;
	}	
}

void BleDataIndSent(void) // Called back externally
{
	bleIndPending = FALSE;
}

#ifdef ATT_USE_BLE_SERIAL
unsigned short BleSendSerialDataOut(void)
{
	// Issue with BLE, indications > mtu-3 are not possible, mtu = 23
	// State
	static enum {READY,SENDING_PARTS,SENDING_LAST} local_state = READY; 
	static unsigned short sent;
	unsigned short max_len = BleAttGetMtu() - 3;
	unsigned short numToSend;

	// Has user enabled output - dump data if not
	if(bleSerialOut.attCfg == 0)
	{
		// Indication(s) have all sent
		DBG_INFO("att serial blocked\n")
		local_state = READY;
		// Remove outgoing data from fifo/data source (handle retains last data)
		LastDataHasSent();	
		return out_data_buffer_len;
	}

	// Check there is controller buffer space for another out packet
	if(hci_connection.packetsToAck >= hci_connection.controllerNumLeBuffers)
		return 0;

	if((bleSerialOut.attCfg & ATT_CFG_INDICATE) && (bleIndPending == TRUE))
		return 0; // Still sending last indication

	// Send a(nother) chunk as indication
	switch (local_state) {
		case READY : {	// First state - init vars
						sent = 0;
						local_state = SENDING_PARTS;
						//break; Fall through
						}				
		case SENDING_PARTS : {
						DBG_INFO("->att serial\n")
						// Calculate packet dimensions
						numToSend = out_data_buffer_len - sent;
						if(numToSend>max_len)numToSend = max_len; // Truncate
						// Set attribute ptr (it may be read after the indication as well)
						bleSerialOut.attData = (unsigned char*)out_data_buffer + sent;
						bleSerialOut.attLen = numToSend;
						if (bleSerialOut.attCfg & ATT_CFG_INDICATE)
						{
							// Send indication(s), if success, increment sent var
							if(BleAttIndicate(bleSerialOut.attData,bleSerialOut.attLen,ATT_SERIAL_OUT_HANDLE))	
								sent += numToSend;
							else numToSend = 0;
							// Set indication pending flag
							bleIndPending = TRUE;
						}	
						else if(bleSerialOut.attCfg & ATT_CFG_NOTIFY)
						{
							// Send notification(s), if success, increment sent var
							if(BleAttNotify(bleSerialOut.attData,bleSerialOut.attLen,ATT_SERIAL_OUT_HANDLE))	
								sent += numToSend;
							else numToSend = 0;
						}
						// See if we are done?
						if(out_data_buffer_len <= sent)
							local_state = SENDING_LAST;
						break;
						}				
		case SENDING_LAST: {
						// Indication(s) have all sent
						DBG_INFO("att serial done\n")
						// Remove outgoing data from fifo/data source (handle retains last data)
						LastDataHasSent();
						//break; Fall through
						}
		default : 		local_state = READY;
						numToSend = 0;
						break;
		}
	// Done
	return numToSend;
}
#endif // ATT_USE_BLE_SERIAL

#endif // HOSTMODE>0

//EOF
