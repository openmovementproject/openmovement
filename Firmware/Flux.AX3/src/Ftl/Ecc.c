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

// Error Correcting Codes (ECC)
// Dan Jackson, 2011

#include "Ftl/Ecc.h"

// Hamming Error Correction Code (ECC) to detect 2-bit errors and correct 1-bit errors per 256 bytes.
//
// Parity generation for 256-byte input:
//   Byte   0:  b7  b6  b5  b4  b3  b2  b1  b0  <LP00> /LP\ ... /~~\.
//   Byte   1:  b7  b6  b5  b4  b3  b2  b1  b0  <LP01> \02/ ... |LP|. 
//   Byte   2:  b7  b6  b5  b4  b3  b2  b1  b0  <LP00> /LP\ ... |14|.
//   Byte   3:  b7  b6  b5  b4  b3  b2  b1  b0  <LP01> \03/ ... |  |. 
//    ... ...   ..  ..  ..  ..  ..  ..  ..  ..              ... ====. 
//   Byte 252:  b7  b6  b5  b4  b3  b2  b1  b0  <LP00> /LP\ ... |  |. 
//   Byte 253:  b7  b6  b5  b4  b3  b2  b1  b0  <LP01> \02/ ... |LP|. 
//   Byte 254:  b7  b6  b5  b4  b3  b2  b1  b0  <LP00> /LP\ ... |15|. 
//   Byte 255:  b7  b6  b5  b4  b3  b2  b1  b0  <LP01> \03/ ... \__/. 
//             CP1 CP0 CP1 CP0 CP1 CP0 CP1 CP0 
//             <-CP3-> <-CP2-> <-CP3-> <-CP2-> 
//             <-----CP5-----> <-----CP4-----> 
//
// The 3-byte ECC code:
//   Ecc0:  LP07 LP06 LP05 LP04 LP03 LP02 LP01 LP00
//   Ecc1:  LP15 LP14 LP13 LP12 LP11 LP10 LP09 LP08
//   Ecc2:  CP5  CP4  CP3  CP2  CP1  CP0  (=1) (=1)
//

// If defined, expect input and output to have the two least-significant bits of ECC2 as 0 rather than 1 (helps identify non-ECC blocks)
#define ECC_LSB0

// Column parity calculation: <CP5> <CP4> <CP3> <CP2> <CP1> <CP0> <=0> <BP>    (BP = byte parity, flag to toggle line parities)
// -- i.e. which column parity bits to toggle for a given data value, the LSB has the data byte parity.
static const unsigned char eccColumnParityTable[256] =
{
    0x00, 0x55, 0x59, 0x0C, 0x65, 0x30, 0x3C, 0x69, 0x69, 0x3C, 0x30, 0x65, 0x0C, 0x59, 0x55, 0x00,
    0x95, 0xC0, 0xCC, 0x99, 0xF0, 0xA5, 0xA9, 0xFC, 0xFC, 0xA9, 0xA5, 0xF0, 0x99, 0xCC, 0xC0, 0x95,
    0x99, 0xCC, 0xC0, 0x95, 0xFC, 0xA9, 0xA5, 0xF0, 0xF0, 0xA5, 0xA9, 0xFC, 0x95, 0xC0, 0xCC, 0x99,
    0x0C, 0x59, 0x55, 0x00, 0x69, 0x3C, 0x30, 0x65, 0x65, 0x30, 0x3C, 0x69, 0x00, 0x55, 0x59, 0x0C,
    0xA5, 0xF0, 0xFC, 0xA9, 0xC0, 0x95, 0x99, 0xCC, 0xCC, 0x99, 0x95, 0xC0, 0xA9, 0xFC, 0xF0, 0xA5,
    0x30, 0x65, 0x69, 0x3C, 0x55, 0x00, 0x0C, 0x59, 0x59, 0x0C, 0x00, 0x55, 0x3C, 0x69, 0x65, 0x30,
    0x3C, 0x69, 0x65, 0x30, 0x59, 0x0C, 0x00, 0x55, 0x55, 0x00, 0x0C, 0x59, 0x30, 0x65, 0x69, 0x3C,
    0xA9, 0xFC, 0xF0, 0xA5, 0xCC, 0x99, 0x95, 0xC0, 0xC0, 0x95, 0x99, 0xCC, 0xA5, 0xF0, 0xFC, 0xA9,
    0xA9, 0xFC, 0xF0, 0xA5, 0xCC, 0x99, 0x95, 0xC0, 0xC0, 0x95, 0x99, 0xCC, 0xA5, 0xF0, 0xFC, 0xA9,
    0x3C, 0x69, 0x65, 0x30, 0x59, 0x0C, 0x00, 0x55, 0x55, 0x00, 0x0C, 0x59, 0x30, 0x65, 0x69, 0x3C,
    0x30, 0x65, 0x69, 0x3C, 0x55, 0x00, 0x0C, 0x59, 0x59, 0x0C, 0x00, 0x55, 0x3C, 0x69, 0x65, 0x30,
    0xA5, 0xF0, 0xFC, 0xA9, 0xC0, 0x95, 0x99, 0xCC, 0xCC, 0x99, 0x95, 0xC0, 0xA9, 0xFC, 0xF0, 0xA5,
    0x0C, 0x59, 0x55, 0x00, 0x69, 0x3C, 0x30, 0x65, 0x65, 0x30, 0x3C, 0x69, 0x00, 0x55, 0x59, 0x0C,
    0x99, 0xCC, 0xC0, 0x95, 0xFC, 0xA9, 0xA5, 0xF0, 0xF0, 0xA5, 0xA9, 0xFC, 0x95, 0xC0, 0xCC, 0x99,
    0x95, 0xC0, 0xCC, 0x99, 0xF0, 0xA5, 0xA9, 0xFC, 0xFC, 0xA9, 0xA5, 0xF0, 0x99, 0xCC, 0xC0, 0x95,
    0x00, 0x55, 0x59, 0x0C, 0x65, 0x30, 0x3C, 0x69, 0x69, 0x3C, 0x30, 0x65, 0x0C, 0x59, 0x55, 0x00,
};

