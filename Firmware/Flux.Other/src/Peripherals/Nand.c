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

// NAND Flash control
// Karim Ladha, 2010-2012
// 28-11-2012 : KL, added support for reading nand flash chip parameters

#define NAND_OPTIMIZE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Peripherals/Nand.h"


#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

#if !defined(NAND_BYTES_PER_PAGE)
	#warning "Define the number of bytes per page in the hardware profile - assuming HY27UF084G2B/M"
	#define NAND_BYTES_PER_PAGE 2112 /*2048 + 64 extra*/
#endif 


// Actual code for the hardware
// --- Low-level NAND Flash functions ---
// Nand flash memory chip commands - micron and hynix same
#define FLASH_READ_COMMAND_1                0x00
#define FLASH_READ_COMMAND_2                0x30
#define FLASH_READ_COMMAND_COPY_BACK_1      0x00
#define FLASH_READ_COMMAND_COPY_BACK_2      0x35
#define FLASH_RESET_COMMAND                 0xff
#define FLASH_GET_STATUS                    0x70
#define FLASH_PAGE_PROGRAM_1                0x80
#define	FLASH_PAGE_PROGRAM_2                0x10
#define	FLASH_PAGE_PROGRAM_COPY_BACK_1      0x85
#define	FLASH_PAGE_PROGRAM_COPY_BACK_2      0x10
#define FLASH_BLOCK_ERASE_1                 0x60
#define FLASH_BLOCK_ERASE_2                 0xD0
#define FLASH_RANDOM_DATA_OUTPUT_1          0x05
#define FLASH_RANDOM_DATA_OUTPUT_2          0xE0
#define FLASH_READ_ID          				0x90

// Hynix flash memory chip status flags
#define FLASH_STATUS_FLAG_FAILED            0x01	// 0: Pass,      1: Failed
#define FLASH_STATUS_FLAG_CONTROLLER_IDLE   0x20	// 0: Active,    1: Idle
#define FLASH_STATUS_FLAG_READY             0x40	// 0: Busy,      1: Ready
#define FLASH_STATUS_FLAG_WRITE_PROTECTED   0x80	// 0: Protected, 1: Not protected

#define FLASH_STATUS_SUCCESS(_x) !((_x) & FLASH_STATUS_FLAG_FAILED)


#define FLASH_TIMEOUT 0xffff        // ~4 msec * number of instructions in loop
static volatile int _rbwait;
#define FLASH_WAIT_RB_TIMEOUT() { if (!FLASH_RB) { for (_rbwait = FLASH_TIMEOUT; !FLASH_RB && --_rbwait; ); } }
#if 1
#define FLASH_WAIT_RB FLASH_WAIT_RB_TIMEOUT             // All waits to be un-lockable
#else
#define FLASH_WAIT_RB() { while (FLASH_RB == 0); }      // Could infinite loop if hardware error
#endif

/*LOW LEVEL PARRALLEL PORT CODE - for PIC18 of PIC24*/
#ifdef FLASH_BITBANGED
	#ifdef USE_PIC24
		// Used to control the timing in bitbanged mode
		#define DelaySetup()	Delay10us(1)
		#define DelayRB()		Delay10us(1)
		#define HSPSPWait()
		#define NandPSPOn()		PMCONbits.PMPEN=0/*PMP off!*/
	#elif defined(USE_PIC18)
		// Used to control the timing in bitbanged mode
		#define DelaySetup()	/*Nop()*/
		#define DelayRB()		Nop()
		#define NandPSPOn()		
		#define PSPWait()
	#endif

	#define NandWriteRaw(_value)	{\
									FLASH_DATA_TRIS = 0x00;\
									FLASH_DATA_WR = _value;\
									DelaySetup();\
									FLASH_WE = 0;\
									DelaySetup();\
									FLASH_WE = 1;\
									}
	static unsigned char FLASH_READ_VALUE; // Careful with this next one!
	#define NandReadRaw()			(FLASH_READ_VALUE);{/*Only way to get inline to work*/\
									FLASH_DATA_TRIS = 0xff;\
									FLASH_RE = 0;\
									DelaySetup();\
									FLASH_READ_VALUE = FLASH_DATA_RD;\
									FLASH_RE = 1;}
