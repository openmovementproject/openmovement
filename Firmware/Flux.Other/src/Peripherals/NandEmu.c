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

// Emulated NAND Flash interface
// Dan Jackson, 2011-2012

#include "HardwareProfile.h"

#ifdef FTL

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "FtlConfig.h"			// Emulation relies on the FTL configuration
#ifdef FTL_USE_XFTL
#include "Peripherals/xNand.h"
#else
#include "Peripherals/Nand.h"
#endif

#define NAND_PHYSICAL_BLOCKS FTL_PHYSICAL_BLOCKS
#define NAND_SECTORS_PER_PAGE FTL_SECTORS_PER_PAGE
#define NAND_PAGES_PER_BLOCK FTL_PAGES_PER_BLOCK
#define NAND_SPARE_OFFSET FTL_SPARE_OFFSET
#define NAND_PLANES FTL_PLANES
#define NAND_SECTOR_SIZE FTL_SECTOR_SIZE
#define NAND_SPARE_BYTES_PER_PAGE FTL_SPARE_BYTES_PER_PAGE


//#define DEBUG_BAD 1              // Bad block debugging (first bad block)

#ifndef NO_EMU_VISUALIZER
#define EMU_VISUALIZER
#endif
#ifdef EMU_VISUALIZER
extern char EmuVisualizerUpdate(char logical, unsigned long sector, unsigned int count, char flags);
#endif

#include <assert.h>

static unsigned char *nandBuffer = NULL;
static unsigned char *nandStore = NULL;
static unsigned short nandDestBlock = 0xffff;
static unsigned char nandDestPage = 0xff;

// Check page write order within each block
static int writeIndex[NAND_PHYSICAL_BLOCKS] = { 0 };


#ifdef FTL_USE_XFTL
// Get the nand parameters for the FTL
NandDeviceInfo_t *NandDeviceInfo(void)
{
	static NandDeviceInfo_t nandDeviceInfo = { 0 };
	nandDeviceInfo.blocks = NAND_PHYSICAL_BLOCKS;		// Block count 
	nandDeviceInfo.blockPlanes = NAND_PLANES;	// Number of planes
	nandDeviceInfo.blockPages = NAND_PAGES_PER_BLOCK;	// Page count
	nandDeviceInfo.pageBytes = NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE;	// Page length
	return &nandDeviceInfo;
}
#else
char NandReadParameters(NandParameters_t *nandParameters)
{
	nandParameters->revisionNumber = 0;
	nandParameters->dataBytesPerPage = NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE;
	nandParameters->spareBytesPerPage = NAND_SPARE_BYTES_PER_PAGE;
	nandParameters->pagesPerBlock = NAND_PAGES_PER_BLOCK;
	nandParameters->blocksPerLogicalUnit = NAND_PHYSICAL_BLOCKS / NAND_PLANES;
	nandParameters->logicalUnits = NAND_PLANES;
	return 0;
}
#endif


//#define NAND_DUMP
#ifdef NAND_DUMP
static void hexdump(const void *buffer, size_t length)
{
	const unsigned char *buf = (const unsigned char *)buffer;
	char w = 16, b;
	unsigned short o;
	for (o = 0; o < length; o += w)
	{
		unsigned char z = w;
		if (o + z >= (int)length) { z = (unsigned char)(length - o); }
		printf("%03x:  ", o);
	    for (b = 0; b < 16; b++)
	    {
		    if (b < z) { printf("%02x ", buf[o + b]); } else { printf("   "); }
			if (b % 4 == 3) { printf(" "); }
	    }
	    for (b = 0; b < 16; b++)
	    {
			if (b < z) { printf("%c", ((buf[o + b] & 0x7f) < 0x20) ? '.' : (buf[o + b])); } else { printf(" "); }
	    }
	    printf("\r\n");	    
	}
}
#endif


char NandInitialize(void)
{
	if (nandBuffer == NULL)
	{
		unsigned long len = (unsigned long)NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE;
		nandBuffer = (unsigned char *)malloc(len);
		if (nandBuffer == NULL) return 0;
		memset(nandBuffer, 0xff, len);
	}
	if (nandStore == NULL)
	{
		unsigned long len = (unsigned long)NAND_PHYSICAL_BLOCKS * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE);
		nandStore = (unsigned char *)malloc(len);
		if (nandStore == NULL) return 0;
		memset(nandStore, 0xff, len);
	}

