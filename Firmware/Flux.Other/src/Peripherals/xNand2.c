// Karim Ladha 24-06-2015
// Driver for combining multiple NAND memories in plane wise fashion to make a larger device
// ONFI parameter page is ignored since all devices need checking manually anyway
// The Control lines are assumed to be all common but allowing separate CE and RB pins per device
// The separate devices appear as different (block) planes to prevent copy-back across devices 

// Includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Peripherals/xNand.h"
#include "Peripherals/GenericIO.h"
#include "HardwareProfile.h"
#include "Config.h"

// Debugging
#define DEBUG_LEVEL_LOCAL	DEBUG_PERIPHERAL
#define DBG_FILE			"xnand.c"	
#include "Utils/debug.h"

// Required definitions
#if !defined(NUMBER_OF_NANDS)
	#error "This is a multi-nand driver. Set maximum device chip select count."
#endif
#ifndef NAND_TIMEOUT
	#warning "Default NAND timeout value used."
	#define NAND_TIMEOUT 0x64000ul	/* This MUST allow at least 4ms */ 
#endif

// Nand flash memory chip commands - ONFI 1.0 
// Device targeting
#define NAND_GET_STATUS					0x70
#define NAND_READ_ID					0x90
#define NAND_READ_INFO					0xEC
#define NAND_RESET_COMMAND				0xFF
// Lowest level, mandatory
#define NAND_READ_COMMAND_1				0x00
#define NAND_READ_COMMAND_2				0x30
#define NAND_READ_UPDATE_COLUMN_1		0x05
#define NAND_READ_UPDATE_COLUMN_2		0xE0
#define NAND_PAGE_PROGRAM_1				0x80
#define	NAND_PAGE_PROGRAM_2				0x10
#define NAND_BLOCK_ERASE_1				0x60
#define NAND_BLOCK_ERASE_2				0xD0
// ONFI 1.0 optional, common 
#define NAND_READ_COMMAND_COPY_BACK_1	0x00
#define NAND_READ_COMMAND_COPY_BACK_2	0x35
#define	NAND_PAGE_PROGRAM_COPY_BACK_1	0x85
#define	NAND_PAGE_PROGRAM_COPY_BACK_2	0x10
#define	NAND_WRITE_UPDATE_COLUMN		0x85

// NAND status flags - minimum only
#define NAND_STATUS_FLAG_FAILED			   0x01	// 0: Pass,		 1: Failed
#define NAND_STATUS_FLAG_READY			   0x40	// 0: Busy,		 1: Ready
#define NAND_STATUS_FLAG_WRITE_PROTECTED   0x80	// 0: Protected, 1: Not protected

// Expected status read for idling device
#define NAND_STATUS_OK	(NAND_STATUS_FLAG_READY | NAND_STATUS_FLAG_WRITE_PROTECTED)

// Other definitions
#define INVALID_NAND_DEVICE		0xff	// Max devices = 253, practical limit is ~10 due to pin capacitance
#define EXPECTED_SECTOR_SIZE	512ul	// Used to determine start of extra page bytes only
#ifndef NAND_MAX_V_PLANES		
	#define NAND_MAX_V_PLANES 		16	// Number of virtual planes to make, max
#endif
#ifndef NAND_MAX_DEVICE_PLANES
	#define NAND_MAX_DEVICE_PLANES	16	// How many planes can a device have up to (normally 1 - 4)
#endif
/* Global NAND state variables */
char nandPresent = 0;  
/* Current device being accessed*/
unsigned char activeNand = INVALID_NAND_DEVICE;
/* Discovered devices and their info */
unsigned char nandDeviceCount = 0;
NandDeviceInfo_t nandDevices[NUMBER_OF_NANDS] = {{0}};
/* The combined device presented to upper levels */
NandDeviceInfo_t combinedNandDevice = {0};
/* The map from block number to device and plane */
unsigned char vPlaneMask, vPlaneShift;
unsigned short vPlaneSize;
unsigned char blockDeviceMap[NAND_MAX_V_PLANES];
unsigned char blockPlaneMap[NAND_MAX_V_PLANES];


/* The gpio pins must be defined in this external array structure */
extern const IO_pin_t nandPins[TOTAL_NAND_PINS];

/* Strictly private internal variables */
/* Persistent address structure for checks, in ONFI format */
unsigned char AddBytes[5];
/* Variables used for data check calculations */
volatile unsigned short nandcrc, nandsum, nandstatus;

/*Built in devices */
const NandType_t nandDefaultTypesList[] = {
	{/*SPANSION_S34ML04G1*/ 			5,	{0x01,0xDC,0x90,0x95,0x54},		{4096, 2, 64, 2112}},
/*	{**SPANSION_S34ML08G101BHI20_2CE**	5,	{0x01,0xDC,0x90,0x95,0x54},		{8192, 2, 64, 2112}}, identical to two x S34ML04G1s */
	{/*SPANSION_S34ML08G101BHI00*/ 		5,	{0x01,0xD3,0xD1,0x95,0x58},		{8192, 2, 64, 2112}},
	{/*HYNIX_HY27UF084G2M*/ 			5,	{0xAD,0xDC,0x80,0x95,0xAD},		{4096, 2, 64, 2112}},
	{/*HYNIX_HY27UF084G2B*/ 			5,	{0xAD,0xDC,0x10,0x95,0x54},		{4096, 2, 64, 2112}}};

/* Private prototypes */
/* LOW LEVEL NAND INTERFACE PROTOTYPES - Hardware specific */
static void NandInterfaceOn(void);
static inline unsigned char __attribute__((always_inline)) NandReadRaw(void);
static inline void __attribute__((always_inline)) NandWriteRaw(unsigned char data);
static void NandReadSpan(unsigned char* buffer, unsigned short length);
static void NandWriteSpan(const unsigned char* buffer, unsigned short length);

