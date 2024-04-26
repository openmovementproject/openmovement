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

// FTL - Flash Translation Layer
// Dan Jackson, 2011


#ifndef _XFTL_H
#define _XFTL_H

// Includes
#include "Ftl/xEcc.h"

// FTL options
#define FTL_NUM_LOG_ENTRIES 3       // Number of simultaneous log blocks open
#define FTL_ECC                     // Generate & check ECC codes

// Banked BAM options
//#define FTL_BAM_IN_RAM            // Store the whole BAM in RAM (8 kB buffer for 512 MB Hynix)
#define FTL_BAM_RECOVER			    // Re-use an existing banked BAM
#define FTL_BAM_PRELOAD             // Pre-load any missing entries from the banked BAM table at start-up (slower startup, but more even run-time performance)
#define FTL_BAM_CACHE 3			    // Cache first N BAM entries

// KL Changes
#define FTL_MAX_PLANES			8
#define FTL_MAX_BAM_NUM_BANKS	32
//#define FTL_TESTMODE_SPEED_ECC_OFF_ON_READ


// Types
// Log entry
typedef struct
{
	char inUse;										// Log block in use flag
	unsigned short logicalBlock;					// Logical address of the block being written
	unsigned short logBlock;						// Physical address of the log block
	unsigned short physicalBlock;					// Physical address of the old block (or 0xffff if none)
	unsigned short lastWriteSerial;					// Last written timestamp (for calculating least recently used)
	char pageIndex;									// Next write page position within the block
//char pageMap[FTL_PAGES_PER_BLOCK];				// Page mapping: logical to physical (< 0 is unused)
	char* pageMap;									// Page mapping: logical to physical (< 0 is unused)
	char inOrder;									// Flag for in-order block commits
} FtlLogEntry;

// Page meta data stored in the extra bits (WARNING: Alignment-sensitive, 1- or 2-byte alignment must be used)
typedef struct {
	unsigned char badBlock;		// OK = 0xff, detected-bad = 0xaa, manufacturer-bad = other
	unsigned char blockType;	// 0b00000000 = block (or last page in an in-order log block), 0b01010101 = log-entry, 0b11111111 = free (or under internal use as a BAM bank)
	unsigned short blockNum;	// 12-bit block number
	unsigned char page;			// 6-bit page number
	unsigned char badBlock2;	// (on some devices this 6th byte is also used for bad block marking) OK = 0xff, detected-bad = 0xaa, manufacturer-bad = other
	unsigned short blockNum1;	// 12-bit block number repetition
	unsigned short blockNum2;	// 12-bit block number repetition 2
	unsigned char page1;		// 6-bit page number repetition
	unsigned char page2;		// 6-bit page number repetition 2	// [dgj] Moved here to improve alignment on 16-bit compilers
} FtlPageInfo;

// FTL memory access parameters 
typedef struct {
	// Flash device information
	struct NandDeviceInfo_tag *device; // NAND device info
									   // Calculated parameters
	unsigned short logicalBlocks;	// Account for spaces
	unsigned short sectorsPerPage;	// Page sector count 
	unsigned short spareBytesOffset;// Start of spare area 
	unsigned short sparePageBytes;	// Extra page bytes
	unsigned short sectorSize;		// Should be 512
	unsigned long logicalSectors;	// Disk capacity
									// Shifts, masks and constants
	unsigned short blockPlaneMask;	// AND mask to get plane
	unsigned short sectorBlockShift;// LBA >> shift = block, 8
	unsigned short sectorPageShift;	// (LBA % pagePerBlock) >> shift = page, 2 
	unsigned short bamBankSize;		// Size of each page of the BAM stored
	unsigned short numBamBanks;		// Number of BAM pages kept in memory
} FtlFlashInfo_t;

