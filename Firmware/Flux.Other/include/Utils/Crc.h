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

#ifndef CRC_H
#define CRC_H

// Polynomials (top poly bit must not be set)
#define CRC_6_ITU_BITS 6			//
#define CRC_6_ITU_POLY 0x03			// CRC-6-ITU (x^6 + x + 1) = (1)000011
#define CRC_6_ITU_INITIAL 0

#define CRC_16_CCITT_BITS 16
#define CRC_16_CCITT_POLY 0x1021	// (1)0001000000100001
#define CRC_16_CCITT_INITIAL 0xffff


// Maximum CRC size
typedef unsigned short crc_t;

// Calculate the CCITT 16-bit CRC on a byte-aligned message
unsigned short CrcCCITT_16bit_Calculate(const unsigned char *buffer, unsigned short len, unsigned short crcInitVal);

// Calculate the ITU 6-bit CRC on a 10-bit message (MSB first)
unsigned short Crc6ITU_10bit_Calculate(unsigned short message);

// (Slow, generic) Calculate the CRC of a message bits (starting from the MSB of each byte, poly must not have top bit set)
crc_t CrcCalculateSlow(const void *message, int messagebits, crc_t poly, char polybits, crc_t initialCrc);

#endif