#else // Hardware PSP
	// Tested KL: 25/3/11
	// Initialise the PSP module to interface to the Nand
	static void NandPSPOn(void)
	{ // Tested 07/03/2011 K.Ladha
		PMCON = 	0b1010001100000000;  /*	:MSB: PMPEN=1 : NA = 0 : PSIDL = 1 : ADRMUX = 00 : PTBEEN = 0 : PTWREN = 1 : PTRDEN = 1 */\
										 /*	:LSB: CSF = 00 : ALP = 0 : CS2P = 0 : CS1P = 0 : BEP = 0 : WRSP = 0 : RDSP = 0 */
		PMMODE = 	0b0000001000000100;  /* :MSB: BUSY = 0 : IRQM = 00 : INCM = 00 : MODE16 = 0 : MODE = 10 */
										 /*	:LSB: WAITB = 00 : WAITM = 0001 : WAITE = 01 (1 Tcy byte write)*/
		//PMMODE = 	0b0000001000000000;  /* Single cycle PSP, not at 16MIPS, too fast*/
		PMADDR =	0b0000000000000000;  /* CS2 = 0 : CS1 = 0 : ADDR : 0b00000000000000 */
		PMAEN  = 	0b0000000000000000;  /* PTEN = 0000 0000 0000 00 : PTEN = 00 */  
		return;
	}
	// If we know the PSP is configured to complete in one clock cycle then we dont need to wait.
		#define HSPSPWait()				{while (PMMODEbits.BUSY);}/*not needed unless single cycle psp writes are possible*/
		//#define PSPWait();				{while (PMMODEbits.BUSY);}	
		#define PSPWait()				{__builtin_nop();__builtin_nop();} 	/*Consecutive writes need 3 Tcy between*/
		#define NandWriteRaw(_value) 	{FLASH_DATA = _value;PSPWait();}	/*Initiates it being clocked out*/
		#define NandReadRaw()			(FLASH_DATA);{PSPWait();}			/*You can't use NandReadRaw() as a dummy read*/
#endif // endif bitbanged

/*MID LEVEL NAND INSTRUCTION CODE*/

	// Write a command
	static void NandWriteCommand(unsigned char command)
	{
		FLASH_CLE = 1;			// Command latch enable
	    NandWriteRaw(command);  // Write command 
		FLASH_CLE = 0;			// Command latch disable
		return;
	}
	
	/*	This is how the 5 address bytes are arranged
		struct {
		b1+2	unsigned int column		: 12; 	// 13bits really, top bit accesses top 218 extra bytes
				unsigned int padding1	: 4;	// 0x0 or 0x1 for extended area
				
		b3+4+5	unsigned int page 		: 6; 	// 64 pages in an erase block
				unsigned int block		: 12; 	// 4096 blocks, even blocks and odd blocks are on different silicon
				unsigned int ZERO		: 6;	// ZERO 
				};
	*/
	
	/*Add address based instructions use this common variable - reduces code overhead*/
	static BYTE AddBytes[5];
	/*Chip specific address construction*/
	#define FormAddressB(_B)			{AddBytes[0] = _B<<6; AddBytes[1] = _B>>2; AddBytes[2] = _B>>10;}							/*3 Bytes*/
	#define FormAddressBP(_B,_P) 		{AddBytes[0] = _B<<6; AddBytes[0] |= (_P&0x3f); AddBytes[1] = _B>>2; AddBytes[2] = _B>>10;} 	/*3 Bytes*/


		// Write a 5 byte address  - only the last 3 bytes are used
	static void NandWriteAddress5B(void)
	{
		FLASH_ALE = 1;			// Address latch enable
	    NandWriteRaw(0);HSPSPWait(); 	// Only write zeros
	    NandWriteRaw(0);HSPSPWait(); 	// to these two bytes
	    NandWriteRaw(AddBytes[0]);HSPSPWait(); //  The next 3 bytes
	    NandWriteRaw(AddBytes[1]);HSPSPWait();//	are the page and
	    NandWriteRaw(AddBytes[2]);			//	block addresses
		FLASH_ALE = 0;			// Address latch disable
		return;
	}

	// Write a 3 byte address  - block
	static void NandWriteAddress3B(void)
	{
	    FLASH_ALE = 1;			// Address latch enable
	    NandWriteRaw(AddBytes[0]);HSPSPWait();	// 	This function
	    NandWriteRaw(AddBytes[1]);HSPSPWait();// only writes the
	    NandWriteRaw(AddBytes[2]);			// 	block address
		FLASH_ALE = 0;			// Address latch disable
		return;
	}

	// Write a 2 byte address  - column
	static void NandWriteAddress2B(unsigned short Add)
	{
	    FLASH_ALE = 1;			// Address latch enable
	    NandWriteRaw(Add);HSPSPWait();	// This one just writes
	    NandWriteRaw(Add>>8);			// the column bytes
		FLASH_ALE = 0;			// Address latch disable
		return;
	}

	static unsigned char FlashReadStatus(void)
	{
	    unsigned char status;
		FLASH_CE = 0;			// Chip select
	 	NandWriteCommand(FLASH_GET_STATUS);HSPSPWait();
		status = NandReadRaw();HSPSPWait();//dummy read
		status = NandReadRaw(); // Read status byte
		FLASH_CE = 1;			// Chip deselect
		return (status);
	}

	static void FlashReset(void)
	{							// Calling this will void active read/writes
		FLASH_CE = 0;			// Chip select 
	 	NandWriteCommand(FLASH_RESET_COMMAND); 
		FLASH_CE = 1;			// Chip deselect
        FLASH_WAIT_RB_TIMEOUT();// wait for it to reset
		return;
	}

	static void FlashStandby(void)
	{
		FLASH_INIT_PINS();
        FLASH_WAIT_RB_TIMEOUT();// Wait for previous operations
	    FlashReset();       	// Shouldn't really be needed, but at least we'll know what state it's in
	    FLASH_CE_PIN = 0; 		// Ensure CE is high (active low)
		FLASH_CE = 1;			// Chip deselect
		return;
	}
	
