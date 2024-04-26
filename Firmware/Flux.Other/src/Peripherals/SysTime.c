/*
	Multi-function generic time/timing module
	This is the hardware in-specific source part

	Co-existing real time clock calendar and timer for:
		- Arbitrary timebase generator (e.g. sample timer)
		- Time stamps (in epochs or packed/unpacked date-times) 
		- Calendar functions (e.g. for scheduled events)
		- Blocking delays in micro or milli-seconds
	
	Time stamp format options:
		- Epoch
		- Time structure
		- Packed time
	
	Further info at end of this file

	Other options to be configured in config.h: 
		SYSTIME_NUM_SECOND_CBS			- Allocated resources
		SYSTIME_NUM_PERIODIC_CBS		- Allocated resources		
		SYS_TICK_RATE					- Tick frequency
		DELAY_US(_d)					- Optional external DelayUs(uint32_t d)
		FUNCTION_CALL_CYCLE_OVERHEAD	- Function call overhead (unless above defined)
		DECREMENT_LOOP_CYCLE_COUNT		- Loop decrement instructions (unless above defined)
		GetSystemMIPS()					- Instruction clock function (instructions per uS) (unless above defined)

	Karim Ladha, 2015 & later, Revisions:	
	2015-01-21. First written
	2015-07-03. Linked to the scheduler module
	2015-07-10. Minor comment changes
	2015-12-06. Splitting hardware specific parts into new files
	2016-04-05. Minor comment and warning changes
*/

// Includes
#include <stdint.h>
#include <string.h>
#include "Peripherals/SysTime.h"
#include "config.h"

// Globals
#ifdef SWWDT_TIMEOUT
volatile uint16_t swwdtCount = SWWDT_TIMEOUT;
#endif

// Private Constants
const uint8_t daysInMonth[12+1]		= {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31		};
const uint16_t daysBeforeMonth[12+1]	= {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334	};

// Callback function variables
#if defined(SYSTIME_NUM_SECOND_CBS) && (SYSTIME_NUM_SECOND_CBS > 0)
const uint16_t sysTimeNumSecondCbs = SYSTIME_NUM_SECOND_CBS;
SysTimeCb_t secondCBs[SYSTIME_NUM_SECOND_CBS+1] = {0}; // Null terminated list
#endif
#if defined(SYSTIME_NUM_PERIODIC_CBS) && (SYSTIME_NUM_SECOND_CBS > 0)
const uint16_t sysTimeNumPeriodicCbs = SYSTIME_NUM_PERIODIC_CBS;
SysTimeCb_t periodCBs[SYSTIME_NUM_PERIODIC_CBS] = {0}; // Usually hardware limited
#endif
#ifndef SYS_TIME_VARS_EXTERNAL
// Current date and time variables
EpochTime_t epochTime;
DateTime_t dateTime;
PackedTime_t packedTime;
#endif

// Private internal prototypes
inline uint8_t __attribute__((always_inline))IsLeap(uint16_t year);

// Source
// Software watch dog timeout function, incremented in the ISR
inline void __attribute__((always_inline)) SwwdtTasks(void)
{
	#ifdef SWWDT_TIMEOUT
	if(--swwdtCount == 0)DebugReset(SW_WDT);
	#endif
}
// The reset timer function called periodically in application
inline void __attribute__((always_inline)) SwwdtReset(void)
{
	#ifdef SWWDT_TIMEOUT
	swwdtCount = SWWDT_TIMEOUT;
	#endif
}
// Rtc structure increment by one second - installed into ISR
inline void __attribute__((always_inline))SysTimeDateTimeInc(void)
{
	// Use to sync DateTime_t + EpochTime_t
	DateTime_t* time = &dateTime;
	
	if(++time->seconds >= 60) 
	{
		uint8_t newDay = 0;
		
		time->seconds = 0;
		if(++time->minutes >= 60)
		{
			time->minutes = 0;
			if(++time->hours >= 24)
			{
				newDay = 1;
				time->hours = 0;
			}
		}
		
		if(newDay)
		{
			uint8_t extraDay = 0;
			if((time->month == 2) && (IsLeap(time->year)))
				extraDay = 1;
			if(++time->day > (daysInMonth[time->month]+extraDay))
			{
				time->day = 1;
				if(++time->month > 12)
				{
					time->month = 1;
					++time->year;
				}
			}
		}
		
		// Re-pack required (1/60 seconds)
		packedTime = SysTimeToPacked(time);	// &dateTime
	} 
	else
	{
		// For 59/60 seconds, we can just increment the packed time to the next second
		packedTime++;
	}
}
// Blocking millisecond delay
void SysTimeDelayMs(uint32_t ms)
{
	while(ms--)
	{
		SwwdtReset();
		SysTimeDelayUs(1000);
	}
}

