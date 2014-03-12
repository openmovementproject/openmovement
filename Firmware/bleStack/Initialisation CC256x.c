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

// CC256x Initialization
// Karim Ladha, 2013-2014

#include <string.h>
#include "TimeDelay.h"
#include "Initialisation CC256x.h"
#include "phy hal.h" 
#include "bt nvm.h"	
#include "bt config.h"
#include "debug.h"

// DEBUG
#if DEBUG_HCI
	static const char* file = "InitCC256x.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "InitCC256x.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

// Externs

// Globals
const unsigned char     		ehcill_enabled = EHCILL_SETTING;	// Low power mode signalling protocol
unsigned short					chipsetId = 0;					/* Set to chipse number (2560) or (2564)*/
static unsigned long 			offset  = 0;					/* Current read position in the script*/
static unsigned long 			init_script_size;				/* Set based on chipset*/
static unsigned long 			init_script_addr;				/* Set based on chipset*/
static unsigned short			expected_packets;				/* Set based on chipset*/

// Externs from nvm vars
extern unsigned long cc2564_init_script_size;
extern unsigned long cc2560_init_script_size;
extern unsigned long null_init_script_size;

const unsigned char cmd_reset[]				= 	{0x01,0x03,0x0c,0x00};
const unsigned char cmd_write_scan_off[]	= 	{0x01,0x1a,0x0c,0x01,0x02};
const unsigned char cmd_baud_1mbaud[] = 		{0x01,0x36,0xff,0x04,0x40,0x42,0x0f,0x00};
const unsigned char cmd_baud_2mbaud[] = 		{0x01,0x36,0xff,0x04,0x80,0x84,0x1e,0x00};
const unsigned char cmd_baud_4mbaud[] = 		{0x01,0x36,0xff,0x04,0x00,0x09,0x3d,0x00};
const unsigned char cmd_read_sys_stat[] = 		{0x01,0x1f,0xfe,0x00};

const unsigned char powerVectorGFSK[15] = {24,-36,-36,-36,-36,-36,-36,-36,-36,-26,-16,-6,4,14,24}; // First entry is BLE power level 10dBm -> 20
const unsigned char powerVectorEDR[15] =  {-40,-40,-40,-40,-40,-40,-40,-40,-40,-30,-20,-10,0,10,20};


// Init script read
int InitScriptLoad(void);
int InitScriptReadNext(unsigned char *buffer);
void InitScriptClose(void);
// Private
unsigned char BtSendCmd(const unsigned char* cmd);

unsigned short CC256x_Startup(void)
{
 	// Inject init script to cc2564
	unsigned short init_script_packets_sent = 0; 	/* The number of packets in the script*/
	/* This input buffer is used during initialisation as output since we can't get any acl inputs during init*/
	unsigned char* command = aclInDataBuffer; 
	//unsigned char response[32];
	unsigned short retrys; 

	// Reset module
	retrys = 4; 			// With timeout 
	while (--retrys)
	{
		if (BtSendCmd(cmd_reset))
			break;
	}
	if (retrys == 0) return 1; // FAIL: Unresponsive module

	// Ensure radio is off
	if (!BtSendCmd(cmd_write_scan_off))
	{
		return 1;				// FAIL: Unresponsive
	}	

	// Change baud
	if (!BtSendCmd(cmd_baud_1mbaud))
	{
		return 2;				// FAIL: No baud change
	}	
	UartSetBaud(1000000ul);	

	// Read hardware version - 24 byte response
	if (BtSendCmd(cmd_read_sys_stat))
	{
		// TODO: See if we can fit both linker on one eeprom and select here
		switch(eventBuffer[8]){
			case (4):{
				chipsetId = 2564ul;
				break;
			}
			case (3):{
				chipsetId = 2560ul;
				break;
			}
			default:{
				chipsetId = 0ul; 	// Null/unknown?
				return 3;			// Chipset fail
				break;
			}
		}
	}
	// Load init script
	InitScriptLoad();
	init_script_packets_sent = 0;
	while(InitScriptReadNext(command))
	{
		init_script_packets_sent++;		if (BtSendCmd(command))
		{
			ASSERT(eventBuffer[5]==0); // Result
		}
		else break;
	}
	InitScriptClose();

	if(init_script_packets_sent != expected_packets) 
	{
		DBG_ERROR("Init count!\n");
		return 4;// FAIL: Initscript corrupt
	}

	// Fake reset event injection
	memcpy(eventBuffer,"\x0e\x04\x01\x03\x0c\x00",6);			// Fake reset event to trigger stack to reconfigure
	blockReceived = HCI_EVENT_BLOCK;							// Set event flag

	DBG_INFO("Init script sent\n");
	return 0;
}