/* LOW LEVEL NAND INTERFACE PROTOTYPES - Hardware in-specific */	
static inline __attribute__((always_inline)) void NandBusyWait(void);
static void NandWriteCommand(unsigned char command);
static void NandWriteAddress5B(void);
static void NandWriteAddress3B(void);	
static void NandWriteAddress2B(unsigned short Add);
static void NandReadStatus(void);
static void NandReset(void);
static char NandReadDeviceId(unsigned char *destination);
static unsigned char NandLoadDevices(void);

/* MID LEVEL NAND ADDRESS PROTOTYPES - Hardware in-specific */	
/* 	Address format:
	This is how the 5 address bytes are arranged for ONFI 1.0 (and onwards so far)
	struct {
	Column address bytes:	
	b1+2	unsigned int column		: Address data area of page, i.e. 2k pages have 11 bits
			unsigned int extended	: Access extended page area with un-used bits zeroed
	Row address bytes:
	b3+4+5	unsigned int page		: Page address within block, 6 bits for 64 pages per block
			unsigned int block		: Block address bits, for 4096 block devices there are 12 bits
			unsigned int ZERO		: Un-used bits must be zeroed
			};

	The ONFI specification has a 5 byte address to specify the block, page and column
	This driver does not support devices with separate page planes i.e. plane copy is unrestricted
	The block planes are supported i.e. odd and even blocks. Different devices are mapped as block planes
	The LSB's of the block number are used to select the block planes. The lowest LSBs select the device
	This offers potential speed up if writing sequentially since subsequent writes do not require waits
*/
// Translate the block to the correct device, plane and real block
unsigned short MapVirtualBlock(unsigned short block)
{
	unsigned short vPlane = block & vPlaneMask;
	if(block >= combinedNandDevice.blocks)
	{
		// Out of range
		activeNand = INVALID_NAND_DEVICE;
		return 0;
	}
	activeNand = blockDeviceMap[vPlane];
	block >>= vPlaneShift;
	block += blockPlaneMap[vPlane] * vPlaneSize;
	return block;
}


#define FormAddressB(_B)			{AddBytes[0] = _B<<6; AddBytes[1] = _B>>2; AddBytes[2] = _B>>10;}								/*3 Bytes*/
#define FormAddressBP(_B,_P)		{AddBytes[0] = _B<<6; AddBytes[0] |= (_P&0x3f); AddBytes[1] = _B>>2; AddBytes[2] = _B>>10;}		/*3 Bytes*/
	
/* LOW LEVEL NAND INTERFACE SOURCE - Hardware specific */
#ifdef __C30__
	// PIC24 Includes
	#include "Compiler.h"
	#include "Peripherals/Crc16.h"
	// Globals
	volatile unsigned char dummy;

	// Hardware PSP support for PIC24
	// Initialise the PSP module to interface to the Nand
	static void NandInterfaceOn(void)
	{ // Tested 07/03/2011 K.Ladha
		PMCON =		0b1010001100000000;	 /*	:MSB: PMPEN=1 : NA = 0 : PSIDL = 1 : ADRMUX = 00 : PTBEEN = 0 : PTWREN = 1 : PTRDEN = 1 */\
										 /*	:LSB: CSF = 00 : ALP = 0 : CS2P = 0 : CS1P = 0 : BEP = 0 : WRSP = 0 : RDSP = 0 */
		PMMODE =	0b0000001000000100;	 /* :MSB: BUSY = 0 : IRQM = 00 : INCM = 00 : MODE16 = 0 : MODE = 10 */
										 /*	:LSB: WAITB = 00 : WAITM = 0001 : WAITE = 01 (1 Tcy byte write)*/
		//PMMODE =	0b0000001000000000;	 /* Single cycle PSP, not at 16MIPS, too fast*/
		PMADDR =	0b0000000000000000;	 /* CS2 = 0 : CS1 = 0 : ADDR : 0b00000000000000 */
		PMAEN  =	0b0000000000000000;	 /* PTEN = 0000 0000 0000 00 : PTEN = 00 */	 
		return;
	}
	// These settings make the PSP module take 4 Tcy per transfer, the compiler generates 2 Tcy to write a constant to the PSP
	// If consecutive reads/writes are uses with constants then 2 Tcy need adding per transfer. Alternatively, the BUSY bit can be polled.
	// Special optimised transfer code is used for bulk data transfers for the PIC24.
	//static inline unsigned char __attribute__((always_inline)) NandReadRaw(void)
	static unsigned char NandReadRaw(void)
		{volatile unsigned char data = PMDIN1;while(PMMODEbits.BUSY);return data;}

	//static inline void __attribute__((always_inline)) NandWriteRaw(unsigned char data)
	static void NandWriteRaw(unsigned char data)
		{Nop();PMDIN1 = data; while(PMMODEbits.BUSY);return;} /* Nop for optimisation error */