/* Code for FTL layer to use*/	

// Initialize the NAND device
char NandInitialize(void)
{
	FLASH_INIT_PINS();			// Setup the pins as in HardwareProfile
    FLASH_WAIT_RB_TIMEOUT();    // wait for startup cycle
	NandPSPOn();				// Enable HW support if provided
    FlashReset();				// Issue the reset command
	return (TRUE);				// Return
}


// Turn off NAND chip
char NandShutdown(void)
{
	FlashStandby();
	return (TRUE);
}


// ONFI chip device identifier parameters (this is now internal to nand.c)
typedef union 
{
	unsigned char byte[6];
	struct 
	{
		unsigned char mfgr_code;
		unsigned char device_code;
		unsigned char param1;
		unsigned char param2;
		unsigned char param3;	
		unsigned char unused;
	};
	struct 
	{
		unsigned char _mfgr_code;
		unsigned char _device_code;
		struct 
		{
			unsigned int die_per_ce 	: 2; 	/*00-1CE pin, 01-2CE pins, ...*/
			unsigned int cell_type 		: 2; 	/*00-2level, 01-4level, 10-8level, 11-16level*/
			unsigned int num_simu_pages	: 2;	/*00-1, 01-2, 10-8, 11-8*/
			unsigned int does_interleave: 1;	/*0-No interleave support, 1-Supported*/
			unsigned int does_wr_cache	: 1;	/*0-No write cache support, 1-Supported*/
		};
		struct
		{
			unsigned int page_size 		: 2; 	/*00-1k, 01-2k, 10-4k, 11-8k*/
			unsigned int spare_area_size: 1;	/*0->=8bytes/512, 1->=16bytes/512, mfgr specific*/
			unsigned int access_time_b0	: 1;	/*lsb*/
			unsigned int block_size		: 2;	/*00-64k, 01-128k, 10-256k, 11-512k*/
			unsigned int organisation	: 1;	/*0-x8, 1-x16*/
			unsigned int access_time_b1	: 1;	/*msb: 00-<=50ns, 01-<=30ns, 10-<=25ns, 11-reserved, mfgr specific*/
		};
		struct
		{
			unsigned int reserved1	: 2;		/*reserved*/
			unsigned int num_planes : 2; 		/*00-1, 01-2, 10-4, 11-8*/
			unsigned int plane_size : 3; 		/*000,001,010,011-512mb,100-1gb,101-2gb,110-4gb,111-8gb*/
			unsigned int reserved2	: 1;		/*reserved*/
		};
		unsigned char _unused;					/*To make it an integer number of bytes - 6*/
	};			
}nand_flash_parameters_t;

static const unsigned char NAND_DEVICE_DONT_CARE[6] = 	  {0x00}; 							// KL - ADDED
//const unsigned char NAND_DEVICE_HY27UF084G2M[6] = {0xAD,0xDC,0x80,0x95,0xAD,0x00};// KL - ADDED
static const unsigned char NAND_DEVICE_HY27UF084G2B[6] = {0xAD,0xDC,0x10,0x95,0x54,0x00};
static const unsigned char NAND_DEVICE_HY27UF084G2x[6] = {0xAD,0xDC,0x00}; 				// KL - ADDED
static const unsigned char NAND_DEVICE_MT29F8G08AAA[6] = {0x2C,0xD3,0x90,0x2E,0x64,0x00};
static const unsigned char NAND_DEVICE_S34ML04G1[6] 	= {0x01,0xDC,0x90,0x95,0x54,0x00}; // KL - ADDED
static const unsigned char NAND_DEVICE_S34ML08G1[6] 	= {0x01,0xD3,0xD1,0x95,0x58,0x00}; // KL - ADDED

