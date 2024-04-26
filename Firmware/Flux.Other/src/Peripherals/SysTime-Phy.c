/*	Multi-function generic time/timing module
	This is an example hardware specific template
	
	Co-existing real time clock calendar and timer for:
		- Arbitrary timebase generator (e.g. sample timer)
		- Time stamps (in *epochs or packed/unpacked date-times) 
		- Calendar functions (e.g. for scheduled events)
		
	Time stamp format options:
		- Epoch
		- Time structure
		- Packed time
		
	Other options to be configured in config.h 
		- Allocated resources
		- Tick frequency
		
	*Epoch base date is 00:00:00, 01/01/2000
	
	Karim Ladha, 2015 Revisions:	
	2015-07-10. Added hardware template "SysTime-Phy.c"	
	2015-01-21. First written
	
*/
// Includes
#include <stdint.h>
#include <string.h>
#include "Compiler.h"
#include "Peripherals/SysTime.h"
#include "config.h"

// Debugging
#define DEBUG_LEVEL_LOCAL	DEBUG_SYS
#define DBG_FILE			"systime.c"	
#include "debug.h"

// Globals
extern SysTimeCb_t secondCBs[SYSTIME_NUM_SECOND_CBS+1]; 
extern SysTimeCb_t periodCBs[SYSTIME_NUM_PERIODIC_CBS]; 

// Hardware definitions required (config.h):
//#define RTC_INT_PRIORITY				2		/* Be careful not to create race conditions  */
//#define T1_INT_PRIORITY  RTC_INT_PRIORITY		/* Priority must match for all systime interrupts */
//#define SYS_TICK_RATE					32768	/* May be set or limited by hardware */
//#define SYSTIME_NUM_SECOND_CBS		4		/* May be set or limited by hardware */
//#define SYSTIME_NUM_PERIODIC_CBS		3		/* May be set or limited by hardware */
//#define TIME_INTS_BLOCK() 		{uint8_t wasBlocked = (__get_PRIMASK() & 0x1); __disable_irq();{ 	/*Open context*/
//#define TIME_INTS_UNBLOCK() 		if(!wasBlocked){__enable_irq();}}}									/*Close context*/

// Hardware definitions normally required:

// The periodic timer state structure
typedef struct {
	uint16_t rate;
	uint16_t error;	
	uint16_t accumulator;
	uint16_t perLong;
	uint16_t perShort;
	SysTimeCb_t cb;
}PeriodicTimer_t;

// Globals
// Time
EpochTime_t rtcEpochTriplicate;
DateTime_t dateTime;
// Timers
SysTimeCb_t		secondCBs[(SYSTIME_NUM_SECOND_CBS+1)] = {0};
PeriodicTimer_t	gTimers[(SYSTIME_NUM_PERIODIC_CBS)];

// Prototypes
static inline void sw_rtc_setup_on(void);
static inline void sw_rtc_setup_off(void);
static inline void sw_rtc_load_epoch(void);
static inline void sw_rtc_set_epoch(EpochTime_t epoch);
static inline EpochTime_t sw_rtc_get_epoch(void);
static inline void sw_rtc_epoch_tick(void);
static inline void sw_rtc_one_sec_init(void);
static inline void sw_rtc_init_timer(PeriodicTimer_t* timer, uint8_t index);
// Using interrupt priority level for blocking
#ifndef SYSTIME_IGNORE_ISR_CONTEXT
	// If the enable could interfere with an external disable, use this version
	#define TIME_INTS_CONTEXT()		uint8_t wasBlocked;
	#define TIME_INTS_BLOCK()		{wasBlocked = (__get_PRIMASK() & 0x1);__disable_irq();}
	#define TIME_INTS_UNBLOCK()	{if(!wasBlocked){__enable_irq();}}
#else
	#define TIME_INTS_CONTEXT()
	#define TIME_INTS_BLOCK()		__disable_irq()			
	#define TIME_INTS_UNBLOCK()	__enable_irq()			
#endif

// Constants
const uint16_t sysTimeNumSecondCbs = SYSTIME_NUM_SECOND_CBS;
const uint16_t sysTimeNumPeriodicCbs = SYSTIME_NUM_PERIODIC_CBS;
const uint32_t sysTimeTickMask = ((1ul<<24)-1);


