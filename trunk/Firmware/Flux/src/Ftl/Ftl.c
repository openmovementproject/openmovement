/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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

// TODO: Consider changing to use <stdint.h> types

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Compiler.h"
#include "Ftl/Ftl.h"
#include "Ftl/Ecc.h"
#include "Peripherals/Nand.h"

#ifdef _WIN32
	#ifdef _DEBUG
		#define PRINT_LOG
	#endif
	#define PRINT_WARNINGS
#else
	#if defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG)
		//#define PRINT_LOG
//		#define PRINT_WARNINGS
	#endif
#endif


// Debugging output
#ifdef PRINT_LOG
#define LOG(...) { printf(__VA_ARGS__); }
#else
#define LOG(...) { ; }
#endif

// Debugging visualizer
#ifdef _WIN32
#define EMU_VISUALIZER
#endif
#ifdef EMU_VISUALIZER
extern void EmuVisualizerUpdate(char logical, unsigned long sector, unsigned int count, char flags);
#endif


// Log entry
typedef struct
{
	char inUse;										// Log block in use flag
	unsigned short logicalBlock;					// Logical address of the block being written
	unsigned short logBlock;						// Physical address of the log block
	unsigned short physicalBlock;					// Physical address of the old block (or 0xffff if none)
	unsigned short lastWriteSerial;					// Last written timestamp (for calculating least recently used)
	char pageIndex;									// Next write page position within the block
	char pageMap[FTL_PAGES_PER_BLOCK];				// Page mapping: logical to physical (< 0 is unused)
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
	// 13-bytes per 4 sectors = 52 bytes
} FtlPageInfo;

#define FTL_PAGEINFO_SIZE 12    // == sizeof(FtlPageInfo)

#ifdef FTL_ECC
	#if (FTL_PAGEINFO_SIZE + FTL_SECTORS_PER_PAGE * ECC_SIZE) > FTL_SPARE_BYTES_PER_PAGE
		#error "Metadata (including ECC) too large to fit in the spare bytes"
	#endif
#else
	#if FTL_PAGEINFO_SIZE > FTL_SPARE_BYTES_PER_PAGE
		#error "Metadata too large to fit in the spare bytes"
	#endif
#endif



// Complete context for the flash translation layer
typedef struct 
{
	char initialized;								// FTL initialized flag
	unsigned short writeSerial;						// For calculating least recently used log block
	FtlLogEntry logEntries[FTL_NUM_LOG_ENTRIES];	// Log block entries
	char pageOwner;									// Current flash page owner (log block), < 0 is none
	unsigned short currentBlock;					// Current logical block of the buffered page (0xffff if none);
	char currentPage;								// Current logical page in buffer (< 0 if none);
	unsigned short lastFreeBlock[FTL_PLANES];		// Last free block number (for find next)
	unsigned short tempBlock;						// Temporary block (for error recovery during log-block rewrite)
#ifndef FTL_BAM_IN_RAM
	unsigned short bamBlock;						// Current block address map block number
	char bamBankPages[FTL_BAM_NUM_BANKS];			// Current page number for each bank
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
	unsigned short blockAddressMap[FTL_BAM_BANK_SIZE];	// Logical-to-physical mapping (banked)
	unsigned char blockUsageMap[(FTL_PHYSICAL_BLOCKS/8)];	// Physical block usage map
	unsigned short badBlocks[FTL_PLANES];			// Count of bad blocks
	unsigned short warningCount;					// Count of warnings since initialization
	unsigned short errorCount;						// Count of errors since initialization
	FtlPageInfo pageInfo;							// Temporary page info structure (to save stack space)
	unsigned short inactivity;						// Inactivity count (0 = no flush required, >0 = requires flush)
#ifdef FTL_ECC
    unsigned char ecc[ECC_SIZE_512];				// Temporary ECC buffer 
#endif
} FtlContext;


// Private functions
static unsigned short FtlGetSpareBlock(unsigned short logicalBlock);						// Finds the next available spare block on a plane that matches the specified block
static char FtlCommitPage(void);															// Commits any outstanding buffer
static char FtlFinalizeLogBlock(unsigned char logId);										// Finalize the specified log block
static void FtlMarkBadBlock(unsigned short block);											// Mark the specified block as bad

// Block Address Map
#define FTL_MAP_UNUSED_BLOCK 0xffff
#define FTL_MAP_READ_ERROR 0xfffe
#define FTL_BAD_BLOCK 0xa5																	// Own bad block marker (distinguish from manufacturer-marked bad blocks)
static char FtlMapInitialize(void);															// Rebuild the map from the NAND flash
static unsigned short FtlMapGetNextFree(unsigned short logicalBlock);						// Examine the map to find the next free entry that matches the plane of the specified logical block
static void FtlMapSetPhysicalBlockUsed(unsigned short physicalBlock, char used);			// Sets the specified physical block as free or used
static unsigned short FtlMapGetEntry(unsigned short logicalBlock);							// Examine the map to find the physical entry for the specified logical block
static void FtlMapSetEntry(unsigned short logicalBlock, unsigned short physicalBlock);		// Set the physical address for the specified logical block entry
#ifndef FTL_BAM_IN_RAM
static unsigned short FtlMapLoadBank(unsigned short logicalBlock);							// Ensures the BAM bank containing the specified logical block is loaded
#endif

// Metadata
#define FTL_PAGE_INFO_BLOCK ECC_4SYMBOL_0    	// 0x03
#define FTL_PAGE_INFO_LOG   ECC_4SYMBOL_1    	// 0x1C
#define FTL_PAGE_INFO_BAM   ECC_4SYMBOL_2    	// 0xE0
#define FTL_PAGE_INFO_FREE  ECC_4SYMBOL_3    	// 0xFF
#define FTL_PAGE_INFO_BAD   ECC_4SYMBOL_ERROR	// 0x01
static void FtlWritePageInfo(unsigned char badBlock, unsigned char blockType, unsigned short block, char page);		// Write page metadata
static void FtlReadPageInfo(unsigned char *badBlock, unsigned char *blockType, unsigned short *block, char *page);		// Read page metadata


// FTL state
static FtlContext ftlContext;


// Serious warning handler
static void FtlWarning(const char *message)
{
	LOG("FtlWarning(): %s\n", message);
	ftlContext.warningCount++;
#ifdef PRINT_WARNINGS
	printf("WARNING #%d: %s\n", ftlContext.warningCount, message);
#endif
}


// Serious error handler
static void FtlError(const char *message)
{
	LOG("FtlError(): %s\n", message);
	ftlContext.errorCount++;
#ifdef PRINT_WARNINGS
	printf("ERROR #%d: %s\n", ftlContext.errorCount, message);
#endif	
#ifdef _DEBUG
//	abort(); 
#endif
}


// Start-up FTL context
char FtlStartup(void)
{
	LOG("FtlStartup() -- using approx. %d bytes\n", sizeof(ftlContext));
	NandInitialize();
	ftlContext.warningCount = 0;
	ftlContext.errorCount = 0;
	ftlContext.initialized = 1;
	ftlContext.inactivity = 0;	// Set inactivity to zero (does not require flush)
	FtlMapInitialize();
	return 1;
}


// Shutdown FTL context
char FtlShutdown(void)
{
	LOG("FtlShutdown() - with %d warning(s), %d error(s)\n", ftlContext.warningCount, ftlContext.errorCount);

	// If uninitialized, return 0
	if (!ftlContext.initialized) { return 0; }

	// Fully flush
	FtlFlush(1);

	// Uninitialize the context
	ftlContext.initialized = 0;
	ftlContext.inactivity = 0;	// Set inactivity to zero (does not require flush)
	
	// Shutdown the NAND
	NandShutdown();
	
	return 1;
}

// Inactivity counter
unsigned short FtlIncrementInactivity(void)
{
	// If some activity (non-zero), return current value then increment
	if (ftlContext.inactivity > 0)
	{
		return ftlContext.inactivity++;
	}
	// Return inactive
	return 0;
}

// Flush all FTL log data
char FtlFlush(char finalize)
{
	LOG("FtlFlush() - currently %d warning(s), %d error(s)\n", ftlContext.warningCount, ftlContext.errorCount);

	// If uninitialized, return 0
	if (!ftlContext.initialized) { return 0; }

	// Commit any outstanding page
	FtlCommitPage();

	#ifndef FTL_BAM_IN_RAM
	// Flushes any modified BAM bank
	FtlMapLoadBank(FTL_MAP_UNUSED_BLOCK);
	#endif

	// If required, close all log blocks
	if (finalize)
	{
		char i;
		for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
		{
			FtlFinalizeLogBlock(i);
		}
	}

	// Not using the temp. block
	ftlContext.tempBlock = FTL_MAP_UNUSED_BLOCK;
	
	// Set inactivity to zero (does not require flush)
	ftlContext.inactivity = 0;
	return 1;
}