// Read chip parameters
char NandReadDeviceId(unsigned char *destination)
{
    FLASH_WAIT_RB_TIMEOUT();                // Wait for previous operations
	FLASH_CE = 0;							// Chip select	
	NandWriteCommand(FLASH_READ_ID);		// Command
	FLASH_ALE = 1;
	NandWriteRaw(0);						// Address byte (this is a special case)
	FLASH_ALE = 0;
	destination[0] = NandReadRaw();HSPSPWait();	// Dummy read
	destination[0] = NandReadRaw();HSPSPWait();	// first byte 
	destination[1] = NandReadRaw();HSPSPWait();	// second byte 
	destination[2] = NandReadRaw();HSPSPWait();	// third byte 
	destination[3] = NandReadRaw();HSPSPWait();	// fourth byte 
	destination[4] = NandReadRaw();HSPSPWait();	// fifth byte 
	destination[5] = 0;							// last byte - unused
	FLASH_CE = 1;							// Chip deselect
	return (TRUE);	 						// return
}

// Globals..
char nandPresent = 0;       // NAND present (call NandVerifyDeviceId() once to set this)
#ifdef NAND_NO_RCB
static unsigned char nandNoRCB = 0;
#endif


#ifndef NAND_DEVICE
#warning "Must define 'NAND_DEVICE' to 'NAND_DEVICE_HY27UF084G2B' or 'NAND_DEVICE_MT29F8G08AAA' for NandVerifyDeviceId() to work."
#else
// Verify the device id
unsigned char NandVerifyDeviceId(void)
{
    unsigned char id[6] = {0};
    nandPresent = 0;
    if (NandReadDeviceId(id))
    {
		// KL - ADDED, to support variable length id's upto 6 chars
		int i;
		nandPresent = 0; 					// Assume not
		for(i=0;i<6;i++)
		{
			if (NAND_DEVICE[i] == '\0') 	{nandPresent = 1; break;}	// Successful end of id string
			if (id[i] != NAND_DEVICE[i]) 	{ break; } 					// Character mismatch
		}

#ifdef NAND_DEVICE_ALT
		if (!nandPresent)
		{
			for(i=0;i<6;i++)
			{
				if (NAND_DEVICE_ALT[i] == '\0') 	{nandPresent = 2; break;}	// Successful end of id string
				if (id[i] != NAND_DEVICE_ALT[i]) 	{ break; } 					// Character mismatch
			}
#ifdef NAND_NO_RCB
			if (nandPresent) { nandNoRCB = 1; }
#endif
		}
#endif

#ifdef NAND_DEVICE_ALT2
		// KL - Added support for new nand chip
		if (!nandPresent)
		{
			for(i=0;i<6;i++)
			{
				if (NAND_DEVICE_ALT2[i] == '\0') 	{nandPresent = 3; break;}	// Successful end of id string
				if (id[i] != NAND_DEVICE_ALT2[i]) 	{ break; } 					// Character mismatch
			}
#ifdef NAND_NO_RCB
			if (nandPresent) { nandNoRCB = 1; }
#endif
		}
#endif

        //if (id[0] == NAND_DEVICE[0] && id[1] == NAND_DEVICE[1] && id[2] == NAND_DEVICE[2] && id[3] == NAND_DEVICE[3] && id[4] == NAND_DEVICE[4])
        //{
        //   nandPresent = 1;
        //}
    }
    return (unsigned char)nandPresent;
}
#endif

// Erase a block
char NandEraseBlock(unsigned short block)
{
	FormAddressB(block);                                // See macro
    FLASH_WAIT_RB_TIMEOUT();                            // Wait for previous operations
	FLASH_CE = 0;                                       // Chip select
	NandWriteCommand(FLASH_BLOCK_ERASE_1);
	NandWriteAddress3B();                               // Specify block address
	NandWriteCommand(FLASH_BLOCK_ERASE_2);	
	FLASH_CE = 1;                                       // Chip deselect
	return (TRUE);                                      // Internal block erase does not throw errors
}

// Load a page in to the buffer for reading
char NandLoadPageRead(unsigned short block, unsigned char page)
{
	FormAddressBP(block,page);                          // See macro
    FLASH_WAIT_RB();                                    // Wait for previous operations
	FLASH_CE = 0;                                       // Chip enable
	NandWriteCommand(FLASH_READ_COMMAND_1);
	NandWriteAddress5B();                               // Specify page
	NandWriteCommand(FLASH_READ_COMMAND_2);
	FLASH_CE = 1;                                       // Chip deselect
	return (TRUE);                                      // No errors possible
}

