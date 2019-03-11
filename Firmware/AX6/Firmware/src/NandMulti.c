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

// Multi-NAND Flash control
// Dan Jackson, 2012


// NOTES:
// * To use this, make sure Nand.c is not included in your project, instead add NandMulti.c, NandMulti1.c and NandMulti2.c
// * Two methods are supported:
//   a. Plane-wise: each device's plane is counted as a separate plane, the total number of blocks is doubled.
//   b. Page-wise: each device's blocks/pages are used in parallel, with the multi-interface presenting a page that is twice the size.
// * Please bear in mind the following:
//   a. Plane-wise is simpler to understand and should just work.
//   b. Page-wise is more complicated, and any speed benefit from this technique would only come when the underlying device interface performs the load/store operations in parallel (currently they're performed in serial).
// * In FtlConfig.h, either:
//   a. #define NAND_MULTI_PLANEWISE, double the number of: FTL_PHYSICAL_BLOCKS, FTL_LOGICAL_BLOCKS, FTL_PLANES.
//   b. #define NAND_MULTI_PAGEWISE, add one to: FTL_SECTOR_TO_PAGE_SHIFT, FTL_SECTOR_TO_BLOCK_SHIFT; roughly double FTL_SPARE_BYTES_PER_PAGE (minus one lot of special metadata bytes)
// * IMPORTANT: At the time of writing, both versions are untested.

#include "HardwareProfile.h"

// Include the abstract interface that this file will meet
#include "Peripherals/Nand.h"

#include "FtlConfig.h"          // FTL and multi-NAND must be in agreement of device being emulated
#if !defined(NAND_MULTI_PLANEWISE) && !defined(NAND_MULTI_PAGEWISE)
    #error "FtlConfig.h is not aware of the plane-wise (= 2 * blocks, on additional planes) or page-wise (= 2 * page size, spare bytes split) multi-nand setup."
#endif


// For page-wise multi-NAND, we have to know the page size and number of spare bytes
#if defined(NAND_MULTI_PAGEWISE)
    // The virtual spare bytes offset
    #define NAND_VIRTUAL_SPARE_OFFSET FTL_SPARE_OFFSET
    
    // The physical page size per device (half the virtual page size)
    #define NAND_PAGE_SIZE (NAND_VIRTUAL_SPARE_OFFSET / 2)
    
    // The number of special bytes of the spare area to map to both devices (must contain any manufacturer-placed bad block marker, but in this case contains the whole FTL block metadata)
    #define NAND_SPARE_SPECIAL 12
    
    #define NAND1_SPARE_BYTES (NAND_PAGE_SIZE >> 9)     // (In addition to special bytes) A tweak so that each physical device contains the ECC for each sector on that device
    #define NAND2_SPARE_BYTES ((FTL_SPARE_BYTES_PER_PAGE / 2) - NAND_SPARE_SPECIAL)  // The number of spare bytes in addition to the special bytes
    
    // Regions within the virtual page for page-wise mapping
    #define NAND_PAGE_START   0
    #define NAND_PAGE_PAGE1   NAND_PAGE_SIZE
    #define NAND_PAGE_PAGE2   (NAND_PAGE_PAGE1 + NAND_PAGE_SIZE)
    #define NAND_PAGE_SPECIAL (NAND_PAGE_PAGE2 + NAND_SPARE_SPECIAL)
    #define NAND_PAGE_SPARE1  (NAND_PAGE_SPECIAL + NAND1_SPARE_BYTES)
    #define NAND_PAGE_SPARE2  (NAND_PAGE_SPARE1 + NAND2_SPARE_BYTES)    
#endif

// --- NAND 1 ---
#define nandPresent         nand1Present
#define NandInitialize      Nand1Initialize
#define NandShutdown        Nand1Shutdown
#define NandReadDeviceId    Nand1ReadDeviceId
#define NandVerifyDeviceId  Nand1VerifyDeviceId
#define NandEraseBlock      Nand1EraseBlock
#define NandCopyPage        Nand1CopyPage
#define NandLoadPageRead    Nand1LoadPageRead
#define NandReadBuffer      Nand1ReadBuffer
#define NandLoadPageWrite   Nand1LoadPageWrite
#define NandWriteBuffer     Nand1WriteBuffer
#define NandStorePage       Nand1StorePage
#define NandStorePageRepeat Nand1StorePageRepeat
#define NandReadParameters  Nand1ReadParameters
#define NandReadBuffer512WordSummed	Nand1ReadBuffer512WordSummed