// Read sector through the FTL
char FtlReadSector(unsigned long sector, unsigned char *buffer)
{
	unsigned char i;
	unsigned short j;
	unsigned short logicalBlock, physicalBlock;
	unsigned char logicalPage, physicalPage;
	unsigned char sectorInPage;
	char found;

#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(1, sector, 1, 2);
#endif

	// If uninitialized, return 0
	if (!ftlContext.initialized)
	{ 
		FtlWarning("Attempt to read while not initialized."); 
		for (j = 0; j < FTL_SECTOR_SIZE; j++) { buffer[j] = 0xff; }
		return 0; 
	}

	// Calculate block, page, and sector in page from the full sector address
	logicalBlock = (unsigned short)(sector >> FTL_SECTOR_TO_BLOCK_SHIFT);
	logicalPage = (unsigned char)(sector >> FTL_SECTOR_TO_PAGE_SHIFT) & (unsigned char)((1 << (FTL_SECTOR_TO_BLOCK_SHIFT - FTL_SECTOR_TO_PAGE_SHIFT)) - 1);	
	sectorInPage = (unsigned char)sector & ((1 << FTL_SECTOR_TO_PAGE_SHIFT) - 1);

	// If sector out of range, return 0
	if (logicalBlock >= FTL_LOGICAL_BLOCKS)
	{
		FtlWarning("Attempt to read invalid logical block (out-of-range).");
		for (j = 0; j < FTL_SECTOR_SIZE; j++) { buffer[j] = 0xff; }
		return 0; 
	}

	// If the buffer has an owner (don't allow reading from a buffer open for writing), or if the current buffer doesn't match the requested block and page...
	if (ftlContext.pageOwner >= 0 || ftlContext.currentBlock != logicalBlock || ftlContext.currentPage != logicalPage)
	{
		// We need to load the page
		LOG("FtlReadSector(%lu)  (%d,%d)+%d  loading page...\n", sector, logicalBlock, logicalPage, sectorInPage);

		// Commit the current flash buffer
		FtlCommitPage();

		// See if currently in a log block, read from mapped location within log block
		physicalBlock = FTL_MAP_UNUSED_BLOCK;
		physicalPage = logicalPage;
		found = 0;
		for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
		{
			FtlLogEntry *logEntry = &ftlContext.logEntries[i];
			if (logEntry->inUse && logEntry->logicalBlock == logicalBlock)
			{
				char p = logEntry->pageMap[logicalPage];
				if (p >= 0 && p < FTL_PAGES_PER_BLOCK)
				{
					physicalBlock = logEntry->logBlock;
					physicalPage = (unsigned char)p;
				}
				else
				{
					physicalBlock = logEntry->physicalBlock;
					physicalPage = logicalPage;
				}
				found = 1;
				break;
			}
		}

		// If not writing to a log block, find the mapped block
		if (!found)
		{
// TODO: Cache this position (invalidate on writes)
			physicalBlock = FtlMapGetEntry(logicalBlock);
			if (physicalBlock < FTL_PHYSICAL_BLOCKS) { found = 1; }
		}

		// If we found it, read the sector
		if (found && physicalBlock < FTL_PHYSICAL_BLOCKS)
		{
			LOG("\t...loading page  @(%d,%d)\n", physicalBlock, physicalPage);
			// Load the flash buffer
			NandLoadPageRead(physicalBlock, physicalPage);
			ftlContext.pageOwner = -1;
			ftlContext.currentBlock = logicalBlock;
			ftlContext.currentPage = logicalPage;
		}
		else
		{
//FtlWarning("Reading a page that's never been written");
			LOG("\t...page never written\n");
			// We don't have an existing page, just load a dummy page from the right plane (matches the logical block's plane)
			NandLoadPageRead(logicalBlock, logicalPage);
			ftlContext.pageOwner = -1;
			ftlContext.currentBlock = logicalBlock;
			ftlContext.currentPage = logicalPage;
		}
	}
	else
	{
		// The required page is already in the buffer
		LOG("FtlReadSector(%lu)  (%d,%d)+%d  page in buffer\n", sector, logicalBlock, logicalPage, sectorInPage);
	}

	// If block was found and loaded, read from the buffer, otherwise return empty sector (of 0xff)
	if (ftlContext.currentBlock == logicalBlock && ftlContext.currentPage == logicalPage)
	{
		NandReadBuffer(sectorInPage * FTL_SECTOR_SIZE, buffer, FTL_SECTOR_SIZE);

#ifdef FTL_ECC
        {
            char ret;

            // Retrieve ecc from flash buffer
            NandReadBuffer(FTL_SPARE_OFFSET + FTL_PAGEINFO_SIZE + (sectorInPage * ECC_SIZE_512), ftlContext.ecc, ECC_SIZE_512);

            // Calculate the ECC value
            ret = EccCheck512(buffer, ftlContext.ecc);

            if (ret < 0)
            {
#ifdef ECC_DEBUG
	#warning "ECC debug output is switched on for reads"
	printf("WARNING: Un-correctable error, sector %lu (%d,%d)+%d\n", sector, logicalBlock, logicalPage, sectorInPage);
#endif
    			LOG("\t...INFO: Un-correctable error, sector %lu (%d,%d)+%d\n", sector, logicalBlock, logicalPage, sectorInPage);
				FtlWarning("Un-correctable error in page");
				return 0;
            }
            else if (ret > 0)
            {
#ifdef ECC_DEBUG
	printf("INFO: Corrected error, sector %lu (%d,%d)+%d\n", sector, logicalBlock, logicalPage, sectorInPage);
#endif
    			LOG("\t...INFO: Corrected error, sector %lu (%d,%d)+%d\n", sector, logicalBlock, logicalPage, sectorInPage);
				return 1;
            }
LOG("\t...INFO: Read OK\n");
			return 1;
        }
#endif
	}
	else
	{
		for (j = 0; j < FTL_SECTOR_SIZE; j++) { buffer[j] = 0xff; }
		return 0;
	}
}


// Directly read a raw NAND sector (FTL-aware) - for recovery
char FtlReadRawSector(unsigned long sector, unsigned char *buffer)
{
	unsigned short physicalBlock;
	unsigned char physicalPage;
	unsigned char sectorInPage;

	// Calculate block, page, and sector in page from the full sector address
	physicalBlock = (unsigned short)(sector >> FTL_SECTOR_TO_BLOCK_SHIFT);
	physicalPage = (unsigned char)(sector >> FTL_SECTOR_TO_PAGE_SHIFT) & (unsigned char)((1 << (FTL_SECTOR_TO_BLOCK_SHIFT - FTL_SECTOR_TO_PAGE_SHIFT)) - 1);
	sectorInPage = (unsigned char)sector & ((1 << FTL_SECTOR_TO_PAGE_SHIFT) - 1);

	// If sector out of range, return 0
	if (physicalBlock >= FTL_PHYSICAL_BLOCKS) { return 0;  }

	// If initialized, commit the current flash buffer
	if (ftlContext.initialized)
    {
        FtlCommitPage();
        ftlContext.pageOwner = -1;
        ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;
        ftlContext.currentPage = -1;
    }

    // Load the page for reading
    NandLoadPageRead(physicalBlock, physicalPage);
	// Read from the buffer
    NandReadBuffer(sectorInPage * FTL_SECTOR_SIZE, buffer, FTL_SECTOR_SIZE);

	return 1;	// read
}


// Write sector through the FTL
char FtlWriteSector(unsigned long sector, const unsigned char *buffer, char ecc)
{
	FtlLogEntry *logEntry;
	unsigned short logicalBlock;
	unsigned char logicalPage;
	unsigned char sectorInPage;
	char logId;
	unsigned char i;

#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(1, sector, 1, 4);
#endif

	// If uninitialized, return 0
	if (!ftlContext.initialized) { FtlWarning("Attempt to write while not initialized."); return 0; }

	// Calculate block, page, and sector in page from the full sector address
	logicalBlock = (unsigned short)(sector >> FTL_SECTOR_TO_BLOCK_SHIFT);
	logicalPage = (unsigned char)(sector >> FTL_SECTOR_TO_PAGE_SHIFT) & (unsigned char)((1 << (FTL_SECTOR_TO_BLOCK_SHIFT - FTL_SECTOR_TO_PAGE_SHIFT)) - 1);	
	sectorInPage = (unsigned char)sector & ((1 << FTL_SECTOR_TO_PAGE_SHIFT) - 1);
	
	LOG("FtlWriteSector(%lu) (%d,%d)+%d\n", sector, logicalBlock, logicalPage, sectorInPage);

	// If sector out of range, return 0
	if (logicalBlock >= FTL_LOGICAL_BLOCKS)
	{
		FtlWarning("Attempt to write invalid logical block (out-of-range).");
		return 0; 
	}

ftl_write_sector_retry:

	// If the current buffer is suitable for writing in to, use this log entry
	if (ftlContext.pageOwner >= 0 && ftlContext.pageOwner < FTL_NUM_LOG_ENTRIES 						// If the flash page buffer has an owner
		&& ftlContext.currentBlock == logicalBlock && ftlContext.currentPage == logicalPage				// ...and the the current page is right...
		&& ftlContext.logEntries[(unsigned char)ftlContext.pageOwner].pageIndex < FTL_PAGES_PER_BLOCK					// ...and we have room to write the buffer into the log block...
	   )
	{
		// Use the log entry that currently owns the buffer
		logId = ftlContext.pageOwner;
		logEntry = &ftlContext.logEntries[(unsigned char)logId];
	}
	else
	{
		unsigned short physicalBlock;
		char physicalPage;

		// We're going to be loading the buffer, commit any existing page out
		FtlCommitPage();

		// Locate an existing log entry for this logical block
		logId = -1;
		for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
		{
			if (ftlContext.logEntries[i].inUse && ftlContext.logEntries[i].logicalBlock == logicalBlock)
			{
				logId = i;
				break;
			}
		}

		// If we've currently got a log block, and it's fully written, finalize the block
		if (logId >= 0 && ftlContext.logEntries[(unsigned char)logId].pageIndex >= FTL_PAGES_PER_BLOCK)
		{
			FtlFinalizeLogBlock((unsigned char)logId);
			logId = -1;
		}

		// If we are currently writing to this block (we have a log entry)...
		if (logId >= 0 && logId < FTL_NUM_LOG_ENTRIES)
		{
			logEntry = &ftlContext.logEntries[(unsigned char)logId];
		}
		else
		{
			// If the block is not currently being written (no log entry), we must start one

			// Find the first free log entry
			for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
			{
				if (!ftlContext.logEntries[i].inUse)
				{
					logId = i;
					break;
				}
			}

			// If there were no free log blocks, finalize the least recently used entry
			if (logId < 0)
			{
				unsigned short maxAge = 0;
				for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
				{
					unsigned short a;
					a = ftlContext.writeSerial - ftlContext.logEntries[i].lastWriteSerial;
					if (i == 0 || a >= maxAge) { maxAge = a; logId = i; }
				}
				FtlFinalizeLogBlock(logId);
			}

			// Start a new log block at this entry
			logEntry = &ftlContext.logEntries[(unsigned char)logId];
			logEntry->logBlock = FtlGetSpareBlock(logicalBlock);
			logEntry->physicalBlock = FtlMapGetEntry(logicalBlock);
			logEntry->logicalBlock = logicalBlock;
			logEntry->pageIndex = 0;
			logEntry->inOrder = 1;

			// Clear the page map
			for (i = 0; i < FTL_PAGES_PER_BLOCK; i++)
			{
				logEntry->pageMap[i] = -1;
			}

			logEntry->lastWriteSerial = ftlContext.writeSerial;

			if (logEntry->logBlock >= FTL_PHYSICAL_BLOCKS)
			{
				// Serious error, failed to get a log block
				FtlError("Failed to get a log block");
				return 0;
			}
			logEntry->inUse = 1;
		}

		// Check the page map to see if the page we need is present in our log block
		physicalPage = logEntry->pageMap[logicalPage];
		if (physicalPage >= 0 && physicalPage < FTL_PAGES_PER_BLOCK)
		{
			// Use the (most recent) page from the log block
			physicalBlock = logEntry->logBlock;
		}
		else
		{
			// Use the page from the original block
			physicalBlock = logEntry->physicalBlock;
			physicalPage = logicalPage;
		}

		// See if we have room to write the page to the log block
		if (logEntry->pageIndex >= FTL_PAGES_PER_BLOCK)
		{
			FtlFinalizeLogBlock(logId);
			goto ftl_write_sector_retry;
		}

		// Load the actual page needed (or don't-care if block never assigned: physicalBlock = 0xffff)
		if (physicalBlock < FTL_PHYSICAL_BLOCKS)
		{
			// Load the actual page needed for writing into
			NandLoadPageWrite(physicalBlock, physicalPage, logEntry->logBlock, logEntry->pageIndex);
		}
		else if (logEntry->logBlock < FTL_PHYSICAL_BLOCKS)
		{
			// We don't have an existing page, but load an empty page (will be in the same plane)
			NandLoadPageWrite(logEntry->logBlock, logEntry->pageIndex, logEntry->logBlock, logEntry->pageIndex);
		}
		else
		{
			FtlWarning("Writing to invalid log block.");
		}
		ftlContext.pageOwner = logId;
		ftlContext.currentBlock = logicalBlock;
		ftlContext.currentPage = logicalPage;

		// Clear 'in-order' flag if block will be committed out-of-order
		if (logEntry->pageIndex != ftlContext.currentPage)
		{
			logEntry->inOrder = 0;
		}
	}

	// Write sector to flash buffer
	NandWriteBuffer(sectorInPage * FTL_SECTOR_SIZE, buffer, FTL_SECTOR_SIZE);

#ifdef FTL_ECC
	if (ecc)
	{
		// Calculate the ECC value
		EccGenerate512(buffer, ftlContext.ecc);
	}
	else
	{
		// Ensure 6-byte ECC is written as none
		memset(ftlContext.ecc, 0xff, ECC_SIZE_512);
	}
    // Write ecc to flash buffer
    NandWriteBuffer(FTL_SPARE_OFFSET + FTL_PAGEINFO_SIZE + (sectorInPage * ECC_SIZE_512), ftlContext.ecc, ECC_SIZE_512);
#else
	;		// Do nothing with ECC variable
#endif

	// Update last written timestamp (for least recently used calculation)
	logEntry->lastWriteSerial = ++ftlContext.writeSerial;

	// Reset activity to non-zero (requires flush, but only just been written to)
	ftlContext.inactivity = 1;
	
	return 1;	// written
}