// Blocking micro second delay - approximate
void SysTimeDelayUs(uint16_t us)
{
	#ifdef DELAY_US
		// User overridden function
		DELAY_US(us);
	#else
		volatile int32_t instructions;
		instructions = us * GetSystemMIPS();
		#if(FUNCTION_CALL_CYCLE_OVERHEAD < DECREMENT_LOOP_CYCLE_COUNT)
			#undef FUNCTION_CALL_CYCLE_OVERHEAD
			#define FUNCTION_CALL_CYCLE_OVERHEAD	DECREMENT_LOOP_CYCLE_COUNT
		#endif
		while(instructions > FUNCTION_CALL_CYCLE_OVERHEAD)
			instructions -= DECREMENT_LOOP_CYCLE_COUNT;
	#endif
}
// Epoch seconds for date and time since 2000/1/1,00:00:00) 
EpochTime_t SysTimeToEpoch(DateTime_t* time)
{
	uint32_t days, seconds;	// Since epoch date
	DateTime_t temp;
	// Get now if NULL provided - same as SysTimeEpoch()
	if(time == NULL)
		time = &dateTime;
	// Days in all years
	days = (time->year * 365) + ((time->year + 3) >> 2);
	// Add days from the start of this year, add leap day if present
	days += daysBeforeMonth[time->month] + (time->day - 1) + (IsLeap(time->year)&&(time->month > 2));
	// Make seconds from all days and current time
	seconds = ((uint32_t)days * 24 + time->hours) * 3600ul + (60 * (uint16_t)time->minutes) + time->seconds;
	return seconds;
}
// Date and time from epoch - optimised for processors with slow 32x32 divide 
DateTime_t* SysTimeFromEpoch(EpochTime_t epoch, DateTime_t* output)
{
	uint32_t unitLen;
	// Enum stage and index to DateTime_t as array
	enum {SM,MH,HD,DM,MY,YY,Y4}stage = Y4;
	// Check
	if(output == NULL)return NULL;
	// Knit struct, day and month correctly
	memset(output,0,sizeof(DateTime_t));
	output->month = output->day = 1;
	// Iterator for each field
	for(;;){switch(stage) {
			case Y4 : /* Count four year sets - divide by 126230400, max loops = 34 */
				unitLen = ((uint32_t)(4*365+1)*24*60*60); /* 4years + 1day */
				if(epoch >= unitLen){epoch -= unitLen;output->year+=4;}
				else stage--;
				continue;
			case YY : /* Count full years - divide by 31536000, max loops = 3 */
				if(IsLeap(output->year))	unitLen = ((uint32_t)366*24*60*60); /* Leap */
				else						unitLen = ((uint32_t)365*24*60*60); /* Normal */
				if(epoch >= unitLen){epoch -= unitLen;output->year++;}
				else stage--;
				continue;
			case MY : /* Count full months - divide by 2678400, max loops = 11 */
				if((output->month == 2)&&(IsLeap(output->year))) unitLen = ((uint32_t)29*24*60*60);	/* 29th Feb */
				else			unitLen = ((uint32_t)daysInMonth[output->month]*24*60*60);			/* Normal */
				if(epoch >= unitLen){epoch -= unitLen;output->month++;}
				else stage--;
				continue;
			case DM : /* Count full days - divide by 86400, max loops = 30 */
				unitLen = ((uint32_t)24*60*60);
				if(epoch >= unitLen){epoch -= unitLen;output->day++;}
				else stage--;
				continue;
			case HD : /* Count full hours - divide by 3600, max loops = 23 */
				unitLen = ((uint16_t)60*60);
				if(epoch >= unitLen){epoch -= unitLen;output->hours++;}
				else stage--;
				continue;
			case MH : /* Don't count minutes - now use faster 16 bit divide */
			case SM : /* Maximum remaining is 3600, set minutes and seconds */
			default : {
				uint16_t remaining = epoch; 
				output->minutes = remaining / 60;
				output->seconds = remaining % 60;
				break;
			}// Last case
		}// Switch
		break;
	}//for
	return output;
}
// Pack a date-time struct 
PackedTime_t SysTimeToPacked(DateTime_t* time)
{
	PackedTime_t packed;
	DateTime_t temp;
	// Get now if NULL provided
	if(time == NULL)
		time = SysTimeRead(&temp);
	if (time->year >= 64) { return 0xFFFFFFFFul; }	// Epoch years 2064-2136 are not valid in the packed format.
	// Pack into 32 bit value
	packed =			(time->year	& 0x3f) << 4;
	packed = (packed +	(time->month	& 0x0f))<< 5;
	packed = (packed +	(time->day		& 0x1f))<< 5;
	packed = (packed +	(time->hours	& 0x1f))<< 6;
	packed = (packed +	(time->minutes	& 0x3f))<< 6;
	packed = (packed +	(time->seconds	& 0x3f));
	return packed;
}
// Un-pack a date-time struct 
DateTime_t* SysTimeFromPacked(PackedTime_t packed, DateTime_t* output)
{
	if(output == NULL) return NULL;
	// Special-case the zero packed formats to be the min epoch
	if(packed == 0) { return SysTimeFromEpoch(0, output); }
	// Special-case the max packed format to be the max epoch
	if(packed == 0xFFFFFFFFul) { return SysTimeFromEpoch(0xFFFFFFFFul, output); }
	output->seconds = packed & 0x3f; packed >>= 6;
	output->minutes = packed & 0x3f; packed >>= 6;
	output->hours	= packed & 0x1f; packed >>= 5;
	output->day	= packed & 0x1f; packed >>= 5;
	output->month	= packed & 0x0f; packed >>= 4;
	output->year	= packed & 0x3f;	
	return output;
}
// Pack the time into a FAT FS format
uint32_t SysTimeToFat(DateTime_t* time)
{
	// FAT epoch is 1980, fields are shifted by one
	uint32_t fat_time;
	DateTime_t temp;
	// Get now if NULL provided
	if(time == NULL)
		time = SysTimeRead(&temp);
	time->year += 20;
	fat_time = ((uint32_t)SysTimeToPacked(time))>>1;
	if(time->year > 63)fat_time |= 0x80000000;
	return fat_time;
}
// Year truncate to 2 digits
uint8_t SysTimeShortenYear(uint16_t year)
{
	while(year >= 1000)year-=1000;
	while(year >= 100)year-=100;
	return year;
}
// Check a date-time struct is valid
uint8_t SysTimeCheckTime(DateTime_t* time)
{
	uint8_t days;
	if(time == NULL)
		return 0;
	// Check time of day
	if(	(time->seconds > 59)||(time->minutes > 59)||(time->hours > 23) )
		return 0;
	// Check other members
	if(time->year > 99)
		return 0;
	if(	(time->month < 1)||(time->month > 12))
		return 0;
	// Get days in month
	days = daysInMonth[time->month];
	if(	(time->month == 2)&&(IsLeap(time->year)) )
		days++;
	if(	(time->day < 1)||(time->day > days) )
		return 0;
	// Success
	return 1;
}
// Internal utility function, check for leap year
inline uint8_t __attribute__((always_inline)) IsLeap(uint16_t year)
{
	// Could be enhanced to work on centuries if needed
	return((year&0x3)==0)?1:0;
}

