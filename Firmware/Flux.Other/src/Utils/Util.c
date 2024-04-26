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

// util.h - Utility functions
// Dan Jackson, Karim Ladha, 2010-2012.


#if !defined(_MSC_VER)

// Includes
#if defined(USE_USART) || defined(DEBUG_USART2_TX)
#include <usart.h>
#endif

#include "HardwareProfile.h"
//#include <stdio.h>
#include "usb_config.h"
#if defined(USB_USE_CDC)
    // KL fix, Util.c will not compile without usb support if you include usb.h
    #include "USB/USB.h"
#endif

#ifdef USB_USE_CDC
    #include "USB/usb_function_cdc.h"
    #include "Usb/USB_CDC_MSD.h"
    // Edit-line buffer
    #pragma idata
    static int serialBufferLength = 0;
    char commEcho = 0;      // Echo input
    #ifndef SERIAL_BUFFER_SIZE
        #define SERIAL_BUFFER_SIZE 128
    #endif
    #pragma udata
    static char serialBuffer[SERIAL_BUFFER_SIZE] = {0};
#endif

#endif

#include <stdlib.h>
#include <string.h>
#include "Utils/Util.h"

#define _user_putc(_x) usb_putchar(_x)

#ifdef USB_USE_CDC
// Retrieve a line of console-edited input
const char *_user_gets(void)
{
    unsigned char i;
    int c;

    for (i = 0; i < 100; i++)       // Maximum number of iterations (bails out early if no input)
    {
        c = usb_getchar();

        if (c <= 0) { break; }
        if (c == 13)                        // CR
        {
            if (commEcho && c != 10) { _user_putc(c); }
            serialBuffer[serialBufferLength] = '\0';
            serialBufferLength = 0;
            if (commEcho) { _user_putc(10); }
            return serialBuffer;
        }
        else if (c == 8 || c == 127)        // BS and DEL
        {
            if (serialBufferLength > 0)
            {
                if (commEcho && c != 10) { _user_putc(8); _user_putc(32); _user_putc(8); }
                serialBufferLength--;
                serialBuffer[serialBufferLength] = '\0';
            }
        }
        else if (c == 10)                   // LF
        {
            ;   // ignore
        }
        else if (serialBufferLength + 1 < SERIAL_BUFFER_SIZE)
        {
            if (commEcho && c != 10) { _user_putc(c); }
            serialBuffer[serialBufferLength] = (char)c;
            serialBufferLength++;
            serialBuffer[serialBufferLength] = '\0';
        }
        else
        {
            if (commEcho && c != 10) { _user_putc('\a'); }
            ;   // Discard (out of buffer)
        }
    }
    return NULL;
}
#endif

// ASCII to integer
unsigned long my_atoi(const char *s)
{
    char sign = 0;
    unsigned long value = 0;
    while (*s == ' '|| *s == '=' || *s == ':') { s++; }
    if (*s == '-') { sign = -1; s++; }
    for (; *s >= '0' && *s <= '9'; s++)
    {
        if (sign == 0) { sign = 1; } 
        value *= 10; 
        value += (*s - '0');
    }
    return (sign == 0) ? 0xffffffff : (sign * value);
}

// Integer to ASCII
const char *my_itoa(int v)
{
    static char buffer[7];    // -12345\0
    char *p;
    *(p = buffer + 6) = '\0';
    if (v >= 0) {
        do { *(--p) = '0' + (v % 10); } while (v /= 10);
    } else {
        do { *(--p) = '0' - (v % 10); } while (v /= 10);
        *(--p) = '-';
    }
    return p;
}

// Integer to ASCII
int my_nitoa(char *dest, int v)
{
    int len;
    static char buffer[7];    // -12345\0
    char *p;
    *(p = buffer + 6) = '\0';
    if (v >= 0) {
        do { *(--p) = '0' + (v % 10); } while (v /= 10);
    } else {
        do { *(--p) = '0' - (v % 10); } while (v /= 10);
        *(--p) = '-';
    }
    len = &buffer[6] - p;
    memcpy(dest, p, len);
    return len;
}