#ifdef DEBUG_BAD
{
    int ofs = DEBUG_BAD, fail = 0;      // 0, 10  // 1, 0
// simulate different manufacturer-marked bad blocks
NandDebugMarkBadBlock(ofs + 0, 0, 0, 0x00); 
NandDebugMarkBadBlock(ofs + 1, NAND_PAGES_PER_BLOCK - 1, 0, 0x00);
NandDebugMarkBadBlock(ofs + 2, 0, 5, 0x00); 
NandDebugMarkBadBlock(ofs + 3, NAND_PAGES_PER_BLOCK - 1, 5, 0x00);
// simulate corrupted manufacturer-marked bad blocks
NandDebugMarkBadBlock(ofs + 4, 0, 0, 0x12); 
NandDebugMarkBadBlock(ofs + 5, NAND_PAGES_PER_BLOCK - 1, 0, 0x12);
NandDebugMarkBadBlock(ofs + 6, 0, 5, 0x12); 
NandDebugMarkBadBlock(ofs + 7, NAND_PAGES_PER_BLOCK - 1, 5, 0x12);
// simulate user-marked bad blocks
NandDebugMarkBadBlock(ofs + 8, 0, 0, 0xa5); NandDebugMarkBadBlock(8, 0, 5, 0xa5);
NandDebugMarkBadBlock(ofs + 9, NAND_PAGES_PER_BLOCK - 1, 0, 0xa5); NandDebugMarkBadBlock(9, NAND_PAGES_PER_BLOCK - 1, 5, 0xa5);
// set it up so the BAM write will fail on the last page
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 12);  // This will cause >10 fails for the BAM write
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 11);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 10);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 9);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 8);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 7);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 6);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 5);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 4);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 3);
NandDebugFail(fail, NAND_PAGES_PER_BLOCK - 2);
//NandDebugFail(10, NAND_PAGES_PER_BLOCK - 1);
}
#endif

	//assert(nandBuffer != NULL && nandStore != NULL );
	return 1;
}

char NandEraseBlock(unsigned short block)
{
	unsigned long offset = (unsigned long)block * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE);
	unsigned long length = (unsigned long)NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE);
	if (block >= NAND_PHYSICAL_BLOCKS) { printf("ERROR: NandEraseBlock() - invalid block.\n"); return 0; }
	memset(nandStore + offset, 0xff, length);
#ifdef EMU_VISUALIZER
if (EmuVisualizerUpdate(0, (unsigned long)block * NAND_PAGES_PER_BLOCK * NAND_SECTORS_PER_PAGE, NAND_PAGES_PER_BLOCK * NAND_SECTORS_PER_PAGE, 1)) { return 0; }
#endif

	writeIndex[block] = 0;	// reset write index when a block is cleared

	return 1;
}

char NandShutdown(void)
{
	// TODO: Free memory, etc?
	return 1;
}
/*
char NandReadParameters(NandParameters_t *nandParameters)
{
    nandParameters->revisionNumber = 0;          // ONFI parameter page offset @4-5
    nandParameters->dataBytesPerPage = 0;        // ONFI parameter page offset @80-83
    nandParameters->spareBytesPerPage = 0;       // ONFI parameter page offset @84-85
    nandParameters->pagesPerBlock = 0;           // ONFI parameter page offset @92-94
    nandParameters->blocksPerLogicalUnit = 0;    // ONFI parameter page offset @96-99
    nandParameters->logicalUnits = 0;            // ONFI parameter page offset @100
    return 0;
}
*/
char NandLoadPageRead(unsigned short block, unsigned char page)
{
	unsigned long offset = ((unsigned long)block * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE));
	unsigned long length = (unsigned long)NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE;
	nandDestBlock = 0xffff;
	nandDestPage = 0xff;
	memcpy(nandBuffer, nandStore + offset, length);

#ifdef DEBUG_BAD
    if (block < 10 && page != 0 && page != NAND_PAGES_PER_BLOCK - 1)
    {
        //printf("WARNING: Unexpected page read of bad-marked blocks during special debugging (%d, %d)\n", block, page);
    }