// Write page metadata
static void FtlWritePageInfo(unsigned char badBlock, unsigned char blockType, unsigned short block, char page)
{
	// Copy into page info structure (triplicate coding)
	ftlContext.pageInfo.badBlock = badBlock;
	ftlContext.pageInfo.blockType = blockType;	
	ftlContext.pageInfo.blockNum = block;
	ftlContext.pageInfo.page = page;
	ftlContext.pageInfo.badBlock2 = badBlock;
	ftlContext.pageInfo.blockNum1 = block;
	ftlContext.pageInfo.page1 = page;
	ftlContext.pageInfo.blockNum2 = block;
	ftlContext.pageInfo.page2 = page;
	NandWriteBuffer(FTL_SPARE_OFFSET, (unsigned char *)&ftlContext.pageInfo, sizeof(FtlPageInfo));
}


// Read page metadata
static void FtlReadPageInfo(unsigned char *badBlock, unsigned char *blockType, unsigned short *block, char *page)
{
	NandReadBuffer(FTL_SPARE_OFFSET, (unsigned char *)&ftlContext.pageInfo, sizeof(FtlPageInfo));

	// Read bad block mark
	if (badBlock != NULL)
	{
		if (ftlContext.pageInfo.badBlock == 0xff && ftlContext.pageInfo.badBlock2 == 0xff)	// Good
		{
			*badBlock = 0xff;
		}
		else if (ftlContext.pageInfo.badBlock == FTL_BAD_BLOCK || ftlContext.pageInfo.badBlock2 == FTL_BAD_BLOCK)	// User-marked bad
		{
			*badBlock = FTL_BAD_BLOCK;
		}
		else	// Factory marked bad
		{
			*badBlock = (ftlContext.pageInfo.badBlock & ftlContext.pageInfo.badBlock2);
		}
	}

	// Read block types (4x repetition coded byte)
	if (blockType != NULL)
	{
		*blockType = EccReadFourSymbol(ftlContext.pageInfo.blockType);
	}

	// Read block address (3x repetition coded)
	if (block != NULL)
	{
		*block = EccReadTriplicate(ftlContext.pageInfo.blockNum, ftlContext.pageInfo.blockNum1, ftlContext.pageInfo.blockNum2);
	}

	// Read page (3x repetition coded)
	if (page != NULL)
	{
		*page = (char)EccReadTriplicate(ftlContext.pageInfo.page, ftlContext.pageInfo.page1, ftlContext.pageInfo.page2);
	}
}


// Commit the current flash page (if any)
static char FtlCommitPage(void)
{
	unsigned char logId;
	logId = ftlContext.pageOwner;
	// If we have a valid page to commit
	if (logId < FTL_NUM_LOG_ENTRIES && ftlContext.logEntries[logId].inUse && ftlContext.logEntries[logId].logBlock < FTL_PHYSICAL_BLOCKS)
	{
		FtlLogEntry *logEntry = &ftlContext.logEntries[logId];

		LOG("\tFtlCommitPage() #%d  (%d,%d)  @(%d,%d)\n", logId, ftlContext.currentBlock, ftlContext.currentPage, logEntry->logBlock, logEntry->pageIndex);

		// Write until no errors (or block full) -- TODO: Deal with an error writing to the last page in a block (without loosing the flash buffer data).
		if (ftlContext.currentPage < 0)
		{
			FtlError("Current page is not valid.");
		}
		else 
		{
			if (logEntry->pageIndex >= FTL_PAGES_PER_BLOCK)
			{
				FtlError("Page index has been exceeded, page lost.");
			}

			// Assign meta data 
			if (logEntry->inOrder && logEntry->pageIndex + 1 >= FTL_PAGES_PER_BLOCK)
			{
				FtlWritePageInfo(0xff, FTL_PAGE_INFO_BLOCK, ftlContext.currentBlock, ftlContext.currentPage);	// last page of an in-order log block
			}
			else
			{
				FtlWritePageInfo(0xff, FTL_PAGE_INFO_LOG, ftlContext.currentBlock, ftlContext.currentPage);		// log entry
			}

			// Store the page
			if (NandStorePage())
			{ 
				// If successful, use this page index
				logEntry->pageMap[(unsigned char)ftlContext.currentPage] = logEntry->pageIndex;
				logEntry->pageIndex++;
			}
			else
			{
				// If unsuccessful, start using the next page index, flag as not in-order
				logEntry->pageIndex++;
				logEntry->inOrder = 0;

				FtlWarning("Log store unsuccessful, retrying\n");

				// Try to repeat the store in any remaining pages in the log block
				for (;;)
				{
					if (logEntry->pageIndex >= FTL_PAGES_PER_BLOCK)
					{
						// An error (re-)writing the last page in a block
						// Store in a new temporary block, finalize the existing block, start a new log block in the temporary block
						// Can't do erase as we'd loose the buffer -- the rest of the code should ensure that the page is already erased
						unsigned short logicalBlock;
						unsigned char logicalPage, physicalPage, p;

						FtlWarning("Log store unsuccessful in last block -- having to store temporarily and finalize the existing log block.");

						ftlContext.tempBlock = FtlMapGetNextFree(logEntry->logicalBlock);
						if (ftlContext.tempBlock >= FTL_PHYSICAL_BLOCKS)
						{
							FtlError("Couldn't allocate/use temporary block, the best we can do is use the last-written (failed) entry.");
							logEntry->pageMap[(unsigned char)ftlContext.currentPage] = logEntry->pageIndex - 1;
							// We're done with the retries
							break;
						}

						// Remember the logical block
						logicalBlock = logEntry->logicalBlock;
						logicalPage = ftlContext.currentPage;

						// Store current buffer in the temporary block
						physicalPage = 0;
						do
						{
							if (NandStorePageRepeat(ftlContext.tempBlock, physicalPage)) { break; }
						} while (physicalPage++ < FTL_PAGES_PER_BLOCK);

						// If we couldn't write to any of the temporary pages
						if (physicalPage >= FTL_PAGES_PER_BLOCK)
						{
							FtlError("Couldn't write new page in to temporary block, the best we can do is use the last-written (failed) entry.");
							logEntry->pageMap[(unsigned char)ftlContext.currentPage] = logEntry->pageIndex - 1;
							// No longer using the temp. block
							FtlMarkBadBlock(ftlContext.tempBlock);
							ftlContext.tempBlock = FTL_MAP_UNUSED_BLOCK;
							// We're done with the retries
							break;
						}

						// Finalize the existing block
						ftlContext.pageOwner = -1;
						ftlContext.currentPage = -1;
						ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;
						FtlFinalizeLogBlock(logId); // Potentially recursive, should only nest at most once as we've just cleared the current owner/block/page

						// Re-create the log structure around the temporary block
						logEntry->inUse = 1;
						logEntry->logicalBlock = logicalBlock;
						logEntry->logBlock = ftlContext.tempBlock;
						logEntry->physicalBlock = FtlMapGetEntry(logEntry->logicalBlock);
						logEntry->lastWriteSerial = ftlContext.writeSerial;
						logEntry->pageIndex = 0;
						logEntry->inOrder = 0;
						for (p = 0; p < FTL_PAGES_PER_BLOCK; p++)
						{
							logEntry->pageMap[p] = -1;
						}

						// Set the one log entry (for the rewritten page)
						logEntry->pageMap[logicalPage] = physicalPage;
						logEntry->pageIndex = physicalPage + 1;

						// No longer using the temp. block
						NandEraseBlock(ftlContext.tempBlock);
						ftlContext.tempBlock = FTL_MAP_UNUSED_BLOCK;

						// We have to finalize again -- we can't keep the page as it is as it may have misleading metadata (we couldn't change the buffer after the first write failed)
						FtlFinalizeLogBlock(logId); // Potentially recursive, should only nest at most once as we've just cleared the current owner/block/page

						// We're done with the retries
						break;

					}

					if (NandStorePageRepeat(logEntry->logBlock, logEntry->pageIndex))
					{
						// If retry successful, use this page index
						logEntry->pageMap[(unsigned char)ftlContext.currentPage] = logEntry->pageIndex;
						logEntry->pageIndex++;
						break;
					}

					// Will use next page index
					logEntry->pageIndex++;
				}
			}

			// See whether this should cascade to a finalize
			if (logEntry->pageIndex >= FTL_PAGES_PER_BLOCK)
			{
				ftlContext.pageOwner = -1;
				ftlContext.currentPage = -1;
				ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;
				// Potentially recursive, should only nest at most once as we've just cleared the current owner/block/page
				FtlFinalizeLogBlock(logId);
			}

		}
	}

	// Invalidate the buffer
	if (ftlContext.pageOwner != -1)
	{
		ftlContext.pageOwner = -1;
		// TODO: It should be possible to comment these two lines out -- but it fails the filesystem test -- investigate and fix (something not checking the pageOwner?)
		ftlContext.currentPage = -1;
		ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;
	}
	return 1;
}