// Fully justifies the 1/SYS_TICK_RATE ticks to 1/2^16
inline __attribute__((always_inline)) uint16_t SysTimeFractionalTicksToFixedPoint(uint16_t fractionalTicks)
{
#if (SYS_TICK_RATE > 65536)
	#error "SYS_TICK_RATE does not fit in uint16_t"
#elif (SYS_TICK_RATE == 65536)
	return fractionalTicks;
#elif (SYS_TICK_RATE == 32768)
	return (fractionalTicks << 1);
#else
	#warning "A SYS_TICK_RATE != 32768 was not really expected - this will be slower."
	return (uint16_t)(((uint32_t)fractionalTicks << 16) / SYS_TICK_RATE);
#endif
}

// Credit to Daniel Jackson for the original time string functions - modified below
// Convert a date/time number to a string 
char* SysTimeToString(DateTime_t* time, char* outPtr)
{
    // "20YY/MM/DD,HH:MM:SS\0" - 20 chars 
    char *c = outPtr;
    uint16_t v;
	v = 2000 + time->year; *c++ = '0' + ((v / 1000) % 10); *c++ = '0' + ((v / 100) % 10); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	v = time->month;       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	v = time->day;         *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ',';
	v = time->hours;       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	v = time->minutes;     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	v = time->seconds;     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10);
	*c++ = '\0';
    return outPtr;
}

