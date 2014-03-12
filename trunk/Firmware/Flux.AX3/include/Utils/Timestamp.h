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

// Serial timestamp functions
// Dan Jackson, 2010-2012

#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

// 'Timestamp' is the time elapsed since the epoch: 01-Jan-2000 00:00:00
//
// If TIMESTAMP_HAS_TIME is set, Timestamp is a:
//   - 32-bit count of the number of seconds since the epoch, representing time up to 06-Feb-2136 06:28:15
// otherwise, Timestamp is a:
//   - 16-bit count of the number of days since the epoch, representing dates only up to 06-Jun-2179
#define TIMESTAMP_HAS_TIME


#ifdef TIMESTAMP_HAS_TIME

// A 32-bit type for representing the seconds elapsed since the epoch (represents date and time for full years 2001-2135) 
typedef unsigned long timestamp_t;
#define TIMESTAMP_TICKS_PER_DAY (60L * 60L * 24L * TIMESTAMP_TIME_TICKS_PER_SECOND)   // 86400
// Increase the number of ticks per second to represent fractions of a second (will drastically lower range)
#define TIMESTAMP_TIME_TICKS_PER_SECOND 1L       // It's best that this is 1

#define TIMESTAMP_MIN_VALID 0x00000001		//  1-Jan-2000 00:00:01
#define TIMESTAMP_MAX_VALID 0xBC19137F		// 31-Dec-2099 23:59:59

#else

// A 16-bit type for representing the days elapsed since the epoch (cannot represent time, represents days in full years 2001-2178)
typedef unsigned short timestamp_t;
#define TIMESTAMP_TICKS_PER_DAY 1

#endif

// Offset from Unix epoch (1-Jan-1970)
#define TIMESTAMP_UNIX_OFFSET (10957L * TIMESTAMP_TICKS_PER_DAY)

// Absolute min/max values
#define TIMESTAMP_MIN ((timestamp_t)0)
#define TIMESTAMP_MAX ((timestamp_t)-1)

#define TIMESTAMP_MAX_STRING 32 // "ddd dd MM (MMM) yyyy, hh:mm:ss"

// String formatting flags
#define TIMESTAMP_FORMAT_NONE            0x0000
#define TIMESTAMP_FORMAT_WEEKDAY         0x0001
#define TIMESTAMP_FORMAT_DAY             0x0002
#define TIMESTAMP_FORMAT_MONTH_NAME      0x0004
#define TIMESTAMP_FORMAT_MONTH_NUMERIC   0x0008
#define TIMESTAMP_FORMAT_YEAR            0x0010
#define TIMESTAMP_FORMAT_HOURS           0x0020
#define TIMESTAMP_FORMAT_MINUTES         0x0040
#define TIMESTAMP_FORMAT_SECONDS         0x0080
#define TIMESTAMP_FORMAT_TWO_DIGIT_YEAR  0x0100
#define TIMESTAMP_FORMAT_NO_TIME_SEP     0x0200
#define TIMESTAMP_FORMAT_WEEKDAY_SHORT   0x0400
#define TIMESTAMP_FORMAT_DEBUG_HEX       0x8000
#define TIMESTAMP_FORMAT_TIME    (TIMESTAMP_FORMAT_HOURS | TIMESTAMP_FORMAT_MINUTES | TIMESTAMP_FORMAT_SECONDS)
#define TIMESTAMP_FORMAT_DATE    (TIMESTAMP_FORMAT_WEEKDAY | TIMESTAMP_FORMAT_DAY | TIMESTAMP_FORMAT_MONTH_NAME | TIMESTAMP_FORMAT_YEAR)
#define TIMESTAMP_FORMAT_SHORT_DATE (TIMESTAMP_FORMAT_DAY | TIMESTAMP_FORMAT_MONTH_NAME | TIMESTAMP_FORMAT_YEAR | TIMESTAMP_FORMAT_TWO_DIGIT_YEAR)
#define TIMESTAMP_FORMAT_DATE_TIME  (TIMESTAMP_FORMAT_DATE | TIMESTAMP_FORMAT_WEEKDAY_SHORT | TIMESTAMP_FORMAT_HOURS | TIMESTAMP_FORMAT_MINUTES | TIMESTAMP_FORMAT_SECONDS)


// Convert a date/time to date and time components (out param pointers can be NULL if you don't want the return value)
void TimestampToYMDHMS(timestamp_t timestamp, unsigned short *year, unsigned char *month, unsigned char *day, unsigned char *weekday, unsigned char *hours, unsigned char *mins, unsigned char *secs);

// Create a date/time number from date and time components
timestamp_t TimestampFromYMDHMS(unsigned short year, unsigned char month, unsigned char day, unsigned char hours, unsigned char mins, unsigned char secs);

// Convert a date/time number to a string using the specified format
const char *TimestampToString(timestamp_t timestamp, unsigned short format);

// Add date and time components to a date/time number
timestamp_t TimestampAdd(timestamp_t timestamp, int addYear, int addMonth, int addDay, int addHours, int addMins, int addSecs);

// Convert a DateTime packed number to a timestamp
timestamp_t TimestampFromDateTime(unsigned long datetime);

// Convert a Timestamp to a DateTime packed number
unsigned long TimestampToDateTime(timestamp_t timestamp);


// For debugging: round-trip conversion function to and from a date/time (should return same value)
timestamp_t TimestampFromTimestampDebug(timestamp_t timestamp);

#endif // _TIMESTAMP_H_