// Finalize a log block
static char FtlFinalizeLogBlock(unsigned char logId)
{
	FtlLogEntry *logEntry;
	unsigned char logicalPage;

	LOG("\tFtlFinalizeLogBlock() id=%d\n", logId);

	// Verify log id
	if (logId >= FTL_NUM_LOG_ENTRIES) { FtlWarning("Attempt to finalize invalid log block id");  return 0; }
	if (!ftlContext.logEntries[logId].inUse) { return 0; }	// not in use
	logEntry = &ftlContext.logEntries[logId];

	// Commit anything in the flash buffer otherwise it'll be over-written
	FtlCommitPage();

	// After the commit, verify the log id is still in use 
	if (!ftlContext.logEntries[logId].inUse) { return 1; }

	// Check whether all of the pages are contiguous
	logEntry = &ftlContext.logEntries[logId];
	if (logEntry->logBlock >= FTL_PHYSICAL_BLOCKS)
	{
		FtlWarning("Attempting to commit unallocated log block.");
	}
	else if (logEntry->inOrder && logEntry->pageIndex >= FTL_PAGES_PER_BLOCK)
	{
		// Update block address mapping to point to the log block
		LOG("\t\t- in-order - #%d -> #%d, X -> #%d\n", logEntry->logicalBlock, logEntry->logBlock, logEntry->physicalBlock);

		FtlMapSetPhysicalBlockUsed(logEntry->logBlock, 1);				// Mark new log block as used
		FtlMapSetEntry(logEntry->logicalBlock, logEntry->logBlock);		// logicalBlock => logEntry
#if !defined(FTL_BAM_IN_RAM) && defined(FTL_BAM_RECOVER)
		FtlMapLoadBank(FTL_MAP_UNUSED_BLOCK);							// Flush BAM now
#endif

		// Mark original physical block as unused and erase it
		if (logEntry->physicalBlock < FTL_PHYSICAL_BLOCKS)
		{ 
			FtlMapSetPhysicalBlockUsed(logEntry->physicalBlock, 0);
			NandEraseBlock(logEntry->physicalBlock);
		}
	}
	else
	{
		unsigned short mergeBlock;
		unsigned char mergeError;

		for (;;)
		{
			// If not contiguous, must obtain a new block to merge into
			mergeBlock = FtlGetSpareBlock(logEntry->logicalBlock);
			LOG("\t\t- merge - #%d -> #%d, X -> #%d, X -> #%d\n", logEntry->logicalBlock, mergeBlock, logEntry->logBlock, logEntry->physicalBlock);
			if (mergeBlock >= FTL_PHYSICAL_BLOCKS)
			{ 
				// Shouldn't happen unless all free blocks in the plane marked as bad
				FtlError("...failed to get a merge block - lost whole log block of data");
				logEntry->inUse = 0;
				FtlMapSetPhysicalBlockUsed(logEntry->logBlock, 0);
				NandEraseBlock(logEntry->logBlock);
				return 0; 
			}

			// Copy the pages from the original physical block and the log block into the new merge block
			mergeError = 0;
			for (logicalPage = 0; logicalPage < FTL_PAGES_PER_BLOCK; logicalPage++)
			{
				unsigned short physicalBlock;
				char physicalPage;

				// Check the page map to see if this is already present in our log block
				physicalPage = logEntry->pageMap[logicalPage];
				if (physicalPage >= 0 && physicalPage < FTL_PAGES_PER_BLOCK)
				{
					// Read entry from log block
					physicalBlock = logEntry->logBlock;
					//physicalPage = physicalPage;
				}
				else
				{
					if (logEntry->physicalBlock < FTL_PHYSICAL_BLOCKS)
					{
						// Read entry from old physical block
						physicalBlock = logEntry->physicalBlock;
						physicalPage = logicalPage;
					}
					else
					{
						// We didn't have a physical block before, read a random page from the log block (at least the plane will match)
						physicalBlock = logEntry->logBlock;
						physicalPage = logicalPage;
					}
				}

				if (logicalPage == 0 || logicalPage >= FTL_PAGES_PER_BLOCK - 1)
				{
					// We're going to add in the page info
					NandLoadPageWrite(physicalBlock, physicalPage, mergeBlock, logicalPage);
					FtlWritePageInfo(0xff, FTL_PAGE_INFO_BLOCK, logEntry->logicalBlock, logicalPage);	// complete block
					if (!NandStorePage())
					{
						mergeError++;
						break;
					}
				}
				else
				{
					// Directly copy the page
					if (!NandCopyPage(physicalBlock, physicalPage, mergeBlock, logicalPage))
					{
						mergeError++;
						break;
					}
				}

			}

			// On success, finish merge
			if (mergeError == 0)
			{
				break;
			}

			FtlMarkBadBlock(mergeBlock);
			FtlWarning("Failed merge, marking as bad and retrying");
		}

		// Update block address mapping to point to new merge block (instead of the old physical block)
		FtlMapSetPhysicalBlockUsed(mergeBlock, 1);						// Mark new merge block as used
		FtlMapSetEntry(logEntry->logicalBlock, mergeBlock);				// logicalBlock => mergeBlock

		// Ensure old log block marked as unused and erase it
		FtlMapSetPhysicalBlockUsed(logEntry->logBlock, 0);
		NandEraseBlock(logEntry->logBlock);

		// If we had one, mark the original physical block as unused and erase it
		if (logEntry->physicalBlock < FTL_PHYSICAL_BLOCKS)
		{ 
			FtlMapSetPhysicalBlockUsed(logEntry->physicalBlock, 0);
			NandEraseBlock(logEntry->physicalBlock);
		}

#if !defined(FTL_BAM_IN_RAM) && defined(FTL_BAM_RECOVER)
		FtlMapLoadBank(FTL_MAP_UNUSED_BLOCK);							// Flush BAM now
#endif
	}

	// Free log block entry
	logEntry->inUse = 0;
	return 1;
}


// Mark block as bad (and not-free)
static void FtlMarkBadBlock(unsigned short block)
{
	// Mark as not-free (until restart)
	FtlMapSetPhysicalBlockUsed(block, 2);	// 0=free,1=used,2=bad

	// Erase block
	NandEraseBlock(block);

	// Write spare page info to mark as bad
	{
		// Write first page
		NandLoadPageWrite(block, 0, block, 0);
		FtlWritePageInfo(FTL_BAD_BLOCK, FTL_PAGE_INFO_FREE, FTL_MAP_UNUSED_BLOCK, 0xff);
		NandStorePage();

		// Write last page
		NandLoadPageWrite(block, FTL_PAGES_PER_BLOCK - 1, block, FTL_PAGES_PER_BLOCK - 1);
		FtlWritePageInfo(FTL_BAD_BLOCK, FTL_PAGE_INFO_FREE, FTL_MAP_UNUSED_BLOCK, 0xff);
		NandStorePage();
	}
}


// Find a new spare block compatible with the specified logical block (on the same plane)
static unsigned short FtlGetSpareBlock(unsigned short logicalBlock)
{
	unsigned char failCount;
	unsigned short block;
	
	for (failCount = 0; failCount < 5; failCount++)
	{
		// Find the next free block (not marked as bad)
		block = FtlMapGetNextFree(logicalBlock);

		// Check for failure to find any free blocks (this should only happen if there are too many bad blocks)
		if (block >= FTL_PHYSICAL_BLOCKS) { continue; }

		// If it is free, then return this as our new log block (after erasing it)
		if (!NandEraseBlock(block))
		{
			// Mark block as bad (and not-free)
			FtlMarkBadBlock(block);
			continue;
		}

		return block;
	}

	// Otherwise, can't find any free log blocks (this should only happen if there are too many bad blocks), return
	return FTL_MAP_UNUSED_BLOCK;
}


