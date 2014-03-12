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

// RC4 Stream Cypher Encryption/Decryption
// Dan Jackson, 2012

// NOTE: See header file Encrypt.h for simple example code.


//#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) && defined(_DEBUG)
#include <stdio.h>
#endif

#include "Utils/Encrypt.h"


#define RC4_BYTE_SWAP(a, b) { unsigned char _t; _t = a; a = b; b = _t; }


// Initialize the RC4 stream state
void RC4Init(rc4_t *rc4, const unsigned char *key, int keyLength, const unsigned char *iv, int ivLength, unsigned int ksaLoops, unsigned int dropN)
{
    unsigned int n;
    unsigned char i, j;

    // Initialize indexes
    rc4->i = 0;
    rc4->j = 0;

    // Initialize position
    rc4->pos = 0;

    // Negative key/iv lengths treat the key/iv as a NULL-terminated string
    if (keyLength < 0) { keyLength = (key == NULL) ? 0 : strlen((const char *)key); }
    if (ivLength < 0) { ivLength = (iv == NULL) ? 0 : strlen((const char *)iv); }

    // Initialize to identity permutation
    i = 0;
    do
    {
        rc4->S[i] = i;
    } while (++i);

    // If a non-empty key
    if (keyLength + ivLength > 0)
    {
        // Set the permutation using the Key Scheduling Algorithm
        j = 0;
        for (n = 0; n < ksaLoops; n++)
        {
			unsigned char ki;
			ki = 0;
			// For each element in the permutation...
            i = 0;
            do
            {
                unsigned char kv = ki < keyLength ? key[ki] : iv[ki - keyLength];
                j += (rc4->S[i] + kv);
                RC4_BYTE_SWAP(rc4->S[i], rc4->S[j]);
				ki++;
				if (ki >= keyLength + ivLength) { ki = 0; }
            } while (++i);
        }
    }

    // Drop-N initial bytes of the pseudo-random generation algorithm -- note, these initially dropped bytes are NOT part of the key-stream position (rc4->pos = 0)
    while (dropN--)
    {
        rc4->j += rc4->S[++rc4->i];                     // Increment i, move j
        RC4_BYTE_SWAP(rc4->S[rc4->i], rc4->S[rc4->j]);  // Swap S[i] and S[j]
    }

    return;
}


// Encrypt or decrypt a buffer based on the current key-stream
void RC4Process(rc4_t *rc4, void *buffer, unsigned int length)
{
    unsigned char i = rc4->i, j = rc4->j, *S = rc4->S;  // Local aliases
    unsigned char *p = (unsigned char *)buffer;

    // Update position
    rc4->pos += length;

    // Process buffer
    while (length--)
    {
#if 1   // (Possibly easier to optimize?)
        unsigned char Si, Sj;
        Si = S[++i];            // Increment i, cache S[i]
        Sj = S[j += Si];        // Increment j by S[i], cache S[j]
        S[i] = Sj; S[j] = Si;   // Swap S[i] and S[j]
        *p++ ^= S[(unsigned char)(Si + Sj)];   // Retrieve final key-stream byte, XOR against data
#else   // (Slightly easier to read)
        // Pseudo-random generation algorithm
        j += S[++i];                // Increment i, move j
        RC4_BYTE_SWAP(S[i], S[j]);  // Swap S[i] and S[j]

        // Retrieve the next byte from the key-stream; XOR the key-stream against the buffer data to encrypt or decrypt it
        *p++ ^= S[(unsigned char)(S[i] + S[j])];
#endif
    }

    // Copy back aliases
    rc4->i = i; rc4->j = j;

    return;
}


// Skip n-bytes of the key-stream
void RC4Skip(rc4_t *rc4, unsigned int length)
{
    // Local aliases
    unsigned char i = rc4->i, j = rc4->j;
    unsigned char *S = rc4->S;

    // Update position
    rc4->pos += length;

    // Drop-N bytes of the pseudo-random generation algorithm
    while (length--)
    {
        j += S[++i];                // Increment i, move j
        RC4_BYTE_SWAP(S[i], S[j]);  // Swap S[i] and S[j]
    }

    // Copy back aliases
    rc4->i = i; rc4->j = j;

    return;
}


// Report the current key-stream position
unsigned long RC4Position(const rc4_t *rc4)
{
    return rc4->pos;
}


// Utility function: Decode an input ASCII hex stream to a binary buffer
int FromHex(unsigned char *buffer, const char *hex, int max)
{
    int count = 0;

    if (buffer == NULL || hex == NULL) { return 0; }
    while ((max < 0 || count / 2 < max))
    {
        unsigned char c = (unsigned char)*hex++;

        // Calculate nibble value
        if (c == '\0' || c == '\r' || c == '\n') { break; }     // End of input
        else if (c >= '0' && c <= '9') { c = c - '0'; }         // Digit (0-9)
        else if (c >= 'a' && c <= 'f') { c = c - 'a' + 10; }    // Lower-case hex (a-f)
        else if (c >= 'A' && c <= 'F') { c = c - 'A' + 10; }    // Upper-case hex (A-F)
        else if ((c == '\t' || c == ' ' || c == ':' || c == '-' || c == ',' || c == '_') && !(count & 1)) { continue; }    // Separator (only allowed at even hex digits)
        else { return -1; }                                     // Invalid character

        // Set nibbles
        if ((count & 1) == 0) { buffer[count / 2] = (c << 4); }
        else { buffer[count / 2] |= (c & 0x0f); }
        count++;
    }

    if (count & 1) { return -1; }   // invalid - odd number of characters
    return count / 2;
}