// Read in from the page buffer
char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length)
{
	unsigned char Dummy;
    FLASH_WAIT_RB();                                    // Wait for previously read page to load
	FLASH_CE = 0;                                   	// Chip select
	NandWriteCommand(FLASH_RANDOM_DATA_OUTPUT_1);
	NandWriteAddress2B(offset);                 		// specify offset
	NandWriteCommand(FLASH_RANDOM_DATA_OUTPUT_2);
	Dummy = NandReadRaw();                              // dummy read - To do with PSP module not flash
#ifdef NAND_OPTIMIZE
    if (length == 512)
    {
        for (length = 16; length != 0; length--)
        {
            buffer[ 0] = NandReadRaw();
            buffer[ 1] = NandReadRaw();
            buffer[ 2] = NandReadRaw();
            buffer[ 3] = NandReadRaw();
            buffer[ 4] = NandReadRaw();
            buffer[ 5] = NandReadRaw();
            buffer[ 6] = NandReadRaw();
            buffer[ 7] = NandReadRaw();
            buffer[ 8] = NandReadRaw();
            buffer[ 9] = NandReadRaw();
            buffer[10] = NandReadRaw();
            buffer[11] = NandReadRaw();
            buffer[12] = NandReadRaw();
            buffer[13] = NandReadRaw();
            buffer[14] = NandReadRaw();
            buffer[15] = NandReadRaw();
            buffer[16] = NandReadRaw();
            buffer[17] = NandReadRaw();
            buffer[18] = NandReadRaw();
            buffer[19] = NandReadRaw();
            buffer[20] = NandReadRaw();
            buffer[21] = NandReadRaw();
            buffer[22] = NandReadRaw();
            buffer[23] = NandReadRaw();
            buffer[24] = NandReadRaw();
            buffer[25] = NandReadRaw();
            buffer[26] = NandReadRaw();
            buffer[27] = NandReadRaw();
            buffer[28] = NandReadRaw();
            buffer[29] = NandReadRaw();
            buffer[30] = NandReadRaw();
            buffer[31] = NandReadRaw();
            buffer += 32;
        }
    }
    else
#endif
	for (;length>0;length--)
	{
		*buffer++ = NandReadRaw();                      // Read in data from device
	}
	FLASH_CE = 1;                                       // Chip deselect
	return (TRUE);                                      // No errors possible on read
}


#ifdef NAND_READ_SECTOR_WORD_SUMMED
// Read in from the page buffer 512 bytes to a word-aligned buffer, summing the word-wise values
char NandReadBuffer512WordSummed(unsigned short offset, unsigned short *wordAlignedBuffer, unsigned short *outSum)
{
	unsigned char *p = (unsigned char *)wordAlignedBuffer;
	unsigned short length = 512;
	unsigned short sum = 0;
	unsigned char Dummy;
	
	// Fall-back for un-aligned buffers
	if ((((unsigned short)wordAlignedBuffer)&1) != 0)
	{
		if (outSum != NULL) { *outSum = 0xffff; }
		return NandReadBuffer(offset, (unsigned char *)wordAlignedBuffer, length);
	}	
	
    FLASH_WAIT_RB();                                    // Wait for previously read page to load
	FLASH_CE = 0;                                   	// Chip select
	NandWriteCommand(FLASH_RANDOM_DATA_OUTPUT_1);
	NandWriteAddress2B(offset);                 		// specify offset
	NandWriteCommand(FLASH_RANDOM_DATA_OUTPUT_2);
	Dummy = NandReadRaw();                              // dummy read - To do with PSP module not flash
	
	for (; length != 0; length -= 16)
	{
		// Word reads (2 bytes each)
		// On optimize-speed (-Os) we need the last NOP, on optimize-level-3 (-O3) we don't need it.
		*(p +  0) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p +  1) = FLASH_DATA; sum += *((unsigned short *)(p +  0)); __builtin_nop(); 
		*(p +  2) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p +  3) = FLASH_DATA; sum += *((unsigned short *)(p +  2)); __builtin_nop(); 
		*(p +  4) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p +  5) = FLASH_DATA; sum += *((unsigned short *)(p +  4)); __builtin_nop(); 
		*(p +  6) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p +  7) = FLASH_DATA; sum += *((unsigned short *)(p +  6)); __builtin_nop(); 
		*(p +  8) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p +  9) = FLASH_DATA; sum += *((unsigned short *)(p +  8)); __builtin_nop(); 
		*(p + 10) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p + 11) = FLASH_DATA; sum += *((unsigned short *)(p + 10)); __builtin_nop(); 
		*(p + 12) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p + 13) = FLASH_DATA; sum += *((unsigned short *)(p + 12)); __builtin_nop(); 
		*(p + 14) = FLASH_DATA; __builtin_nop(); __builtin_nop(); *(p + 15) = FLASH_DATA; sum += *((unsigned short *)(p + 14)); __builtin_nop(); 
		
		p += 16;
	}
	FLASH_CE = 1;                                       // Chip deselect
	
	// Out parameter
	if (outSum != NULL) { *outSum = sum; }
	
	return (TRUE);                                      // No errors possible on read
}