// Line parity calculation: <LP15> <LP14> <LP13> <LP12> <LP11> <LP10> <LP09> <LP08> <LP07> <LP06> <LP05> <LP04> <LP03> <LP02> <LP01> <LP00> 
// -- i.e. which line parity bits to toggle at a given address when the data byte has odd parity.
static const unsigned short eccLineParityTable[256] =
{
    0x5555, 0x5556, 0x5559, 0x555A, 0x5565, 0x5566, 0x5569, 0x556A, 0x5595, 0x5596, 0x5599, 0x559A, 0x55A5, 0x55A6, 0x55A9, 0x55AA,
    0x5655, 0x5656, 0x5659, 0x565A, 0x5665, 0x5666, 0x5669, 0x566A, 0x5695, 0x5696, 0x5699, 0x569A, 0x56A5, 0x56A6, 0x56A9, 0x56AA,
    0x5955, 0x5956, 0x5959, 0x595A, 0x5965, 0x5966, 0x5969, 0x596A, 0x5995, 0x5996, 0x5999, 0x599A, 0x59A5, 0x59A6, 0x59A9, 0x59AA,
    0x5A55, 0x5A56, 0x5A59, 0x5A5A, 0x5A65, 0x5A66, 0x5A69, 0x5A6A, 0x5A95, 0x5A96, 0x5A99, 0x5A9A, 0x5AA5, 0x5AA6, 0x5AA9, 0x5AAA,
    0x6555, 0x6556, 0x6559, 0x655A, 0x6565, 0x6566, 0x6569, 0x656A, 0x6595, 0x6596, 0x6599, 0x659A, 0x65A5, 0x65A6, 0x65A9, 0x65AA,
    0x6655, 0x6656, 0x6659, 0x665A, 0x6665, 0x6666, 0x6669, 0x666A, 0x6695, 0x6696, 0x6699, 0x669A, 0x66A5, 0x66A6, 0x66A9, 0x66AA,
    0x6955, 0x6956, 0x6959, 0x695A, 0x6965, 0x6966, 0x6969, 0x696A, 0x6995, 0x6996, 0x6999, 0x699A, 0x69A5, 0x69A6, 0x69A9, 0x69AA,
    0x6A55, 0x6A56, 0x6A59, 0x6A5A, 0x6A65, 0x6A66, 0x6A69, 0x6A6A, 0x6A95, 0x6A96, 0x6A99, 0x6A9A, 0x6AA5, 0x6AA6, 0x6AA9, 0x6AAA,
    0x9555, 0x9556, 0x9559, 0x955A, 0x9565, 0x9566, 0x9569, 0x956A, 0x9595, 0x9596, 0x9599, 0x959A, 0x95A5, 0x95A6, 0x95A9, 0x95AA,
    0x9655, 0x9656, 0x9659, 0x965A, 0x9665, 0x9666, 0x9669, 0x966A, 0x9695, 0x9696, 0x9699, 0x969A, 0x96A5, 0x96A6, 0x96A9, 0x96AA,
    0x9955, 0x9956, 0x9959, 0x995A, 0x9965, 0x9966, 0x9969, 0x996A, 0x9995, 0x9996, 0x9999, 0x999A, 0x99A5, 0x99A6, 0x99A9, 0x99AA,
    0x9A55, 0x9A56, 0x9A59, 0x9A5A, 0x9A65, 0x9A66, 0x9A69, 0x9A6A, 0x9A95, 0x9A96, 0x9A99, 0x9A9A, 0x9AA5, 0x9AA6, 0x9AA9, 0x9AAA,
    0xA555, 0xA556, 0xA559, 0xA55A, 0xA565, 0xA566, 0xA569, 0xA56A, 0xA595, 0xA596, 0xA599, 0xA59A, 0xA5A5, 0xA5A6, 0xA5A9, 0xA5AA,
    0xA655, 0xA656, 0xA659, 0xA65A, 0xA665, 0xA666, 0xA669, 0xA66A, 0xA695, 0xA696, 0xA699, 0xA69A, 0xA6A5, 0xA6A6, 0xA6A9, 0xA6AA,
    0xA955, 0xA956, 0xA959, 0xA95A, 0xA965, 0xA966, 0xA969, 0xA96A, 0xA995, 0xA996, 0xA999, 0xA99A, 0xA9A5, 0xA9A6, 0xA9A9, 0xA9AA,
    0xAA55, 0xAA56, 0xAA59, 0xAA5A, 0xAA65, 0xAA66, 0xAA69, 0xAA6A, 0xAA95, 0xAA96, 0xAA99, 0xAA9A, 0xAAA5, 0xAAA6, 0xAAA9, 0xAAAA,
};


