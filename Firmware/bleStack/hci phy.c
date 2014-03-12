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

// Bluetooth physical layer interface
// Karim Ladha, 2013-2014

#include <stdlib.h>
#include "hci phy.h"	// Definitions
#include "phy hal.h"	// Install cb	
#include "bt config.h"
#include "debug.h"

#if DEBUG_PHY
	static const char* file = "hciphy.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
	#ifdef DBG_CLASS_DUMP_ALL
    	#define DBG_DUMP(X, Y)		DBG_dump(X, Y);
	#endif
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "hciphy.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

// GLOBALS
static unsigned char commandBuffer[MAX_CMD_PACKET_LENGTH]; 	// Outgoing commands
static unsigned char commandBufferCheckedOut = FALSE_;		// Flags that it is currently in use
static unsigned char aclOutDataBuffer[MAX_OUT_ACL_LENGTH];	// Outgoing acl data
static unsigned char aclOutDataBufferCheckedOut = FALSE_;	// Flags that it is currently in use

static void(*EventCallBack)	(const unsigned char * data, unsigned short len) = NULL; 	// Installed by HCI
static void(*AclCallBack)	(const unsigned char * data, unsigned short len) = NULL; 	// Installed by HCI
static unsigned short (*OutputDataTry)(void) = NULL; 									// Installed by App
static void (*TimeoutCallBack)(void) = NULL;											// Installed by App										// Installed by App

// TRANSPORT SPECIFIC CONTROL FUNCTIONS
void HciPhyPause(void) 	{UartPause();}
void HciPhyPlay(void) 	{UartPlay();}
void HciPhyAdaptClock(void)	{UartSetBaud(0);}
void HciPhySafeSleep(void)	{UartSafeSleep();}

// PRIVATE PROTOTYPES
static void HciPhyBlockSentHandler(void);
static void HciPhyEventHandler (void);

// CODE
unsigned char HciPhyInitOn(void)
{
	// Turn on the module and uart
	if (!PhyPowerUp())
		return HCI_PHY_ERROR;
	// Zero flags
	commandBufferCheckedOut = FALSE_;
	aclOutDataBufferCheckedOut = FALSE_;
	// If OK, install handler
	PhyInstallEventHandlerCB(HciPhyEventHandler);
	return HCI_PHY_SUCCESS;
}

unsigned char HciPhyInitOff(void)
{
	PhyPowerOff();
	return HCI_PHY_SUCCESS;
}

unsigned char HciSupportsLe(void)
{
	return PhySupportsLe();
}

unsigned char* HciPhyGetAclBuffer(void)
{
	// Check not currently sending?
	if(aclOutDataBufferCheckedOut == TRUE_)
	{
		DBG_INFO("acl wait\n");
		PhySendWait();
		HciPhyBlockSentHandler(); 
		ASSERT(aclOutDataBufferCheckedOut == FALSE_);
	}
	aclOutDataBufferCheckedOut = TRUE_;
	return aclOutDataBuffer;
}
unsigned char HciPhyWriteAcl(const unsigned char * data, unsigned short len)
{
	#if DEBUG_PHY_DUMP
	DBG_INFO( "ACLo: ");
	DBG_DUMP(data,len);
	#endif
	if(len == 0)aclOutDataBufferCheckedOut = FALSE_;
	else PhySendBlock(data, len, HCI_ACL_HEADER);
	return HCI_PHY_SUCCESS;
} 

unsigned char* HciPhyGetCmdBuffer(void)
{
	// Check not currently sending?
	if(commandBufferCheckedOut == TRUE_)
	{
		DBG_INFO("cmd wait\n");
		PhySendWait();
		HciPhyBlockSentHandler();
		ASSERT(commandBufferCheckedOut == FALSE_);
	}
	commandBufferCheckedOut = TRUE_;
	return commandBuffer;
} 
unsigned char HciPhyWriteCmd(const unsigned char * data, unsigned short len)
{
	#if DEBUG_PHY_DUMP
	DBG_INFO( "CMD: ");
	DBG_DUMP(data,len);
	#endif
	if(len == 0)commandBufferCheckedOut = FALSE_;
	else PhySendBlock(data, len, HCI_CMD_HEADER);
	return HCI_PHY_SUCCESS;
}

void  HciInstallAclCB	(void(*CallBack)(const unsigned char * data, unsigned short len))
{
	AclCallBack = CallBack;
}

void  HciInstallEventCB	(void(*CallBack)(const unsigned char * data, unsigned short len))
{
	EventCallBack = CallBack;
}	
void HciInstallDataTryCB(unsigned short (*CallBack)(void))
{
	OutputDataTry = CallBack;
}
void HciInstallTimeOutCB(void (*CallBack)(void))
{
	TimeoutCallBack = CallBack;
}

// EVENT HANDLERS
static void HciPhyBlockSentHandler(void)
{
	do{
		if (blockSent == HCI_ACL_BLOCK) 
		{
			DBG_INFO( "TxAcl\n");
			aclOutDataBufferCheckedOut = FALSE_;
			blockSent = FALSE_;
			break;
		}
		else if (blockSent == HCI_CMD_BLOCK) 
		{
			DBG_INFO( "TxCmd\n");
			commandBufferCheckedOut = FALSE_;
			blockSent = FALSE_;
			break;
		}
	}while(blockSent != FALSE_);
}

static void HciPhyEventHandler (void)
{
	// Process data received
	do {
		unsigned char blockReceivedCopy = blockReceived;
		if(blockReceivedCopy == HCI_ACL_BLOCK) 
		{
			unsigned short len = aclInDataBuffer[3];
			len = (len<<8) + aclInDataBuffer[2] + 4;
			#if DEBUG_PHY_DUMP
			DBG_INFO( "ACLi: ");
			DBG_DUMP(aclInDataBuffer,len);
			#else
			DBG_INFO( "RxAcl\n");
			#endif
			ASSERT(len <= MAX_IN_ACL_LENGTH);
	        if(AclCallBack != NULL)
				AclCallBack(aclInDataBuffer, len);
			// Block processed - resume reception
			blockReceived = FALSE_;
			PhyReceiveBlock(); // Clears flag
			break;
		}
		// Process events received
		else if(blockReceivedCopy == HCI_EVENT_BLOCK) 
		{
			unsigned short len = eventBuffer[1]+2;
			#if DEBUG_PHY_DUMP
			DBG_INFO( "EVT: ");
			DBG_DUMP(eventBuffer,len);
			#else
			DBG_INFO( "RxEvt\n");
			#endif
			ASSERT(len <= (MAX_EVENT_PACKET_LENGTH));
	        if(EventCallBack != NULL)
				EventCallBack(eventBuffer, len);
			// Block processed - resume reception
			blockReceived = FALSE_;
			PhyReceiveBlock(); // Clears flag
			break;
		}
		else if (blockReceived == HCI_HCILL_BLOCK)
		{
			// Will happen if event handler is at same priority as eHCILL handler
			// We can't allow sends now incase it is a go to sleep message. So we
			// will return now so it can be handled.
			DBG_ERROR("eHCILL\n")
			return;
		}
	}while(blockReceived != FALSE_);


	// Handle packet sent events
	HciPhyBlockSentHandler();

	// Try output data every time a Blutooth event occurs
	if(OutputDataTry!= NULL)
	{
		OutputDataTry();	
	}

	// Finally - check timeouts
	if(TimeoutCallBack != NULL)
	{
		TimeoutCallBack();
	}

}

//EOF