// Unsigned integer to ASCII
const char *my_uitoa(unsigned int v)
{
    static char buffer[6];    // -12345\0
    char *p;
    *(p = buffer + 5) = '\0';
    do { *(--p) = '0' + (v % 10); } while (v /= 10);
    return p;
}

const char *my_ultoa(unsigned long v)
{
    static char buffer[11];    // 4000000000\0
    char *p;
    *(p = buffer + 10) = '\0';
    do { *(--p) = '0' + (v % 10); } while (v /= 10);
    return p;
}


// Signed *10^n fixed-point integer to ASCII (e.g. n = 2: "-327.68" to "327.67")
const char *fixed_itoa(short value, char n)
{
    static char buffer[8];
    char negative;
    unsigned short v;
    int p;
    int i = 8;

    if (value < 0) { negative = 1; v = (unsigned short)-value; }
    else { negative = 0; v = (unsigned short)value; }

    buffer[--i] = '\0';
    for (p = 0; p < 6 && (p <= n || v != 0); p++, v /= 10)
    {
        if (p == n)
        {
            buffer[--i] = '.';
        }
        buffer[--i] = '0' + (v % 10);
    }
    if (negative) { buffer[--i] = '-'; }

    return &buffer[i];
}

// Convert nibble into hex representation as a char (WITH NIBBLE MASK)
char NibbleToHex(unsigned char nibble)
{
    // Truncate to nibble
    nibble &= 0xF;
    // Simple version
    #if 0     
    if(nibble <= 9)    nibble += '0';
    else             nibble += 'A' - 0xA;
    // Faster version
    #else    
    nibble += '0';
    if(nibble > '9')nibble += 'A' - '9' - 1;
    #endif
    // Return character
    return (char)nibble;
}

#if !(defined(_MSC_VER) && defined(_INC_STRING))
// Macro to lower-case an ASCII character
#define _CHAR_TO_LOWER(_c) (((_c) >= 'A' && (_c) <= 'Z') ? ((_c) + 'a' - 'A') : (_c))

// Case-insensitive string comparison
int strnicmp(const char *a, const char *b, int max)
{
    for (;;)
    {
        if (max-- == 0) { return 0; }
        if (_CHAR_TO_LOWER(*a) != _CHAR_TO_LOWER(*b)) { return 1; }
        if (*a == '\0' || *b == '\0') { return 0; }
        a++; b++;
    }
}
#endif


#if defined(PIC24) || defined(__C30__) || defined(__dsPIC33E__)
#ifndef WRITE_DEFAULT_HANDLER
    #ifdef USB_USE_CDC
        #define WRITE_DEFAULT_HANDLER usb_write
    #else
        #define WRITE_DEFAULT_HANDLER NULL
    #endif
#endif
write_handler_t writeHandler = WRITE_DEFAULT_HANDLER;
__attribute__ ( (section(".libc"))) int write(int handle, void *buffer, unsigned int len)
{
    switch (handle)
    {
        #ifdef USB_USE_CDC
        /*KL: Divert stderr to usb_write by default.*/
        case 2: // handle 2 = stderr
            usb_write(buffer, len);
            break;
        #endif
        case 0: // handle 0 = stdout
        case 1: // handle 1 = stdin
        default:
            if (writeHandler != NULL) { writeHandler(buffer, len); }
    }
    return len;
}
#endif

// Checksum - 16-bit word-size addition, returns two's compliment of sum (bitwise NOT, then add 1) -- then total sum of words including checksum will be zero.
unsigned short checksum(const void *buffer, size_t len)
{
    const unsigned short *data = (const unsigned short *)buffer;
    size_t words = (len >> 1);
    unsigned short value = 0x0000;                        // Initial sum of zero
    for (; words; --words) { value += *data++; }        // Sum data words
    if (len & 1) { value += ((unsigned char *)buffer)[len - 1]; }          // Add odd byte
    return (~value) + 1;                                // ...take bitwise NOT of sum, then add 1 (total sum of words including checksum will be zero)
}


