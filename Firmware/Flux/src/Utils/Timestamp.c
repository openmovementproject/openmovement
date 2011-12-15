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

// Serial timestamp functions
// Dan Jackson, 2010-2011

#include <stdlib.h>
#include <stdio.h>

#include "Utils/Timestamp.h"


// Months                                                                      01,    02,    03,    04,    05,    06,    07,    08,    09,    10,    11,    12
static const unsigned char *MonthName[TIMESTAMP_MONTHS_PER_YEAR + 1]   = { "---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const unsigned char DaysPerMonth[TIMESTAMP_MONTHS_PER_YEAR + 1] = {     0,    31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };
static const unsigned short DaysBeforeMonthFromMarch[TIMESTAMP_MONTHS_PER_YEAR] =                 { 0,    31,    61,    92,   122,   153,   184,   214,   245,   275,   306,   337 };

// Day of week                                                         00,    01,    02,    03,    04,    05,    06
static const unsigned char *DayName[TIMESTAMP_DAYS_PER_WEEK]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

// Temporary string
static char timestampString[TIMESTAMP_MAX_STRING];

// (Private) number of days in a specified month and year
static int DaysInMonth(int month, int year)
{
    while (month <= 0) { month += TIMESTAMP_MONTHS_PER_YEAR; year--; }
    year += (month - 1) / 12; month = (month - 1) % 12 + 1;
    if (year < TIMESTAMP_START_YEAR || (year == TIMESTAMP_START_YEAR && month <= 2)) { return 0; }
    if (month == 2 && (year % 4 == 0) && (!(year % 100 == 0) || (year % 400 == 0))) { return DaysPerMonth[month] + 1; }
    return DaysPerMonth[month];
}


// Convert a timestamp to date and time components (out param pointers can be NULL if you don't want the return value)
void TimestampToYMDHMS(Timestamp timestamp, unsigned short *outYear, unsigned char *outMonth, unsigned char *outDay, unsigned char *outWeekday, unsigned char *outHours, unsigned char *outMins, unsigned char *outSecs)
{
    unsigned short year = 0;
    unsigned char month = 0, day = 0, weekday = 0;
    unsigned char hours = 0, mins = 0, secs = 0, fracs = 0;
    unsigned short cycle;
    unsigned short offset;
    unsigned long v;
    int i;

    // Initial time value is from the passed-in Timestamp
    v = (timestamp % TIMESTAMP_TICKS_PER_DAY);
#ifdef TIMESTAMP_HAS_TIME
    fracs =  (v % TIMESTAMP_TIME_TICKS_PER_SECOND);
    v /= TIMESTAMP_TIME_TICKS_PER_SECOND;
#endif
    hours = (char)(v / 60 / 60 % 24);
    mins = (char)(v / 60 % 60);
    secs = (char)(v % 60);

    // Initial date value is the number of days in the passed in Timestamp
    v = (timestamp / TIMESTAMP_TICKS_PER_DAY);
    weekday = (v + 3) % 7;      // The epoch is a Wednesday(!)

    // The only leap year exception in our date range (2000 to 2136 or 2179) is the year 2100
#if TIMESTAMP_START_YEAR != 2000
#error "The Timestamp offset calculation will need fixing for a start year other than 2000"
#endif
    offset = (unsigned short)((v / 36524L) - (v / (4 * 36524L)));
    v += offset;

    // Calculate cycle number and the day within the cycle
    cycle = (short)(v / TIMESTAMP_DAYS_PER_CYCLE);
    v = (v % TIMESTAMP_DAYS_PER_CYCLE);

    // Calculate year (leap day after end of 4-year cycle)
    year = TIMESTAMP_START_YEAR + TIMESTAMP_YEARS_PER_CYCLE * cycle;
    for (i = 0; i < (TIMESTAMP_YEARS_PER_CYCLE - 1); i++)
    {
        if (v >= TIMESTAMP_DAYS_PER_YEAR) { v -= TIMESTAMP_DAYS_PER_YEAR; year++; }
    }

    for (i = 11; i >= 0; i--)
    {
        if (v >= DaysBeforeMonthFromMarch[i])
        {
            day = (char)(v - DaysBeforeMonthFromMarch[i]) + 1;
            month = i + 3;
            if (month > 12) { month -= 12; year++; }
            break;
        }
    }

    if (outYear != NULL) { *outYear = year; }
    if (outMonth != NULL) { *outMonth = month; }
    if (outDay != NULL) { *outDay = day; }
    if (outWeekday != NULL) { *outWeekday = weekday; }
    if (outHours != NULL) { *outHours = hours; }
    if (outMins != NULL) { *outMins = mins; }
    if (outSecs != NULL) { *outSecs = secs; }
    return;
}


// Create a timestamp serial number from date and time components
Timestamp TimestampFromYMDHMS(unsigned short year, unsigned char month, unsigned char day, unsigned char hours, unsigned char mins, unsigned char secs)
{
    Timestamp timestamp = 0;
    unsigned short cycle;
    unsigned short offset;

    if (year >= TIMESTAMP_START_YEAR) { year -= TIMESTAMP_START_YEAR; }          // Start year
    else if (year >= 200) { return 0; }
    if (month == 0 || day == 0) { return 0; }
    month--;                                    // month base 0
    day--;                                      // day base 0
    if (month < 2)
    {
        if (year == 0) { return 0; }
        year--; month += 10;
    } else { month -= 2; }

#if TIMESTAMP_START_YEAR != 2000
#error "The Timestamp offset calculation will need fixing for a start year other than 2000"
#endif
    offset = (year / 100) - (year / 400);

    cycle = year / TIMESTAMP_YEARS_PER_CYCLE;    // cycle number
    year = year % TIMESTAMP_YEARS_PER_CYCLE;     // year within cycle

    timestamp = (unsigned long)TIMESTAMP_TICKS_PER_DAY * ((unsigned long)cycle * TIMESTAMP_DAYS_PER_CYCLE + (unsigned long)year * TIMESTAMP_DAYS_PER_YEAR + (unsigned long)DaysBeforeMonthFromMarch[month] + (unsigned long)day - offset);
#ifdef TIMESTAMP_HAS_TIME
    timestamp += (((((unsigned long)hours * 60L) + (unsigned long)mins) * 60L) + (unsigned long)secs) * (unsigned long)TIMESTAMP_TIME_TICKS_PER_SECOND; 
#endif
    return timestamp;
}


// Convert a timestamp serial number to a string using the specified format
char *TimestampToString(Timestamp timestamp, unsigned short format)
{
    char *c = timestampString;
    unsigned short year;
    unsigned char month, day, weekday, hours, mins, secs;
    char dateSep = '-';

    // Debug hex dump of value
    if (format & TIMESTAMP_FORMAT_DEBUG_HEX)
    {
        int nibble;
        unsigned char v;
        for (nibble = 7; nibble >= 0; nibble--)
        {
            v = ((timestamp >> (nibble << 2)) & 0xf);
            if (v <= 9) { *c++ = '0' + v; }
            else { *c++ = 'A' + v - 10; }
        }
        *c++ = '\0';
        return timestampString;
    }

    // Convert to YMD/HMS
    TimestampToYMDHMS(timestamp, &year, &month, &day, &weekday, &hours, &mins, &secs);

    if (format & TIMESTAMP_FORMAT_WEEKDAY)
    {
        *c++ = DayName[weekday][0];
        *c++ = DayName[weekday][1];
        if (!(format & TIMESTAMP_FORMAT_WEEKDAY_SHORT)) { *c++ = DayName[weekday][2]; }
        if ((format & ~TIMESTAMP_FORMAT_WEEKDAY) != 0) { *c++ = ' '; }
    }

    if (format & TIMESTAMP_FORMAT_DAY)
    {
        *c++ = '0' + ((day    / 10) % 10);
        *c++ = '0' + ((day        ) % 10);
        if ((format & TIMESTAMP_FORMAT_MONTH_NAME) != 0 || (format & TIMESTAMP_FORMAT_MONTH_NUMERIC) != 0) { *c++ = dateSep; }
    }

    if ((format & TIMESTAMP_FORMAT_MONTH_NAME) != 0 || (format & TIMESTAMP_FORMAT_MONTH_NUMERIC) != 0)
    {
        if (format & TIMESTAMP_FORMAT_MONTH_NAME)
        {
            *c++ = MonthName[month][0];
            *c++ = MonthName[month][1];
            *c++ = MonthName[month][2];
            if (format & TIMESTAMP_FORMAT_MONTH_NUMERIC) { *c++ = ' '; }
        }
        if (format & TIMESTAMP_FORMAT_MONTH_NUMERIC)
        {
            if (format & TIMESTAMP_FORMAT_MONTH_NAME) { *c++ = '('; }
            *c++ = '0' + ((month /   10) % 10);
            *c++ = '0' + ((month       ) % 10);
            if (format & TIMESTAMP_FORMAT_MONTH_NAME) { *c++ = ')'; }
        }
        if ((format & TIMESTAMP_FORMAT_YEAR) != 0) { *c++ = dateSep; }
    }

    if (format & TIMESTAMP_FORMAT_YEAR)
    {
        if (!(format & TIMESTAMP_FORMAT_TWO_DIGIT_YEAR))
        {
            *c++ = '0' + ((year / 1000) % 10);
            *c++ = '0' + ((year /  100) % 10);
        }
        *c++ = '0' + ((year /   10) % 10);
        *c++ = '0' + ((year       ) % 10);
        if (format & TIMESTAMP_FORMAT_TIME) { *c++ = ' '; }
    }

    if (format & TIMESTAMP_FORMAT_HOURS)
    {
        *c++ = '0' + ((hours  / 10) % 10);
        *c++ = '0' + ((hours      ) % 10);
    }

    if (format & TIMESTAMP_FORMAT_MINUTES)
    {
        if ((format & TIMESTAMP_FORMAT_HOURS) && !(format & TIMESTAMP_FORMAT_NO_TIME_SEP)) { *c++ = ':'; }
        *c++ = '0' + ((mins   / 10) % 10);
        *c++ = '0' + ((mins       ) % 10);
    }

    if (format & TIMESTAMP_FORMAT_SECONDS)
    {
        if ((format & TIMESTAMP_FORMAT_MINUTES) && !(format & TIMESTAMP_FORMAT_NO_TIME_SEP)) { *c++ = ':'; }
        *c++ = '0' + ((secs   / 10) % 10);
        *c++ = '0' + ((secs       ) % 10);
    }

    *c++ = '\0';
    return timestampString;
}


// Add date and time components to a timestamp serial number
Timestamp TimestampAdd(Timestamp timestamp, int addYear, int addMonth, int addDay, int addHours, int addMins, int addSecs)
{
    unsigned short year;
    unsigned char month, day, hours, mins, secs;
    Timestamp delta = 0;

    // Get current date and time (we need to know the year/month)
    TimestampToYMDHMS(timestamp, &year, &month, &day, NULL, &hours, &mins, &secs);

    // Convert year additions into month additions
    addMonth += TIMESTAMP_MONTHS_PER_YEAR * addYear;
    addYear = 0;

    // Convert month additions into required day additions
    if (addMonth >= 0)
    {
        for (; addMonth > 0; addMonth--)
        {
            addDay += DaysInMonth(month, year);
            month++;
            if (month > 12) { month = 1; year++; }
        }
    }
    else
    {
        for (; addMonth < 0; addMonth++)
        {
            month--;
            if (month <= 0) { month = 12; year--; } 
            addDay -= DaysInMonth(month, year);
        }
    }

    timestamp = (Timestamp)((long)timestamp + ((long)addDay * TIMESTAMP_TICKS_PER_DAY));
#ifdef TIMESTAMP_HAS_TIME
    timestamp += (((((long)addHours * 60) + addMins) * 60) + addSecs) * TIMESTAMP_TIME_TICKS_PER_SECOND;
#endif

    return timestamp;
}


// Convert a DateTime packed number to a timestamp
Timestamp TimestampFromDateTime(unsigned long datetime)
{
	unsigned char year  = ((unsigned char)((datetime >> 26) & 0x3f));
	unsigned char month = ((unsigned char)((datetime >> 22) & 0x0f));
	unsigned char day   = ((unsigned char)((datetime >> 17) & 0x1f));
	unsigned char hours = ((unsigned char)((datetime >> 12) & 0x1f));
	unsigned char mins  = ((unsigned char)((datetime >>  6) & 0x3f));
	unsigned char secs  = ((unsigned char)((datetime      ) & 0x3f));
	return TimestampFromYMDHMS(year, month, day, hours, mins, secs);
}


// Convert a Timestamp to a DateTime packed number
unsigned long TimestampToDateTime(Timestamp timestamp)
{
    unsigned short year;
    unsigned char month, day, hours, mins, secs;
    TimestampToYMDHMS(timestamp, &year, &month, &day, NULL, &hours, &mins, &secs);
	return (((unsigned long)(year % 100) & 0x3f) << 26) | (((unsigned long)(month) & 0x0f) << 22) | (((unsigned long)(day) & 0x1f) << 17) | (((unsigned long)(hours) & 0x1f) << 12) | (((unsigned long)(mins) & 0x3f) <<  6) | ((unsigned long)(secs) & 0x3f);
}


// For debugging: round-trip conversion function to and from a timestamp serial (should return same value)
Timestamp TimestampFromTimestampDebug(Timestamp timestamp)
{
    unsigned short year;
    unsigned char month, day, hours, mins, secs;
    TimestampToYMDHMS(timestamp, &year, &month, &day, NULL, &hours, &mins, &secs);
    return TimestampFromYMDHMS(year, month, day, hours, mins, secs);
}