#endif


// Write page directly -- KL, ADDED - This is basic functionality and should always be supported
char NandWritePage(unsigned short srcBlock, unsigned char srcPage, unsigned char *buffer)
{
	unsigned char status;
	unsigned short length = NAND_BYTES_PER_PAGE;
	FormAddressBP(srcBlock,srcPage);                    // See macro
	FLASH_CE = 0;                                       // Chip enable
    FLASH_WAIT_RB();                                    // Wait for previous operations
	NandWriteCommand(FLASH_PAGE_PROGRAM_1);				// Write page
	NandWriteAddress5B();                               // Specify source address

#ifdef NAND_OPTIMIZE
	for (;length >= 32; length-=32)
	{
		NandWriteRaw(buffer[ 0]);NandWriteRaw(buffer[ 1]);NandWriteRaw(buffer[ 2]);NandWriteRaw(buffer[ 3]);
		NandWriteRaw(buffer[ 4]);NandWriteRaw(buffer[ 5]);NandWriteRaw(buffer[ 6]);NandWriteRaw(buffer[ 7]);
		NandWriteRaw(buffer[ 8]);NandWriteRaw(buffer[ 9]);NandWriteRaw(buffer[10]);NandWriteRaw(buffer[11]);
		NandWriteRaw(buffer[12]);NandWriteRaw(buffer[13]);NandWriteRaw(buffer[14]);NandWriteRaw(buffer[15]);
		NandWriteRaw(buffer[16]);NandWriteRaw(buffer[17]);NandWriteRaw(buffer[18]);NandWriteRaw(buffer[19]);
		NandWriteRaw(buffer[20]);NandWriteRaw(buffer[21]);NandWriteRaw(buffer[22]);NandWriteRaw(buffer[23]);
		NandWriteRaw(buffer[24]);NandWriteRaw(buffer[25]);NandWriteRaw(buffer[26]);NandWriteRaw(buffer[27]);
		NandWriteRaw(buffer[28]);NandWriteRaw(buffer[29]);NandWriteRaw(buffer[30]);NandWriteRaw(buffer[31]);
		buffer += 32;
	}
#endif
	for (;length > 0; length--)
	{
		NandWriteRaw(*buffer);
		buffer++;
	}
	NandWriteCommand(FLASH_PAGE_PROGRAM_2);
	FLASH_CE = 1;                                       // Deselect
    FLASH_WAIT_RB();                                    // Wait for page program
	status = FlashReadStatus();                         // Get status
	if (status & FLASH_STATUS_FLAG_FAILED) return (FALSE); 	// Failed
	else	return (TRUE);    
}


// Read copy back disabled, write buffer in MCU ram
#ifdef NAND_NO_RCB

// Page buffer for noRCB functionality KL - Added
static unsigned char __attribute__((aligned(2))) pageBuffer[NAND_BYTES_PER_PAGE];
static unsigned short _RCBdestBlock, _RCBdestPage;

// Copy a page - this copies via MCU buffer
char rcb_NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)            // Copes with unknown source
{
	unsigned char retval = 1;
	retval &= NandLoadPageRead(srcBlock, srcPage);					// Load page to NAND ram
	retval &= NandReadBuffer(0, pageBuffer, NAND_BYTES_PER_PAGE);	// Read into MCU buffer
	retval &= NandWritePage(destBlock, destPage, pageBuffer);		// Copy back to desination
	return (retval); 												// Any fails will report fail
}

// Load a page in to the buffer for writing to the specified location
char rcb_NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
	unsigned char retval = 1;
	_RCBdestBlock = destBlock;										// Store the desination here for now
	_RCBdestPage = destPage;										// Store the desination here for now
	retval &= NandLoadPageRead(srcBlock, srcPage);					// Load page to NAND ram
	retval &= NandReadBuffer(0, pageBuffer, NAND_BYTES_PER_PAGE);	// Read into MCU buffer
	return (retval); 
}

// Write in to the page buffer
char rcb_NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
	unsigned char* pageBufferPtr = pageBuffer;			// Pointer to page buffer
	pageBufferPtr += offset;							// Add offset
	memcpy(pageBufferPtr, buffer, length);				// Copy in new data
	return (TRUE);                                      // No errors possible
}