// Hex dump of memory
#ifdef ENABLE_printhexdump
void printhexdump(void *buffer, size_t offset, size_t length)
{
    unsigned char *buf = (unsigned char *)buffer;
    char w = 16, b;
    unsigned short o;
    for (o = 0; o < length; o += w)
    {
        char z = w;
        if (o + z >= length) { z = (char)(length - o); }
        printf("%04x: ", offset + o);
        for (b = 0; b < 16; b++)
        {
            if (b > 0 && b % 4 == 0) { printf(" "); }        // optional, additional spacing
            
            if (b < z) { printf("%02x ", buf[offset + o + b]); }
            else { printf("   "); }
        }
        printf(" "); 
        for (b = 0; b < 16; b++)
        {
            unsigned char c = buf[offset + o + b];
            if (b < z) { printf("%c", (c < 0x1f || (c >= 0x7f && c <= 0x9f) || c >= 0xff) ? '.' : c); }
            else { printf(" "); }
        }
        printf("\r\n");
        #ifdef USB_USE_CDC
        USBCDCWait();    
        #endif
    }
}
#endif



// Check valid filename.  allowWildcard: 0=none, 1=?, 2=*, 3=?/*
char ValidFilename(const char *filename, char allowWildcard)
{
       const char *c;
    char len = 0;
    char ext = -1;
    char wildcards = 0;
    if (filename == NULL) { return 0; }
    for (c = filename; ; c++)
    {
        if (*c == '\0')
        {
            // Empty filename or empty extension (no extension at all is valid)
            if (len <= 0 || ext == 0) { return 0; }
            break;
        }
        if (*c < 32 || *c >= 127 || *c == ' ' || *c == '\"' || *c == '/' || *c == ':' || *c == '<' || *c == '>' || *c == '\\' || *c == '|' || *c == '+' || *c == ',' || *c == ';' || *c == '=' || *c == '[' || *c == ']') { return 0; }
        // Check: no wildcard characters unless we're expecting them
        if (!(allowWildcard & 1) && *c == '?') { return 0; }
        // Check: if we see '*', must be expecting wildcards
        if (*c == '*')
        { 
            if (!(allowWildcard & 2)) { return 0; }
            // Check: at most, one '*' for name and one '*' for extension
            if (ext >= 0) { if (wildcards & 2) { return 0; } wildcards |= 2; }
            else { if (wildcards & 1) { return 0; } wildcards |= 1; }
        }
        // Check: at most one extension separator
        if (*c == '.')
        {
            if (ext >= 0) { return 0; }
            ext = 0;
        }
        else
        {
            if (ext >= 0) { ext++; }
            else { len++; }
            // Check: filename and extension length
            if (len > 8 || ext > 3) { return 0; }
        }
    }
    // Normalize case
    //for (c = filename; *c != '\0'; c++) { if (*c >= 'a' && *c <= 'z') { *c = *c + 'A' - 'a'; } }
    return (wildcards) ? 2 : 1;
}


/*

static unsigned long rand_seed = 0;

void rand_seed(unsigned long value)
{
    rand_seed = value;
}

unsigned long rand_next(void)
{
    unsigned long rh, rl;
    rl = 16807 * (unsigned short)rand_seed;
    rh = 16807 * (unsigned short)(rand_seed >> 16);
    rl = rl + ((rh & 0x7FFF) << 16) + (rh >>15);
    if (rl & 0x80000000) { rl -= 0x7fffffff; }
    rand_seed = rl;
    return rand_seed;
}

*/



