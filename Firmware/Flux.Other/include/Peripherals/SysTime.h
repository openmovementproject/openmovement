/*	Multi-function generic time/timing module
	Co-existing real time clock calendar and timer for:
		- Arbitrary timebase generator (e.g. sample timer)
		- Time stamps (in epochs or packed/unpacked date-times) 
		- Calendar functions (e.g. for scheduled events)
		- Blocking delays in micro or milliseconds
	
	Time stamp format options:
		- Epoch
		- Time structure
		- Packed time
	
	Further info at end of this header file regarding the common
	hardware API implementation.

	Other options to be configured in config.h: 
		SYSTIME_NUM_SECOND_CBS			- Allocated resources
		SYSTIME_NUM_PERIODIC_CBS		- Allocated resources		
		SYS_TICK_RATE					- Tick frequency
	
	The blocking microsecond and millisecond delays also require:
		GetSystemMIPS()				- Instruction clock
		FUNCTION_CALL_CYCLE_OVERHEAD	- Function call overhead
		DECREMENT_LOOP_CYCLE_COUNT		- Loop decrement instructions

	Karim Ladha, 2015, Revisions:	
	2015-07-10. Minor comment changes
	2015-07-03. Linked to the scheduler module
	2015-12-06. Splitting hardware specific parts into new files
	2015-01-21. First written
*/
#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

/* Includes */
#include <stdint.h>

/* Types */
/* Unpacked date time type*/
typedef	struct
{	/* Simple byte array */
	uint8_t seconds;// 0-59
	uint8_t minutes;// 0-59
	uint8_t hours;	// 0-23
	uint8_t day;	// 1-31
	uint8_t month;	// 1-12
	uint8_t year;	// 0-99
} DateTime_t;
/* Packed time type */
typedef uint32_t PackedTime_t;
/* Epoch seconds type */
typedef uint32_t EpochTime_t;
/* Callbacks - warning, called at SysTime interrupt level */
typedef void (*SysTimeCb_t)(void);

/* Private global variables - **for debug only** */
/* Installed callback functions */
extern const uint16_t sysTimeNumSecondCbs;
extern const uint16_t sysTimeNumPeriodicCbs;
extern SysTimeCb_t secondCBs[];
extern SysTimeCb_t periodCBs[];
/* Current date and time variables */
extern EpochTime_t epochTime;		// linear seconds since epoch
extern DateTime_t dateTime;			// unpacked struct
extern PackedTime_t packedTime;		// packed DWORD
/* RTC counter bit mask, depends on counter bits */
extern const uint32_t sysTimeTickMask;

/* Prototypes in hardware in-specific source file*/

// Private functions used in hardware modules

// Increment the global date and time variable by one second
// inline void __attribute__((always_inline))SysTimeDateTimeInc(void);
extern void SysTimeDateTimeInc(void);

// Software watchdog timer functionality. Deccrement (each second) and reset
extern volatile uint16_t swwdtCount;
extern void SwwdtTasks(void);
extern void SwwdtReset(void);

// Blocking delay functions 
// Millisecond delay
void SysTimeDelayMs(uint32_t ms);
// Micro second delay - accuracy may be optimiser dependent
void SysTimeDelayUs(uint16_t us);

// Time conversions
// Convert an epoch to a time struct
EpochTime_t SysTimeToEpoch(DateTime_t* time);
// Date and time from epoch 
DateTime_t* SysTimeFromEpoch(EpochTime_t epoch, DateTime_t* output);
// Make a time struct into a packed time
PackedTime_t SysTimeToPacked(DateTime_t* time);
// Un-pack a packed time into a caller supplied struct
DateTime_t* SysTimeFromPacked(PackedTime_t packed, DateTime_t* output);
// Make a time struct into the FAT file system format
uint32_t SysTimeToFat(DateTime_t* time);

// Other miscellaneous
// Year mod 100 without divide
uint8_t SysTimeShortenYear(uint16_t year);
// Check a time struct date and time is valid
uint8_t SysTimeCheckTime(DateTime_t* time);

/* Prototypes in hardware specific source file.*/
/* New hardware must implement this functionality. */

// Initialisation/start-up 
uint8_t SysTimeInit(void);
// Set the time (zeros seconds)
uint8_t SysTimeSet(DateTime_t* time);
// Epoch version of the set time method (optional)
uint8_t SysTimeSetEpoch(uint32_t epoch);
// Stop all resources
uint8_t SysTimeStop(void);

// Read time, epoch and/or ticks
// Slower get tick, guaranteed to be 32 bit
uint32_t	 SysTimeTicks(void);
// Seconds since epoch date 
EpochTime_t  SysTimeEpoch(void);
// Seconds since epoch with ticks (ticks not left justified)
EpochTime_t  SysTimeEpochTicks(uint16_t * ticks);
// Get the current time struct, caller supplies struct
DateTime_t*  SysTimeRead(DateTime_t* copy);

// Micro implementation only - external tick implementation
void SysTimeEpochInc(void);