#include "Peripherals/Nand.h"

#undef nandPresent
#undef NandInitialize
#undef NandShutdown
#undef NandReadDeviceId
#undef NandVerifyDeviceId
#undef NandEraseBlock
#undef NandCopyPage
#undef NandLoadPageRead
#undef NandReadBuffer
#undef NandLoadPageWrite
#undef NandWriteBuffer
#undef NandStorePage
#undef NandStorePageRepeat
#undef NandReadParameters
#undef NandReadBuffer512WordSummed
// --------------


// --- NAND 2 ---
#define nandPresent         nand2Present
#define NandInitialize      Nand2Initialize
#define NandShutdown        Nand2Shutdown
#define NandReadDeviceId    Nand2ReadDeviceId
#define NandVerifyDeviceId  Nand2VerifyDeviceId
#define NandEraseBlock      Nand2EraseBlock
#define NandCopyPage        Nand2CopyPage
#define NandLoadPageRead    Nand2LoadPageRead
#define NandReadBuffer      Nand2ReadBuffer
#define NandLoadPageWrite   Nand2LoadPageWrite
#define NandWriteBuffer     Nand2WriteBuffer
#define NandStorePage       Nand2StorePage
#define NandStorePageRepeat Nand2StorePageRepeat
#define NandReadParameters  Nand2ReadParameters
#define nandParameters		nand2Parameters	//KL
#define NandReadBuffer512WordSummed	Nand2ReadBuffer512WordSummed

#include "Peripherals/Nand.h"

#undef nandPresent
#undef NandInitialize
#undef NandShutdown
#undef NandReadDeviceId
#undef NandVerifyDeviceId
#undef NandEraseBlock
#undef NandCopyPage
#undef NandLoadPageRead
#undef NandReadBuffer
#undef NandLoadPageWrite
#undef NandWriteBuffer
#undef NandStorePage
#undef NandStorePageRepeat
#undef NandReadParameters
#undef nandParameters	//KL
#undef NandReadBuffer512WordSummed
// --------------

// Globals..
char nandPresent;       // Both NANDs present (call NandVerifyDeviceId() once to set this)

// Initialize the NAND devices
char NandInitialize(void)
{
    char ret;
    ret =  Nand1Initialize();
    ret &= Nand2Initialize();
    return ret;
}

// Shutdown the NAND device
char NandShutdown(void)
{
    char ret;
    ret =  Nand1Shutdown();
    ret &= Nand2Shutdown();
    return ret;
}

// Read the device id (6 bytes)
char NandReadDevicedId(unsigned char* destination)
{
    int i;
    char ret;
    unsigned char deviceId1[6] = {0};
    unsigned char deviceId2[6] = {0};
    ret =  Nand1ReadDeviceId(deviceId1);
    ret &= Nand2ReadDeviceId(deviceId2);
    for (i = 0; i < 6; i++)
    {
        if (deviceId1[i] == deviceId2[i]) { destination[i] = deviceId1[i]; }
        else { destination[i] = 0x00; ret = 0; }
    }
    return ret;
}

// Verify the device id
unsigned char NandVerifyDeviceId(void)
{
    char ret;
    ret =  Nand1VerifyDeviceId();
    ret &= Nand2VerifyDeviceId();
    nandPresent = nand1Present && nand2Present;
    return ret;
}


// NAND_MULTI_PLANEWISE - treating each NAND device as additional planes, where the LSB of the block identifies the physical device (and the next LSB identifies the actual planes)
#if defined(NAND_MULTI_PLANEWISE)
// Remember the last loaded block so that NandStorePage() and NandStorePageRepeat() map to the correct physical device
static unsigned short nandLoadedBlock = 0;

// PLANEWISE: Erase a block
char NandEraseBlock(unsigned short block)
{
    if (!(block & 1)) { return Nand1EraseBlock(block >> 1); }
    else              { return Nand2EraseBlock(block >> 1); }
}

