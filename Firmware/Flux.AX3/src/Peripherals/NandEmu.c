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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Peripherals/Nand.h"
#include "FtlConfig.h"			// Emulation relies on the FTL configuration


#define EMU_VISUALIZER
#ifdef EMU_VISUALIZER
extern char EmuVisualizerUpdate(char logical, unsigned long sector, unsigned int count, char flags);
#endif

#include <assert.h>

static unsigned char *nandBuffer = NULL;
static unsigned char *nandStore = NULL;
static unsigned short nandDestBlock = 0xffff;
static unsigned char nandDestPage = 0xff;


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
		unsigned long len = (unsigned long)FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE; 
		nandBuffer = (unsigned char *)malloc(len);
		memset(nandBuffer, 0xff, len);
	}
	if (nandStore == NULL)
	{
		unsigned long len = (unsigned long)FTL_PHYSICAL_BLOCKS * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE);
		nandStore = (unsigned char *)malloc(len);
		memset(nandStore, 0xff, len);
	}
	assert(nandBuffer != NULL && nandStore != NULL );
	return 1;
}

char NandEraseBlock(unsigned short block)
{
	unsigned long offset = (unsigned long)block * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE);
	unsigned long length = (unsigned long)FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE);
	if (block >= FTL_PHYSICAL_BLOCKS) { printf("ERROR: NandEraseBlock() - invalid block.\n"); return 0; }
	memset(nandStore + offset, 0xff, length);
#ifdef EMU_VISUALIZER
if (EmuVisualizerUpdate(0, (unsigned long)block * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE, 1)) { return 0; }
#endif
	return 1;
}

char NandShutdown(void)
{
	// TODO: Free memory, etc?
	return 1;
}

char NandReadParameters(NandParameters *nandParameters)
{
    nandParameters->revisionNumber = 0;          // ONFI parameter page offset @4-5
    nandParameters->dataBytesPerPage = 0;        // ONFI parameter page offset @80-83
    nandParameters->spareBytesPerPage = 0;       // ONFI parameter page offset @84-85
    nandParameters->pagesPerBlock = 0;           // ONFI parameter page offset @92-94
    nandParameters->blocksPerLogicalUnit = 0;    // ONFI parameter page offset @96-99
    nandParameters->logicalUnits = 0;            // ONFI parameter page offset @100
    return 0;
}

char NandLoadPageRead(unsigned short block, unsigned char page)
{
	unsigned long offset = ((unsigned long)block * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE));
	unsigned long length = (unsigned long)FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE;
	nandDestBlock = 0xffff;
	nandDestPage = 0xff;
	memcpy(nandBuffer, nandStore + offset, length);
#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, ((unsigned long)block * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE) + ((unsigned long)page * FTL_SECTORS_PER_PAGE), FTL_SECTORS_PER_PAGE, 2);
#endif
	return 1;
}

char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
	unsigned long offset = ((unsigned long)srcBlock * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + ((unsigned long)srcPage * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE));
	unsigned long length = (unsigned long)FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE;
	if ((srcBlock & (FTL_PLANES - 1)) != (destBlock & (FTL_PLANES - 1))) { printf("ERROR: NandLoadPageWrite() - cross-plane operation attempted.\n"); return 0; }
	nandDestBlock = destBlock;
	nandDestPage = destPage;
	memcpy(nandBuffer, nandStore + offset, length);
#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, ((unsigned long)srcBlock * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE) + ((unsigned long)srcPage * FTL_SECTORS_PER_PAGE), FTL_SECTORS_PER_PAGE, 2);
#endif
	return 1;
}

char NandStorePage(void)
{
	unsigned long offset = ((unsigned long)nandDestBlock * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + ((unsigned long)nandDestPage * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE));
	unsigned long length = (unsigned long)FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE;

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

	if (nandDestBlock >= FTL_PHYSICAL_BLOCKS || nandDestPage >= FTL_PAGES_PER_BLOCK) { printf("ERROR: NandStorePage() - invalid destination.\n"); return 0; }

	memcpy(nandStore + offset, nandBuffer, length);
#ifdef EMU_VISUALIZER
if (EmuVisualizerUpdate(0, ((unsigned long)nandDestBlock * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE) + ((unsigned long)nandDestPage * FTL_SECTORS_PER_PAGE), FTL_SECTORS_PER_PAGE, 4))
{
	nandDestBlock = 0xffff;
	nandDestPage = 0xff;
	return 0;
}
#endif
	nandDestBlock = 0xffff;
	nandDestPage = 0xff;
	return 1;
}

char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
	if (offset + length >= (unsigned short)FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE) { printf("ERROR: NandWriteBuffer() - invalid address.\n"); return 0; }
	if (nandDestBlock >= FTL_PHYSICAL_BLOCKS || nandDestPage >= FTL_PAGES_PER_BLOCK) { printf("ERROR: NandWriteBuffer() - not open for writing.\n"); return 0; }
	#ifdef NAND_DUMP
	hexdump(buffer, length);
	#endif
	memcpy(nandBuffer + offset, buffer, length);
	return 1;
}