//	{	/* Write byte over parrallel port (independent of optimisation) */
//		__asm__ volatile(
//		"mov.w %[data], w0 \n\t"		/*  data into w0 */
//		"mov.b WREG, _PMDIN1 \n\t"		/*  w0 into PMDOUT */
//		"1: \n\t"						/*  lable for goto */
//		"cp0.b _PMMODE+1 \n\t"			/*  test PMMODEbits.BUSY */
//		"bra lt, 1b \n\t"				/*  bra if busy */
//		: "=r"(data) 					/*  outputs */
//		: [data]"r"(data)				/*  inputs, data */
//		: "w0", "cc"); 					/*  clobbered */
//	}

	#define DUMMY_READ() {dummy = NandReadRaw();} /*The dummy reads are due to the PSP module*/

	#define READ_PSP()	"mov.w _PMDIN1, w0 \n\t"		/* Read psp into wreg */\
						"mov.b wreg, _CRCDAT \n\t"		/* Put byte into crc queue */\
						"mov.b w0, [%[dest]++] \n\t"	/* Put byte at destination + increment */\
						"add.w w0, %[csL], %[csL] \n\t"	/* Add byte to low checksum counter */\
						"mov.w _PMDIN1, w0 \n\t"		/* Read psp byte into wreg */\
						"mov.b wreg, _CRCDAT \n\t"		/* Put byte into crc queue */\
						"mov.b w0, [%[dest]++] \n\t"	/* Put byte at destination + increment */\
						"add.w w0, %[csH], %[csH] \n\t"	/* Add byte to high checksum counter */

	#define WRITE_PSP()	"mov.b [%[src]++], w0 \n\t"		/* Move byte from source to wreg + increment */\
						"mov.b wreg, _CRCDAT \n\t"		/* Put byte into crc queue */\
						"mov.b wreg, _PMDIN1 \n\t"		/* Put byte over psp */\
						"add.w w0, %[csL], %[csL] \n\t"	/* Add byte to low checksum counter */\
						"mov.b [%[src]++], w0 \n\t"		/* Move byte from source to wreg + increment */\
						"mov.b wreg, _CRCDAT \n\t"		/* Put byte into crc queue */\
						"mov.b wreg, _PMDIN1 \n\t"		/* Put byte over psp */\
						"add.w w0, %[csH], %[csH] \n\t"	/* Add byte to high checksum counter */
	
	// Read loop
	static void NandReadSpan(unsigned char* buffer, unsigned short length)
	{
		// Checks
		if(!buffer || !length) return;

		//Required dummy read, due to PSP module hardware
		DUMMY_READ();	
	
		// Init crc
		CrcStart();

		/*
			Checksum made using two separate checksums with final add. Does not require read
			back of buffer hence buffer alignment unimportant. Must have non-zero even number 
			of bytes to read.
		*/
		{
			/* Name the register located variables */
			register unsigned char *dataPtr	asm("w1") = (unsigned char*)buffer;
			register unsigned short remaining asm("w2") = length;
			register unsigned short csLow asm("w3") = 0;
			register unsigned short csHigh asm("w4") = 0;
			
			__asm__ volatile(
			"bra 2f \n\t"							/* Branch forward to 2: */

			"1: \n\t"								/* Label 1: Read 32 bytes with crc + sum */
			READ_PSP() READ_PSP() READ_PSP() READ_PSP() 
			READ_PSP() READ_PSP() READ_PSP() READ_PSP() 
			READ_PSP() READ_PSP() READ_PSP() READ_PSP() 
			READ_PSP() READ_PSP() READ_PSP() READ_PSP() 
			"sub #32, %[rem] \n\t"					/* Decrement remaining by 32 */

			"2: \n\t"								/* Label 2: */
			"cp %[rem], #31 \n\t"					/* Compare remaining to 31 */
			"bra gtu, 1b \n\t" 						/* Branch back to 1: if greater than 31 */
			"bra 4f \n\t"							/* Branch forware to 4: */

			"3: \n\t" 								/* Label 3: */
			READ_PSP()								/* Read 2 bytes with crc + sum */
			"sub #2, %[rem] \n\t"					/* Decrement remaining by 2 */

			"4: \n\t"								/* Label 4: */
			"cp %[rem], #1 \n\t"					/* Compare remaining to 1 */
			"bra gtu, 3b \n\t" 						/* Branch back to 3: if greater than 1 */
			"bra ltu, 5f \n\t" 						/* Branch to 5: if zero */

			"mov.w _PMDIN1, w0 \n\t"				/* Read 1 psp byte to wreg 0 */
			"mov.b wreg, _CRCDAT \n\t"				/* Put the byte into the crc queue */
			"mov.b w0, [%[dest]++] \n\t"			/* Put the byte at the destination pointer + increment */
			"add.w w0, %[csL], %[csL] \n\t"			/* Add the byte to the low checksum counter */
			"dec.w %[rem], %[rem] \n\t"				/* Decrement the remaining */

			"5: \n\t"								/* Label 5: */
			"sl %[csH], #8, %[csH] \n\t"			/* Left shift the high checksum by 8 */
			"add.w %[csH], %[csL], %[csL] \n\t"		/* Add the two checksums into the low one */
			: "=&r"(dataPtr), "=r"(remaining), "=&r"(csLow), "=&r"(csHigh)					/* Name the outputs */
			: [dest]"0"(dataPtr), [rem]"1"(remaining), [csL]"2"(csLow), [csH]"3"(csHigh)	/* Name the inputs */
			: "w0", "cc" 							/* List the clobbered registers not in the outputs */
			);
	
			// Complete CRC and checksum
			nandsum = csLow;
			CrcEnd();
			nandcrc = CrcResult();
		}
		return;
	}
	
	static void NandWriteSpan(const unsigned char* buffer, unsigned short length)
	{
		
		// Checks
		if(!buffer || !length) return;
	
		// Init crc
		CrcStart();
	
		/*
			Checksum made using two separate checksums with final add. Does not require read
			back of buffer hence buffer alignment unimportant. Must have non-zero even number 
			of bytes to read.
		*/
		{
			/* Name the register located variables */
			register unsigned char *dataPtr	asm("w1") = (unsigned char*)buffer;
			register unsigned short remaining asm("w2") = length;
			register unsigned short csLow	asm("w3") = 0;
			register unsigned short csHigh	asm("w4") = 0;
		
			__asm__ volatile(
			"bra 2f \n\t"							/* Branch forwared to 2: */

			"1: \n\t"								/* Write 32 bytes with crc and sum */
			WRITE_PSP() WRITE_PSP() WRITE_PSP() WRITE_PSP() 
			WRITE_PSP() WRITE_PSP() WRITE_PSP() WRITE_PSP() 
			WRITE_PSP() WRITE_PSP() WRITE_PSP() WRITE_PSP() 
			WRITE_PSP() WRITE_PSP() WRITE_PSP() WRITE_PSP() 
			"sub #32, %[rem] \n\t"					/* Subtract 32 from the remaining */

			"2: \n\t"								/* Label 2: */
			"cp %[rem], #31 \n\t"					/* Compare remaining to 31 */
			"bra gtu, 1b \n\t" 						/* Branch back to 1: if greater than 31 */
			"bra 4f \n\t"							/* Branch forware to 4: */
			"3: \n\t" 								/* Label 3: */

			WRITE_PSP()								/* Write 2 bytes with crc and sum */
			"sub #2, %[rem] \n\t"					/* Subtract 2 from the remaining */

			"4: \n\t"								/* Label 4: */
			"cp %[rem], #1 \n\t"					/* Compare remaining to 1 */
			"bra gtu, 3b \n\t" 						/* Branch back to 3: if greater than 1 */
			"bra ltu, 5f \n\t" 						/* Branch forward to 5: if zero */

			"mov.b [%[src]++], w0 \n\t"				/* Read a byte from the source to wreg with increment */
			"mov.b wreg, _CRCDAT \n\t"				/* Put the byte into the crc queue */
			"mov.b wreg, _PMDIN1 \n\t"				/* Put the byte over the psp */
			"add.w w0, %[csL], %[csL] \n\t"			/* Add the byte to the low checksum counter */
			"dec.w %[rem], %[rem] \n\t"				/* Decrement the remaining count */
			
			"5: \n\t"								/* Label 5: */
			"sl %[csH], #8, %[csH] \n\t"			/* Shift the high checksum left by 8 */
			"add.w %[csH], %[csL], %[csL] \n\t"		/* Add the checksums into the low counter */
			: "=&r"(dataPtr), "=r"(remaining), "=&r"(csLow), "=&r"(csHigh)				/* Name the outputs */
			: [src]"0"(dataPtr), [rem]"1"(remaining), [csL]"2"(csLow), [csH]"3"(csHigh) /* Name the inputs */
			: "w0", "cc" 							/* List the clobbered registers not in the outputs */
			);
	
			// Complete CRC and checksum
			nandsum = csLow;
			CrcEnd();
			nandcrc = CrcResult();
		}
		return;
	}	

