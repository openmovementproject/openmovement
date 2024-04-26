/*	Multi-function generic time/timing module
	This is a SysTime driver for minimal implementations.
	The initial implementation is for the Nordic timer SDK driver
	The timebase functions are missing since the external system provides these instead
	The driver is provided to enable minimalistic RTC time keeping functions
	
	Usable definitions:
	Settings:
	#define SYS_TIME_VARS_EXTERNAL		// Prevent globals being created 
	#define SYSTIME_NUM_SECOND_CBS		Set to number of one sec callbacks
	#define SYSTIME_NUM_PERIODIC_CBS	0
	#define SYSTIME_RTC					Set to the RTC peripheral used to enable second fractional part
	Options:
	#define SYS_TIME_EPOCH_ONLY			For no timer support, just maintains epoch seconds tick
	
	*Epoch base date is 00:00:00, 01/01/2000
	
	Karim Ladha, 2017 Revisions:	
	2017-02-28: First written
	
*/
// Includes
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"
#if defined(S130) || defined(s132)
#include "nrf_clock.h"
#ifdef NRF51	
#include "app_timer_appsh.h"
#endif
#else
#endif
#include "app_scheduler.h"
#include "Compiler.h"
#include "Peripherals/SysTime.h"
#include "app_config.h"


// Debugging
#define DEBUG_LEVEL_LOCAL	DEBUG_SYS
#define DBG_FILE			"systime.c"	
#include "Utils/debug.h"

// Globals
#if (SYSTIME_NUM_SECOND_CBS > 0)
const uint32_t sysTimeTickMask = ((1ul<<24)-1);
const uint16_t sysTimeNumSecondCbs = SYSTIME_NUM_SECOND_CBS;
SysTimeCb_t secondCBs[SYSTIME_NUM_SECOND_CBS+1] = {0}; 
#endif
#ifndef SYS_TIME_EPOCH_ONLY
DateTime_t dateTime;
#endif
// One second tick tasks timer instantiation
#ifdef APP_TIMER_DEF
APP_TIMER_DEF(one_sec_tick_timer);
#else
app_timer_id_t one_sec_tick_timer;
#endif

// Persistent epoch value - create in persistent memory section
extern EpochTime_t rtcEpochTriplicate[3];

// Prototypes
static inline void sw_rtc_load_epoch(void);
static inline void sw_rtc_set_epoch(EpochTime_t epoch);
static inline EpochTime_t sw_rtc_get_epoch(void);
static inline void sw_rtc_epoch_tick(void);

