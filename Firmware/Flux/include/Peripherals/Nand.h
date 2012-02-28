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

// NAND Flash abstract interface
// Dan Jackson, 2011-2012
// Karim Ladha 28-11-2012 : Added support for ONFI read id function (typedef structure now internal to nand.c)

// Globals..
extern char nandPresent;       // NAND present (call NandVerifyDeviceId() once to set this)

// Initialize the NAND device
char NandInitialize(void);

// Shutdown the NAND device
char NandShutdown(void);

// Read the device id (6 bytes)
extern const unsigned char NAND_DEVICE_HY27UF084G2B[6];
extern const unsigned char NAND_DEVICE_MT29F8G08AAA[6];
char NandReadId(unsigned char* destination);

// Verify the device id
unsigned char NandVerifyDeviceId(void);

// Erase a block
char NandEraseBlock(unsigned short block);

// Copy a page
char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage);

// Load a page in to the buffer for reading
char NandLoadPageRead(unsigned short block, unsigned char page);

// Read in from the page buffer
char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length);

// Load a page in to the buffer for writing to the specified location
char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage);

// Write in to the page buffer
char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length);

// Commit the loaded page buffer
char NandStorePage(void);

// Commit the loaded page buffer
char NandStorePageRepeat(unsigned short block, unsigned char page);

// Read device parameters
typedef struct
{
    unsigned short revisionNumber;          // ONFI parameter page offset @4-5
    unsigned long  dataBytesPerPage;        // ONFI parameter page offset @80-83
    unsigned short spareBytesPerPage;       // ONFI parameter page offset @84-85
    unsigned long  pagesPerBlock;           // ONFI parameter page offset @92-94
    unsigned long  blocksPerLogicalUnit;    // ONFI parameter page offset @96-99
    unsigned char  logicalUnits;            // ONFI parameter page offset @100
} NandParameters;
char NandReadParameters(NandParameters *nandParameters);

// Debug functions for emulated NAND
#ifdef _WIN32
void NandDebugRead(unsigned short block, unsigned char page, unsigned short offset, unsigned char *buffer, unsigned short length);
void NandDebugCorrupt(unsigned short block, unsigned char page);
void NandDebugFail(unsigned short block, unsigned char page);
#endif