#else
	#error "Provide the following interface for this hardware platform."
	#define DUMMY_READ()	{;} /* If required */
	static void NandInterfaceOn(void){return;}
	extern static inline unsigned char __attribute__((always_inline)) NandReadRaw(void){return 0};
	extern static inline void __attribute__((always_inline)) NandWriteRaw(unsigned char data){return;}
	static void NandReadSpan(unsigned char* buffer, unsigned short length){return;}
	static void NandWriteSpan(const unsigned char* buffer, unsigned short length){return;}	
#endif

/* LOW LEVEL NAND INTERFACE CODE - Hardware in-specific */	
// Ready/busy wait, copied in place
static inline __attribute__((always_inline)) void NandBusyWait(void)
{
	unsigned short NandRBTimeout = NAND_TIMEOUT;
	const IO_pin_t* rbPin = &nandPins[(PIN_NAND_RB1 + activeNand)];
	// Ignore if no device selected
	if(activeNand >= NUMBER_OF_NANDS) return;
	// Wait for the pin to go high with time out, i.e. not busy
	while(!ReadIO(*rbPin) && (--NandRBTimeout > 0));
	// Time out indicates an error
	DBG_ASSERT(NandRBTimeout > 0);
	return;
}

/*MID LEVEL NAND CONTROL CODE*/
// Write a command
static void NandWriteCommand(unsigned char command)
{
	SetIO(nandPins[PIN_NAND_CLE]);	// Command latch enable
	NandWriteRaw(command);			// Write command 
	ClearIO(nandPins[PIN_NAND_CLE]);// Command latch disable
	return;
}

// Write a 5 byte address  - where only the last 3 bytes are used
static void NandWriteAddress5B(void)
{
	SetIO(nandPins[PIN_NAND_ALE]);	// Address latch enable
	NandWriteRaw(0);				// Only write zeros
	NandWriteRaw(0);				// to these two bytes
	NandWriteRaw(AddBytes[0]);		// The next 3 bytes
	NandWriteRaw(AddBytes[1]);		// are the page and
	NandWriteRaw(AddBytes[2]);		// block addresses
	ClearIO(nandPins[PIN_NAND_ALE]);// Address latch disable
	return;
}

// Write a 3 byte address  - for block address
static void NandWriteAddress3B(void)
{
	SetIO(nandPins[PIN_NAND_ALE]);	// Address latch enable
	NandWriteRaw(AddBytes[0]);		// This function
	NandWriteRaw(AddBytes[1]);		// only writes the
	NandWriteRaw(AddBytes[2]);		// block address
	ClearIO(nandPins[PIN_NAND_ALE]);// Address latch disable
	return;
}

// Write a 2 byte address  - for column address
static void NandWriteAddress2B(unsigned short Add)
{
	SetIO(nandPins[PIN_NAND_ALE]);	// Address latch enable
	NandWriteRaw(Add);				// This just writes
	NandWriteRaw(Add>>8);			// the column bytes
	ClearIO(nandPins[PIN_NAND_ALE]);// Address latch disable
	return;
}

// Read the status byte from the active device
static void NandReadStatus(void)
{
	const IO_pin_t* cePin;
	nandstatus = NAND_STATUS_OK;		// Default response
	if(activeNand >= NUMBER_OF_NANDS)
		return;
	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_GET_STATUS);	// Get status command
	DUMMY_READ();						// Dummy read (hw specific)
	nandstatus = NandReadRaw();			// Read status byte
	SetIO(*cePin);						// De-select	
	return;
}

// Reset the active device
static void NandReset(void)
{	
	const IO_pin_t* cePin;
	if(activeNand >= NUMBER_OF_NANDS)
		return;/* Out of range */
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	NandBusyWait();						// Wait for nand operations to complete
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_RESET_COMMAND); 
	SetIO(*cePin);						// De-select	
	return;
}