// PLANEWISE: Copy a page
char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
    if (!(srcBlock & 1)) { return Nand1CopyPage(srcBlock >> 1, srcPage, destBlock >> 1, destPage); }
    else                 { return Nand2CopyPage(srcBlock >> 1, srcPage, destBlock >> 1, destPage); }
}

// PLANEWISE: Load a page in to the buffer for reading
char NandLoadPageRead(unsigned short block, unsigned char page)
{
    // Remember the block the last page read was from so that NandReadBuffer() maps to the correct physical device
    nandLoadedBlock = block;
    if (!(block & 1)) { return Nand1LoadPageRead(block >> 1, page); }
    else              { return Nand2LoadPageRead(block >> 1, page); }
}

// PLANEWISE: Read in from the page buffer
char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length)
{
    if (!(nandLoadedBlock & 1)) { return Nand1ReadBuffer(offset, buffer, length); }
    else                        { return Nand2ReadBuffer(offset, buffer, length); }
}

// PLANEWISE: Load a page in to the buffer for writing to the specified location
char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
    // Remember the block the last page write was for so that NandWriteBuffer(), NandStorePage() and NandStorePageRepeat() map to the correct physical device
    nandLoadedBlock = srcBlock;
    if (!(srcBlock & 1)) { return Nand1LoadPageWrite(srcBlock >> 1, srcPage, destBlock >> 1, destPage); }
    else                 { return Nand2LoadPageWrite(srcBlock >> 1, srcPage, destBlock >> 1, destPage); }
}

// PLANEWISE: Write in to the page buffer
char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
    if (!(nandLoadedBlock & 1)) { return Nand1WriteBuffer(offset, buffer, length); }
    else                        { return Nand2WriteBuffer(offset, buffer, length); }
}

// PLANEWISE: Commit the loaded page buffer
char NandStorePage(void)
{
// KL: Detect which device write faults    
#ifdef __DEBUG
    char retval;
    if (!(nandLoadedBlock & 1)) { retval = Nand1StorePage(); }
    else                        { retval = Nand2StorePage(); }  
    if(retval != TRUE)
    {
        Nop();
    }
    return retval;
#else   
    if (!(nandLoadedBlock & 1)) { return Nand1StorePage(); }
    else                        { return Nand2StorePage(); }
#endif
}

// PLANEWISE: Commit the loaded page buffer
char NandStorePageRepeat(unsigned short block, unsigned char page)
{
    if (!(nandLoadedBlock & 1)) { return Nand1StorePageRepeat(block >> 1, page); }
    else                        { return Nand2StorePageRepeat(block >> 1, page); }
}

#ifdef NAND_READ_SECTOR_WORD_SUMMED
// Read in from the page buffer 512 bytes to a word-aligned buffer, summing the word-wise values
char NandReadBuffer512WordSummed(unsigned short offset, unsigned short *wordAlignedBuffer, unsigned short *outSum)
{
    if (!(nandLoadedBlock & 1)) { return Nand1ReadBuffer512WordSummed(offset, wordAlignedBuffer, outSum); }
    else                        { return Nand2ReadBuffer512WordSummed(offset, wordAlignedBuffer, outSum); }
}

#endif



// NAND_MULTI_PAGEWISE - treating each NAND device as increasing the page size
#if defined(NAND_MULTI_PAGEWISE)
// PAGEWISE: Erase a block
char NandEraseBlock(unsigned short block)
{
    char ret;
    ret =  Nand1EraseBlock(block);
    ret &= Nand2EraseBlock(block);
    return ret;
}

// PAGEWISE: Copy a page
char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
    char ret;
    ret =  Nand1CopyPage(srcBlock, srcPage, destBlock, destPage);
    ret &= Nand2CopyPage(srcBlock, srcPage, destBlock, destPage);
    return ret;
}

// PAGEWISE: Load a page in to the buffer for reading
char NandLoadPageRead(unsigned short block, unsigned char page)
{
    char ret;
    ret =  Nand1LoadPageRead(block, page);
    ret &= Nand2LoadPageRead(block, page);
    return ret;
}