// Enable table generation code
#if 0

// Slow column parity calculation for cp0-cp5, and byte parity
static unsigned char EccColumnParity(unsigned char value)
{
    unsigned char b0 = ((value     ) & 1), b1 = ((value >> 1) & 1), b2 = ((value >> 2) & 1), b3 = ((value >> 3) & 1);
    unsigned char b4 = ((value >> 4) & 1), b5 = ((value >> 5) & 1), b6 = ((value >> 6) & 1), b7 = ((value >> 7) & 1);
    unsigned char cp0 = b6 ^ b4 ^ b2 ^ b0;
    unsigned char cp1 = b7 ^ b5 ^ b3 ^ b1;
    unsigned char cp2 = b5 ^ b4 ^ b1 ^ b0;
    unsigned char cp3 = b7 ^ b6 ^ b3 ^ b2;
    unsigned char cp4 = b3 ^ b2 ^ b1 ^ b0;
    unsigned char cp5 = b7 ^ b6 ^ b5 ^ b4;
    unsigned char bp = b7 ^ b6 ^ b5 ^ b4 ^ b3 ^ b2 ^ b1 ^ b0;
    return (cp5 << 7) | (cp4 << 6) | (cp3 << 5) | (cp2 << 4) | (cp1 << 3) | (cp0 << 2)  |  (bp);
}

// Slow line parity calculation for lp0-lp15
static unsigned short EccLineParity(unsigned char value)
{
    unsigned short result;
    unsigned char b;
    result = 0;
    for (b = 0; b < 8; ++b)
    {
        result |= ((unsigned short)1 << ((b << 1) + ((value >> b) & 1)));
    }
    return result;
}

#include <stdio.h>

// Output pre-calculated tables
void EccPrintParityTables(void)
{
    int value;
    
    printf("\n");
    printf("static const unsigned char eccColumnParityTable[256] =\n");
    printf("{\n");
    for (value = 0; value < 256; value++)
    {
        if (value % 16 == 0) { printf("    "); }
        printf("0x%02X, ", EccColumnParity(value));
        if (value % 16 == 15) { printf("\n"); }
    }
    printf("};\n");
    printf("\n");
    printf("static const unsigned short eccLineParityTable[256] =\n");
    printf("{\n");
    for (value = 0; value < 256; value++)
    {
        if (value % 16 == 0) { printf("    "); }
        printf("0x%04X, ", EccLineParity(value));
        if (value % 16 == 15) { printf("\n"); }
    }
    printf("};\n");
    printf("\n");
}

#endif