// Read device identifier bytes from active device - This should be static
static char NandReadDeviceId(unsigned char *destination)
{
	unsigned char i;
	const IO_pin_t* cePin;
	NandBusyWait();						// Wait for previous operations
	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_READ_ID);		// Command
	SetIO(nandPins[PIN_NAND_ALE]);		// Address latch enable
	NandWriteRaw(0);					// Address byte (this is a special case)
	ClearIO(nandPins[PIN_NAND_ALE]);	// Address latch disable
	DUMMY_READ();						// Dummy read (hw specific)
	for(i = 0; i < MAX_NAND_ID_LENGTH; i++)
	{
		destination[i] = NandReadRaw();	// Read bytes
	}
	SetIO(*cePin);						// De-select
	return NAND_SUCCESS;				// return
}

// Check for and load available devices, return count
static unsigned char NandLoadDevices(void)
{
	unsigned char i, id[MAX_NAND_ID_LENGTH];
	
	// Wipe current state, set indicators
	nandDeviceCount = 0;
	nandPresent = 0;
	activeNand = INVALID_NAND_DEVICE;

	// For all possible devices 	
	for(activeNand = 0; activeNand < NUMBER_OF_NANDS; activeNand++)
	{
		// Wait for previous operations
		NandBusyWait();						
		// Read it's ID
		if(NandReadDeviceId(id))
		{
			unsigned char identified = 0;
			// Compare to default devices
			for(i=0;i<(sizeof(nandDefaultTypesList)/sizeof(NandType_t));i++)
			{
				if(memcmp(id,nandDefaultTypesList[i].devId,nandDefaultTypesList[i].devIdLen) == 0)
				{
					// Device matched, copy info
					memcpy(&nandDevices[activeNand], &nandDefaultTypesList[i].info, sizeof(NandDeviceInfo_t));
					// Exit compare loop
					nandDeviceCount++;
					identified = 1;
					break;
				}
			}
			// Read next device id
			if(identified)continue;
			// Compare to optional externally defined devices
			#ifdef ADDITIONAL_NAND_TYPES
			for(i=0;i<(sizeof(nandTypesList)/sizeof(NandType_t));i++)
			{
				if(memcmp(id,nandTypesList[i].devId,nandTypesList[i].devIdLen) == 0)
				{
					// Device matched, copy info
					memcpy(&nandDevices[activeNand], nandTypesList[i].info, sizeof(NandDeviceInfo_t));
					// Read next device position
					nandDeviceCount++;
					identified = 1;
					break;
				}
			}
			// Read next device id
			if(identified)continue;			
			#endif
			// Device not identified
			DBG_INFO("NAND ignored/missing/unknown");
		}
		// Set device information entry to all zero
		memset(&nandDevices[activeNand], 0, sizeof(NandDeviceInfo_t));
	}
		
	// Create an aggregated virtual device from devices
	memset(&combinedNandDevice, 0, sizeof(NandDeviceInfo_t));
	if(nandDeviceCount > 0)
	{
		unsigned short pageMin = 0xFFFF, blockMin = 0xFFFF, blocksPerPlaneMin = 0xFFFF;
		unsigned long totalBlocks = 0;
		unsigned short blocksPerPlane;
		unsigned char requiredPlanes;
	
		// Calculate size and other constraints
		for(activeNand = 0;activeNand < NUMBER_OF_NANDS; activeNand++)
		{
			if(nandDevices[activeNand].blocks == 0)
				continue; /* Skip missing devices */
			// Sum totals
			totalBlocks += nandDevices[activeNand].blocks;
			planes += nandDevices[activeNand].blockPlanes;
			blocksPerPlane = nandDevices[activeNand].blocks / nandDevices[activeNand].blockPlanes;
			if(nandDevices[activeNand].pageBytes < pageMin)
				pageMin = nandDevices[activeNand].pageBytes;
			if(nandDevices[activeNand].blockPages < blockMin)
				blockMin = nandDevices[activeNand].blockPages;
			if(blocksPerPlane < blocksPerPlaneMin)
				blocksPerPlaneMin = blocksPerPlane;
		}
	
		/* Calculate number of virtual planes:
			- Each plane holds the blocks-per-plane of the smallest device
			- The plane number is rounded up to a power of two if not already
			- The plane distribution across devices is made to optimise usage
		*/
		{
			unsigned short virtualPlanesMin, virtualPlanes, virtualPlanesToTry;
			unsigned short utilisedBlocks, lastUtilisedBlocks = 0;
			unsigned char devicePlaneRatios[NUMBER_OF_NANDS][NAND_MAX_DEVICE_PLANES];
			virtualPlanesMin = totalBlocks / blocksPerPlaneMin;
			// Set global mask variable
			vPlaneMask = 0;
			vPlaneShift = 0;
			// Now try to combine the devices found into a virtual device
			// Get first power of two to try and initialise value
			virtualPlanes = 1;
			// Must be at least the number of virtual planes
			while(virtualPlanes < virtualPlanesMin)
				{virtualPlanes <<= 1;vPlaneShift++;}
			virtualPlanesToTry = virtualPlanes;
			for(;;)
			{
				unsigned short blocksPerVirtualPlane = 0xFFFF;
				unsigned char device, plane, virtualPlaneNum;
				// Checks, can't be more than the maximum planes setting
				if(virtualPlanes > NAND_MAX_V_PLANES)
					break;
				// Initialise the device virtual plane map
				memset(devicePlaneRatios, 0, sizeof(devicePlaneRatios));
				// Calculate the device block plane mapping
				for(device = 0; device < NUMBER_OF_NANDS; device++)
				{
					if(nandDevices[device].blocks == 0)
						continue; /* Skip missing devices */					
					// Calculate number of virtual planes on this device (with rounding)
					unsigned short virtualPlaneCount, virtualPlanesPerPlane, virtualPlanesRemainder;
					unsigned short maxVirtualPlanes, minBlocksPerVirtualPlane;
					virtualPlaneCount = ((unsigned long)virtualPlanesToTry * nandDevices[device].blocks + (totalBlocks >> 1)) / totalBlocks;
					// Calculate how many virtual planes per real plane on this device
					virtualPlanesPerPlane = virtualPlaneCount / nandDevices[device].blockPlanes;
					virtualPlanesRemainder = virtualPlaneCount % nandDevices[device].blockPlanes;
					maxVirtualPlanes = 0;
					// Plane ratios
					for(plane = 0; plane < nandDevices[device].blockPlanes; plane++)
					{
						devicePlaneRatios[device][plane] = virtualPlanesPerPlane;
						if(virtualPlanesRemainder > 0)
						{
							// Account for remainder if not exact
							devicePlaneRatios[device][plane]++;
							virtualPlanesRemainder--;
						}
						// Find the maximum as well
						if(maxVirtualPlanes < devicePlaneRatios[device][plane])
							maxVirtualPlanes = devicePlaneRatios[device][plane];
					}
					// Find the minimum blocks per virtual plane
					minBlocksPerVirtualPlane = nandDevices[device].blockPlanes / maxVirtualPlanes;
					if(blocksPerVirtualPlane > minBlocksPerVirtualPlane)
						blocksPerVirtualPlane = minBlocksPerVirtualPlane;
				} // Per device
				// Check if any improvement over last iteration
				utilisedBlocks = blocksPerVirtualPlane * virtualPlanesToTry;
				if(utilisedBlocks <= lastUtilisedBlocks)
				{
					// No improvement, fix shift variable, exit loop
					vPlaneShift--;
					utilisedBlocks = lastUtilisedBlocks;
					break;
				}
				// Keep this result for comparison
				vPlaneSize = blocksPerVirtualPlane;
				lastUtilisedBlocks = utilisedBlocks;
				// Keep this solution for number of virtual planes
				virtualPlanes = virtualPlanesToTry;
				// Make the blockNum to device/plane translation table
				device = 0;
				plane = 0;
				for(virtualPlaneNum = 0; virtualPlaneNum < virtualPlanes; virtualPlaneNum++)
				{
					unsigned char success = 0;
					// Scan each device for a available virtual plane
					for(;;device++)
					{
						// Wrap
						if(device >= NUMBER_OF_NANDS)
							device = 0;
						// Skip missing devices
						if(nandDevices[device].blocks == 0)
							continue; 	
						// Get a plane on this device if available
						for(plane = 0; plane < nandDevices[device].blockPlanes; plane++)
						{						
							// Allocate the virtual plane on this device to map
							if(devicePlaneRatios[device][plane] > 0)
							{
								// Remove it from the list
								blockDeviceMap[virtualPlaneNum] = device;
								blockPlaneMap[virtualPlaneNum] = plane;
								devicePlaneRatios[device][plane]--;
								// Successfully allocated, next plane goes on next device
								success = 1;
								break;
							}
						}
						// If successful, allocate next virtual plane one
						if(success) break;
						// If not successful, try next device
					}
				}	
				// This mapping is complete, get next power of two and re-try
				virtualPlanesToTry <<= 1;
				vPlaneShift++;
			}
			// Checks
			if(virtualPlanes > NAND_MAX_V_PLANES)
			{
				// Failed to create mapping
				combinedNandDevice.blocks = 0;
				vPlaneMask = 0;
				vPlaneShift = 0;
				nandPresent = 0;
				return 0;
			}
			// Set corrected number of blocks
			combinedNandDevice.blocks = utilisedBlocks;
			// Set mask variable
			vPlaneMask = virtualPlanes - 1;
			
		} // Virtual plane mapping scope
		
		// Set other outputs for aggregated NAND chip
		combinedNandDevice.blockPlanes = planes;	// Number of planes (power of 2)
		combinedNandDevice.blockPages = blockMin;	// Minimum block page count
		combinedNandDevice.pageBytes = pageMin;		// Minimum page length			

		// Set global indicators
		nandPresent = 1;							// At least one device found
		activeNand = 0;								// Reset active NAND
	} // If not already loaded
	// Return number of loaded devices
	return nandDeviceCount;
}

	
/*HIGH LEVEL NAND ACCESS API*/