#endif

#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, ((unsigned long)block * NAND_PAGES_PER_BLOCK * NAND_SECTORS_PER_PAGE) + ((unsigned long)page * NAND_SECTORS_PER_PAGE), NAND_SECTORS_PER_PAGE, 2);
#endif
	return 1;
}

char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
	unsigned long offset = ((unsigned long)srcBlock * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)srcPage * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE));
	unsigned long length = (unsigned long)NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE;
	if ((srcBlock & (NAND_PLANES - 1)) != (destBlock & (NAND_PLANES - 1))) { printf("ERROR: NandLoadPageWrite() - cross-plane operation attempted.\n"); return 0; }


	// Check the page within our block is the correct next ordered write index for the block
	if (writeIndex[destBlock] != destPage)
	{
		if (writeIndex[destBlock] == 1 && destPage == NAND_PAGES_PER_BLOCK - 1)
		{
			printf("NOTE: Not strictly all write-requested in order, but assume bad block marking (%d, %d)->(%d, %d) expected (,%d)\n", srcBlock, srcPage, destBlock, destPage, writeIndex[destBlock]);
		}
		else
		{
			printf("WARNING: Requested next block write index is not in order (%d, %d)->(%d, %d) expected (,%d)\n", srcBlock, srcPage, destBlock, destPage, writeIndex[destBlock]);
		}
	}
//else { printf("<"); }	// ???


#ifdef DEBUG_BAD
    if (srcBlock >= DEBUG_BAD && srcBlock < DEBUG_BAD + 10 && srcPage != 0 && srcPage != NAND_PAGES_PER_BLOCK - 1)
    {
        printf("WARNING: Unexpected page read for write back of bad-marked blocks during special debugging (%d, %d)->(%d, %d)\n", srcBlock, srcPage, destBlock, destPage);
    }
#endif

	nandDestBlock = destBlock;
	nandDestPage = destPage;
	memcpy(nandBuffer, nandStore + offset, length);
#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, ((unsigned long)srcBlock * NAND_PAGES_PER_BLOCK * NAND_SECTORS_PER_PAGE) + ((unsigned long)srcPage * NAND_SECTORS_PER_PAGE), NAND_SECTORS_PER_PAGE, 2);
#endif
	return 1;
}

char NandStorePage(void)
{
	unsigned long offset = ((unsigned long)nandDestBlock * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)nandDestPage * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE));
	unsigned long length = (unsigned long)NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE;

	// Check the page within our block is the correct next ordered write index for the block
	if (writeIndex[nandDestBlock] != nandDestPage)
	{
		if (writeIndex[nandDestBlock] == 1 && nandDestPage == NAND_PAGES_PER_BLOCK - 1)
		{
			printf("NOTE: Not strictly all stored in order, but assume bad block marking (%d, %d) expected (,%d)\n", nandDestBlock, nandDestPage, writeIndex[nandDestBlock]);
		}
		else
		{
			printf("ERROR: Store to write index not in order (%d, %d) expected (,%d)\n", nandDestBlock, nandDestPage, writeIndex[nandDestBlock]);
		}
	}
//else { printf(">"); }	// ???

	// Check we're not trying to set any bits
#if 1
	int errors = 0;
	unsigned long i;
	for (i = 0; i < length; i++)
	{
		unsigned char s = nandBuffer[i];
		unsigned char d = nandStore[offset + i];
		unsigned char b;
		for (b = 8; b != 0; --b)
		{
			if ((s & 1) && !(d & 1)) { errors++; }
			s >>= 1;
			d >>= 1;
		}
	}
	if (errors > 0)
	{
		printf("ERROR: Attempting to set %d bits in block %d, page %d.\n", errors, nandDestBlock, nandDestPage);
	}
#endif

	if (nandDestBlock == 0xffff || nandDestPage == 0xff)
	{
		printf("ERROR: Attempting to store without write.\n");
		return 0;
	}

	if (nandDestBlock >= NAND_PHYSICAL_BLOCKS || nandDestPage >= NAND_PAGES_PER_BLOCK) { printf("ERROR: NandStorePage() - invalid destination.\n"); return 0; }

	memcpy(nandStore + offset, nandBuffer, length);

	// Update the next write index
	writeIndex[nandDestBlock] = nandDestPage + 1;