char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length)
{
	if (offset + length >= (unsigned short)FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE) { printf("ERROR: NandReadBuffer() - invalid address.\n"); return 0; }
	if (nandDestBlock < FTL_PHYSICAL_BLOCKS || nandDestPage < FTL_PAGES_PER_BLOCK) { printf("ERROR: NandWriteBuffer() - not open for reading.\n"); return 0; }
	memcpy(buffer, nandBuffer + offset, length);
	#ifdef NAND_DUMP
	hexdump(buffer, length);
	#endif
	return 1;
}

char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
	if (srcBlock >= FTL_PHYSICAL_BLOCKS || srcPage >= FTL_PAGES_PER_BLOCK) { printf("ERROR: NandCopyPage() - invalid source.\n"); return 0; }
	if (destBlock >= FTL_PHYSICAL_BLOCKS || destPage >= FTL_PAGES_PER_BLOCK) { printf("ERROR: NandCopyPage() - invalid destination.\n"); return 0; }
	if ((srcBlock & (FTL_PLANES - 1)) != (destBlock & (FTL_PLANES - 1))) { printf("ERROR: NandCopyPage() - cross-plane copy attempted.\n"); return 0; }
	NandLoadPageWrite(srcBlock, srcPage, destBlock, destPage);
	return NandStorePage();
}

char NandStorePageRepeat(unsigned short block, unsigned char page)
{
	if (block >= FTL_PHYSICAL_BLOCKS || page >= FTL_PAGES_PER_BLOCK) { printf("ERROR: NandStorePageRepeat() - invalid destination.\n"); return 0; }
	nandDestBlock = block;
	nandDestPage = page;
	return NandStorePage();
}

#ifdef _WIN32
// Debug direct read for emulated NAND
void NandDebugRead(unsigned short block, unsigned char page, unsigned short offset, unsigned char *buffer, unsigned short length)
{
	unsigned long nandOffset = ((unsigned long)block * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + offset;
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
	unsigned long nandOffset = ((unsigned long)block * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE));
    char sector, i;
    unsigned short bit;

#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, (((unsigned long)block * FTL_PAGES_PER_BLOCK + page) * FTL_SECTORS_PER_PAGE), FTL_SECTORS_PER_PAGE, 32);
#endif

printf("CORRUPTING-PAGE: (%d,%d)\n", block, page);

    // Corrupt one bit in each half of each sector in the page
    for (sector = 0; sector < FTL_SECTORS_PER_PAGE; sector++)
    {
        // Corrupt one bit in the first 256 bytes
        bit = rand() % 2048;
        nandStore[nandOffset + sector * FTL_SECTOR_SIZE + (bit / 8)] ^= (1 << (bit & 0x07));
//        printf("CORRUPTING-DATA: (%d,%d)+%d bit %d\n", block, page, sector);

        // Corrupt one bit in the second 256 bytes
        bit = 2048 + (rand() % 2048);
        nandStore[nandOffset + sector * FTL_SECTOR_SIZE + (bit / 8)] ^= (1 << (bit & 0x07));
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
        nandStore[1 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07));
    }
//    printf("CORRUPTING-META: (%d,%d) blockType=%02x\n", block, page, nandStore[1 + nandOffset + FTL_SPARE_OFFSET]);

    // For each bit in the block number, corrupt one of the three repetition entries
    for (bit = 0; bit < 16; bit++)
    {
        i = (rand() % 3);
        if      (i == 0) { nandStore[2 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 1) { nandStore[6 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 2) { nandStore[9 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
    }
//    printf("CORRUPTING-META: (%d,%d) blockNum=%04x,%04x,%04x,\n", block, page, *(unsigned short *)&nandStore[2 + nandOffset + FTL_SPARE_OFFSET], *(unsigned short *)&nandStore[6 + nandOffset + FTL_SPARE_OFFSET], *(unsigned short *)&nandStore[9 + nandOffset + FTL_SPARE_OFFSET]);

    // For each bit in the page number, corrupt one of the three repetition entries
    for (bit = 0; bit < 8; bit++)
    {
        i = (rand() % 3);
        if      (i == 0) { nandStore[4 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 1) { nandStore[8 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
        else if (i == 2) { nandStore[11 + nandOffset + FTL_SPARE_OFFSET + (bit / 8)] ^= (1 << (bit & 0x07)); }
    }
//    printf("CORRUPTING-META: (%d,%d) page=%02x,%02x,%02x,\n", block, page, nandStore[4 + nandOffset + FTL_SPARE_OFFSET], nandStore[8 + nandOffset + FTL_SPARE_OFFSET], nandStore[11 + nandOffset + FTL_SPARE_OFFSET]);

    return;
}

void NandDebugFail(unsigned short block, unsigned char page)
{
	unsigned long nandOffset = ((unsigned long)block * FTL_PAGES_PER_BLOCK * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE)) + ((unsigned long)page * (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE + FTL_SPARE_BYTES_PER_PAGE));

	// (This relies on visualizer, fix the whole mess!)
#ifdef EMU_VISUALIZER
EmuVisualizerUpdate(0, (((unsigned long)block * FTL_PAGES_PER_BLOCK + page) * FTL_SECTORS_PER_PAGE), FTL_SECTORS_PER_PAGE, 64);
#endif

printf("FAILING-PAGE: (%d,%d)\n", block, page);

    return;
}

#endif