// Decode the metadata stored at the specified physical block
static unsigned char FtlBlockInfoDecode(unsigned short block, unsigned short *outBlock)
{
	unsigned char badBlock0, badBlockN;
	unsigned char blockType0, blockTypeN;
	unsigned short block0, blockN;
	char page0, pageN;

	// Read the first and last page metadata
	NandLoadPageRead(block, 0);
	FtlReadPageInfo(&badBlock0, &blockType0, &block0, &page0);
	NandLoadPageRead(block, FTL_PAGES_PER_BLOCK - 1);
	FtlReadPageInfo(&badBlockN, &blockTypeN, &blockN, &pageN);

	// Check for a definite manufacturer's bad block mark
	if (badBlock0 == 0x00 || badBlockN == 0x00)
	{
		return FTL_PAGE_INFO_BAD;
	}

	// If we couldn't extract the block information, this is severely bad!
	if (blockType0 == FTL_PAGE_INFO_BAD || blockTypeN == FTL_PAGE_INFO_BAD) { return FTL_PAGE_INFO_BAD; }

	// Check for a complete (merge-written) block or an in-order, completed log block
	if (blockTypeN == FTL_PAGE_INFO_BLOCK)
	{
		if (blockN < FTL_LOGICAL_BLOCKS)
		{
			if (outBlock != NULL) { *outBlock = blockN; }
			return FTL_PAGE_INFO_BLOCK;
		}
	}

	// Check for a block-address-map
	if (blockType0 == FTL_PAGE_INFO_BAM)
	{
		return FTL_PAGE_INFO_BAM;
	}

	// Check for an incomplete log block
	if (blockType0 == FTL_PAGE_INFO_LOG && blockTypeN == FTL_PAGE_INFO_FREE)
	{
 		if (block0 < FTL_LOGICAL_BLOCKS)
		{
			if (outBlock != NULL) { *outBlock = block0; }
			return FTL_PAGE_INFO_LOG;
		}
	}

	// Before we return as free, check for a possible manufacturer's bad block mark
	if (badBlock0 != 0xff || badBlockN != 0xff)
	{
		return FTL_PAGE_INFO_BAD;
	}

	// Otherwise it is free
	return FTL_PAGE_INFO_FREE;
}


// Wipe the FTL drive data
unsigned short FtlDestroy(char clearUserBad)
{
	unsigned char mark[4];
	unsigned short block;
	unsigned short badCount;
	
	badCount = 0;
	for (block = 0; block < FTL_PHYSICAL_BLOCKS; block++)
	{
		char erase;
		
		// Reading first page in block
		NandLoadPageRead(block, 0);
		// First mark position (first spare byte)
		NandReadBuffer(FTL_SPARE_OFFSET + 0, (unsigned char *)&mark[0], 1);
		// Second mark position (sixth spare byte)
		NandReadBuffer(FTL_SPARE_OFFSET + 5, (unsigned char *)&mark[1], 1);
		
		// Reading last page in block
		NandLoadPageRead(block, FTL_PAGES_PER_BLOCK - 1);
		// First mark position (first spare byte)
		NandReadBuffer(FTL_SPARE_OFFSET + 0, (unsigned char *)&mark[2], 1);
		// Second mark position (sixth spare byte)
		NandReadBuffer(FTL_SPARE_OFFSET + 5, (unsigned char *)&mark[3], 1);
		
		erase = 0;
		if (mark[0] == 0xff && mark[1] == 0xff && mark[2] == 0xff && mark[3] == 0xff)
		{
			erase = 1;	// non-bad block: erase
		}
		else if (clearUserBad && mark[0] == FTL_BAD_BLOCK && mark[1] == FTL_BAD_BLOCK)
		{
			erase = 1;	// user-marked bad block but want to clear: continue
		}
		else if (clearUserBad && mark[2] == FTL_BAD_BLOCK && mark[3] == FTL_BAD_BLOCK)
		{
			erase = 1;	// user-marked bad block but want to clear: continue
		}
		else if (clearUserBad == 42)	// special number
		{
			erase = 1;	// Always erase, even manufacturer-marked bad blocks... WARNING: THIS IS REALLY NOT A GOOD IDEA!
		}
		
		if (erase)
		{
			NandEraseBlock(block);
		}
		else
		{
			badCount++;
		}	
	}	
	
	FtlMapInitialize();
	return badCount;
}


// Rebuild the map from the NAND flash
static char FtlMapInitialize(void)
{
	unsigned short i, block;
	char logId;
#ifndef FTL_BAM_IN_RAM
    unsigned short bamBlock;
#endif

	ftlContext.writeSerial = 0;
	ftlContext.pageOwner = -1;
	ftlContext.currentPage = -1;
	ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;

	// Initialize 'last free block'
	for (i = 0; i < FTL_PLANES; i++)
	{
		ftlContext.lastFreeBlock[i] = FTL_MAP_UNUSED_BLOCK;
    	ftlContext.badBlocks[i] = 0;
	}

	// Initialize the usage map (mark all physical blocks as 'not free' for now)
	for (i = 0; i < (FTL_PHYSICAL_BLOCKS/8); i++)
	{
		ftlContext.blockUsageMap[i] = 0xff;
	}

#ifdef FTL_BAM_IN_RAM
	// Clear the RAM block address map (mark all logical addresses as unallocated)
	for (i = 0; i < FTL_BAM_BANK_SIZE; i++)
	{
		ftlContext.blockAddressMap[i] = FTL_MAP_UNUSED_BLOCK;
	}
#else
	// Clear the banked BAM
	for (i = 0; i < FTL_BAM_BANK_SIZE; i++)
	{
		ftlContext.blockAddressMap[i] = FTL_MAP_UNUSED_BLOCK;
	}
	ftlContext.bamBlock = FTL_MAP_UNUSED_BLOCK;
	for (i = 0; i < FTL_BAM_NUM_BANKS; i++)
	{
		ftlContext.bamBankPages[i] = -1;
	}
	ftlContext.bamBankNumber = -1;
	ftlContext.bamModified = 0;
	ftlContext.bamBankNextPage = 0;
#ifdef FTL_BAM_CACHE
	// Clear the BAM cache
	for (i = 0 ; i < FTL_BAM_CACHE; i++)
	{
		ftlContext.bamCache[i] = FTL_MAP_UNUSED_BLOCK;
	}
#endif
#endif

	// Clear any log blocks
	for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
	{
		ftlContext.logEntries[i].inUse = 0;
	}

	// Using no temporary block
	ftlContext.tempBlock = FTL_MAP_UNUSED_BLOCK;

	// Build the usage map from the NAND flash
	logId = 0;
#ifndef FTL_BAM_IN_RAM
	bamBlock = FTL_MAP_UNUSED_BLOCK;
#endif
	for (block = 0; block < FTL_PHYSICAL_BLOCKS; block++)
	{
		unsigned short logicalBlock = 0;
		unsigned char type;

		type = FtlBlockInfoDecode(block, &logicalBlock);

		if (type == FTL_PAGE_INFO_BLOCK)	// Complete
		{
			FtlMapSetPhysicalBlockUsed(block, 1);		// Mark as not-free (used)
			#ifdef FTL_BAM_IN_RAM
				// Update the RAM BAM
				if (ftlContext.blockAddressMap[logicalBlock] == FTL_MAP_UNUSED_BLOCK)
                {
    				ftlContext.blockAddressMap[logicalBlock] = block;
                }
                else
				{
#if 1               // Ignore new occurance
					FtlWarning("Duplicate block found - loosing data from duplicate block.");
        			FtlMapSetPhysicalBlockUsed(block, 0);		// Mark free
					NandEraseBlock(block);
#else
					FtlWarning("Duplicate block found - loosing data from first block.");
					FtlMapSetPhysicalBlockUsed(ftlContext.blockAddressMap[logicalBlock], 0);
					NandEraseBlock(ftlContext.blockAddressMap[logicalBlock]);
    				ftlContext.blockAddressMap[logicalBlock] = block;
#endif
				}
			#endif
		}
		else if (type == FTL_PAGE_INFO_BAD)				// Bad
		{
#if FTL_PLANES > 1
        	unsigned char plane = (logicalBlock & (FTL_PLANES - 1));
#else
        	unsigned char plane = 0;
#endif
			ftlContext.badBlocks[plane]++;              // Increment bad block count
			FtlMapSetPhysicalBlockUsed(block, 2);		// Mark as not-free (bad)
		}
		else if (type == FTL_PAGE_INFO_LOG && logId < FTL_NUM_LOG_ENTRIES)	// Incomplete log block
		{
			FtlLogEntry *logEntry;
			unsigned char physicalPage;
			char logicalPage;

			FtlMapSetPhysicalBlockUsed(block, 0);		// Mark as free (won't actually return as free as it's a log block in use)

			logEntry = &ftlContext.logEntries[(unsigned char)logId];
			logEntry->inUse = 1;
			logEntry->logicalBlock = logicalBlock;
			logEntry->logBlock = block;
			logEntry->physicalBlock = FTL_MAP_UNUSED_BLOCK;		// Will fill-in existing physical block addresses later...
			logEntry->lastWriteSerial = ftlContext.writeSerial;
			logEntry->pageIndex = 0;
			logEntry->inOrder = 1;
			// Mark all pages as unallocated for now
			for (logicalPage = 0; logicalPage < FTL_PAGES_PER_BLOCK; logicalPage++)
			{
				logEntry->pageMap[(unsigned char)logicalPage] = -1;
			}

			// Scan physical pages
			for (physicalPage = 0; physicalPage < FTL_PAGES_PER_BLOCK; physicalPage++)
			{
				unsigned char blockType;

				NandLoadPageRead(block, physicalPage);
				FtlReadPageInfo(NULL, &blockType, NULL, &logicalPage);

				if (blockType == FTL_PAGE_INFO_LOG || blockType == FTL_PAGE_INFO_BLOCK)
				{
					// Ignore invalid pages (may be a dummy 0xff page at the start of the block if a temp-block rewrite took place)
					if (logicalPage >= 0 && logicalPage < FTL_PAGES_PER_BLOCK)
					{
						logEntry->pageMap[(unsigned char)logicalPage] = physicalPage;
						if (logicalPage != physicalPage) { logEntry->inOrder = 0; }
						logEntry->pageIndex = physicalPage + 1;
					}
				}
			}

			logId++;
		}
#ifndef FTL_BAM_IN_RAM
		else if (type == FTL_PAGE_INFO_BAM
#if FTL_PLANES > 1
				 && (block & (FTL_PLANES - 1)) == 0
#endif
			)			// Block Address Map (and must be in plane 0)
		{
			// Mark as a free block
			FtlMapSetPhysicalBlockUsed(block, 0);
			if (bamBlock == FTL_MAP_UNUSED_BLOCK)
			{
				bamBlock = block;
			}
			else
			{
				FtlWarning("Duplicate BAM block found (will ignore all and re-scan).");
				// We've found two BAM blocks
				if (bamBlock < FTL_PHYSICAL_BLOCKS)
				{
					// Don't use the old one (force reconstruct)
					NandEraseBlock(bamBlock);
					bamBlock = FTL_MAP_UNUSED_BLOCK - 1;	// -1 so will work for any >1 match
				}
				// Don't use this one (force reconstruct)
				NandEraseBlock(block);
			}
		}
#endif
		else	// FTL_PAGE_INFO_FREE -- free
		{
			// If this isn't a true free block
			if (type != FTL_PAGE_INFO_FREE)
			{
				if (type == FTL_PAGE_INFO_LOG)
				{
					FtlWarning("Erasing a log block on re-start as there are too many log blocks");
				}
				else
				{
					FtlWarning("Erasing an unexpected block");
				}
				NandEraseBlock(block);
			}
			FtlMapSetPhysicalBlockUsed(block, 0);		// Mark as free
		}
	}

#ifndef FTL_BAM_IN_RAM
#ifdef FTL_BAM_RECOVER
	// If we found a BAM block, read the contents
	if (bamBlock < FTL_PHYSICAL_BLOCKS)
	{
		char bamErrors;
		char page;

		bamErrors = 0;
		ftlContext.bamBlock = bamBlock;
		ftlContext.bamBankNextPage = 0;
		for (page = 0; page < FTL_PAGES_PER_BLOCK; page++)
		{
			unsigned char blockType;
			char bank;

			NandLoadPageRead(ftlContext.bamBlock, page);

			FtlReadPageInfo(NULL, &blockType, NULL, &bank);

			if (blockType == FTL_PAGE_INFO_BAM && bank >= 0 && bank < FTL_BAM_NUM_BANKS)
			{
				ftlContext.bamBankPages[(unsigned char)bank] = page;
				ftlContext.bamBankNextPage = page + 1;
			} 
			else if (blockType != FTL_PAGE_INFO_FREE)
			{
				FtlWarning("Unexpected entry in recovered BAM, will re-scan..");
				bamErrors++;
			}
		}

		// If not loaded ok, invalidate BAM in memory (force re-load)
		if (bamErrors != 0)
		{
			unsigned char bank;
			FtlWarning("One or more problems recovering BAM, will re-scan.");
			ftlContext.bamBlock = FTL_MAP_UNUSED_BLOCK;
			ftlContext.bamBankNextPage = -1;
			for (bank = 0; bank < FTL_BAM_NUM_BANKS; bank++)
			{
				ftlContext.bamBankPages[bank] = -1;
			}
		}
	}
#endif

#ifdef FTL_BAM_PRELOAD
	// Pre-load the BAM banks (reconstructing if missing)
	for (i = 0; i < FTL_BAM_NUM_BANKS; i++)
	{
		if (ftlContext.bamBankPages[i] < 0)
		{
			FtlMapLoadBank(i * FTL_BAM_BANK_SIZE);
		}
	}
	FtlMapLoadBank(FTL_MAP_UNUSED_BLOCK);
#endif
#endif

	// Now we can need to find the physical addresses of the log blocks
	for (i = 0; i < FTL_NUM_LOG_ENTRIES; i++)
	{
		if (ftlContext.logEntries[i].inUse)
		{
			ftlContext.logEntries[i].physicalBlock = FtlMapGetEntry(ftlContext.logEntries[i].logicalBlock);
		}
	}

	// Ensure fully flushed on start (will now close any log blocks remaining from a previously interrupted state)
	FtlFlush(1);

	return 1;
}


