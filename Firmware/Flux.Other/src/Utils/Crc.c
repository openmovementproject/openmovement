/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// Simple, low-speed CRC calculation
// Dan Jackson, 2012

#include <stdlib.h>
#include "Utils/Crc.h"



// Calculate the CCITT 16-bit CRC on a byte-aligned message
unsigned short CrcCCITT_16bit_Calculate(const unsigned char *buffer, unsigned short len, unsigned short crc)
{
	int i;
	while (len--)
	{
		crc ^= (unsigned short)*buffer++ << 8;
		for (i = 0; i < 8; i++) { crc = crc & 0x8000 ? (crc << 1) ^ CRC_16_CCITT_POLY : (crc << 1); }
	}
	return crc;
}


// Calculate the ITU 6-bit CRC on a 10-bit message (MSB first)
unsigned short Crc6ITU_10bit_Calculate(unsigned short message)
{
    int i;
    unsigned short result = 0;

    // 10-bit message
    for (i = (10 - 1); i >= 0; i--)
    {
        // Read the incoming message bit
        char invert = (message >> i) & 1;

        // XOR with top bit of the CRC to see if we should invert
        invert ^= ((result >> (CRC_6_ITU_BITS - 1)) & 0x01);

        // Shift and mask result
        result <<= 1;
        result &= ((1 << CRC_6_ITU_BITS) - 1);

        // If inverting, toggle poly bits
        if (invert) result ^= CRC_6_ITU_POLY;       // 0x03
    }

    return result;
}



// (Slow, generic) Calculate the CRC of a message bits (starting from the MSB of each byte, poly must not have top bit set)
crc_t CrcCalculateSlow(const void *message, int messagebits, crc_t poly, char polybits, crc_t initialCrc)
{
	int i;
	crc_t result = initialCrc;

	// Mask off to ensure top poly bit clear ('polybit' bit position is always treated as '1')
	poly &= ((1 << (polybits - 1)) - 1);

	for (i = 0; i < messagebits; i++)
	{
		// Read the incoming message bit
		char invert = ((const unsigned char *)message)[i >> 3] & (0x80 >> (i & 0x07)) ? 1 : 0;

		// XOR with top bit of the CRC to see if we should invert
		invert ^= ((result >> (polybits - 1)) & 0x01);

		// Shift and mask result
		result <<= 1;
		result &= ((1 << polybits) - 1);

		// If inverting, toggle poly bits
		if (invert) result ^= poly;
	}

	return result;
}