// Source
// Installed into second period event
static inline __attribute__((always_inline)) void SecondEventHandler(void* unused)
{
	SysTimeCb_t* cb;
	// Add second to time
	sw_rtc_epoch_tick();
	#ifndef SYS_TIME_EPOCH_ONLY
	SysTimeDateTimeInc();
	#endif
	#if (SYSTIME_NUM_SECOND_CBS > 0)
	// Callback list
	cb = &secondCBs[0];
	while(*cb){(*cb)();cb++;}
	#endif
}	
// Initialise and start the rtc
inline __attribute__((always_inline)) uint8_t SysTimeInit(void)
{
	uint32_t   err_code, now;
	uint8_t retVal = 1;
        #ifdef NRF52
        if (!nrf_clock_lf_is_running())
        {
            nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);

            // Wait for the clock to be ready.
            while (!nrf_clock_lf_is_running()) {;}
        }
        #endif
	#ifndef SYS_TIME_EPOCH_ONLY
	// Clear all callbacks
	SysTimeClearAllCB();
	#endif
	// Load last epoch
	sw_rtc_load_epoch();
	// Set time from epoch
	now = sw_rtc_get_epoch();
	#ifndef SYS_TIME_EPOCH_ONLY
	SysTimeFromEpoch(now, &dateTime);
	// If invalid, reset to start of epoch
	if(!SysTimeCheckTime(&dateTime))
	{
		now = 0;
		sw_rtc_set_epoch(now);
		SysTimeFromEpoch(now, &dateTime);
		retVal = 0; // Indicate failure
	}	
	#endif
	// Initialize timer module, making it use the scheduler.
	#ifdef NRF51 // KL: Old SDK used
	#ifdef APP_TIMER_APPSH_INIT
	APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, APP_TIMER_CONFIG_USE_SCHEDULER);
	#else
    // Initialize timer module, making it use the scheduler.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);
	#endif
	#elif defined(NRF52)
	app_timer_init();
	#else
	#error "What device"
	#endif
    // Really the app should be doing the scheduler setup externally...
	#if (APP_TIMER_CONFIG_USE_SCHEDULER > 0) 
	// Initialize the scheduler module
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
	#endif
	// Initialize the low frequency tasks timer
	err_code = app_timer_create(&one_sec_tick_timer, APP_TIMER_MODE_REPEATED, SecondEventHandler);
	APP_ERROR_CHECK(err_code);
	// Start the low frequency tasks timer
	#ifdef APP_TIMER_PRESCALER
	err_code = app_timer_start(one_sec_tick_timer, APP_TIMER_TICKS(1000,APP_TIMER_PRESCALER), NULL);
	#else
	err_code = app_timer_start(one_sec_tick_timer, APP_TIMER_TICKS(1000), NULL);
	#endif
	APP_ERROR_CHECK(err_code);
	// Check for errors
	if(err_code != NRF_SUCCESS)
	{
		retVal = 0;
	}
	// Return initialisation result
	return retVal;
}
// Stop RTC timer running
inline __attribute__((always_inline)) uint8_t SysTimeStop(void)
{
	uint32_t   err_code;
	err_code = app_timer_stop(one_sec_tick_timer);
	APP_ERROR_CHECK(err_code);
	#if (SYSTIME_NUM_SECOND_CBS > 0)
	// Clear all callbacks
	SysTimeClearAllCB();
	#endif
	// Return, no errors
	return 1;
}
// Read the time in epoch seconds
EpochTime_t SysTimeEpoch(void)
{
	return sw_rtc_get_epoch();
}

// Optionally remove support for timers and DateTime_t
#ifndef SYS_TIME_EPOCH_ONLY
// Set the time after check
uint8_t SysTimeSet(DateTime_t* time)
{
	uint32_t epoch, ccNext;
	//  Check for valid time first
	if(!SysTimeCheckTime(time))
		return 0;
	// Set module time
	epoch = SysTimeToEpoch(time);
	sw_rtc_set_epoch(epoch);
	memcpy(&dateTime,time,sizeof(DateTime_t));
	return 1;
}
// Check the (rate-1) mask will work as expected
#if SYS_TICK_RATE != 32768UL
#error "System ticks calculation will not work as is for this clock rate."
#endif
// Read a 32bit tick value
uint32_t SysTimeTicks(void)
{
	uint32_t epoch_ticks, counter_ticks, ticks;
	// Cache values while blocking
	epoch_ticks = sw_rtc_get_epoch() * SYS_TICK_RATE;
	counter_ticks = SYSTIME_RTC->COUNTER & (SYS_TICK_RATE - 1);
	// Calculate ticks
	ticks = epoch_ticks + counter_ticks;
	// Return ticks
	return ticks;
}
// Read the time in epoch seconds with fractional
EpochTime_t SysTimeEpochTicks(uint16_t * ticks)
{
	uint32_t epoch_time, fractional_ticks;
	// Cache values while blocking
	epoch_time = sw_rtc_get_epoch();
	fractional_ticks = (SYSTIME_RTC->COUNTER & (SYS_TICK_RATE - 1)) << 1;
	// Calculate ticks
	*ticks = (uint16_t)fractional_ticks;
	// Return ticks
	return epoch_time;
}
// Read the current time as a structure
DateTime_t*  SysTimeRead(DateTime_t* copy)
{
	if(copy != NULL)
	{
		// Date time struct is maintained externally
		memcpy(copy,&dateTime,sizeof(DateTime_t));
	}
	return copy;
}
#else
uint8_t SysTimeSetEpoch(uint32_t epoch)
{
	sw_rtc_set_epoch(epoch);
	return 1;
}
#endif // #ifndef SYS_TIME_EPOCH_ONLY