// Look in the map to find the physical address for the specified logical block
static unsigned short FtlMapGetEntry(unsigned short logicalBlock)
{
	unsigned short entry;
	LOG("\tFtlMapGetEntry(block=%d)\n", logicalBlock);
#ifdef FTL_BAM_IN_RAM
	entry = logicalBlock;
#else
#ifdef FTL_BAM_CACHE
	// Return cached entry if present
	if (logicalBlock < FTL_BAM_CACHE && ftlContext.bamCache[logicalBlock] != FTL_MAP_UNUSED_BLOCK)
	{
		return ftlContext.bamCache[logicalBlock];
	}
#endif
	entry = FtlMapLoadBank(logicalBlock);
#endif
	if (entry >= FTL_BAM_BANK_SIZE)
	{ 
		FtlWarning("FtlMapGetEntry request out-of-range of logical blocks.");
		return FTL_MAP_UNUSED_BLOCK;
	}
	LOG("\t... (+%d):  %d ==> %d\n", entry, logicalBlock, ftlContext.blockAddressMap[entry]);
#ifdef FTL_BAM_CACHE
	// Cache entry
	if (logicalBlock < FTL_BAM_CACHE)
	{
		ftlContext.bamCache[logicalBlock] = ftlContext.blockAddressMap[entry];
	}
#endif
	return ftlContext.blockAddressMap[entry];
}


// Scan map to find the next free entry that matches the plane of the specified logical block
static unsigned short FtlMapGetNextFree(unsigned short logicalBlock)
{
	unsigned short i;
	unsigned short block;
#if FTL_PLANES > 1
	unsigned char plane = (logicalBlock & (FTL_PLANES - 1));
#else
	unsigned char plane = 0;
#endif

	// Scan the map to find the next free physical block, starting at the block following the last returned block
	// (TODO: Could speed up the search by scanning the whole byte/word for non 0xff values before determining the first clear bit)
	block = ftlContext.lastFreeBlock[plane];
	if (block >= FTL_PHYSICAL_BLOCKS) { block = plane; }
	else
	{
		block = (block & ~(unsigned short)(FTL_PLANES - 1)) + plane;	// ensure on right plane
		block += FTL_PLANES;	// Next block
	}
	for (i = 0; i < FTL_PHYSICAL_BLOCKS; i++, block++)  // TODO: Change increment from  block++  to  block += FTL_PLANES  -- should work but have to be *certain*!
	{
		if (block >= FTL_PHYSICAL_BLOCKS) { block = plane; }

#if FTL_PLANES > 1
		// TODO: Remove this check if above increment was changed -- should work but would have to be *certain*!
		if ((block & (FTL_PLANES - 1)) != plane) { continue;  }
#endif

#ifndef FTL_BAM_IN_RAM
		// Don't return the BAM block
		if (ftlContext.bamBlock != FTL_MAP_UNUSED_BLOCK && block == ftlContext.bamBlock) { continue; }
#endif

		// Don't return the temporary block (if in use)
		if (ftlContext.tempBlock != FTL_MAP_UNUSED_BLOCK && block == ftlContext.tempBlock) { continue; }
	
		// Check if the block is marked free in the usage map
		if ((ftlContext.blockUsageMap[block >> 3] & (unsigned char)((unsigned char)1 << (block & 0x07))) == 0)
		{
			// Check against log entries
			unsigned char used = 0, j;
			for (j = 0; j < FTL_NUM_LOG_ENTRIES; j++)
			{
				if (ftlContext.logEntries[j].inUse && ftlContext.logEntries[j].logBlock == block) { used = 1; break; }
			}
			if (used) { continue; }

			// Use this block
			ftlContext.lastFreeBlock[plane] = block;
			LOG("\tFtlMapGetNextFree() - %d (matching plane of block %d)\n", block, logicalBlock);
			return block;
		}
	}
	FtlWarning("Failed to get a free block from the plane.");
	return FTL_MAP_UNUSED_BLOCK;
}


#ifndef FTL_BAM_IN_RAM
// Read block address map page
char FtlBamRead(unsigned short block, char page, char expectedBank)
{
	char errors = 0;
	unsigned char blockType;
	char bank;

	if (block >= FTL_PHYSICAL_BLOCKS || page < 0 || page >= FTL_PAGES_PER_BLOCK)
	{
		FtlError("Attempted to read an invalid BAM bank block/page (re-scanning).");
		return 0;
	}

	// Invalidate NAND buffer (should already be flushed)
	ftlContext.pageOwner = -1;
	ftlContext.currentPage = -1;
	ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;

	// Read BAM bank (first repetition)
	NandLoadPageRead(block, page);
	FtlReadPageInfo(NULL, &blockType, NULL, &bank);
	NandReadBuffer(0, (unsigned char *)ftlContext.blockAddressMap, sizeof(ftlContext.blockAddressMap));

	// Check the block metadata
	if (blockType != FTL_PAGE_INFO_BAM || bank != expectedBank)
	{
		FtlError("Read BAM bank is not as expected (re-scanning).");
		return 0;
	}

	// Read repetition codes
	{
		unsigned short i, j;

		for (i = 0, j = 0; i < FTL_BAM_BANK_SIZE; i++)
		{
			if (j == 0)
			{
				unsigned short elements = FTL_BAM_CHECK_ENTRIES;
				unsigned short k = i / FTL_BAM_CHECK_ENTRIES;
				if (i + elements > FTL_BAM_BANK_SIZE) { elements = FTL_BAM_BANK_SIZE - i; }
				// Load both repeat buffers
				NandReadBuffer(1 * sizeof(ftlContext.blockAddressMap) + k * FTL_BAM_CHECK_ENTRIES * sizeof(unsigned short), (unsigned char *)ftlContext.repeatBam1, elements * sizeof(unsigned short));
				NandReadBuffer(2 * sizeof(ftlContext.blockAddressMap) + k * FTL_BAM_CHECK_ENTRIES * sizeof(unsigned short), (unsigned char *)ftlContext.repeatBam2, elements * sizeof(unsigned short));
			}

			// Shortcut if the main entry matches either of the repeat entries
			if (ftlContext.blockAddressMap[i] != ftlContext.repeatBam1[j] && ftlContext.blockAddressMap[i] != ftlContext.repeatBam2[j])
			{
				ftlContext.blockAddressMap[i] = EccReadTriplicate(ftlContext.blockAddressMap[i], ftlContext.repeatBam1[j], ftlContext.repeatBam2[j]);
			}

			j++;
			if (j >= FTL_BAM_CHECK_ENTRIES) { j = 0; }
		}
	}

	return (errors == 0);
}
#endif