// Initialize the NAND device
char NandInitialize(void)
{
	// Once at startup
	if(activeNand == INVALID_NAND_DEVICE)
	{
		// Enable HW parallel interface
		NandInterfaceOn();			
		// Issue the reset command to all possible devices
		for(activeNand = 0; activeNand < NUMBER_OF_NANDS; activeNand++)
		{
			NandReset();
		}
		// Discover available devices, aggregated to one interface			
		NandLoadDevices();		
	}	
	// Return success if any devices found
	if(nandPresent)	return NAND_SUCCESS;	
	else 			return NAND_ERROR;	
}

// Turn off all nand chips
char NandShutdown(void)
{
	// Reset all possible devices to a known, low-power state
	for(activeNand = 0; activeNand < NUMBER_OF_NANDS; activeNand++)
	{
		NandReset();
	}
	// Set active device to invalid (in case hardware shut down as well)
	activeNand = INVALID_NAND_DEVICE;
	return NAND_SUCCESS;
}

// Verify the device id's
unsigned char NandVerifyDeviceId(void)
{
	// Make sure devices have been initialized
	if(activeNand == INVALID_NAND_DEVICE)
		NandInitialize();	
	return nandPresent;
}

// Get total combined memory found as aggregated by driver
NandDeviceInfo_t* NandDeviceInfo(void)
{
	// Make sure devices have been initialized
	if(activeNand == INVALID_NAND_DEVICE)
		NandInitialize();

	// Check devices are present and non-zero
	if((nandDeviceCount == 0) || (combinedNandDevice.blocks == 0))
		return NULL;

	return &combinedNandDevice;
}	

