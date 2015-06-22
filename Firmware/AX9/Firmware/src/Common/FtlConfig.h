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

// FTL Configuration
// Dan Jackson, 2011

#define FTL_PHYSICAL_BLOCKS 4096					// 4096 x 128 kB blocks (512 MB)
#define FTL_LOGICAL_BLOCKS 3876						// Reserving 220 (~5%) of blocks for log and bad blocks
#define FTL_PLANES 2								// Number of 'planes' on the device
#define FTL_FIRST_SECTOR (2 + 92)					// Align first sector on disk so our default format makes a page-aligned file-system with first data cluster beginning exactly at block 1

#define FTL_SECTOR_SIZE 512							// Sectors are always 512 bytes

#define FTL_SECTOR_TO_PAGE_SHIFT 2					// 4 sectors per 2 kB page
#define FTL_SECTORS_PER_PAGE (1<<FTL_SECTOR_TO_PAGE_SHIFT)	// 4 sectors per 2 kB page
#define FTL_PAGES_PER_BLOCK 64						// 64 2 kB pages per 128 kB block
#define FTL_SECTOR_TO_BLOCK_SHIFT 8					// Shift right by 8 (/256) to convert an absolute sector into a block address

#define FTL_SPARE_OFFSET (FTL_SECTORS_PER_PAGE * FTL_SECTOR_SIZE)	// Offset of spare bytes within the page
#define FTL_SPARE_BYTES_PER_PAGE 64					// 64 spare bytes per page