#ifndef FTL_BAM_IN_RAM
// Ensures the BAM bank for the specified logical block is in the buffer, returns the index to the required entry
unsigned short FtlMapLoadBank(unsigned short logicalBlock)
{
	unsigned short entry;
	char requiredBank;

	if (logicalBlock == FTL_MAP_UNUSED_BLOCK)	// Will just flush any modified bank data
	{
		requiredBank = -1;
		entry = FTL_MAP_UNUSED_BLOCK;
	}
	else
	{
		if (logicalBlock >= FTL_LOGICAL_BLOCKS)
		{
			return 0xffff;
		}
		requiredBank = (char)(logicalBlock / FTL_BAM_BANK_SIZE);
		if (requiredBank < 0 || requiredBank >= FTL_BAM_NUM_BANKS) 
		{ 
			return 0xffff; 
		}
		entry = logicalBlock % FTL_BAM_BANK_SIZE;
	}

	// See if we need to read in a new bank of the BAM
	if (ftlContext.bamBankNumber != requiredBank || requiredBank < 0)
	{
		// Write out old (modified) bank of the BAM
		if (ftlContext.bamModified && ftlContext.bamBankNumber >= 0 && ftlContext.bamBankNumber < FTL_BAM_NUM_BANKS)
		{
			char retries, bamError;

			bamError = 0;
			for (retries = 10; !bamError && retries >= 0; --retries)
			{

				// The a BAM block has never been allocated, or if no more space in the old BAM block...
				if (ftlContext.bamBlock >= FTL_PHYSICAL_BLOCKS || ftlContext.bamBankNextPage >= FTL_PAGES_PER_BLOCK)
				{
					unsigned short newBamBlock, oldBamBlock;

					// Allocate a new BAM block
					oldBamBlock = ftlContext.bamBlock;
					newBamBlock = FtlGetSpareBlock(0);

					// If we have a BAM block
					if (newBamBlock < FTL_PHYSICAL_BLOCKS)
					{
						// Start using the new BAM block
						ftlContext.bamBlock = newBamBlock;
						ftlContext.bamBankNextPage = 0;

						// If there was an old block, move the old block entries
						if (oldBamBlock < FTL_PHYSICAL_BLOCKS)
						{
							unsigned short i;
							for (i = 0; i < FTL_BAM_NUM_BANKS; i++)
							{
								// If we had that page, copy it over
								if (ftlContext.bamBankPages[i] >= 0 && ftlContext.bamBankPages[i] < FTL_PAGES_PER_BLOCK)
								{
									for (;;)
									{
										// Copy the BAM bank page
										if (NandCopyPage(oldBamBlock, ftlContext.bamBankPages[i], ftlContext.bamBlock, ftlContext.bamBankNextPage))
										{
											ftlContext.bamBankPages[i] = ftlContext.bamBankNextPage;
											ftlContext.bamBankNextPage++;
											break;
										}
										// Copy failed, trying next page
										ftlContext.bamBankNextPage++;
										if (ftlContext.bamBankNextPage >= FTL_PAGES_PER_BLOCK - 1 - (FTL_BAM_NUM_BANKS - i))
										{
											unsigned short j;
											FtlWarning("BAM copy to new block failed, dropping whole BAM (will re-scan).");
											NandEraseBlock(oldBamBlock);
											FtlMarkBadBlock(ftlContext.bamBlock);
											// Invalidate BAM
											ftlContext.bamBlock = 0;
											ftlContext.bamBankNextPage = 0;
											ftlContext.bamBankNumber = -1;
											for (j = 0; j < FTL_BAM_NUM_BANKS; j++)
											{
												ftlContext.bamBankPages[j] = -1;
											}
											ftlContext.bamModified = 0;
											// Will jump out to load new bank by scanning
											bamError = 1;
											break;
										}
									}
								}
								else
								{
									// We don't have that page, leave it as unallocated
									ftlContext.bamBankPages[i] = -1;
								}
								if (bamError) { break; }
							}
						}
					}
					else
					{
						// No BAM block to rewrite old bank too.
						FtlError("No new BAM block available to rewrite BAM to.");
					}
				}

				if (ftlContext.pageOwner >= 0)		// ftlContext.currentPage != -1   ftlContext.currentBlock != FTL_MAP_UNUSED_BLOCK
				{
					// Serious error, buffer was open for writing
					FtlError("Writing BAM bank but existing write buffer content was not committed (now lost).");
				}
				// Invalidate NAND buffer (should already be flushed)
				ftlContext.pageOwner = -1;
				ftlContext.currentPage = -1;
				ftlContext.currentBlock = FTL_MAP_UNUSED_BLOCK;

				// If we have a BAM block and a valid page to write to, write out new entry 
				if (ftlContext.bamBlock < FTL_PHYSICAL_BLOCKS && ftlContext.bamBankNextPage < FTL_PAGES_PER_BLOCK)
				{
					// Write BAM with 3x repetition codes
					NandLoadPageWrite(ftlContext.bamBlock, ftlContext.bamBankNextPage, ftlContext.bamBlock, ftlContext.bamBankNextPage);
					NandWriteBuffer(0, (unsigned char *)ftlContext.blockAddressMap, sizeof(ftlContext.blockAddressMap));										// Entry 1
					NandWriteBuffer(sizeof(ftlContext.blockAddressMap), (unsigned char *)ftlContext.blockAddressMap, sizeof(ftlContext.blockAddressMap));		// Entry 2
					NandWriteBuffer(2 * sizeof(ftlContext.blockAddressMap), (unsigned char *)ftlContext.blockAddressMap, sizeof(ftlContext.blockAddressMap));	// Entry 3
					FtlWritePageInfo(0xff, FTL_PAGE_INFO_BAM, 0x0000, ftlContext.bamBankNumber);		// Write BAM bank spare page info

					// If successful write...
					if (NandStorePage())
					{
						// Use this page reference
						ftlContext.bamBankPages[(unsigned char)ftlContext.bamBankNumber] = ftlContext.bamBankNextPage;
						ftlContext.bamBankNextPage++;
						break;	// Successful, escape the retry loop
					}

					FtlWarning("Retrying failed BAM page write...");

					// Increment for next page
					ftlContext.bamBankNextPage++;
				}
				else
				{
					FtlError("No free BAM block/page to write to.");
					break;
				}

				if (retries <= 0)
				{
					FtlError("Failed to write BAM block/page (even with retries).");
				}
			}


		}
		ftlContext.bamModified = 0;

		if (requiredBank < 0 || requiredBank >= FTL_BAM_NUM_BANKS)
		{
			// If we were requesting an invalid bank, do nothing (allows flushing any modified BAM)
#if 0
// TODO: Remove these lines
unsigned short j;
ftlContext.bamBankNumber = -1;
for (j = 0; j < (FTL_BAM_BANK_SIZE); j++) { ftlContext.blockAddressMap[j] = FTL_MAP_UNUSED_BLOCK; }
#endif
		}
		else
		{
			// If we have an allocated BAM bank to read from (a BAM block and an allocated page)...
			if (ftlContext.bamBlock < FTL_PHYSICAL_BLOCKS && ftlContext.bamBankPages[(unsigned char)requiredBank] >= 0)
			{
				// Read the requested bank of the BAM
				if (FtlBamRead(ftlContext.bamBlock, ftlContext.bamBankPages[(unsigned char)requiredBank], requiredBank))
				{
					// If read ok, set the current bank number
					ftlContext.bamBankNumber = requiredBank;
				}
				else
				{
					// Otherwise, invalidate that bank
					ftlContext.bamBankPages[(unsigned char)requiredBank] = -1;
				}
			}

			// If we haven't read the bank successfully, re-scan the blocks to construct the logical-to-physical mapping
			if (ftlContext.bamBankNumber != requiredBank)
			{
				unsigned short j;

				// We're reading a never-allocated BAM bank -- set all logical addresses in the bank as unallocated
				for (j = 0; j < FTL_BAM_BANK_SIZE; j++)
				{
					ftlContext.blockAddressMap[j] = FTL_MAP_UNUSED_BLOCK;
				}

				// Scan through all non-free blocks
				for (j = 0; j < FTL_PHYSICAL_BLOCKS; j++)
				{
					if ((ftlContext.blockUsageMap[j >> 3] & (unsigned char)((unsigned char)1 << (j & 0x07))) != 0)
					{
						unsigned short logicalBlock;
						unsigned char type;
						type = FtlBlockInfoDecode(j, &logicalBlock);
						if (type == FTL_PAGE_INFO_BLOCK && logicalBlock < FTL_LOGICAL_BLOCKS && (logicalBlock / FTL_BAM_BANK_SIZE) == requiredBank)
						{
							if (ftlContext.blockAddressMap[logicalBlock % FTL_BAM_BANK_SIZE] != FTL_MAP_UNUSED_BLOCK)
							{
								FtlWarning("Duplicate block found - loosing data from first block.");
								FtlMapSetPhysicalBlockUsed(ftlContext.blockAddressMap[logicalBlock % FTL_BAM_BANK_SIZE], 0);
								NandEraseBlock(ftlContext.blockAddressMap[logicalBlock % FTL_BAM_BANK_SIZE]);
							}
							ftlContext.blockAddressMap[logicalBlock % FTL_BAM_BANK_SIZE] = j;
#ifdef FTL_BAM_CACHE
// Cache entry
if (logicalBlock < FTL_BAM_CACHE)
{
	ftlContext.bamCache[logicalBlock] = j;
}
#endif
						}
// Not needed, but ensures block usage map up to date
#if 1
else if (type == FTL_PAGE_INFO_FREE)
{
	FtlError("Block usage map doesn't match a free block found in scan.");
	FtlMapSetPhysicalBlockUsed(j, 0);
}
#endif
					}
				}

				// Set the current bank number
				ftlContext.bamBankNumber = requiredBank;

				// Mark as modified so we get saved out on next BAM bank load
				ftlContext.bamModified = 1;
			}

		}
	}
	return entry;
}
#endif