// Utility function: Decode an input ASCII hex stream (Base16) to a binary buffer (safe for use in-place)
int DecodeHex(unsigned char *output, const char *input, int max)
{
    int count = 0;

    if (!output || !input) { return 0; }
    while ((max < 0 || count / 2 < max))
    {
        unsigned char c = (unsigned char)*input++;

        // Calculate nibble value
        if (c == '\0' || c == '\r' || c == '\n') { break; }     // End of input
        else if (c >= '0' && c <= '9') { c = c - '0'; }         // Digit (0-9)
        else if (c >= 'a' && c <= 'f') { c = c - 'a' + 10; }    // Lower-case hex (a-f)
        else if (c >= 'A' && c <= 'F') { c = c - 'A' + 10; }    // Upper-case hex (A-F)
        else if ((c == '\t' || c == ' ' || c == ':' || c == '-' || c == ',' || c == '_') && !(count & 1)) { continue; }    // Separator (only allowed at even hex digits)
        else { return -1; }                                     // Invalid character

        // Set nibbles
        if ((count & 1) == 0) { output[count / 2] = (c << 4); }
        else { output[count / 2] |= (c & 0x0f); }
        count++;
    }

    if (count & 1) { return -1; }   // invalid - odd number of characters
    return count / 2;
}


// Utility function: Decode an input ASCII Base64-encoded (RFC 3548) stream to a binary buffer (safe for use in-place)
int DecodeBase64(unsigned char *output, const char *input)
{
    unsigned short value;
    unsigned char bitcount;
    int count;

    if (!output || !input) { return 0; }
    count = 0;
    value = 0; 
    bitcount = 0;
    for (;;)
    {
        char c = (char)*input++;

#if 0			// Not yet tested
        #define bE -128    // end of input
        #define bX -1      // invalid
        const static char base64lookup[128] = 
        {
        //  _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _A, _B, _C, _D, _E, _F, 
            bE, bX, bX, bX, bX, bX, bX, bX, bX, bX, bE, bX, bX, bE, bX, bX, // 0x00-0x0F - 0x00=NUL, 0x0A=LF, 0x0D=CR
            bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, // 0x10-0x1F - 
            bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, bX, 62, bX, 62, 62, 63, // 0x20-0x2F - 0x2B='+', 0x2D='-', 0x2E='.', 0x2F='/'
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, bX, bX, bX, bE, bX, bX, // 0x30-0x3F - 0x30='0'...0x39='9', 0x3D='='
            bX,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 0x40-0x4F - 0x41='A'...
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, bX, bX, bX, bX, 63, // 0x50-0x5F - ...0x5A='Z', 0x5F='_'
            bX, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 0x60-0x6F - 0x61='a'...
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, bX, bX, bX, bX, bX, // 0x70-0x7F - ...0x7A='Z'
        };
        if (c < 0) return -1;    // Invalid character (not in table)
        c = base64lookup[c];
        if (c == bX) return -1;    // Invalid character (in table)
        if (c == bE) return -1;    // End of input
#else
        // Calculate next 6-bit value
        if (c == '=' || c == '\0' || c == '\r' || c == '\n') { break; }  // Will be treated as end of input
        else if (c >= 'A' && c <= 'Z') { c = c - 'A'; }         // 0-25
        else if (c >= 'a' && c <= 'z') { c = c - 'a' + 26; }    // 26-51
        else if (c >= '0' && c <= '9') { c = c - '0' + 52; }    // 52-61
        else if (c == '+' || c == '-' || c == '.') { c = 62; }  // 62 (with alternatives)
        else if (c == '/' || c == '_') { c = 63; }              // 63 (with alternative)
        else { return -1; }                                     // Invalid character
#endif

        // Add these 6 new bits to the accumulator
        value = (value << 6) | c;
        bitcount += 6;
        
        // If we have a new byte to output...
        if (bitcount >= 8)
        {
            // Write the top 8 bits from the accumulator
            output[count++] = (unsigned char)(value >> (bitcount - 8));
            // We now have 8 fewer data bits in the accumulator
            bitcount -= 8;
        }
    }

    return count;
}