// Commit the loaded page buffer
char rcb_NandStorePage(void)
{
	unsigned char retval = NandWritePage(_RCBdestBlock, _RCBdestPage, pageBuffer);	// Copy back to desination
	return (retval); 																// Report result
}

// Commit the loaded page buffer to a different destination
char rcb_NandStorePageRepeat(unsigned short destBlock, unsigned char destPage)
{
	unsigned char retval = NandWritePage(destBlock, destPage, pageBuffer);	// Copy back to desination
	return (retval); 														// Report result
}

#endif


// Copy a page
char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)            // Copes with unknown source
{
	unsigned char status;
#ifdef NAND_NO_RCB
	if (nandNoRCB) return rcb_NandCopyPage(srcBlock, srcPage, destBlock, destPage);
#endif
	FormAddressBP(srcBlock,srcPage);                    // See macro
	FLASH_CE = 0;                                       // Chip enable
    FLASH_WAIT_RB();                                    // Wait for previous operations
	NandWriteCommand(FLASH_READ_COMMAND_COPY_BACK_1);	// Read for copy back
	NandWriteAddress5B();                               // Specify source address
	NandWriteCommand(FLASH_READ_COMMAND_COPY_BACK_2);
	FormAddressBP(destBlock,destPage);                  // See macro
    FLASH_WAIT_RB();                                    // Wait for page to load
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_1);
	NandWriteAddress5B();                               // Specify destination address
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_2);
	FLASH_CE = 1;                                       // Deselect
    FLASH_WAIT_RB();                                    // Wait for page program
	status = FlashReadStatus();                         // Get status
	if (status & FLASH_STATUS_FLAG_FAILED) return (FALSE); 	// Failed
	else	return (TRUE);                              // Pass
}

// Load a page in to the buffer for writing to the specified location
char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
#ifdef NAND_NO_RCB
	if (nandNoRCB) return rcb_NandLoadPageWrite(srcBlock, srcPage, destBlock, destPage);
#endif
	FormAddressBP(srcBlock,srcPage);                    // See macro
	FLASH_CE = 0;                                       // Chip select
    FLASH_WAIT_RB();                                    // Wait for previous operations
	NandWriteCommand(FLASH_READ_COMMAND_COPY_BACK_1);	
	NandWriteAddress5B();                               // Specify source page
	NandWriteCommand(FLASH_READ_COMMAND_COPY_BACK_2);
	FormAddressBP(destBlock,destPage);                  // See macro
    FLASH_WAIT_RB();                                    // Wait for page to load
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_1 );
	NandWriteAddress5B();                               // Specify desination
	FLASH_CE = 1;                                       // Chip deselect
	return (TRUE);                                      // No errors possible
}

// Write in to the page buffer
char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
#ifdef NAND_NO_RCB
	if (nandNoRCB) return rcb_NandWriteBuffer(offset, buffer, length);
#endif
	FLASH_CE = 0;                                       // Chip select
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_1 );
	NandWriteAddress2B(offset);                         // Specify offset
	for (;length>0;length--)
	{
		NandWriteRaw(*buffer++);                        // Write into device data register
	}
	FLASH_CE = 1;                                       // Chip deselect
	return (TRUE);                                      // No errors possible
}

// Commit the loaded page buffer
char NandStorePage(void)
{
	unsigned char status;
#ifdef NAND_NO_RCB
	if (nandNoRCB) return rcb_NandStorePage();
#endif
	FLASH_CE = 0;                                       // Chip select
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_2 );
	FLASH_CE = 1;                                       // Chip deselect
    FLASH_WAIT_RB();                                    // Wait for page to copy
	status = FlashReadStatus();                         // Get status
	if (status & FLASH_STATUS_FLAG_FAILED) return (FALSE); 	// Failed
	else	return (TRUE);                              // Pass
}

// Commit the loaded page buffer to a different destination
char NandStorePageRepeat(unsigned short destBlock, unsigned char destPage)
{
	unsigned char status;
#ifdef NAND_NO_RCB
	if (nandNoRCB) return rcb_NandStorePageRepeat(destBlock, destPage);
#endif
	FormAddressBP(destBlock,destPage);                  // See macro
    FLASH_WAIT_RB();                                    // Wait for previous operations
	FLASH_CE = 0;                                       // Chip select
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_1 );
	NandWriteAddress5B();                               // Specify new desination
	NandWriteCommand(FLASH_PAGE_PROGRAM_COPY_BACK_2 );
	FLASH_CE = 1;                                       // Chip deselect
    FLASH_WAIT_RB();                                    // Wait for page to copy
	status = FlashReadStatus();                         // Get status
	if (status & FLASH_STATUS_FLAG_FAILED) return (FALSE); 	// Failed
	else	return (TRUE);                              // Pass
}