// Generate 256 byte Hamming ECC
void EccGenerate256(const unsigned char *data, unsigned char *ecc)
{
    unsigned char v, i = 0, cp = 0;
    unsigned short lp = 0;
    do                                      // Iterate over all 256 data bytes
    {
        v = eccColumnParityTable[data[i]];  // Look up the column parity & byte parity values for this data
        cp ^= v;                            // Toggle the column parity bits
        if (v & 1)                          // If the byte byte parity bit is set...
            lp ^= eccLineParityTable[i];    // ...toggle the associated line parity bits at this address
    } while (++i);	                        // Loop until wrap from 255 to 0
	
#ifdef ECC_LSB0
	cp &= ~0x03;                            // Remove the junk from the column parity -- clear the two LSBs
#else
	cp |= 0x03;                             // Remove the junk from the column parity -- set the two LSBs
#endif

    // Set the output ECC values
    ecc[0] = (unsigned char)lp;
    ecc[1] = (unsigned char)(lp >> 8);
    ecc[2] = cp;

    return;
}


// Checks 256 byte Hamming ECC and corrects the data (1-bit), or detects failure (2-bit)
// (0 = ok, 1 = repaired-data, 2 = repaired-ecc, -1 = unrecoverable)
char EccCheck256(unsigned char *data, unsigned char *storedEcc)
{
    static const unsigned char bitsSetTable[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    unsigned char newEcc[3], ecc[3];
    unsigned char bitsSet;

#ifdef ECC_LSB0
	// If two LSBs are usually zero but are both set, we have no ECC
	if ((storedEcc[2] & 0x03) == 0x03)
	{
		return 0;
	}
#endif

    // Compute ECC for the data
    EccGenerate256(data, newEcc);

#ifdef ECC_LSB0
    newEcc[2] |= 0x03;	// Two LSBs should be set for comparison
#endif

    // XOR the ECC codes
    ecc[0] = newEcc[0] ^ storedEcc[0];
    ecc[1] = newEcc[1] ^ storedEcc[1];
#ifdef ECC_LSB0
    ecc[2] = newEcc[2] ^ (storedEcc[2] | 0x03);		// Set two LSBs in comparison
#else
    ecc[2] = newEcc[2] ^ storedEcc[2];
#endif

    // If zero, no bit errors
    if ((ecc[0] | ecc[1] | ecc[2]) == 0)
    {
        return 0;
    }

    // Count the number of bits set in the XOR'd ECC code
    bitsSet = bitsSetTable[ecc[0] & 0xf] + bitsSetTable[ecc[0] >> 4]
            + bitsSetTable[ecc[1] & 0xf] + bitsSetTable[ecc[1] >> 4]
            + bitsSetTable[ecc[2] & 0xf] + bitsSetTable[ecc[2] >> 4];

    // If exactly 11 bits set, we can recover the 1-bit error
    if (bitsSet == 11)
    {
        unsigned char address, bit;

        // Calculate the byte address and bit position
        bit = ((ecc[2] >> 3) & 0x01) | ((ecc[2] >> 4) & 0x02) | ((ecc[2] >> 5) & 0x04);
        address = ((ecc[0] >> 1) & 0x01) | ((ecc[0] >> 2) & 0x02) | ((ecc[0] >> 3) & 0x04) |
                  ((ecc[0] >> 4) & 0x08) | ((ecc[1] << 3) & 0x10) | ((ecc[1] << 2) & 0x20) |
                  ((ecc[1] << 1) & 0x40) | ((ecc[1]     ) & 0x80);

        // Correct the bit error by flipping it
        data[address] ^= (1 << bit);

        return 1;       // Corrected data error
    }
    else if (bitsSet == 1)  // If 1 bit set, we have an error in the ECC code
    {
        // Fix the supplied ECC code
        storedEcc[0] = newEcc[0];
        storedEcc[1] = newEcc[1];
#ifdef ECC_LSB0
        storedEcc[2] = (newEcc[2] & ~0x03);	// Output two LSBs as clear
#else
        storedEcc[2] = newEcc[2];
#endif
		return 2;       // Corrected ECC error
    }

#ifdef ECC_DEBUG
	#warning "ECC debug output is switched on"
	#ifdef ECC_LSB0
		printf("WARNING: Un-correctable error 0x%02x%02x%02x != 0x%02x%02x%02x\n", newEcc[0], newEcc[1], newEcc[2] | 0x03, storedEcc[0], storedEcc[1], storedEcc[2] | 0x03);
	#else
		printf("WARNING: Un-correctable error 0x%02x%02x%02x != 0x%02x%02x%02x\n", newEcc[0], newEcc[1], newEcc[2], storedEcc[0], storedEcc[1], storedEcc[2]);
	#endif
#endif

    // Un-correctable error
    return -1;
}


// Generate 512 byte Hamming ECC 
void EccGenerate512(const unsigned char *data, unsigned char *ecc)
{
    EccGenerate256(data, ecc);
    EccGenerate256(data + 256, ecc + ECC_SIZE_256);
}


// Checks 512 byte Hamming ECC and corrects the data (1-bit per 256 bytes), or detects failure (2-bits per 256 bytes)
// (0 = ok, 1 = repaired-data, 2 = repaired-ecc, 3 = repaired-ecc-and-data, 4 = repaired-both-ecc, -1 = unrecoverable)
char EccCheck512(unsigned char *data, unsigned char *storedEcc)
{
    char r1, r2;
    
    r1 = EccCheck256(data, storedEcc);
    if (r1 < 0) { return r1; }

    r2 = EccCheck256(data + 256, storedEcc + ECC_SIZE_256);
    if (r2 < 0) { return r2; }

    return (r1 + r2);
}




// Read a 3x repetition-coded 16-bit values, corrects with all 1-bit error, and 15/16 2-bit errors (if they're in different bit positions within each number)
unsigned short EccReadTriplicate(unsigned short value, unsigned short repeat1, unsigned short repeat2)
{
	// If the current value doesn't match either of the repeats, we need to find the correct value...
	if (value != repeat1 && value != repeat2)
	{
		// There was an error, so the repeats should match...
		if (repeat1 == repeat2)
		{
			// Use that value
			value = repeat1;
		}
		else
		{
			// (Unlikely we'll have to, but) hunt for the correct value by inspecting each bit
			unsigned char bit;
			for (bit = 0; bit < sizeof(unsigned short) * 8; bit++)
			{
				unsigned char b = (unsigned char)((value >> bit) & 1);
				unsigned char b1 = (unsigned char)((repeat1 >> bit) & 1);
				unsigned char b2 = (unsigned char)((repeat2 >> bit) & 1);
				// If neither repeat agrees with the bit already set in the value...
				if (b != b1 && b != b2)
				{
					// ...it must be wrong (the repeat bits logically have to match), so toggle the existing bit value
					value ^= ((unsigned short)1 << bit);
				}
			}
		}
	}
	return value;
}


// Read four symbol coded byte value
// The symbol codes have a Hamming Distance of at least 5 (to cope with any 2-bit error)
//   00000011, 00011100, 11100000, 11111111
//       0x03,     0x1C,     0xE0,     0xFF
unsigned char EccReadFourSymbol(unsigned char value)
{
	// Shortcut expected values
	if (value == ECC_4SYMBOL_0 || value == ECC_4SYMBOL_1 || value == ECC_4SYMBOL_2 || value == ECC_4SYMBOL_3)
	{
		return value;
	}

	// Special case all-zeros to be invalid
	if (value == 0x00)
	{
		return ECC_4SYMBOL_ERROR;
	}

	//LOG("Having to attempt reconstruction of page type %02x...\n", value);

	// Find closest symbol from hamming distance
	{
		char b;
		char dist0 = 0, dist1 = 0, dist2 = 0, dist3 = 0;
		for (b = 0; b < 8; b++)
		{
			unsigned char mask = (1 << b);
			unsigned char maskedValue = (value & mask);
			if ((ECC_4SYMBOL_0 & mask) != maskedValue) { dist0++; }
			if ((ECC_4SYMBOL_1 & mask) != maskedValue) { dist1++; }
			if ((ECC_4SYMBOL_2 & mask) != maskedValue) { dist2++; }
			if ((ECC_4SYMBOL_3 & mask) != maskedValue) { dist3++; }
		}
		// Return closest symbol
		if (                 dist0 < dist1 && dist0 < dist2 && dist0 < dist3) { return ECC_4SYMBOL_0; }
		if (dist1 < dist0                  && dist1 < dist2 && dist1 < dist3) { return ECC_4SYMBOL_1; }
		if (dist2 < dist0 && dist2 < dist1                  && dist2 < dist3) { return ECC_4SYMBOL_2; }
		if (dist3 < dist0 && dist3 < dist1 && dist3 < dist2                 ) { return ECC_4SYMBOL_3; }
	}

	// Closest symbol was a tie -- cannot recover, return bad
	return ECC_4SYMBOL_ERROR;
}
