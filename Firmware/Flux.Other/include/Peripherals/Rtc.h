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

// Real Time Clock
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef _RTC_H
#define _RTC_H


// Microchip RTCC support
#if defined(__18CXX) || defined(__C30__) || defined( __C32__ ) || defined(__PIC32MX__)

// Implementation-specific
#include "Compiler.h"


// HACK: Fixes the fact that Rtcc.h on dsPIC includes Generic.h with redefinitions of some generic types
#ifdef __dsPIC33E__
#include "GenericTypeDefs.h"
#define _GENERIC_H_
#endif


#include "HardwareProfile.h"

#ifdef __PIC24FJ1024GB606__ // KL: From code configurator
// KL: .h Not required
// union/structure for read/write of time and date from/to the RTCC device
typedef union
{ 
    struct
    {
        BYTE    year;       // BCD codification for year, 00-99
        BYTE    rsvd;       // reserved for future use
        BYTE    mday;       // BCD codification for day of the month, 01-31
        BYTE    mon;        // BCD codification for month, 01-12
        BYTE    hour;       // BCD codification for hours, 00-24
        BYTE    wday;       // BCD codification for day of the week, 00-06   
        BYTE    sec;        // BCD codification for seconds, 00-59 
        BYTE    min;        // BCD codification for minutes, 00-59
    }f;                     // field access
    BYTE        b[8];       // BYTE access
    unsigned short int      w[4];       // 16 bits access
    unsigned long int      l[2];       // 32 bits access
}rtccTimeDate;
#else
#include <Rtcc.h>
#endif
#ifdef __C30__
	typedef rtccTimeDate rtcInternalTime;

#elif defined( __C32__ )
	typedef union {
		struct {rtccTime time;rtccDate date;
		};
		unsigned long l[2];
    }rtcInternalTime;
	
#else 
	#warning "Define date time structure used"
#endif

#else
    // Other platform (e.g. PC), dummy elements to compile
    #define RTC_NO_INTERNAL_FORMAT
	typedef union {
		unsigned long l[2];
    } rtcInternalTime;
    #define RTC_IPL_shadow_t extern char
    #define RTC_INTS_DISABLE()	{;}
    #define RTC_INTS_ENABLE()	{;}
    #ifdef _WIN32
        #define inline _inline
    #endif
#endif


// DateTime - a single long integer date/time value
/*
    typedef union
    {
        unsigned long value;                // 32-bit
        struct
        {
            unsigned char year    : 6;      // Wraps every 64 years
            unsigned char month   : 4;      // 1-12
            unsigned char day     : 5;      // 1-31
            unsigned char hours   : 5;      // 0-23
            unsigned char minutes : 6;      // 0-59
            unsigned char seconds : 6;      // 0-59
        };
    } DateTime;
*/

// Interrupt mask protection functions - used to block data stream interrupts
#ifdef __C30__
	typedef unsigned char RTC_IPL_shadow_t;
	#define RTC_INTS_DISABLE()	{IPLshadow = SRbits.IPL; if(SRbits.IPL<RTC_INT_PRIORITY)SRbits.IPL=RTC_INT_PRIORITY;}
	#define RTC_INTS_ENABLE()	{SRbits.IPL = IPLshadow;}
#elif defined(__C32__)
	typedef unsigned long RTC_IPL_shadow_t;
	#define RTC_INTS_DISABLE()	{IPLshadow = _CP0_BCS_STATUS((_CP0_STATUS_IPL_MASK&((~RTC_INT_PRIORITY)<<_CP0_STATUS_IPL_POSITION)),(_CP0_STATUS_IPL_MASK&(RTC_INT_PRIORITY<<_CP0_STATUS_IPL_POSITION)));} // Assign IPL equal to RTC_INT_PRIORITY
	#define RTC_INTS_ENABLE()	{_CP0_BCS_STATUS((_CP0_STATUS_IPL_MASK&(~IPLshadow)),(_CP0_STATUS_IPL_MASK&IPLshadow));}
#endif
//#define RTC_INTS_DISABLE()	{IEC3bits.RTCIE = 0;IEC0bits.T1IE = 0;}
//#define RTC_INTS_ENABLE()	{IEC3bits.RTCIE = 1;IEC0bits.T1IE = 1;}



// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
typedef unsigned long DateTime;
#define DATETIME_FROM_YMDHMS(_year, _month, _day, _hours, _minutes, _seconds) ( (((unsigned long)(_year % 100) & 0x3f) << 26) | (((unsigned long)(_month) & 0x0f) << 22) | (((unsigned long)(_day) & 0x1f) << 17) | (((unsigned long)(_hours) & 0x1f) << 12) | (((unsigned long)(_minutes) & 0x3f) <<  6) | ((unsigned long)(_seconds) & 0x3f) )
#define DATETIME_YEAR(_v)       ((unsigned char)(((_v) >> 26) & 0x3f))
#define DATETIME_MONTH(_v)      ((unsigned char)(((_v) >> 22) & 0x0f))
#define DATETIME_DAY(_v)        ((unsigned char)(((_v) >> 17) & 0x1f))
#define DATETIME_HOURS(_v)      ((unsigned char)(((_v) >> 12) & 0x1f))
#define DATETIME_MINUTES(_v)    ((unsigned char)(((_v) >>  6) & 0x3f))
#define DATETIME_SECONDS(_v)    ((unsigned char)(((_v)      ) & 0x3f))

// Valid range
#define DATETIME_MIN DATETIME_FROM_YMDHMS(2000,1,1,0,0,0)
#define DATETIME_MAX DATETIME_FROM_YMDHMS(2063,12,31,23,59,59)
#define DATETIME_INVALID 0xffffffff

// Fractional seconds to milliseconds
#define RTC_FRACTIONAL_TO_MS(_t) ((unsigned short)((1000UL * (_t)) >> 16))

// Directly reads the RTC (WARNING: RTC interrupts must not be on)
#ifdef __C30__
	#define RtcRead(_ptr) RtccReadTimeDate(_ptr)
#endif
#define RtcClear() RtcWrite(DATETIME_MIN)

// Initialize the RTC
void RtcStartup(void);

// Reads the current (cached) date/time (interrupts enabled)
DateTime RtcNow(void);

// Calibrate the RCT to speed up (+ive value) or slow down (-ive value) - send signed int in ppm
void RtcCal(signed short);

// Reads the current (cached) date/time (interrupts enabled) and a fractional part of a second (1/65536th of a second)
DateTime RtcNowFractional(unsigned short *fractional);

// The current second count since start (wraps every 18.2 hours)
extern volatile unsigned short rtcTicksSeconds;
#define RtcSeconds() (rtcTicksSeconds)

// Reads the current second count since start (wraps every 18.2 hours) as a 16.16 fixed point number, updated every 30.5 us
unsigned long RtcTicks(void);

// Directly reads the RTC (WARNING: RTC interrupts must not be on)
void RtcRead(rtcInternalTime *time);

// Writes the specified date/time from the RTC
void RtcWrite(DateTime value);

// Convert a date/time number from a string ("YY/MM/DD,HH:MM:SS")
DateTime RtcFromString(const char *value);

// Convert a date/time number to a string ("YY/MM/DD,HH:MM:SS")
const char *RtcToString(DateTime value);

// Convert a date/time from the implementation-specific representation
DateTime RtcFromInternal(rtcInternalTime *value);

// Convert a date/time to the implementation-specific representation
rtcInternalTime *RtcToInternal(DateTime value);

// Add/subtract a second offset from a DateTime ~1000 opperations
DateTime RtcCalcOffset(DateTime start, signed long seconds);

// Initialize RTC
//void RtcClear(void);

// Increment time by 1 second
//DateTime RtcIncrement(DateTime value);


// RTC interrupts on
void RtcInterruptOn(unsigned short newRate);

// RTC interrupts off
void RtcInterruptOff(void);

/*
// Sets up the periodic timer on the RTC to pulse the interrupt line low every time the interval (seconds or minutes) is reached
void RtcSetupTimer(unsigned char interval, char multipliedBySixty);

// Sets up the timer on the RTC to pulse the interrupt line low when the time is reached
void RtcSetupAlarm(DateTime value);
*/

extern volatile unsigned short rtcTicksSeconds;        // Seconds (wraps every 18.2 hours)


//extern void SetTime(void);		// Call to update the module from the globals
//extern void UpdateTime(void);	// Call to update the globals
//extern void ClearRTC(void);
//extern void SetupAlarm (void);
//extern void SetupChimeOn (void); // Note: Chime enable just lets the alarm keep repeating indefinitely, still need one alarm to start it.
//extern void SetupChimeOff (void);
//extern void TurnOnRTCInt(void);
//extern void TurnOffRTCInt(void);


//extern void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void);
// (internal update function)
inline void RtcTasks(void);

//extern void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
// (internal update function)
inline void RtcTimerTasks(void);


#if (defined(RTC_SWWDT) || defined(RTC_SWWDT_TIMEOUT))
	extern volatile unsigned int rtcSwwdtValue;
	// These functions are actually defines to ensure they're actually inline
	//#warning "SW WDT ENABLED"
	#define RtcSwwdtReset() { rtcSwwdtValue = 0; }
	#define RtcSwwdtIncrement() { if (++rtcSwwdtValue >= RTC_SWWDT_TIMEOUT) { LED_SET(LED_MAGENTA); DelayMs(1000); Reset(); } }
#else
	#define RtcSwwdtReset() 	{}
	#define RtcSwwdtIncrement() {}
#endif


#endif