// Complete context for the flash translation layer
typedef struct 
{
	char initialized;								// FTL initialized flag
	unsigned short writeSerial;						// For calculating least recently used log block
	FtlLogEntry logEntries[FTL_NUM_LOG_ENTRIES];	// Log block entries
	char pageOwner;									// Current flash page owner (log block), < 0 is none
	unsigned short currentBlock;					// Current logical block of the buffered page (0xffff if none);
	char currentPage;								// Current logical page in buffer (< 0 if none);
	unsigned short lastFreeBlock[FTL_MAX_PLANES];		// Last free block number (for find next)
	unsigned short tempBlock;						// Temporary block (for error recovery during log-block rewrite)
#ifndef FTL_BAM_IN_RAM
	unsigned short bamBlock;						// Current block address map block number
	char bamBankPages[FTL_MAX_BAM_NUM_BANKS];			// Current page number for each bank
	char bamBankNextPage;							// Next page to write to
	char bamBankNumber;								// Currently loaded BAM bank number (-1 for none)
	char bamModified;								// BAM bank dirty flag
	#define FTL_BAM_CHECK_ENTRIES 32
	unsigned short repeatBam1[FTL_BAM_CHECK_ENTRIES];	// Temporary BAM check buffer 1 (to save stack space)
	unsigned short repeatBam2[FTL_BAM_CHECK_ENTRIES];	// Temporary BAM check buffer 2 (to save stack space)
#ifdef FTL_BAM_CACHE
	unsigned short bamCache[FTL_BAM_CACHE];			// Cache of the first few blocks on the device
#endif
#endif
//unsigned short blockAddressMap[FTL_BAM_BANK_SIZE];	// Logical-to-physical mapping (banked)
unsigned short* blockAddressMap;
//unsigned char blockUsageMap[(FTL_PHYSICAL_BLOCKS/8)];	// Physical block usage map
unsigned char* blockUsageMap;	// Physical block usage map
	unsigned short badBlocks[FTL_MAX_PLANES];			// Count of bad blocks
	unsigned short warningCount;					// Count of warnings since initialization
	unsigned short errorCount;						// Count of errors since initialization
	FtlPageInfo pageInfo;							// Temporary page info structure (to save stack space)
	unsigned short inactivity;						// Inactivity count (0 = no flush required, >0 = requires flush)
#ifdef FTL_ECC
    unsigned char ecc[ECC_SIZE_512];				// Temporary ECC buffer 
#endif
	FtlFlashInfo_t flashParameters;					// NAND device info
} FtlContext;

// Change constants to use table
#define FTL_PHYSICAL_BLOCKS				ftlContext.flashParameters.device->blocks
#define FTL_PAGES_PER_BLOCK				ftlContext.flashParameters.device->blockPages
#define FTL_PLANES 						ftlContext.flashParameters.device->blockPlanes
#define FTL_LOGICAL_BLOCKS				ftlContext.flashParameters.logicalBlocks
#define FTL_SECTOR_TO_BLOCK_SHIFT		ftlContext.flashParameters.sectorBlockShift
#define FTL_SECTOR_TO_PAGE_SHIFT		ftlContext.flashParameters.sectorPageShift
#define FTL_SECTORS_PER_PAGE			ftlContext.flashParameters.sectorsPerPage
#define FTL_SPARE_OFFSET				ftlContext.flashParameters.spareBytesOffset
#define FTL_BAM_BANK_SIZE				ftlContext.flashParameters.bamBankSize
#define FTL_BAM_NUM_BANKS				ftlContext.flashParameters.numBamBanks
#define FTL_SPARE_BYTES_PER_PAGE		ftlContext.flashParameters.sparePageBytes
#define FTL_SECTOR_SIZE					512

// Globals
// FTL state
extern FtlContext ftlContext;



// --- Public functions ---

char FtlStartup(void);																// Starts the FTL layer
unsigned short FtlDestroy(char clearUserBad);										// Wipes the FTL drive
char FtlShutdown(void);																// Shuts down the FTL layer
char FtlFlush(char finalize);														// Flushes all FTL buffers
char FtlReadSector(unsigned long sector, unsigned char *buffer);					// Reads the specified sector through the FTL (verifying ECC data if present)
char FtlWriteSector(unsigned long sector, const unsigned char *buffer, char ecc);	// Writes the specified sector through the FTL (optionally writing ECC data)

// Specialist utility functions
char FtlReadRawSector(unsigned long physicalSector, unsigned char *buffer);         // Directly read a raw NAND sector (FTL-aware) - for recovery
char FtlTranslateLogicalSectorToPhysical(unsigned long logicalSector, unsigned short *physicalBlock, unsigned char *page, unsigned char *sectorInPage);	// Translate a logical sector address to a physical block address (and, optionally, the physical page address and sector-in-page offset)
char FtlRelocatePhysicalBlockAndMarkBad(unsigned short physicalBlock);				// Relocate the block contents and mark the specified physical block as bad

char FtlDiskParameters(unsigned long *logicalSectors, unsigned short *sectorSize, unsigned short *eraseBlockSectors);	// Return the FTL's disk parameters.

unsigned short FtlIncrementInactivity(void);										// Increment inactivity counter and return current value

unsigned short FtlVerifyNandParameters(void);                                       // Verify NAND device parameters match compiled-in constants

const char *FtlHealth(void);                                                        // String describing the 'health' of the FTL

#ifdef _DEBUG
#include <stdio.h>
void FtlDebugDump(FILE *fp);
#endif

#endif