#ifdef EMU_VISUALIZER
{
    int flags = 0;
    if (nandBuffer[NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + 0] != 0xff || nandBuffer[NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + 5] != 0xff)
    {
        // Bad block marker witten - update visualizer
        flags |= 128;
    }

    if (EmuVisualizerUpdate(0, ((unsigned long)nandDestBlock * NAND_PAGES_PER_BLOCK * NAND_SECTORS_PER_PAGE) + ((unsigned long)nandDestPage * NAND_SECTORS_PER_PAGE), NAND_SECTORS_PER_PAGE, 4 | flags))
    {
	    nandDestBlock = 0xffff;
	    nandDestPage = 0xff;
	    return 0;
    }
}
#endif
	nandDestBlock = 0xffff;
	nandDestPage = 0xff;
	return 1;
}

char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
	if (offset + length >= (unsigned short)NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE) { printf("ERROR: NandWriteBuffer() - invalid address.\n"); return 0; }
	if (nandDestBlock >= NAND_PHYSICAL_BLOCKS || nandDestPage >= NAND_PAGES_PER_BLOCK) { printf("ERROR: NandWriteBuffer() - not open for writing.\n"); return 0; }
	#ifdef NAND_DUMP
	hexdump(buffer, length);
	#endif
	memcpy(nandBuffer + offset, buffer, length);
	return 1;
}

char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length)
{
	if (offset + length >= (unsigned short)NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE) { printf("ERROR: NandReadBuffer() - invalid address.\n"); return 0; }
	if (nandDestBlock < NAND_PHYSICAL_BLOCKS || nandDestPage < NAND_PAGES_PER_BLOCK) { printf("ERROR: NandWriteBuffer() - not open for reading.\n"); return 0; }
	memcpy(buffer, nandBuffer + offset, length);
	#ifdef NAND_DUMP
	hexdump(buffer, length);
	#endif
	return 1;
}

char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
	if (srcBlock >= NAND_PHYSICAL_BLOCKS || srcPage >= NAND_PAGES_PER_BLOCK) { printf("ERROR: NandCopyPage() - invalid source.\n"); return 0; }
	if (destBlock >= NAND_PHYSICAL_BLOCKS || destPage >= NAND_PAGES_PER_BLOCK) { printf("ERROR: NandCopyPage() - invalid destination.\n"); return 0; }
	if ((srcBlock & (NAND_PLANES - 1)) != (destBlock & (NAND_PLANES - 1))) { printf("ERROR: NandCopyPage() - cross-plane copy attempted.\n"); return 0; }
	NandLoadPageWrite(srcBlock, srcPage, destBlock, destPage);
	return NandStorePage();
}

char NandStorePageRepeat(unsigned short block, unsigned char page)
{
	if (block >= NAND_PHYSICAL_BLOCKS || page >= NAND_PAGES_PER_BLOCK) { printf("ERROR: NandStorePageRepeat() - invalid destination.\n"); return 0; }
	nandDestBlock = block;
	nandDestPage = page;
	return NandStorePage();
}

