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

#ifndef ENCRYPT_H
#define ENCRYPT_H

/*
// NOTE: The key must remain private, but the initialization vector can be publicly known (e.g. stored with the data).
// IMPORTANT: You MUST use a different initialization vector for each stream that uses the same key, otherwise information will be easily recoverable.

// Simple RC4 example -- encrypt or decrypt one stream to another using strings for the key and initialization vector
void ProcessStream(char *key, char *iv, FILE *in, FILE *out)
{
    rc4_t rc4;
    #define BUFFER_SIZE 512
    unsigned char buffer[BUFFER_SIZE];
    int length;

    // Initialize the RC4 stream (using strings for key and initialization vector)
    RC4Init(&rc4, key, RC4_BUFFER_IS_STRING, iv, RC4_BUFFER_IS_STRING, RC4_KSA_LOOPS_DEFAULT, RC4_DROP_DEFAULT);

    // Process the incoming stream
    while (!feof(in))
    {
        if ((length = fread(buffer, 1, BUFFER_SIZE, in)) <= 0) { break; }   // Read the input to the buffer
        RC4Process(&rc4, buffer, length);                                   // Encrypt/decrypt the buffer
        fwrite(out, buffer, length);                                        // Write the output from the buffer
    }
}

// Use stdin/stdout, take parameter of secret key and optional initialization vector
void main(int argc, char *argv[])
{
    const char *key = (argc > 1) ? argv[1] : NULL;
    const char *iv = (argc > 2) ? argv[2] : NULL;
    ProcessStream(key, iv, stdin, stdout);
}

*/


// RC4 stream state
typedef struct
{
    unsigned char S[256];
    unsigned long pos;
    unsigned char i;
    unsigned char j;
} rc4_t;

// keyLength or ivLength value if the key or initialization vector are actually NULL-terminated strings
#define RC4_BUFFER_IS_STRING -1

// Number of KSA loop initializations to perform (1 is standard)
#define RC4_KSA_LOOPS_SINGLE  1
#define RC4_KSA_LOOPS_DEFAULT RC4_KSA_LOOPS_SINGLE

// Initial drop-N amount to discard (768 - 3072 recommended)
#define RC4_DROP_NONE         (0)           //    0
#define RC4_DROP_MEDIUM       (3 * 256)     //  768
#define RC4_DROP_CONSERVATIVE (12 * 256)    // 3072
#define RC4_DROP_DEFAULT      RC4_DROP_MEDIUM

// Initialize the RC4 stream state with a given key (private, -1 length treats as a string), initialization-vector (use a different one per stream, can be public, -1 length treats as a string), number of loops (e.g. 1), and a drop-N amount (e.g. 768).
void RC4Init(rc4_t *rc4, const unsigned char *key, int keyLength, const unsigned char *iv, int ivLength, unsigned int ksaLoops, unsigned int dropN);

// Encrypt or decrypt a buffer based against the key-stream
void RC4Process(rc4_t *rc4, void *buffer, unsigned int length);

// Inline version of RC4 that directly uses a stream state (there's a small chance it may be faster)
#define RC4_DIRECT_PROCESS(rc4, _buffer, _inLength) { unsigned int length = (_inLength); unsigned char i = rc4.i, j = rc4.j, *p = (unsigned char *)(_buffer); rc4.pos += length; while (length--) { unsigned char Si, Sj; Si = rc4.S[++i]; Sj = rc4.S[j += Si]; rc4.S[i] = Sj; rc4.S[j] = Si; *p++ ^= rc4.S[(unsigned char)(Si + Sj)]; } rc4.i = i; rc4.j = j; }

// Skip n-bytes of the key-stream
void RC4Skip(rc4_t *rc4, unsigned int length);

// Report the current key-stream position (it should be in sync with the data stream position)
unsigned long RC4Position(const rc4_t *rc4);

// Utility function: Decode an input ASCII hex stream to a binary buffer
int FromHex(unsigned char *buffer, const char *hex, int max);

#endif