void __attribute__ ((interrupt ("IRQ"))) SYSTIME_RTC_VECTOR(void)
{
	// Handle all counters and the one second event
	volatile uint32_t *event, *cc;
	PeriodicTimer_t *timer;
	uint32_t i, period, ccNext; 
	// Set values
	event = SYSTIME_RTC->EVENTS_COMPARE;
	cc = SYSTIME_RTC->CC;
	timer = gTimers;
	// For each event + second event
	for(i=0;i<(SYSTIME_NUM_SECOND_CBS+1);i++, timer++, event++, cc++)
	{
		ccNext = *cc;									// Current event tick
		while(*event)									// While event active
		{
			period = timer->perShort;					// Short period is default
			if(timer->error)							// If using a corrected rate
			{
				timer->accumulator += timer->error;		// Accumulate error
				if(timer->accumulator >= timer->rate)	// If error >= rate
				{
					timer->accumulator -= timer->rate;	// Subtract rate
					period = timer->perLong;			// Use long period instead
				}
			}
			ccNext = ccNext + period;					// Calculate next compare tick
			if(ccNext > SYSTIME_RTC->COUNTER)			// If event tick > current tick
			{
				*event = 0;								// Clear event flag
				*cc = ccNext & SYSTIME_CC_MASK;			// Set new compare tick
			}											// Else, increment again
			if(timer->cb)(*timer->cb)();				// Call callback
		}
	}
}	
// Installed into second period event
void SecondEventHandler(void)
{
	SysTimeCb_t* cb;
	// Add second to time
	sw_rtc_epoch_tick();
	SysTimeDateTimeInc();
	// Callback list
	cb = &secondCBs[0];
	while(*cb){(*cb)();cb++;}	
}		