// Convert a date/time number from a string "YY/MM/DD,HH:MM:SS" 
DateTime_t* SysTimeFromString(const char *value, DateTime_t* dateTimePtr)
{
    uint16_t index, v;
    uint8_t year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    const char *c;

	if (value[0] == '\0')						{ return NULL; }
	if (value[0] == '0' && value[1] == '\0')	{ return NULL; }
	if (value[0] == '-')						{ return NULL; }
	
	if(dateTimePtr == NULL) 
		dateTimePtr = &dateTime;

    index = 0;
    v = 0xffff;
    c = value;
    for (;;)
    {
        if (*c >= '0' && *c <= '9') // Part of value
        {
            if (v == 0xffff) { v = 0; }
            v = (v * 10) + (*c - '0');
        }
        else
        {
            if (v != 0xffff)  // End of value
            {
                if      (index == 0) { year = (v >= 2000) ? (v - 2000) : v; }
                else if (index == 1) { month = v; }
                else if (index == 2) { day = v; }
                else if (index == 3) { hours = v; }
                else if (index == 4) { minutes = v; }
                else if (index == 5) { seconds = v; }
                else { break; }
                index++;
                v = 0xffff;
                if (index >= 6) { break; }
            }
            if (*c == '\0') { break; }
        }
        c++;
    }

    // Check if parsed six elements and check validity of members
    if (    index == 6 &&
            (month >= 1 && month <= 12) &&
            (day >= 1 && day <= daysInMonth[month]) &&
            (hours <= 23) &&
            (minutes <= 59) &&
            (seconds <= 59)
        )
    {
		// Turn 29-Feb in non-leap years into 1-Mar
		if (month == 2 && day == 29 && (year & 3) != 0) 
			{ month = 3; day = 1; }	
		// Update value
		dateTimePtr->year = year % 100;
        dateTimePtr->month = month;
        dateTimePtr->day = day;
        dateTimePtr->hours = hours;
        dateTimePtr->minutes = minutes;
        dateTimePtr->seconds = seconds; 
    }
	return dateTimePtr;
}

// EOF