// Extract parameters from device ID rather than ONFI parameter page
#define NAND_PARAMETERS_FROM_DEVICE_ID

// Read parameters from ONFI parameter page
char NandReadParameters(NandParameters_t *nandParameters)
{
#ifdef NAND_PARAMETERS_FROM_DEVICE_ID
    nand_flash_parameters_t deviceId;
    
    // Clear parameters
    memset(nandParameters, 0x00, sizeof(NandParameters_t));
    
    // Use the device ID to determine the ONFI parameters
    if (!NandReadDeviceId(deviceId.byte))
    {
        return 0;                           // Failed to read parameters
    }

#if 1       // Hard-code return values for known hardware
    if (strcmp((char *)deviceId.byte, (char *)NAND_DEVICE_HY27UF084G2B) == 0)
    {
        // Returning parameters expected for Hynix
        nandParameters->revisionNumber = 0x0001;    
        nandParameters->dataBytesPerPage = 2048;    
        nandParameters->spareBytesPerPage = 64;     
        nandParameters->pagesPerBlock = 64;         
        nandParameters->blocksPerLogicalUnit = 2048;
        nandParameters->logicalUnits = 2;           
        return 1;                             // Return successfully read parameters
    }
    
    if (strcmp((char *)deviceId.byte, (char *)NAND_DEVICE_MT29F8G08AAA) == 0)
    {
        // Returning parameters expected for Micron
        nandParameters->revisionNumber = 0x0001;    
        nandParameters->dataBytesPerPage = 4096;    
        nandParameters->spareBytesPerPage = 218;    
        nandParameters->pagesPerBlock = 64;         
        nandParameters->blocksPerLogicalUnit = 2048;
        nandParameters->logicalUnits = 2;           
        return 1;                             // Return successfully read parameters
    }
#endif
    
    // Extract parameters from device ID
    // TODO: This is not at all right, fix this
    nandParameters->revisionNumber = 0x0001;
    nandParameters->dataBytesPerPage = 1024 << (deviceId.page_size);                                // 1kB * 2^page_size
    nandParameters->spareBytesPerPage = 16 << (deviceId.page_size + deviceId.spare_area_size);      // 16 bytes * 2^(page_size + spare_area_size)
    nandParameters->pagesPerBlock = (64 << (deviceId.block_size)) >> (deviceId.page_size);          // 64 * 2^block_size / 2^page_size
    nandParameters->blocksPerLogicalUnit = (1024UL << deviceId.plane_size) >> (deviceId.page_size); // 1024 * 2^plane_size / 2^page_size
    nandParameters->logicalUnits = deviceId.num_planes;                                             // num_planes
    return 1;
        
#else

    // TODO: Read parameters from the ONFI parameter page
    //nandParameters->revisionNumber = 0x0000;                // 2-bytes @4-5
    //nandParameters->dataBytesPerPage = 0x00000000UL;        // 4-bytes @80-83
    //nandParameters->spareBytesPerPage = 0x0000;             // 2-bytes @84-85
    //nandParameters->pagesPerBlock = 0x00000000UL;           // 4-bytes @92-94
    //nandParameters->blocksPerLogicalUnit = 0x00000000UL;    // 4-bytes @96-99
    //nandParameters->logicalUnits = 0x00;                    // 1-byte  @100
    //return 1;                             // Return successfully read parameters

    #warning "ONFI read parameter page not implemented"

    return 0;       // failed to read parameters
#endif
}

/*	Karim Ladha 19-09-2015: Extended to allow future drivers to work with this legacy driver
	Summary:
	 - Requires a 'get parameters' function for adaptive FTL implementations
	 - Using existing ftl parameters since this driver doesn't read nand parameters
*/
#if LEGACY_EXTENSION_LEVEL > 0
// Includes
#include "Peripherals/xNand.h"	// For the type below
#include "FtlConfig.h"			// For the NAND parameters

// Installed nand chip info
const NandDeviceInfo_t nandInfo = {
	.blocks 		=	FTL_PHYSICAL_BLOCKS,
	.blockPlanes	=	FTL_PLANES,
	.blockPages		=	FTL_PAGES_PER_BLOCK,
	.pageBytes		=	(FTL_SPARE_OFFSET + FTL_SPARE_BYTES_PER_PAGE)
};

// Get total combined memory found as aggregated by driver
NandDeviceInfo_t* NandDeviceInfo(void)
{
	return (NandDeviceInfo_t* )&nandInfo;
}
#endif