// Source
// Initialise and start the rtc
uint8_t SysTimeInit(void)
{
	TIME_INTS_CONTEXT();
	uint8_t retVal = 1;
	EpochTime_t now;
	// Clear all callbacks
	SysTimeClearAllCB();
	// Stop interrupts
	TIME_INTS_BLOCK();
	// Load last epoch
	sw_rtc_load_epoch();
	// Set time from epoch
	now = sw_rtc_get_epoch();
	SysTimeFromEpoch(now, &dateTime);
	// If invalid, reset to start of epoch
	if(!SysTimeCheckTime(&dateTime))
	{
		now = 0;
		sw_rtc_set_epoch(now);
		SysTimeFromEpoch(now, &dateTime);
		retVal = 0; // Indicate failure
	}
	packedTime = SysTimeToPacked(&dateTime);
	// Setup the 1Hz interrupt timer
	gSecTimer->rate = 1;
	gSecTimer->error = 0;	
	gSecTimer->accumulator = 0;
	gSecTimer->perLong = SYS_TICK_RATE;
	gSecTimer->perShort = SYS_TICK_RATE;
	// Set the one sec tick handler
	gSecTimer->cb = SecondEventHandler;
	// Start rtc and one sec interrupt
	sw_rtc_setup_on();
	sw_rtc_one_sec_init();
	// Unblock interrupts
	TIME_INTS_UNBLOCK();
	// Running
	return retVal;
}
// Stop all resources 
uint8_t SysTimeStop(void)
{
	TIME_INTS_CONTEXT();
	// Clear all callbacks
	SysTimeClearAllCB();
	// Halt rtc ints
	TIME_INTS_BLOCK();	
	// Rtc hardware off
	sw_rtc_setup_off();
	// Unblock interrupts
	TIME_INTS_UNBLOCK();	
	// Stopped
	return 1;
}
// Set the time after check
uint8_t SysTimeSet(DateTime_t* time)
{
	TIME_INTS_CONTEXT();
	uint32_t epoch, ccNext;
	//  Check for valid time first
	if(!SysTimeCheckTime(time))
		return 0;
	// Halt rtc interrupts
	TIME_INTS_BLOCK();	
	// Set module time
	epoch = SysTimeToEpoch(time);
	sw_rtc_set_epoch(epoch);
	memcpy(&dateTime,time,sizeof(DateTime_t));
	// Clear fractional timer, restart it
	ccNext = SYSTIME_RTC->COUNTER + SYS_TICK_RATE;
	SYSTIME_RTC->CC[SYSTIME_SEC_INDEX] = ccNext & SYSTIME_CC_MASK;
	// Restore interrupts
	TIME_INTS_UNBLOCK();
	return 1;
}
// Read a 32bit tick value
uint32_t SysTimeTicks(void)
{
	TIME_INTS_CONTEXT();
	uint32_t epoch, counter, compare, ticks;
	// Stop rtc interrupts briefly
	TIME_INTS_BLOCK();
	// Cache values while blocking
	epoch = sw_rtc_get_epoch();
	counter = SYSTIME_RTC->COUNTER;
	compare = SYSTIME_RTC->CC[SYSTIME_SEC_INDEX];
	// Unblock interrupts
	TIME_INTS_UNBLOCK();
	// Calculate ticks
	ticks = SYS_TICK_RATE + counter - compare;
	ticks = (ticks & SYSTIME_CC_MASK) + (epoch * SYS_TICK_RATE);
	// Return ticks
	return ticks;
}
// Read the time in epoch seconds
EpochTime_t SysTimeEpoch(void)
{
	TIME_INTS_CONTEXT();
	EpochTime_t now;
	// Stop rollover
	TIME_INTS_BLOCK();
	now = sw_rtc_get_epoch();
	TIME_INTS_UNBLOCK();
	return now;
}
// Read the time in epoch seconds with fractional
EpochTime_t SysTimeEpochTicks(uint16_t * ticks)
{
	TIME_INTS_CONTEXT();
	uint32_t epoch, counter, compare, tickCount;
	// Stop rtc interrupts briefly
	TIME_INTS_BLOCK();
	// Cache values while blocking
	epoch = sw_rtc_get_epoch();
	counter = SYSTIME_RTC->COUNTER;
	compare = SYSTIME_RTC->CC[SYSTIME_SEC_INDEX];
	// Unblock interrupts
	TIME_INTS_UNBLOCK();
	// Calculate ticks
	tickCount = (SYS_TICK_RATE + counter - compare) & SYSTIME_CC_MASK;
	*ticks = tickCount & ((1<<SYSTIME_TICK_BITS)-1);
	// Add missing seconds if present
	epoch += tickCount >> SYSTIME_TICK_BITS;
	// Return ticks
	return epoch;
}
// Read the current time as a structure
DateTime_t*  SysTimeRead(DateTime_t* copy)
{
	if(copy != NULL)
	{
		TIME_INTS_CONTEXT();
		// Stop roll-over
		TIME_INTS_BLOCK();
		memcpy(copy,&dateTime,sizeof(DateTime_t));
		TIME_INTS_UNBLOCK();
	}
	return copy;
}
// Stop all callbacks
void SysTimeClearAllCB(void)
{
	TIME_INTS_CONTEXT();
	uint8_t i;
	// Block interrupts
	TIME_INTS_BLOCK();
	// Stop the hardware timers
	for(i=0;i<SYSTIME_NUM_PERIODIC_CBS;i++) sw_rtc_init_timer(NULL,i);
	// Stop and clear periodic timers, leave second timer
	memset(gTimers,0,(sizeof(PeriodicTimer_t) * SYSTIME_NUM_PERIODIC_CBS));
	// Clear all 1Hz callbacks
	memset(secondCBs,0,sizeof(secondCBs));
	// Unblock interrupts
	TIME_INTS_UNBLOCK();
}
// One hertz callback installation/removal
uint8_t SysTimeAddSecondCB(SysTimeCb_t cb, uint8_t doAdd)
{
	TIME_INTS_CONTEXT();
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
				TIME_INTS_BLOCK();
				secondCBs[i] = cb;
				TIME_INTS_UNBLOCK();
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
				TIME_INTS_BLOCK();
				memmove(&secondCBs[i],&secondCBs[i+1],sizeof(SysTimeCb_t));
				secondCBs[SYSTIME_NUM_SECOND_CBS] = NULL;
				TIME_INTS_UNBLOCK();
				return 1;
			}
		}
	}
	return 0;
}