// Set the physical address for the specified logical block entry
void FtlMapSetEntry(unsigned short logicalBlock, unsigned short physicalBlock)
{
	unsigned short entry;
	LOG("\tFtlMapSetEntry() %d ==> %d\n", logicalBlock, physicalBlock);
#ifdef FTL_BAM_IN_RAM
	entry = logicalBlock;
#else
#ifdef FTL_BAM_CACHE
	// Cache entry
	if (logicalBlock < FTL_BAM_CACHE)
	{
		ftlContext.bamCache[logicalBlock] = physicalBlock;
	}
#endif
	entry = FtlMapLoadBank(logicalBlock);
#endif
	if (entry >= FTL_BAM_BANK_SIZE)
	{ 
		FtlWarning("FtlMapSetEntry request out-of-range of logical blocks.");
		return;
	}
	LOG("\t... (+%d)\n", entry);
	if (ftlContext.blockAddressMap[entry] != physicalBlock)
	{
		ftlContext.blockAddressMap[entry] = physicalBlock;
#ifndef FTL_BAM_IN_RAM
		ftlContext.bamModified = 1;
#endif
	}
	return;
}


// Set whether a physical address is used
void FtlMapSetPhysicalBlockUsed(unsigned short physicalBlock, char used)
{
//		LOG("\t...FtlMapSetPhysicalBlockUsed() %d = USED\n", physicalBlock);
	if (physicalBlock >= FTL_PHYSICAL_BLOCKS)
	{
		FtlWarning("FtlMapSetPhysicalBlockUsed at invalid address (out of range).");
		return;
	}

#ifdef EMU_VISUALIZER
	// Update visualizer
	if (!used)		// free
	{
		EmuVisualizerUpdate(0, physicalBlock * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, 8);
	}
	else if (used == 2)	// not free: bad
	{
		EmuVisualizerUpdate(0, physicalBlock * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, 8);
	}
	else	// not free: in use
	{
		EmuVisualizerUpdate(0, physicalBlock * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, 16);
	}
#endif

	// Mark block usage map
	if (used)
	{
		ftlContext.blockUsageMap[physicalBlock >> 3] |= (unsigned char)((unsigned char)1 << (physicalBlock & 0x07));
	}
	else
	{
		ftlContext.blockUsageMap[physicalBlock >> 3] &= (unsigned char)~((unsigned char)1 << (physicalBlock & 0x07));
	}

	return;
}


#ifdef _DEBUG
void FtlDebugDump(FILE *fp)
{
	int i, j, count;
	fprintf(fp, "----------\n");

	fprintf(fp, "Usage: ");
	for (i = 0; i < FTL_PHYSICAL_BLOCKS; i++)
	{
		if (ftlContext.blockUsageMap[i >> 3] & (1 << (i & 0x07))) { fprintf(fp, "%d; ", i); }
	}
	fprintf(fp, "\n");

	fprintf(fp, "Map: ");
	for (i = 0; i < FTL_LOGICAL_BLOCKS; i++)
	{
		j = FtlMapGetEntry(i);
		if (j != FTL_MAP_UNUSED_BLOCK) { fprintf(fp, "%d => %d; ", i, j); }
	}
	fprintf(fp, "\n");

	count = 0;
	for (j = 0; j < FTL_NUM_LOG_ENTRIES; j++)
	{
		if (ftlContext.logEntries[j].inUse) { count++; }
	}
	fprintf(fp, "Log entries: %d\n", count);

	for (i = 0; i < FTL_LOGICAL_BLOCKS; i++)
	{
		for (j = 0; j < FTL_NUM_LOG_ENTRIES; j++)
		{
			if (ftlContext.logEntries[j].inUse && ftlContext.logEntries[j].logicalBlock == i)
			{
				char k;
				fprintf(fp, "Log: ");
				fprintf(fp, "%d => %d [%d] %s @%d {", ftlContext.logEntries[j].logicalBlock, ftlContext.logEntries[j].logBlock, ftlContext.logEntries[j].physicalBlock, ftlContext.logEntries[j].inOrder ? "in-order" : "out-order", ftlContext.logEntries[j].pageIndex);
				for (k = 0; k < FTL_PAGES_PER_BLOCK; k++)
				{
					char m = ftlContext.logEntries[j].pageMap[k];
					if (m >= 0)
					{
						fprintf(fp, "%d -> %d; ", k, m);
					}
				}
				fprintf(fp, "}\n");
			}
		}
	}

#if 1
#ifndef FTL_BAM_IN_RAM
	fprintf(fp, "BAM: next-page=%d, bank=%d, block=%d, modified=%d\n", ftlContext.bamBankNextPage, ftlContext.bamBankNumber, ftlContext.bamBlock, ftlContext.bamModified);
#endif
	fprintf(fp, "Misc: cur-block=%d, cur-page=%d, last-free-block=%d, page-owner=%d\n", ftlContext.currentBlock, ftlContext.currentPage, ftlContext.lastFreeBlock, ftlContext.pageOwner);
#endif

	fprintf(fp, "==========\n");
}
#endif


/*
	// Debug print out the number of free blocks
	{
		short free = 0;
		for (i = 0; i < FTL_PHYSICAL_BLOCKS; i++)
			if ((ftlContext.blockUsageMap[i >> 3] & (1 << (i & 0x07))) == 0) free++;
		LOG("[%04d]\n", free);
	}
*/



// Internal function
static char *writeNumber(char *p, unsigned int number)
{
    if (number >= 10000) { *p++ = '0' + (number / 10000) % 10; }
    if (number >=  1000) { *p++ = '0' + (number /  1000) % 10; }
    if (number >=   100) { *p++ = '0' + (number /   100) % 10; }
    if (number >=    10) { *p++ = '0' + (number /    10) % 10; }
                         { *p++ = '0' + (number        ) % 10; }
    return p;
}


// Verify NAND device parameters match compiled-in constants
unsigned short FtlVerifyNandParameters(void)
{
    unsigned int errors = 0x0000;
    static NandParameters nandParameters;
    if (!NandReadParameters(&nandParameters)) { errors |= 0x8000; }                                               // Failed to read parameters
    else
    {
        // Lower byte contains warnings (unexpected values)
        if (!(nandParameters.revisionNumber & 0x0001))                                      { errors |= 0x0001; } // ONFI 1.0 not supported
        if (nandParameters.dataBytesPerPage != (FTL_SECTOR_SIZE * FTL_SECTORS_PER_PAGE))    { errors |= 0x0002; } // Page size not expected
        if (nandParameters.spareBytesPerPage != FTL_SPARE_BYTES_PER_PAGE)                   { errors |= 0x0004; } // Spare bytes per page not expected
        if (nandParameters.pagesPerBlock != FTL_PAGES_PER_BLOCK)                            { errors |= 0x0008; } // Pages per block not expected
        if (nandParameters.blocksPerLogicalUnit != FTL_PHYSICAL_BLOCKS / FTL_PLANES)        { errors |= 0x0010; } // Blocks per plane not expected
        if (nandParameters.logicalUnits != FTL_PLANES)                                      { errors |= 0x0020; } // Number of planes not expected

        // Upper byte contains errors (insufficient values)
        if (!(nandParameters.revisionNumber & 0x0001))                                      { errors |= 0x0100; } // ONFI 1.0 not supported
        if (nandParameters.dataBytesPerPage + nandParameters.spareBytesPerPage
                < FTL_SPARE_OFFSET + FTL_PAGEINFO_SIZE + (FTL_SECTORS_PER_PAGE * ECC_SIZE_512))
                                                                                            { errors |= 0x0600; } // Total bytes (page + spare) per page not sufficient
        if (nandParameters.pagesPerBlock < FTL_PAGES_PER_BLOCK)                             { errors |= 0x0800; } // Pages per block insufficient
        if (nandParameters.blocksPerLogicalUnit * nandParameters.logicalUnits
                < FTL_PHYSICAL_BLOCKS)                                                      { errors |= 0x1000; } // Total blocks insufficient
        if (nandParameters.logicalUnits > FTL_PLANES)                                       { errors |= 0x2000; } // More planes than expected (plane affinity will fail)
    }
    return errors;
}


// String describing the 'health' of the FTL
const char *FtlHealth(void)
{
    // "warn#,erro#,spare,2,bad-0,bad-1\0"
    #define FTL_HEALTH_BUFFER (6 + 6 + 6 + 3 + (6 * FTL_PLANES))
    static char health[FTL_HEALTH_BUFFER];
    unsigned char i;
    unsigned int spareBlocksPerPlane;
    char *p;

    spareBlocksPerPlane = ((FTL_PHYSICAL_BLOCKS - FTL_LOGICAL_BLOCKS) / FTL_PLANES) - FTL_NUM_LOG_ENTRIES - 1;

    p = health;
    p = writeNumber(p, ftlContext.warningCount);                    // Number of warning messages
    *p++ = ','; p = writeNumber(p, ftlContext.errorCount);          // Number of error messages
    *p++ = ','; p = writeNumber(p, spareBlocksPerPlane);            // Number of spare blocks per plane
    *p++ = ','; p = writeNumber(p, FTL_PLANES);                     // Number of planes
	for (i = 0; i < FTL_PLANES; i++)                                // For each plane...
	{
        *p++ = ','; p = writeNumber(p, ftlContext.badBlocks[i]);    // ...number of bad blocks
	}
    *p++ = '\0';
    return health;
}