#if (SYSTIME_NUM_SECOND_CBS > 0)
// Stop all callbacks
void SysTimeClearAllCB(void)
{
	// Clear all 1Hz callbacks
	memset(secondCBs,0,sizeof(secondCBs));
}
// One hertz callback installation/removal
uint8_t SysTimeAddSecondCB(SysTimeCb_t cb, uint8_t doAdd)
{
	uint16_t i;
	// Checks
	if(cb==NULL) return 0;
	// Adding
	if(doAdd)
	{
		for(i=0;i<SYSTIME_NUM_SECOND_CBS;i++)
		{
			// Find blank cb
			if(secondCBs[i] == NULL)
			{
				// Add callback
				secondCBs[i] = cb;
				return 1;
			}
		}
	}
	else
	{
		for(i=0;i<SYSTIME_NUM_SECOND_CBS;i++)
		{
			// Remove callback, shift pointer array
			if(secondCBs[i] == cb)
			{			
				memmove(&secondCBs[i],&secondCBs[i+1],sizeof(SysTimeCb_t));
				secondCBs[SYSTIME_NUM_SECOND_CBS] = NULL;
				return 1;
			}
		}
	}
	return 0;
}
#endif // #if SYSTIME_NUM_SECOND_CBS > 0

#ifdef SYSTIME_ENABLE_RTC_TIMER	
#error "Unimplemented in this simple driver instance."
#endif

////////////////////////////////////////////////////////////
// Internal hardware specific functions ////////////////////
////////////////////////////////////////////////////////////
// Example provided for the NRF51822...
// The NRF51822 only has a 24 bit counter to use as an RTC so
// a software RTC method must be used. To preserve the RTC 
// value on reset, the 32 bit second count is stored in an
// un-initialised memory region and triplicate encoded. The
// RTC software implementation follows.

/* Load triplicate values */
static inline __attribute__((always_inline)) void sw_rtc_load_epoch(void)
{
	/* Read with triplicate correction */
	if(rtcEpochTriplicate[1] == rtcEpochTriplicate[0])
		rtcEpochTriplicate[2] = rtcEpochTriplicate[0];
	else if (rtcEpochTriplicate[2] == rtcEpochTriplicate[1])
		rtcEpochTriplicate[0] = rtcEpochTriplicate[1];
	else if (rtcEpochTriplicate[2] == rtcEpochTriplicate[0])
		rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	else
	{
		rtcEpochTriplicate[0] = 0; // No agreement.. set all to zero
		rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
		rtcEpochTriplicate[2] = rtcEpochTriplicate[0];	
	}
}
/* Write epoch value */
static inline __attribute__((always_inline)) void sw_rtc_set_epoch(EpochTime_t epoch)
{
	/* Write with triplicate redundancy */
	rtcEpochTriplicate[0] = epoch;
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];	
}
/* Read epoch value */
static inline __attribute__((always_inline)) EpochTime_t sw_rtc_get_epoch(void)
{
	/* Write with triplicate redundancy */
	return rtcEpochTriplicate[0];
}
/* Epoch tick increment - unprotected */
static inline __attribute__((always_inline)) void sw_rtc_epoch_tick(void)
{
	/* Triplicate update of epoch*/
	rtcEpochTriplicate[0]++;
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];
}

// Restart WDT if running - minimalist
#ifdef NRF51
inline __attribute__((always_inline)) void ClrWdt(void)
{
	// If the WDT is running, service enabled channels
	if(NRF_WDT->RUNSTATUS)
	{
		uint32_t channel, mask;
		// Write special value to all enabled channels, check with bit mask
		for(channel = 0, mask = 1; channel < NRF_WDT_CHANNEL_NUMBER; channel++, mask <<= 1)
		{
			// For all enable reset reload registers, write special reset value
			if( NRF_WDT->RREN & mask ) 
				NRF_WDT->RR[channel] = NRF_WDT_RR_VALUE;
		}
	}
	// All channels request WDT reset, countdown is restarted
}
#endif
// EOF