// Time based callback installation, zero ticks = removal
uint8_t SysTimeAddIntervalCB(SysTimeCb_t cb, uint32_t ticks)
{
	TIME_INTS_CONTEXT();
	unsigned char index;
	PeriodicTimer_t* timer = gTimers;
	for(index=0; index < SYSTIME_NUM_PERIODIC_CBS; index++, timer++)
	{
		// If ticks is zero, remove callback
		if((ticks == 0)&&(timer->cb == cb))
		{
			TIME_INTS_BLOCK();
			// Setup timer state 
			memset(timer,0,sizeof(PeriodicTimer_t));
			// Start periodic timer
			sw_rtc_init_timer(NULL, index);
			TIME_INTS_UNBLOCK();
			return 1;
		}
		// Find unused timer
		else if((ticks != 0)&&(timer->cb == NULL))
		{
			TIME_INTS_BLOCK();
			// Setup timer state 
			timer->rate = SYS_TICK_RATE / ticks;
			timer->error = 0;	
			timer->accumulator = 0;
			timer->perLong = ticks;
			timer->perShort = ticks;
			timer->cb = cb;		
			// Start periodic timer
			sw_rtc_init_timer(timer, index);
			TIME_INTS_UNBLOCK();
			return 1;
		}
	}
	return 0;
}
// Periodic callback installation, zero rate = removal
uint8_t SysTimeAddRateCB(SysTimeCb_t cb, uint32_t rate)
{
	TIME_INTS_CONTEXT();
	unsigned char index;
	PeriodicTimer_t* timer = gTimers;
	for(index=0; index < SYSTIME_NUM_PERIODIC_CBS; index++, timer++)
	{
		// If ticks is zero, remove callback
		if((rate == 0)&&(timer->cb == cb))
		{
			TIME_INTS_BLOCK();
			// Setup timer state 
			memset(timer,0,sizeof(PeriodicTimer_t));
			// Start periodic timer
			sw_rtc_init_timer(NULL, index);
			TIME_INTS_UNBLOCK();
			return 1;
		}
		// Find unused timer
		else if((rate != 0)&&(timer->cb == NULL))
		{
			TIME_INTS_BLOCK();
			// Setup timer state 
			timer->rate = rate;
			timer->perShort = SYS_TICK_RATE / timer->rate;
			timer->perLong = timer->perShort + 1;
			timer->error = SYS_TICK_RATE - (timer->perShort * timer->rate);	
			timer->accumulator = 0;
			timer->cb = cb;		
			// Start periodic timer
			sw_rtc_init_timer(timer, index);
			TIME_INTS_UNBLOCK();
			return 1;
		}
	}
	return 0;
}
#ifdef SYSTIME_ENABLE_RTC_TIMER	
// This section is optional since it uses a periodic timer slot
// The example below is non-functional but explains the required actions
// Non-blocking "call back after a delay" functionality 
// Set the timer callback. Only one needed
uint8_t SysTimeTimerSetCB(SysTimeCb_t cb)
{
	TimedEventCB = cb;
	return 1;
}
// Get the time until the specified RTC counter tick
int32_t SysTimeTimerGetTicks(uint32_t tick)
{
	uint32_t counter = SysTimeTicks();
	return ((int32_t)tick - counter);
}
// Set the tick at which to call the call-back
uint8_t SysTimeTimerSetTicks(uint32_t ticks)
{
	gEventTimer->perShort = ticks;	
	; // Start the timer to expire after 'ticks' 
	return 1;	
}
// Stop the timer module running
uint8_t SysTimeTimerStop(void)
{
	; // Disable the timer if running
	return 1;	
}
// If required, invoke the timer ISR
uint8_t SysTimeTimerTriggerISR(void)
{
	; // Set the event flag so the timer callback will be called if enabled
	return 1;
}
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