// Read device ONFI compliant information page for debug purposes (up to 256 bytes)
char NandReadDeviceInfoPage(unsigned char chip, unsigned char *buffer, unsigned short length)
{
	const IO_pin_t* cePin;
	// Check passed parameters
	if((chip >= NUMBER_OF_NANDS) || (buffer == NULL))
		return NAND_ERROR;

	NandBusyWait();						// Wait for last chip to idle
	activeNand = chip;					// Select specified chip
	NandBusyWait();						// Wait for new chip to idle

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];					
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_READ_INFO);	// Command
	SetIO(nandPins[PIN_NAND_ALE]);		// Address latch enable
	NandWriteRaw(0);					// Address byte (this is a special case)
	ClearIO(nandPins[PIN_NAND_ALE]);	// Address latch disable
	NandBusyWait();						// Wait for read to complete
	DUMMY_READ();						// Dummy read (hw specific)
	while(length-- > 0)					// Read without dummy byte
	{
		*buffer++ = NandReadRaw();		// Read bytes
	}
	SetIO(*cePin);						// De-select
	return NAND_SUCCESS;				// return
}

// Erase a block
char NandEraseBlock(unsigned short block)
{
	const IO_pin_t* cePin;
	
	// Find real block, set active device
	block = MapVirtualBlock(block);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];

	// Erase the block
	FormAddressB(block);				// See macro
	NandBusyWait();						// Wait for selected device to idle
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_BLOCK_ERASE_1);
	NandWriteAddress3B();				// Specify block address
	NandWriteCommand(NAND_BLOCK_ERASE_2);	
	SetIO(*cePin);						// De-select
	return NAND_SUCCESS;				// Internal block erase does not throw errors
}

// Write page directly
char NandWritePage(unsigned short block, unsigned char page, unsigned char *buffer)
{
	unsigned short length;
	const IO_pin_t* cePin;

	// Find real block, set active device
	block = MapVirtualBlock(block);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];

	// Get devices page length
	length = nandDevices[activeNand].pageBytes;
	
	FormAddressBP(block,page);				// See macro
	NandBusyWait();							// Wait for selected device to idle		
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_PAGE_PROGRAM_1);	// Write page command
	NandWriteAddress5B();					// Specify source address

	NandWriteSpan(buffer, length);			// Write
		
	NandWriteCommand(NAND_PAGE_PROGRAM_2);
	SetIO(*cePin);							// De-select
	NandBusyWait();							// Wait for selected device to idle	
	NandReadStatus();						// Get status
	if (nandstatus & NAND_STATUS_FLAG_FAILED) 
		return (NAND_ERROR);				// Failed
	return NAND_SUCCESS;					// Complete
}

// Read page directly
char NandReadPage(unsigned short block, unsigned char page, unsigned char *buffer)
{
	unsigned short length;
	const IO_pin_t* cePin;

	// Find real block, set active device
	block = MapVirtualBlock(block);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];

	FormAddressBP(block,page);				// See macro
	NandBusyWait();							// Wait for selected device to idle		
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_READ_COMMAND_1);
	NandWriteAddress5B();					// Specify page
	NandWriteCommand(NAND_READ_COMMAND_2);
	NandBusyWait();							// Wait for selected device to idle	

	// Get devices page length
	length = nandDevices[activeNand].pageBytes;
	NandReadSpan(buffer, length);			// Read
	
	SetIO(*cePin);							// De-select
	return NAND_SUCCESS;					// Complete, errors not detected	
}

// Load a page in to the buffer for reading
char NandLoadPageRead(unsigned short block, unsigned char page)
{
	const IO_pin_t* cePin;

	// Find real block, set active device
	block = MapVirtualBlock(block);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	FormAddressBP(block,page);				// See macro
	NandBusyWait();							// Wait for selected device to idle
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_READ_COMMAND_1);
	NandWriteAddress5B();					// Specify page
	NandWriteCommand(NAND_READ_COMMAND_2);
	SetIO(*cePin);							// De-select
	return NAND_SUCCESS;					// No errors possible
}

// Read in from the page buffer
char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length)
{
	const IO_pin_t* cePin;
	NandBusyWait();							// Wait for selected device to idle	

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_READ_UPDATE_COLUMN_1);
	NandWriteAddress2B(offset);				// specify offset
	NandWriteCommand(NAND_READ_UPDATE_COLUMN_2);
	
	NandReadSpan(buffer, length);			// Read
	
	SetIO(*cePin);							// De-select
	return NAND_SUCCESS;					// No errors possible for command
}

// Read in from the page buffer 512 bytes to a word-aligned buffer, summing the word-wise values
char NandReadBuffer512WordSummed(unsigned short offset, unsigned short *buffer, unsigned short *outSum)
{
	char ret = NandReadBuffer(offset, (unsigned char*)buffer, 512);
	if(outSum)*outSum = nandsum;
	return ret;
}

// Read in from the page buffer 512 bytes to a word-aligned buffer, summing the word-wise values and making the byte-wise CCITT CRC16
char NandReadBuffer512WordSummedCrc(unsigned short offset, unsigned short *buffer, unsigned short *outSum, unsigned short *crc)
{
	char ret = NandReadBuffer(offset, (unsigned char*)buffer, 512);
	if(outSum)*outSum = nandsum;
	if(crc)*crc = nandcrc;	
	return ret;
}