// Callback installation/adjust (to remove set rate/ticks = 0)
// Stop all SysTime callbacks
void SysTimeClearAllCB(void);
// Add a callback to occur each second
typedef enum {SYS_TIME_SEC_CB_REMOVE = 0, SYS_TIME_SEC_CB_ADD = 1}
SysTimeSecCbAction_t; // Added for code readability of below function
uint8_t SysTimeAddSecondCB(SysTimeCb_t cb, uint8_t doAdd);
// Add a callback to occur every N ticks
uint8_t SysTimeAddIntervalCB(SysTimeCb_t cb, uint32_t ticks);
// Add a callback to occur at a precise rate
uint8_t SysTimeAddRateCB(SysTimeCb_t cb, uint32_t rate);

// Non-blocking callback on timer functionality - optional
// Set the timer callback. Only one is available
uint8_t SysTimeTimerSetCB(SysTimeCb_t cb);
// Get the current RTC counter tick
uint32_t SysTimeTimerGetTick(void);
// Set the tick at which to call the call-back
uint8_t SysTimeTimerSetTick(uint32_t tick);
// Stop the timer module running	
uint8_t SysTimeTimerStop(void);
// If required, invoke the timer ISR
uint8_t SysTimeTimerTriggerISR(void);

// String funtions
#define SYSTIME_STRING_LEN	20
char* SysTimeToString(DateTime_t* time, char* outPtr);
DateTime_t* SysTimeFromString(const char *value, DateTime_t* dateTimePtr);

// For min/max PackedTime_t constants
// Packed bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
#define PACKEDTIME_FROM_YMDHMS(_year, _month, _day, _hours, _minutes, _seconds) ( (((unsigned long)(_year % 100) & 0x3f) << 26) | (((unsigned long)(_month) & 0x0f) << 22) | (((unsigned long)(_day) & 0x1f) << 17) | (((unsigned long)(_hours) & 0x1f) << 12) | (((unsigned long)(_minutes) & 0x3f) <<  6) | ((unsigned long)(_seconds) & 0x3f) )
//#define PACKEDTIME_YEAR(_v)	   ((unsigned char)(((_v) >> 26) & 0x3f))
//#define PACKEDTIME_MONTH(_v)	  ((unsigned char)(((_v) >> 22) & 0x0f))
//#define PACKEDTIME_DAY(_v)		((unsigned char)(((_v) >> 17) & 0x1f))
//#define PACKEDTIME_HOURS(_v)	  ((unsigned char)(((_v) >> 12) & 0x1f))
//#define PACKEDTIME_MINUTES(_v)	((unsigned char)(((_v) >>  6) & 0x3f))
//#define PACKEDTIME_SECONDS(_v)	((unsigned char)(((_v)	  ) & 0x3f))
#define PACKEDTIME_MIN PACKEDTIME_FROM_YMDHMS(2000,1,1,0,0,0)
#define PACKEDTIME_MAX PACKEDTIME_FROM_YMDHMS(2063,12,31,23,59,59)
#define PACKEDTIME_INVALID		0xfffffffful
#define SYSTIME_VALUE_INVALID	0xfffffffful


// Fully justifies the 1/SYS_TICK_RATE ticks to 1/2^16
uint16_t  SysTimeFractionalTicksToFixedPoint(uint16_t fractionalTicks);

/* 
Notes on hardware and usage:
This api is written to be cross-platform and hardware unspecific. The hardware driver
implementation of the the functions must be written to assume a single initialisation
at start up and that the clock providing the tick is started and running externally. The
driver should be able to provide at least one rate/interval callback and one second 
callback. The time fetch functions must be safe at all priorities, at all times and 
when interleaved with other simultaneous fetches.

Timer/Rtc control api:
This timer and rtc api is written to be hardware unspecific across multiple
platforms. Some platforms have integrated rtc hardware, some have external peripherals
and some devices have no hardware and a software rtc is used. As a result, when using
the api for timing sensitive code, the user should choose epoch time for applications
requiring fast time stamping. For the rate and interval callbacks, some hardware may
have multiple counter/comparator modules and generate many different time-bases; in most 
cases this will be limited also. The rate callback allows time-bases to be generated that 
are, on average, an exact rate but will have jitter. The interval timebase does not add
jitter, but only produces rates that are multiples of the tick frequency. Some of the 
functionality may be configurable; this should be set in config.h.

Interrupt usage and control flags:
This API is written to be hardware independent. Since some hardware has no dedicated
rtc peripheral, the interrupts may not be under user control; e.g. If the rtc is in 
software and the second count roll-over is in an interrupt. In addition, multiple timers
may be operating simultaneously at the same interrupt priority level. For these reasons, 
the application should observe the following to make code portable:
	- Not control the rtc/timer peripheral interrupts directly.
	- Not place interrupts at higher priorities that take more than a few ticks.
	- Use short callbacks (a few ticks max) or software trigger a lower isr handler.
*/
#endif
//EOF
