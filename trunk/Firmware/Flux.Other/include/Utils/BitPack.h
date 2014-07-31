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

// Bit Packer
// Dan Jackson, 2012

#ifndef BITPACK_H
#define BITPACK_H



// Bit-pack 4x 10-bit samples into 5-byte groups (stored little-endian):
// AAAAAAAA BBBBBBAA CCCCBBBB DDCCCCCC DDDDDDDD
// 76543210 54321098 32109876 10987654 98765432

// Number of bytes required to pack 'n' 10-bit values:  size = ((n / 4) * 5); if ((n % 4) != 0) size += (n % 4) + 1;
#define BITPACK10_SIZEOF(_n) ((((_n) / 4) * 5) + ((((_n) & 0x03) == 0) ? 0 : (((_n) & 0x03) + 1)))

#ifndef BITPACK_H_NO_FUNCTIONS

// Pack 4x 10-bit samples into 5-byte groups (stored little-endian).
// IMPORTANT: Samples must be written sequentially (otherwise destination values will be overwritten or combined incorrectly).
static void BitPack_uint10(void *buffer, unsigned short index, unsigned short value)
{
    unsigned char *p = (unsigned char *)buffer + ((index >> 2) * 5);
    switch (index & 0x03)
    {
        case 0: p[0]  = (unsigned char)(value     ); p[1] = (unsigned char)((value >> 8) & 0x0003); break;  // A
        case 1: p[1] |= (unsigned char)(value << 2); p[2] = (unsigned char)((value >> 6) & 0x000f); break;  // B
        case 2: p[2] |= (unsigned char)(value << 4); p[3] = (unsigned char)((value >> 4) & 0x003f); break;  // C
        case 3: p[3] |= (unsigned char)(value << 6); p[4] = (unsigned char)((value >> 2)         ); break;  // D
    }
    return;
}


// Un-pack 4x 10-bit samples from each 5-byte group (stored little-endian).
static unsigned short BitUnpack_uint10(void *buffer, unsigned short index)
{
	unsigned short value;
    unsigned char *p = (unsigned char *)buffer + ((index >> 2) * 5);
    switch (index & 0x03)
    {
        case 0: return ((unsigned short)p[0]     ) | (((unsigned short)p[1] & 0x0003) << 8);    // A
        case 1: return ((unsigned short)p[1] >> 2) | (((unsigned short)p[2] & 0x000f) << 6);    // B
        case 2: return ((unsigned short)p[2] >> 4) | (((unsigned short)p[3] & 0x003f) << 4);    // C
        case 3: return ((unsigned short)p[3] >> 6) | (((unsigned short)p[4]         ) << 2);    // D
    }
}


/*
// 2x 12-bit samples into 3-byte groups, stored little-endian:
// AAAAAAAA BBBBAAAA BBBBBBBB
// 76543210 3210ba98 ba987654
// IMPORTANT: Samples must be written sequentially (otherwise destination values will be overwritten or combined incorrectly).
static void BitPack_uint12(void *buffer, unsigned short index, unsigned short value)
{
    unsigned char *p = (unsigned char *)buffer + ((index >> 1) * 3);
    switch (index & 0x01)
    {
        case 0: p[0]  = (unsigned char)(value     ); p[1] = (unsigned char)((value >> 8) & 0x000f); break;  // A
        case 1: p[1] |= (unsigned char)(value << 4); p[2] = (unsigned char)((value >> 4)         ); break;  // B
    }
    return;
}
*/

#endif
#endif
