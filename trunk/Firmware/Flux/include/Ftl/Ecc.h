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

// Error Correcting Code (ECC)
// Dan Jackson, 2011

#ifndef ECC_H
#define ECC_H


// Debug ECC
//#define ECC_DEBUG



// Hamming ECC code size for 256 byte buffer
#define ECC_SIZE_256 3

// Generate 256 byte Hamming ECC
void EccGenerate256(const unsigned char *data, unsigned char *ecc);

// Checks 256 byte Hamming ECC and corrects the data (1-bit), or detects failure (2-bit)
// (0 = ok, 1 = repaired-data, 2 = repaired-ecc, -1 = unrecoverable)
char EccCheck256(unsigned char *data, unsigned char *storedEcc);


// Hamming ECC code size for 512 byte buffer
#define ECC_SIZE_512 (ECC_SIZE_256 + ECC_SIZE_256)

// Generate 512 byte Hamming ECC
void EccGenerate512(const unsigned char *data, unsigned char *ecc);

// Checks 512 byte Hamming ECC corrects the data (1-bit per 256 bytes), or detects failure (2-bits per 256 bytes)
// (0 = ok, 1 = repaired-data, 2 = repaired-ecc, 3 = repaired-ecc-and-data, 4 = repaired-both-ecc, -1 = unrecoverable)
char EccCheck512(unsigned char *data, unsigned char *storedEcc);



// Read a 3x repetition-coded 16-bit values, corrects with all 1-bit error, and 15/16 2-bit errors (if they're in different bit positions within each number)
unsigned short EccReadTriplicate(unsigned short value, unsigned short repeat1, unsigned short repeat2);



// The symbol codes have a Hamming Distance of at least 5 (to cope with any 2-bit error)
#define ECC_4SYMBOL_0 0x03	// 00000011
#define ECC_4SYMBOL_1 0x1C	// 00011100
#define ECC_4SYMBOL_2 0xE0	// 11100000
#define ECC_4SYMBOL_3 0xFF	// 11111111 (default value for an erased NAND block)
#define ECC_4SYMBOL_ERROR 0x01	// (error code)

// Read four-symbol coded byte value
unsigned char EccReadFourSymbol(unsigned char value);



#endif