// Utility function: Encode a binary input as an ASCII Base64-encoded (RFC 3548) stream with NULL ending -- output buffer must have capacity for (((length + 2) / 3) * 4) + 1 bytes
int EncodeBase64(char *output, const unsigned char *input, int length)
{
    unsigned short value;
    unsigned char bitcount;
    int count;
    int i;

    if (!output || !input) { return 0; }
    count = 0;
    value = 0; 
    bitcount = 0;
    for (i = 0; i < length; i++)
    {
        unsigned char c;
        
        // Add next byte into accumulator
        value = (value << 8) | input[i];
        bitcount += 8;
        
        // End of stream padding to next 6-bit boundary
        if (i + 1 >= length)
        {
            char boundary = ((bitcount + 5) / 6) * 6;
            value <<= (boundary - bitcount);
            bitcount = boundary;
        }

        // While we have 6-bit values to write
        while (bitcount >= 6)
        {
            // Get highest 6-bits and remove from accumulator
            bitcount -= 6;
            c = (unsigned char)((value >> bitcount) & 0x3f);

#if 1
            {
                const static char base64lookup[64] = 
                {
                    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
                    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
                    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
                    '+', '/'
                };
                c = base64lookup[c];
            }    
#else

            // Convert highest 6-bits to an ASCII character
            if (c <= 25) { c = c + 'A'; }                           // 0-25 'A'-'Z'
            else if (c <= 51) { c = c - 26 + 'a'; }                 // 26-51 'a'-'z'
            else if (c <= 61) { c = c - 52 + '0'; }                 // 52-61 '0'-'9'
            else if (c == 62) { c = '+'; }                          // 62 '+'
            else if (c == 63) { c = '/'; }                          // 63 '/'
#endif
            
            // Output
            output[count++] = c;
        }
    }

    // Padding for correct Base64 encoding
    while ((count & 3) != 0) { output[count++] = '='; }

    // NULL ending (without incrementing count)
    output[count] = '\0';
    return count;
}



// Look up table for the day of the month (this table is safe for invalid 4-bit months 0, 13, 14 & 15).
static const unsigned short daysBeforeMonth[16] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 365, 365 };


// Return the day number of the year (0-based), given a year, month (1-based), and day-of-month (1-based)
unsigned short DayOfYear(unsigned short year, unsigned char month, unsigned char day)
{
    unsigned short dayOfYear = daysBeforeMonth[month] + (day - 1);

    // If this year is a leap year, and we're after February, include the leap day
    if (((year & 0x03) == 0) && (month >= 3)) { dayOfYear++; }

    return dayOfYear;
}


// Return the day since the epoch (1st Jan 2000)
unsigned short DaysSinceEpoch(unsigned short year, unsigned char month, unsigned char day)
{
    unsigned short daysSinceEpoch;

    // Years can be passed 00-63, but if 2000-2063, re-base to zero
    if (year >= 2000) { year -= 2000; }

    // Calculate the total number of days in all the years before this one, including the leap days (within the range 2000-2099, this is every year which is divisible by 4)
    daysSinceEpoch = (year * 365) + ((year + 3) >> 2);

    // Add the days from this year
    daysSinceEpoch += DayOfYear(year, month, day);

    return daysSinceEpoch;
}


// Return the number of seconds since the epoch (2000-01-01 00:00:00)
unsigned long SecondsSinceEpoch(unsigned short year, unsigned char month, unsigned char day, unsigned char hours, unsigned char minutes, unsigned char seconds)
{
    unsigned long s;
    s = DaysSinceEpoch(year, month, day);           // Days
    s = s * 24 + hours;                              // Total hours
    s *= 3600ul;                                    // Convert to seconds
    s += (60 * (unsigned short)minutes) + seconds;  // Add seconds from minutes and seconds
    return s;
}