// Copy a page
char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)	
{
	const IO_pin_t* cePin;
	
	unsigned char srcDevice;
	// Find real block, set active device
	srcBlock = MapVirtualBlock(srcBlock);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;
	srcDevice = activeNand;

	// Find real block, set active device
	destBlock = MapVirtualBlock(destBlock);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;
	
	if(srcDevice != activeNand)
		return NAND_ERROR; /* Can't copy across devices */
	
	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];	
	FormAddressBP(srcBlock,srcPage);		// See macro
	NandBusyWait();							// Wait for selected device to idle		
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_READ_COMMAND_COPY_BACK_1);	
	NandWriteAddress5B();					// Specify source address
	NandWriteCommand(NAND_READ_COMMAND_COPY_BACK_2);
	FormAddressBP(destBlock,destPage);		// See macro
	NandBusyWait();							// Wait for selected device to idle	
	NandWriteCommand(NAND_PAGE_PROGRAM_COPY_BACK_1);
	NandWriteAddress5B();					// Specify destination address
	NandWriteCommand(NAND_PAGE_PROGRAM_COPY_BACK_2);
	SetIO(*cePin);							// De-select
	NandBusyWait();							// Wait for selected device to idle	
	NandReadStatus();						// Get status
	if (nandstatus & NAND_STATUS_FLAG_FAILED) 
		return (NAND_ERROR);				// Failed
	return NAND_SUCCESS;					// Complete
}

// Load a page in to the buffer for writing to the specified location
char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
	const IO_pin_t* cePin;

	unsigned char srcDevice;
	// Find real block, set active device
	srcBlock = MapVirtualBlock(srcBlock);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;
	srcDevice = activeNand;

	// Find real block, set active device
	destBlock = MapVirtualBlock(destBlock);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;
	
	if(srcDevice != activeNand)
		return NAND_ERROR; /* Can't copy across devices */

	// Issue command to correct chip
	cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	FormAddressBP(srcBlock,srcPage);		// See macro
	NandBusyWait();							// Wait for selected device to idle		
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_READ_COMMAND_COPY_BACK_1);	
	NandWriteAddress5B();					// Specify source page
	NandWriteCommand(NAND_READ_COMMAND_COPY_BACK_2);
	FormAddressBP(destBlock,destPage);		// See macro
	NandBusyWait();							// Wait for selected device to idle	
	NandWriteCommand(NAND_PAGE_PROGRAM_COPY_BACK_1 );
	NandWriteAddress5B();					// Specify destination
	SetIO(*cePin);							// De-select
	return NAND_SUCCESS;					// No errors detectable
}

// Write in to the page buffer
char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
	const IO_pin_t* cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	ClearIO(*cePin);						// Select
	NandWriteCommand(NAND_WRITE_UPDATE_COLUMN);
	NandWriteAddress2B(offset);				// Specify offset
	
	NandWriteSpan(buffer, length);			// Write
	
	SetIO(*cePin);							// De-select
	return NAND_SUCCESS;					// No errors detectable
}

// Commit the current page buffer
char NandStorePage(void)
{
	const IO_pin_t* cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_PAGE_PROGRAM_COPY_BACK_2 );
	SetIO(*cePin);						// De-select
	NandBusyWait();						// Wait for selected device to idle	
	NandReadStatus();					// Get status
	if (nandstatus & NAND_STATUS_FLAG_FAILED) 
		return (NAND_ERROR);			// Failed
	return NAND_SUCCESS;				// Complete
}

// Commit the loaded page buffer to a different destination
char NandStorePageRepeat(unsigned short destBlock, unsigned char destPage)
{
	const IO_pin_t* cePin = &nandPins[(PIN_NAND_CE1 + activeNand)];
	
	unsigned char srcDevice = activeNand;
	// Find real block, set active device
	destBlock = MapVirtualBlock(destBlock);
	// Check read is within available blocks
	if(activeNand == INVALID_NAND_DEVICE) 
		return NAND_ERROR;
	
	if(srcDevice != activeNand)
		return NAND_ERROR; /* Can't copy across devices */
	
	
	FormAddressBP(destBlock,destPage);	// See macro
	NandBusyWait();						// Wait for selected device to idle	
	ClearIO(*cePin);					// Select
	NandWriteCommand(NAND_PAGE_PROGRAM_COPY_BACK_1 );
	NandWriteAddress5B();				// Specify new destination
	NandWriteCommand(NAND_PAGE_PROGRAM_COPY_BACK_2 );
	SetIO(*cePin);						// De-select
	NandBusyWait();						// Wait for selected device to idle	
	NandReadStatus();					// Get status
	if (nandstatus & NAND_STATUS_FLAG_FAILED) 
		return (NAND_ERROR);			// Failed
	return NAND_SUCCESS;				// Complete
}

// Read ONFI parameter page - Compatibility to older FTL
typedef struct 
{
    unsigned short revisionNumber;          // ONFI parameter page offset @4-5
    unsigned long  dataBytesPerPage;        // ONFI parameter page offset @80-83
    unsigned short spareBytesPerPage;       // ONFI parameter page offset @84-85
    unsigned long  pagesPerBlock;           // ONFI parameter page offset @92-94
    unsigned long  blocksPerLogicalUnit;    // ONFI parameter page offset @96-99
    unsigned char  logicalUnits;            // ONFI parameter page offset @100
} NandParameters_t;

char NandReadParameters(NandParameters_t *nandParameters)
{
	if(nandParameters == NULL)
		return NAND_ERROR;

	memset(nandParameters, 0, sizeof(NandParameters_t));

	if((combinedNandDevice.blocks == 0))
		return NAND_ERROR;

	// Fill in structure to reflect devices found
	if(nandDeviceCount)
	{
		nandParameters->revisionNumber = 0x0001;	
		nandParameters->dataBytesPerPage = combinedNandDevice.pageBytes & (~(EXPECTED_SECTOR_SIZE - 1));	
		nandParameters->spareBytesPerPage = combinedNandDevice.pageBytes & (EXPECTED_SECTOR_SIZE - 1);	
		nandParameters->pagesPerBlock = combinedNandDevice.blockPages;			
		nandParameters->blocksPerLogicalUnit = combinedNandDevice.blocks;
		nandParameters->logicalUnits = combinedNandDevice.blockPlanes; /* Assuming device plane count is 2 */	
		return NAND_SUCCESS;
	}
	// No devices present
	return NAND_ERROR;
}