// PAGEWISE: Read in from the page buffer
char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length)
{
    char ret = 1;
    
    // If the read contains some data from the physical page on device 1...
    if (offset >= NAND_PAGE_START && offset < NAND_PAGE_PAGE1)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_PAGE1 - offset) { len = NAND_PAGE_PAGE1 - offset; }     // clamp within page 1
        ret &= Nand1ReadBuffer(offset - NAND_PAGE_START, buffer, len);              // read device 1
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }
    
    // If the read contains some data from the physical page on device 2...
    if (offset >= NAND_PAGE_PAGE1 && offset < NAND_PAGE_PAGE2)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_PAGE2 - offset) { len = NAND_PAGE_PAGE2 - offset; }     // clamp within page 2
        ret &= Nand2ReadBuffer(offset - NAND_PAGE_PAGE1, buffer, len);              // read device 2
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }

    // If the read contains some data from the special bytes of the spare area...
    if (offset >= NAND_PAGE_PAGE2 && offset < NAND_PAGE_SPECIAL)
    {
        unsigned short len = length;
        unsigned char buffer1[NAND_SPARE_SPECIAL];
        unsigned char buffer2[NAND_SPARE_SPECIAL];
        int i;
        if (len > NAND_PAGE_SPECIAL - offset) { len = NAND_PAGE_SPECIAL - offset; } // clamp within special area
        ret &= Nand1ReadBuffer(offset - NAND_PAGE_PAGE2, buffer1, len);             // read special area from device 1
        ret &= Nand2ReadBuffer(offset - NAND_PAGE_PAGE2, buffer2, len);             // read special area from device 2
        // Set output bytes
        for (i = 0; i < len; i++)
        {
            buffer[i] = (buffer1[i] & buffer2[i]);      // AND so that a bad block markers on either device will be seen
        }
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }

    // If the read contains some data from the spare bytes for the physical page on device 1...
    if (offset >= NAND_PAGE_SPECIAL && offset < NAND_PAGE_SPARE1)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_SPARE1 - offset) { len = NAND_PAGE_SPARE1 - offset; }   // clamp within spare area for page 1
        ret &= Nand1ReadBuffer(NAND_SPARE_SPECIAL + offset - NAND_PAGE_SPECIAL, buffer, len); // read non-special spare bytes from device 1
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }

    // If the read contains some data from the spare bytes for the physical page on device 2...
    if (offset >= NAND_PAGE_SPARE1 && offset < NAND_PAGE_SPARE2)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_SPARE2 - offset) { len = NAND_PAGE_SPARE2 - offset; }   // clamp within spare area for page 2
        ret &= Nand2ReadBuffer(NAND_SPARE_SPECIAL + offset - NAND_PAGE_SPARE1, buffer, len); // read non-special spare bytes from device 2
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }
    
    // NOTE: Shouldn't get here unless the read contains bytes outside the virtual page area
    
    return ret;
}


// PAGEWISE: Load a page in to the buffer for writing to the specified location
char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage)
{
    char ret;
    ret =  Nand1LoadPageWrite(srcBlock, srcPage, destBlock, destPage);
    ret &= Nand2LoadPageWrite(srcblock, srcPage, destBlock, destPage);
    return ret;
}