/*
// If not using the macro
#ifndef SecondsSinceEpoch_DateTime
// Return the number of seconds since the epoch (2000-01-01 00:00:00)
unsigned long SecondsSinceEpoch_DateTime(unsigned long packed)
{
    static const unsigned short daysBeforeMonth[16] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 365, 365 };
    // Unpack
    unsigned char year    = ((unsigned char)((packed >> 26) & 0x3f));
    unsigned char month   = ((unsigned char)((packed >> 22) & 0x0f));
    unsigned char day     = ((unsigned char)((packed >> 17) & 0x1f));
    unsigned char hours   = ((unsigned char)((packed >> 12) & 0x1f));
    unsigned char minutes = ((unsigned char)((packed >>  6) & 0x3f));
    unsigned char seconds = ((unsigned char)((packed      ) & 0x3f));
    unsigned short d;    // days since epoch
    unsigned long t;    // seconds since epoch

    // Calculate the total number of days in all the years before this one, including the leap days (within the range 2000-2099, this is every year which is divisible by 4)
    d = (year * 365) + ((year + 3) >> 2);

    // Add the number of days from the start of this year (include the leap day after February on leap years)
    d += daysBeforeMonth[month] + (day - 1) + ((((year & 0x03) == 0) && (month >= 3)) ? 1 : 0);

    // Convert days to seconds and add the time of day
    t = ((unsigned long)d * 24 + hours) * 3600ul + (60 * (unsigned short)minutes) + seconds;
    
    return t;
}
#endif
*/


/*
// Compliments the SecondsSinceEpoch_DateTime() macro.
// Convert a timestamp of seconds since the epoch (1/1/2000) to a packed date/time
unsigned long DateTimeFromEpoch(unsigned long epochSeconds)
{
    //unsigned char weekday = (unsigned char)(((unsigned short)(epochSeconds / 86400) + 6) % 7);      // (The epoch is a Saturday) 0 = Sunday, ...
    unsigned char year, month, day, hours, mins, secs;
    unsigned long t;
    unsigned short d;

    // Calculate time parts
    t = epochSeconds;
    secs  = (char)(t % 60); t = t / 60;
    mins  = (char)(t % 60); t = t / 60;
    hours = (char)(t % 24); t = t / 24;
    d = (unsigned short)t;                // Days since epoch

    // Special case dates before the cycle calculation starts on 1 March 2000 (otherwise it's a big mess with our unsigned numbers)
    if (d < 60)
    {
        if (d < 31) { year = 0; month = 1; day = (char)(d + 1); }
        else        { year = 0; month = 2; day = (char)(d - 30); }
    }
    else
    {
        static const unsigned short daysBeforeMonthFromMarch[12] = { 0, 31, 61, 92, 122, 153, 184, 214, 245, 275, 306, 337 };
        unsigned short cycle;
        int i;

        // Align to days since 1-Mar-2000
        d -= 60;
        
        // NOTE: Although our input date range does include the leap year exception of 2100, the limit of our packed representation is 2063, so this code ignores this.
        // Calculate cycle number and the day within the cycle (4 * 365 + 1 days per cycle)
        cycle = (unsigned short)(d / 1461);
        d = d % 1461;

        // Calculate year (leap day after end of 4-year cycle), and day (from 1 March) of the year
        year = 4 * cycle;
             if (d >= 3 * 365) { d -= 3 * 365; year += 3; }
        else if (d >= 2 * 365) { d -= 2 * 365; year += 2; }
        else if (d >=     365) { d -=     365; year += 1; }

        // Calculate the month from the day of the year (from 1 March)
        for (i = 11; i >= 0; i--)
        {
            if (d >= daysBeforeMonthFromMarch[i])
            {
                day = (char)(d - daysBeforeMonthFromMarch[i]) + 1;    // Days are 1-based
                month = i + 3;                                        // Start on March
                if (month > 12) { month -= 12; year++; }            // Wrap January/February to the following year
                break;
            }
        }
    }
    
    // Return packed values
    return ((unsigned long)year  << 26) 
         | ((unsigned long)month << 22) 
         | ((unsigned long)day   << 17) 
         | ((unsigned long)hours << 12) 
         | ((unsigned long)mins  <<  6)
         | (               secs       );
}
*/