// Function most useful for initialisation purposes
unsigned char BtSendCmd(const unsigned char* cmd)
{
	unsigned short timeout;	
	timeout = 0xffff;						// x 0.01ms, 0.65s
	PhyReceiveBlock();						// Enable reception
	PhySendBlock(&cmd[1],cmd[3]+3,cmd[0]);	// Send command
	while(blockSent==FALSE_);				// Wait send
	while((blockReceived==FALSE_)&&--timeout)// Wait for receive to complete
		{Delay10us(1);}
	if(timeout==0)UartRxIdle();				// Clear receiver, no/bad response detected
	blockReceived = FALSE_;					// Clear flag
	return(timeout!=0);						// Return result
}

#ifdef INIT_SCRIPT_IN_PROG_SPACE 
	extern __prog__ unsigned char __attribute__((space(prog))) cc256x_init_script[];
	unsigned char InitScriptOpen(void)
	{
		// No inititialisation needed for prog space
		return 1;
	}
	void InitScriptReadBytes(unsigned char* dest , unsigned short num)
	{
		ReadNvm(((unsigned long)init_script_addr + offset), dest, num);
	}
	void InitScriptClose(void)
	{
		// No closing procedures for prog space
	}
#elif defined INIT_SCRIPT_IN_EXT_EEPROM
	#include "Peripherals/Eeprom.h"
	unsigned char InitScriptOpen(void)
	{
		return (I2C_eeprom_present());
	}
	void InitScriptReadBytes(unsigned char* dest , unsigned short num)
	{
		I2C_eeprom_read((init_script_addr+offset), dest, num);
	}
	void InitScriptClose(void)
	{
		// No closing procedures for eeprom
	}
#elif defined INIT_SCRIPT_IN_FILE
	#include "MDD File System/FSIO.h"
	FSFILE* init_script = NULL; 
	unsigned char InitScriptOpen(void)
	{
		if(FSInit())
		{
			init_script = FSfopen ("BT_INIT.BIN",FS_READ);
			if (init_script==NULL)
			{
				DBG_ERROR("Init missing!\n");
				bluetoothState = BT_FAIL;
				return(0); /*Give up on initialisation*/
			}
		}
		else
		{
			DBG_ERROR("File sys fail!\n");
			bluetoothState = BT_FAIL;
			return(0); /*Give up on initialisation*/
		}
		return 1;
	}
	void InitScriptReadBytes(unsigned char* dest , unsigned short num)
	{
		// The FSfread will keep track of the offset into the file for me
		FSfread(dest, 1, num, init_script); 			
	}
	void InitScriptClose(void)
	{
		FSfclose(init_script);
	}
#else
	#error "Where is the init script?"
#endif


int InitScriptLoad(void){
	offset = 0;
	switch(chipsetId){
		#ifdef BT_CHIPSET_CC2564
		case (2564) : {
			init_script_addr = INIT_SCRIPT_CC2564;
			init_script_size = cc2564_init_script_size;
			expected_packets = INIT_SCRIPT_PACKETS_2564;
			break;
		}
		#endif
		#ifdef BT_CHIPSET_CC2560
		case (2560) : {
			init_script_addr = INIT_SCRIPT_CC2560;
			init_script_size = cc2560_init_script_size;
			expected_packets = INIT_SCRIPT_PACKETS_2560;
			break;
		}
		#endif
		default: {
		#if !defined(BT_CHIPSET_CC2560) && !defined(BT_CHIPSET_CC2564)
			init_script_addr = INIT_SCRIPT_NULL;
			init_script_size = null_init_script_size;
			expected_packets = 1;
		#else
			init_script_addr = 0;
			offset = 1;	// Force fail on first cmd
			init_script_size = 0;
			expected_packets = 0xffff;			
		#endif
			break;
		}
	}
	return 0;
}

int InitScriptReadNext(unsigned char *buffer)
{

	if (offset == 0) // On first call
	{
		if(!InitScriptOpen())
			return 0; /*Init error*/
	}


    if (offset >= init_script_size) 
	{
		InitScriptClose();
	    return 0; /*Initialisation over*/
    }
    
	unsigned short payload_len;
	InitScriptReadBytes(&buffer[0],4); 	// Read 4 bytes
	payload_len = buffer[3];			// Number of bytes in payload
	if(buffer[0] != 1)					// Test for misalignment
	{
		DBG_ERROR("Init pkts!\n");
		return(0);
	}
	offset+=4;
	InitScriptReadBytes(&buffer[4],payload_len); // Read in payload
	offset += payload_len;

    // Edit known script cmds
    unsigned short opcode = BT_READLE16(&buffer[1]);

	//TODO: Modify script directly
    if (opcode == 0xFD82) { if(buffer[4] == 0)			memcpy(&buffer[6],powerVectorGFSK,15);
							else if (buffer[4] == 1)	memcpy(&buffer[6],powerVectorEDR,15);
							else if (buffer[4] == 2)	memcpy(&buffer[6],powerVectorEDR,15);
							else ;}
    if (opcode == 0xFD0C) {buffer[5] = ehcill_enabled;}	// 1 or 0


	// Debug
	#if 0
	DBG_INFO("Op:%04X,Len:%u,Off:%lu\n",opcode,buffer[3],offset);
	#endif

	// Packet ready, return
    return 1; 
}