// PAGEWISE: Write in to the page buffer
char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length)
{
    char ret = 1;
    
    // If the write contains some data from the physical page on device 1...
    if (offset >= NAND_PAGE_START && offset < NAND_PAGE_PAGE1)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_PAGE1 - offset) { len = NAND_PAGE_PAGE1 - offset; }     // clamp within page 1
        ret &= Nand1WriteBuffer(offset - NAND_PAGE_START, buffer, len);             // write device 1
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }
    
    // If the write contains some data from the physical page on device 2...
    if (offset >= NAND_PAGE_PAGE1 && offset < NAND_PAGE_PAGE2)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_PAGE2 - offset) { len = NAND_PAGE_PAGE2 - offset; }     // clamp within page 2
        ret &= Nand2WriteBuffer(offset - NAND_PAGE_PAGE1, buffer, len);             // write device 2
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }

    // If the write contains some data from the special bytes of the spare area...
    if (offset >= NAND_PAGE_PAGE2 && offset < NAND_PAGE_SPECIAL)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_SPECIAL - offset) { len = NAND_PAGE_SPECIAL - offset; } // clamp within special area
        ret &= Nand1WriteBuffer(offset - NAND_PAGE_PAGE2, buffer1, len);            // write special area from device 1
        ret &= Nand2WriteBuffer(offset - NAND_PAGE_PAGE2, buffer2, len);            // write special area from device 2
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }

    // If the write contains some data from the spare bytes for the physical page on device 1...
    if (offset >= NAND_PAGE_SPECIAL && offset < NAND_PAGE_SPARE1)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_SPARE1 - offset) { len = NAND_PAGE_SPARE1 - offset; }   // clamp within spare area for page 1
        ret &= Nand1WriteBuffer(NAND_SPARE_SPECIAL + offset - NAND_PAGE_SPECIAL, buffer, len); // write non-special spare bytes from device 1
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }

    // If the write contains some data from the spare bytes for the physical page on device 2...
    if (offset >= NAND_PAGE_SPARE1 && offset < NAND_PAGE_SPARE2)
    {
        unsigned short len = length;
        if (len > NAND_PAGE_SPARE2 - offset) { len = NAND_PAGE_SPARE2 - offset; }   // clamp within spare area for page 2
        ret &= Nand2WriteBuffer(NAND_SPARE_SPECIAL + offset - NAND_PAGE_SPARE1, buffer, len); // write non-special spare bytes from device 2
        length -= len;
        if (length == 0) { return ret; }                                            // early out
        offset += len;
        buffer += len;
    }
    
    // NOTE: Shouldn't get here unless the write contains bytes outside the virtual page area
    
    return ret;
}

// PAGEWISE: Commit the loaded page buffer
char NandStorePage(void)
{
    char ret;
    ret =  Nand1StorePage();
    ret &= Nand2StorePage();
    return ret;
}

// PAGEWISE: Commit the loaded page buffer
char NandStorePageRepeat(unsigned short block, unsigned char page)
{
    char ret;
    ret =  Nand1StorePageRepeat(block, page);
    ret &= Nand2StorePageRepeat(block, page);
    return ret;
}
#endif

// Read device parameters
char NandReadParameters(NandParameters_t *nandParameters)
{
    char ret;
    NandParameters_t param1 = {0};
    NandParameters_t param2 = {0};
    ret =  Nand1ReadParameters(&param1);
    ret &= Nand2ReadParameters(&param2);

    // Check for mismatches
    if (param1.dataBytesPerPage     != param2.dataBytesPerPage    ) { return 0; }
    if (param1.spareBytesPerPage    != param2.spareBytesPerPage   ) { return 0; }
    if (param1.pagesPerBlock        != param2.pagesPerBlock       ) { return 0; }
    if (param1.blocksPerLogicalUnit != param2.blocksPerLogicalUnit) { return 0; }
    if (param1.logicalUnits         != param2.logicalUnits        ) { return 0; }
    
#if defined(NAND_MULTI_PLANEWISE)
    // Set output values for plane-wise multple-NANDs (= 2 * blocks, on additional planes)
    nandParameters->revisionNumber       = param1.revisionNumber;
    nandParameters->dataBytesPerPage     = param1.dataBytesPerPage;
    nandParameters->spareBytesPerPage    = param1.spareBytesPerPage;
    nandParameters->pagesPerBlock        = param1.pagesPerBlock;
    nandParameters->blocksPerLogicalUnit = param1.blocksPerLogicalUnit;                 // same number of blocks per logical unit...
    nandParameters->logicalUnits         = param1.logicalUnits + param2.logicalUnits;   // ...but double the number of logical units
#elif defined(NAND_MULTI_PAGEWISE)
    // Set output values for page-wise multple-NAND (= 2 * page size)
    nandParameters->revisionNumber       = param1.revisionNumber;
    nandParameters->dataBytesPerPage     = param1.dataBytesPerPage + param2.dataBytesPerPage;
    nandParameters->spareBytesPerPage    = param1.spareBytesPerPage + param2.spareBytesPerPage;
    nandParameters->pagesPerBlock        = param1.pagesPerBlock;
    nandParameters->blocksPerLogicalUnit = param1.blocksPerLogicalUnit;
    nandParameters->logicalUnits         = param1.logicalUnits;
#else
    #error "NAND_MULTI type not defined"
#endif
        
    return ret;
}

#endif