/* LPRC calibration ISR if using calibrated LPRC source */
void __attribute__ ((weak, interrupt ("IRQ"))) POWER_CLOCK_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	// Power off warning event!
	if(NRF_POWER->EVENTS_POFWARN)NRF_POWER->EVENTS_POFWARN = 0;
	// Check events for calibrating LFRC 
	if(NRF_CLOCK->EVENTS_DONE)			/* Calibration complete */
	{
		NRF_CLOCK->EVENTS_DONE = 0;		/* Clear event cal done	*/
		NRF_CLOCK->TASKS_CTSTART = 1;	/* Start calibration timer */
	} 
	else if (NRF_CLOCK->EVENTS_CTTO)	/* Calibration timed out */
	{
		NRF_CLOCK->EVENTS_CTTO = 0;		/* Clear event cal timed out */
		NRF_CLOCK->TASKS_CAL = 1;		/* Start calibration event */
	}
}
/* Load triplicate values */
static inline void sw_rtc_load_epoch(void)
{
	/* Read with triplicate correction */
	if(rtcEpochTriplicate[1] == rtcEpochTriplicate[0])
		rtcEpochTriplicate[2] = rtcEpochTriplicate[0];
	else if (rtcEpochTriplicate[2] == rtcEpochTriplicate[1])
		rtcEpochTriplicate[0] = rtcEpochTriplicate[1];
	else if (rtcEpochTriplicate[2] == rtcEpochTriplicate[0])
		rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	/* Save triplicate back */
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];	
}
/* Write epoch value */
static inline void sw_rtc_set_epoch(EpochTime_t epoch)
{
	/* Write with triplicate redundancy */
	rtcEpochTriplicate[0] = epoch;
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];	
}
/* Read epoch value */
static inline EpochTime_t sw_rtc_get_epoch(void)
{
	/* Write with triplicate redundancy */
	return rtcEpochTriplicate[0];
}
/* Epoch tick increment - unprotected */
static inline void sw_rtc_epoch_tick(void)
{
	/* Triplicate update for use in isr*/
	rtcEpochTriplicate[0]++;
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];
}
/* Start the rtc peripheral */
static inline void sw_rtc_setup_on(void)
{
	// System real time clock init, no interrupts
	SYSTIME_RTC->POWER = 1;					/* Power on */
	SYSTIME_RTC->TASKS_STOP = 1;			/* Stop rtc */
	SYSTIME_RTC->TASKS_CLEAR = 1;			/* Clear it, loses fractional count */
	SYSTIME_RTC->PRESCALER = 0;				/* Set prescaler div by 1 */
	SYSTIME_RTC->EVTENCLR = 0x000F0003;		/* Stop all events */
	SYSTIME_RTC->INTENCLR = 0x000F0003;		/* Stop all rtc interrupts */
	SYSTIME_RTC->TASKS_START = 1;			/* Start timer */	
	// Older revisions have different RTC module features
	CHECK_SYSTIME_HARDWARE();
}
/* Stop the rtc peripheral */
static inline void sw_rtc_setup_off(void)
{
	// System real time clock off
	NVIC_DisableIRQ(SYSTIME_RTC_VECNUM);	/* Interrupt off */		
	SYSTIME_RTC->TASKS_STOP = 1;			/* Stop rtc */
	SYSTIME_RTC->EVTENCLR = 0x000F0003;		/* Stop all events */
	SYSTIME_RTC->INTENCLR = 0x000F0003;		/* Stop all rtc interrupts */
	SYSTIME_RTC->POWER = 0;					/* Power off */
}
/* Start one second interrupt */
static inline void sw_rtc_one_sec_init(void)
{
	// Configure the one sec interrupt to fire each second
	SYSTIME_RTC->CC[SYSTIME_SEC_INDEX] = (SYSTIME_RTC->COUNTER + SYS_TICK_RATE) & SYSTIME_CC_MASK;
	SYSTIME_RTC->EVENTS_COMPARE[SYSTIME_SEC_INDEX] = 0;			/* Clear seconds event */
	SYSTIME_RTC->INTENSET = (1ul<<(RTC_INTENSET_COMPARE0_Pos + SYSTIME_SEC_INDEX));
	// Configure the interrupts
	NVIC_SetPriority(SYSTIME_RTC_VECNUM, RTC_INT_PRIORITY);		/* Priority */
	NVIC_ClearPendingIRQ(SYSTIME_RTC_VECNUM);					/* Clear flag */
	NVIC_EnableIRQ(SYSTIME_RTC_VECNUM);							/* Enable int */	
}
/* Start periodic interrupt */
static inline void sw_rtc_init_timer(PeriodicTimer_t* timer, uint8_t index)
{
	// If timer is null, stop the capture[index] based timer
	if(timer == NULL)
	{
		// Stop the interrupt and event
		SYSTIME_RTC->CC[index] = (uint32_t)(-1);
		SYSTIME_RTC->EVENTS_COMPARE[index] = 0;
		SYSTIME_RTC->INTENCLR = (1ul<<RTC_INTENSET_COMPARE0_Pos) << index;
		SYSTIME_RTC->EVTENCLR = (1ul<<RTC_EVTENSET_COMPARE0_Pos) << index;
	}
	else
	{
		// Configure the capture[index] to begin interrupting
		uint32_t nextCC;
		nextCC = SYSTIME_RTC->COUNTER + timer->perShort;
		SYSTIME_RTC->CC[index] = nextCC & SYSTIME_CC_MASK;
		SYSTIME_RTC->EVENTS_COMPARE[index] = 0;
		SYSTIME_RTC->INTENSET = (1ul<<RTC_INTENSET_COMPARE0_Pos) << index;
	}
	// Interrupts should be pre-configured
	return;
}


// EOF

