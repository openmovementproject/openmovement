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


#ifndef _FTL_H
#define _FTL_H

#include "FtlConfig.h"				// FTL configuration

#ifdef FTL_USE_XFTL
#error "ftl.h should not be included, use xftl.h instead"
#endif

// FTL options
#define FTL_NUM_LOG_ENTRIES 3       // Number of simultaneous log blocks open
#define FTL_ECC                     // Generate & check ECC codes

// Banked BAM options
//#define FTL_BAM_IN_RAM            // Store the whole BAM in RAM (8 kB buffer for 512 MB Hynix)
#define FTL_BAM_RECOVER			    // Re-use an existing banked BAM
#define FTL_BAM_PRELOAD             // Pre-load any missing entries from the banked BAM table at start-up (slower startup, but more even run-time performance)
#define FTL_BAM_CACHE 3			    // Cache first N BAM entries


// Block address map is loaded in banks
#ifndef FTL_BAM_IN_RAM

	#if FTL_LOGICAL_BLOCKS < 256
	#define FTL_BAM_BANK_SIZE (FTL_LOGICAL_BLOCKS/10)	// Debugging small size (in entries, *2 for bytes) of the block address map
	#else
	#define FTL_BAM_BANK_SIZE 256 // 256 // 340						// Size (in entries, *2 for bytes) of the block address map
	#endif

	#define FTL_BAM_NUM_BANKS ((FTL_LOGICAL_BLOCKS + FTL_BAM_BANK_SIZE - 1) / FTL_BAM_BANK_SIZE)
#else
	#define FTL_BAM_BANK_SIZE FTL_LOGICAL_BLOCKS
#endif


// Verify config...
// NAND
#if FTL_SECTOR_SIZE != 512
	#error "Sectors have to be 512 bytes"
#endif
#if ((1 << FTL_SECTOR_TO_BLOCK_SHIFT) != (FTL_SECTORS_PER_PAGE * FTL_PAGES_PER_BLOCK))
	#error "Sectors-per-block (from block shift) does equal sectors-per-page * pages-per-block"
#endif
#if FTL_PAGES_PER_BLOCK > 128
	#error "Pages-per-block would require the full 8 bits or more in the 'pageMap' - must increase all related types to a short"
#endif
#if FTL_PHYSICAL_BLOCKS % 8 != 0
	#error "Physical blocks must be a multiple of 8 for the BAM bitmap"
#endif
//#if (FTL_HARDWARE != FTL_HARDWARE_HYNIX) && (FTL_HARDWARE != FTL_HARDWARE_MICRON)
//	#error "Hardware type not defined"
//#endif
// ...FTL
#if FTL_NUM_LOG_ENTRIES < 2
	#error "You should have at least 2 log entries"
#endif
#if ((5 * (FTL_PHYSICAL_BLOCKS / FTL_PLANES) / 100) + FTL_NUM_LOG_ENTRIES + 3) > ((FTL_PHYSICAL_BLOCKS - FTL_LOGICAL_BLOCKS) / FTL_PLANES)
	#error "There aren't enough spare blocks per plane for 5% spare blocks plus enough for overhead"
#endif
#if !defined(FTL_BAM_IN_RAM) && (FTL_BAM_BANK_SIZE >= (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE / 2 / 3))		// 341
	#error "The bank size is too large to store in a single flash page (with a triple repetition code) -- make bank size smaller or add multiple pages-per-bank functionality (be careful to always fit all consecutive pages within a block)"
#elif defined(FTL_BAM_IN_RAM) && (FTL_BAM_BANK_SIZE != FTL_LOGICAL_BLOCKS)
	#error "The bank size is not correct (should equal the number of logical blocks when stored entirely in RAM)"
#endif
#if FTL_BAM_BANK_SIZE > FTL_LOGICAL_BLOCKS
	#error "The bank size is wastefully too large (should be no bigger than the number of logical blocks)."
#endif
#if FTL_BAM_NUM_BANKS > (FTL_PAGES_PER_BLOCK / 2)
    #error "Probably too many banks for current implementation (only has a single BAM block at a time) -- make the bank size larger or add multi-bam-block functionality"
#endif


// --- Public types ---
//typedef unsigned short ftlblock_t;				// 12-bit block address (0-4095)
//typedef unsigned long ftlsector_t;				// 0.5 GB requires 20-bit sector (512-byte) addresses (0-1048575)



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

unsigned short FtlIncrementInactivity(void);										// Increment inactivity counter and return current value

unsigned short FtlVerifyNandParameters(void);                                       // Verify NAND device parameters match compiled-in constants

const char *FtlHealth(void);                                                        // String describing the 'health' of the FTL

#ifdef _DEBUG
#include <stdio.h>
void FtlDebugDump(FILE *fp);
#endif

#endif