#ifdef _WIN32
// Debug direct read for emulated NAND
void NandDebugRead(unsigned short block, unsigned char page, unsigned short offset, unsigned char *buffer, unsigned short length)
{
	unsigned long nandOffset = ((unsigned long)block * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + offset;
	if (nandStore == NULL)
	{
		memset(buffer, 0xff, length);
		return;
	}
	memcpy(buffer, nandStore + nandOffset, length);
}

#include <math.h>
#include <stdio.h>
void NandDebugCorrupt(unsigned short block, unsigned char page)
{
	unsigned long nandOffset = ((unsigned long)block * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE));
    char sector, i;
    unsigned short bit;

#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, (((unsigned long)block * NAND_PAGES_PER_BLOCK + page) * NAND_SECTORS_PER_PAGE), NAND_SECTORS_PER_PAGE, 32);
#endif

printf("CORRUPTING-PAGE: (%d,%d)\n", block, page);

    // Corrupt one bit in each half of each sector in the page
    for (sector = 0; sector < NAND_SECTORS_PER_PAGE; sector++)
    {
        // Corrupt one bit in the first 256 bytes
        bit = rand() % 2048;
        nandStore[nandOffset + sector * NAND_SECTOR_SIZE + (bit / 8)] ^= (1 << (bit & 0x07));
//        printf("CORRUPTING-DATA: (%d,%d)+%d bit %d\n", block, page, sector);

        // Corrupt one bit in the second 256 bytes
        bit = 2048 + (rand() % 2048);
        nandStore[nandOffset + sector * NAND_SECTOR_SIZE + (bit / 8)] ^= (1 << (bit & 0x07));
//        printf("CORRUPTING-DATA: (%d,%d)+%d bit %d\n", block, page, sector);
    }

    /*
    0	unsigned char badBlock;		
    1	unsigned char blockType;	
    2	unsigned short blockNum;	
    4	unsigned char page;			
    5	unsigned char badBlock2;	
    6	unsigned short blockNum1;	
    8	unsigned char page1;		
    9	unsigned short blockNum2;	
    11	unsigned char page2;		
    */

    // Corrupt two bits from the block type
    for (i = 0; i < 2; i++)
    {
        bit = (rand() % 8);
        nandStore[1 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07));
    }
//    printf("CORRUPTING-META: (%d,%d) blockType=%02x\n", block, page, nandStore[1 + nandOffset + NAND_SPARE_OFFSET]);

    // For each bit in the block number, corrupt one of the three repetition entries
    for (bit = 0; bit < 16; bit++)
    {
        i = (rand() % 3);
        if      (i == 0) { nandStore[2 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 1) { nandStore[6 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 2) { nandStore[9 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
    }
//    printf("CORRUPTING-META: (%d,%d) blockNum=%04x,%04x,%04x,\n", block, page, *(unsigned short *)&nandStore[2 + nandOffset + NAND_SPARE_OFFSET], *(unsigned short *)&nandStore[6 + nandOffset + NAND_SPARE_OFFSET], *(unsigned short *)&nandStore[9 + nandOffset + NAND_SPARE_OFFSET]);

    // For each bit in the page number, corrupt one of the three repetition entries
    for (bit = 0; bit < 8; bit++)
    {
        i = (rand() % 3);
        if      (i == 0) { nandStore[4 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 1) { nandStore[8 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 2) { nandStore[11 + nandOffset + NAND_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
    }
//    printf("CORRUPTING-META: (%d,%d) page=%02x,%02x,%02x,\n", block, page, nandStore[4 + nandOffset + NAND_SPARE_OFFSET], nandStore[8 + nandOffset + NAND_SPARE_OFFSET], nandStore[11 + nandOffset + NAND_SPARE_OFFSET]);

    return;
}

void NandDebugFail(unsigned short block, unsigned char page)
{
	unsigned long nandOffset = ((unsigned long)block * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE));

	// (This relies on visualizer, fix the whole mess!)
#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, (((unsigned long)block * NAND_PAGES_PER_BLOCK + page) * NAND_SECTORS_PER_PAGE), NAND_SECTORS_PER_PAGE, 64);
#endif

printf("FAILING-PAGE: (%d,%d)\n", block, page);

    return;
}

void NandDebugMarkBadBlock(unsigned short block, unsigned char page, int offset, unsigned char value)
{
	unsigned long nandOffset;

    /*
    0	unsigned char badBlock;		
    1	unsigned char blockType;	
    2	unsigned short blockNum;	
    4	unsigned char page;			
    5	unsigned char badBlock2;	
    6	unsigned short blockNum1;	
    8	unsigned char page1;		
    9	unsigned short blockNum2;	
    11	unsigned char page2;		
    */
    nandOffset = ((unsigned long)block * NAND_PAGES_PER_BLOCK * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE + NAND_SPARE_BYTES_PER_PAGE));
printf("BAD-MARKING-BLOCK: (%d,%d+%d=%d)\n", block, page, offset, value);
    nandStore[nandOffset + (NAND_SECTORS_PER_PAGE * NAND_SECTOR_SIZE) + offset] = value;

#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, (((unsigned long)block * NAND_PAGES_PER_BLOCK + page) * NAND_SECTORS_PER_PAGE), NAND_SECTORS_PER_PAGE, 128);
#endif

    return;
}

#endif

#endif
