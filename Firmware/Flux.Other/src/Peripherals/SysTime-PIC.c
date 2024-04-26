/*	Multi-function generic time/timing module 
	This is the PIC24/dsPIC33 implementation.
	Epoch base date is 00:00:00, 01/01/2000
	
	Co-existing real time clock calendar and timer for:
		-Arbitrary timebase generator (e.g. sample timer)
		-Time stamps (in epochs or packed/unpacked date-times) 
		-Calendar functions (e.g. for scheduled events)
		
	Time stamp format options:
		-Epoch
		-Time structure
		-Packed time
		
	Other options to be configured in config.h 
		-Allocated resources
		-Tick frequency
		
	Karim Ladha, Revisions:	
	2015-07-10. Tidied up some of the code and comments.
	2015-01-21. First written
*/

// PIC24/33 has only one counter driven off the rtc clock with no compares,
// it has only one rate callback as a result. The counter must also make 
// fractional second field too; thus complicating the implementation.

// Includes
#include <stdint.h>
#include <string.h>
#include "Compiler.h"
#include "Peripherals/SysTime.h"
#include "HardwareProfile.h"
#include "config.h"

// Debugging
#define DEBUG_LEVEL_LOCAL	DEBUG_SYS
#define DBG_FILE			"systime.c"	
#include "Utils/debug.h"

// Globals
// Hardware specific globals - PIC24/dsPIC33
static uint16_t lastSecTick;
static uint16_t tickCounter;
// Arbitrary rate timebase
static uint16_t timerRate = 1;
static uint16_t timerError = 0;	
static uint16_t errorAccumulator = 0;
static uint16_t perLong = 0xffff,perShort=0xffff;

// Prototypes for PIC implementation - HW rtcc + timer1 control
inline void pic24_rtcc_init(void);
inline void pic24_rtcc_read(DateTime_t* time);
inline void pic24_rtcc_write(DateTime_t* time);
inline void pic24_rtcc_off(void);
inline void pic24_timer1_init(void);
inline void pic24_timer1_setup(uint32_t value, uint8_t isRate);
inline void pic24_timer1_off(void);

// Using interrupt priority level for blocking, SRBITS type from device header
#define TIME_INTS_BLOCK() 	{SRsave.IPL=SRbits.IPL;if(SRbits.IPL<RTC_INT_PRIORITY)SRbits.IPL=RTC_INT_PRIORITY;}
#define TIME_INTS_UNBLOCK() {SRbits.IPL=SRsave.IPL;}

// Checks
#if (SYSTIME_NUM_PERIODIC_CBS > 1)
#error "Multiple periodic callbacks unsupported on this hardware." 
#endif
#if (T1_INT_PRIORITY != RTC_INT_PRIORITY)
#error "Selected interrupt priority unsupported." 
#endif

// Interrupts
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	// Clear flag
	IFS3bits.RTCIF = 0;
	// Store current tick count for second
	lastSecTick = tickCounter + TMR1;
	// Add period on if rolled over
	if(IFS0bits.T1IF && IEC0bits.T1IE)
		lastSecTick += (PR1+1);
	// Increment epoc time
	epochTime++;
	// Increment date time struct
	SysTimeDateTimeInc();
	// Callback(s)
	{
		SysTimeCb_t* cb;
		cb = &secondCBs[0];
		while(*cb){(*cb)();cb++;}
	}
	// Software watchdog timer
	#if defined(SWWDT_TIMEOUT) && defined(SWWDT_TASK_IN_RTC_TICK)
	SwwdtTasks();
	#endif
}
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
	// Clear flag
	IFS0bits.T1IF = 0;			// Clear isr flag
	// Fraction counter
	tickCounter += (PR1+1);
	// Rate correction, accumulates error 
	errorAccumulator += timerError;	 
	if (errorAccumulator >= timerRate)
	{
		errorAccumulator -= timerRate;
		if(PR1 != perLong) PR1 = perLong;
	}
	else
	{
		if(PR1 != perShort) PR1 = perShort;	
	}
	// Callback - this platform has only one rate CB
	{
		if(periodCBs[0])periodCBs[0]();
	}
}

// Source
// Initialise and start the rtc
uint8_t SysTimeInit(void)
{
	// Stop interrupts
	SRBITS SRsave;
	TIME_INTS_BLOCK();
	// Start the rtc
	pic24_rtcc_init();
	// Start timer
	pic24_timer1_init();
	// Read time and epoch time
	pic24_rtcc_read(&dateTime);
	epochTime = SysTimeToEpoch(&dateTime);
	packedTime = SysTimeToPacked(&dateTime);
	lastSecTick = 0; /* Clear second fraction */
	// Clear all callbacks
	SysTimeClearAllCB();
	// Unblock interrupts
	TIME_INTS_UNBLOCK();
	// Running
	return 1;
}
// Stop all resources - will loose time
uint8_t SysTimeStop(void)
{
	// Interrupts off
	SRBITS SRsave;	
	TIME_INTS_BLOCK();	
	// Rtc hardware off
	pic24_rtcc_off();
	pic24_timer1_off();
	// Unblock
	TIME_INTS_UNBLOCK();	
	return 1;
}
// Set the time after check
uint8_t SysTimeSet(DateTime_t* time)
{
	SRBITS SRsave;
	if(!SysTimeCheckTime(time))
		return 0;
	// Halt rtc ints
	TIME_INTS_BLOCK();
	// Set module time
	pic24_rtcc_write(time);
	// Set system time & epoch
	pic24_rtcc_read(&dateTime);
	epochTime = SysTimeToEpoch(&dateTime);
	// Clear second fraction
	lastSecTick = tickCounter;
	// Unblock ints
	TIME_INTS_UNBLOCK();
	return 1;
}
// Read a 32bit tick value
uint32_t SysTimeTicks(void)
{
	SRBITS SRsave;
	uint32_t tickLong;
	uint16_t tickVal;
	uint8_t check;
	// Stop rtc roll over
	TIME_INTS_BLOCK();
	// Can't stop timer overflow!
	do{ // Pre/post overflow check
		check = IFS0bits.T1IF;
		tickLong = epochTime;
		tickVal = TMR1;
	}while(IFS0bits.T1IF != check);
	// Add period on if rolled over
	if(IFS0bits.T1IF && IEC0bits.T1IE)
		tickVal += (PR1+1);
	// Add the tick counter
	tickVal += tickCounter;
	// Subtract last second tick
	tickVal -= lastSecTick;
	// Scale seconds to ticks
	#if (SYS_TICK_RATE != 32768)
	#error "Untested rate. Expecting 32.768 kHz (>32768 will fail)"
	tickLong *= SYS_TICK_RATE;
	#else
	tickLong <<= 15; // *= 32768;
	#endif
	// Make long tick val
	tickLong += tickVal;
	TIME_INTS_UNBLOCK();
	return tickLong;
}
// Read the time in epoch seconds
EpochTime_t SysTimeEpoch(void)
{
	SRBITS SRsave;
	EpochTime_t now;
	// Stop roll over
	TIME_INTS_BLOCK();
	now = epochTime;
	TIME_INTS_UNBLOCK();
	return now;
}
// Read the time in epoch seconds with fractional
EpochTime_t SysTimeEpochTicks(uint16_t * ticks)
{
	SRBITS SRsave;
	EpochTime_t now;
	uint16_t tickVal;
	uint8_t check;
	// Stop rtc roll over
	TIME_INTS_BLOCK();
	// Can't stop timer overflow!
	do{ // Pre/post overflow check
		check = IFS0bits.T1IF;
		now = epochTime;
		tickVal = TMR1;
	}while(IFS0bits.T1IF != check);
	// Add period on if rolled over
	if(IFS0bits.T1IF && IEC0bits.T1IE)
		tickVal += (PR1+1);
	// Add the tick counter
	tickVal += tickCounter;
	// Subtract last second tick
	tickVal -= lastSecTick;
	// Clamp (ensure < 1sec ticks, ticks%=TICK_RATE)
	if(tickVal >= SYS_TICK_RATE)
		{tickVal -= SYS_TICK_RATE; now++;}
	// Output ticks, return seconds
	*ticks = tickVal;
	TIME_INTS_UNBLOCK();
	return now;
}		
// Read the current time as a structure
DateTime_t*  SysTimeRead(DateTime_t* copy)
{
	if(copy != NULL)
	{
		SRBITS SRsave;
		// Stop roll over
		TIME_INTS_BLOCK();
		memcpy(copy,&dateTime,sizeof(DateTime_t));
		TIME_INTS_UNBLOCK();
	}
	return copy;
}
// Stop all callbacks
void SysTimeClearAllCB(void)
{
	// Stop periodic timer
	pic24_timer1_setup(0, 0);
	// T1 interrupts are off
	memset(periodCBs,0,(sizeof(SysTimeCb_t)*sysTimeNumPeriodicCbs));	
	// Callbacks all cleared
	memset(secondCBs,0,(sizeof(SysTimeCb_t)*sysTimeNumSecondCbs));
}
// One hertz callback installation/removal
uint8_t SysTimeAddSecondCB(SysTimeCb_t cb, uint8_t doAdd)
{
	uint16_t i;
	SRBITS SRsave;
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
			// Remove callback
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
uint8_t SysTimeAddTimedCB(SysTimeCb_t cb, uint32_t ticks)
{
	if(cb == NULL) return 0;
	if(ticks == 0) cb = NULL;
	periodCBs[0] = cb;
	pic24_timer1_setup(ticks,0);
	return 1;
}
// Periodic callback installation, zero rate = removal
uint8_t SysTimeAddRateCB(SysTimeCb_t cb, uint32_t rate)
{
	if(cb == NULL) return 0;
	if(rate == 0) cb = NULL;
	periodCBs[0] = cb;
	pic24_timer1_setup(rate,1);
	return 1;
}


////////////////////////////////////////////////////////////
// PIC24 hardware specific functions, private //////////////
////////////////////////////////////////////////////////////
// Timer 1 free running, no interrupts
inline void pic24_timer1_init(void)
{
	// Interrupts off
	IEC0bits.T1IE = 0;
	IFS0bits.T1IF = 0;	
	IPC0bits.T1IP = T1_INT_PRIORITY;
	// Start timer, Async. T1CK edge
	TMR1 = 0;
	PR1 = 0xFFFF;
	T1CON = 0x0002;
	T1CONbits.TON = 1;	
	// No interrupt operation
	timerRate = 0; 
	perShort = 0;
	perLong = 0;
	timerError = 0;
	tickCounter = 0;
	errorAccumulator = 0;
}
// Timer 1 stopped
inline void pic24_timer1_off(void)
{
	// Interrupts off
	IEC0bits.T1IE = 0;	
	// Clear timer vars
	timerRate = 0; 
	// Stop timer
	T1CONbits.TON = 0;
}
// Timer 1 periodic callback
inline void pic24_timer1_setup(uint32_t value, uint8_t isRate)
{
	SRBITS SRsave;
	TIME_INTS_BLOCK();	
	// If turning off
	if(value == 0)
	{
		// No interrupt mode
		timerRate = 0; 
		perShort = 0xFFFF;
		perLong = perShort;
		timerError = 0;	
		IEC0bits.T1IE = 0;
	}
	else if(isRate)
	{
		// Calculate values for frequency
		timerRate = value; 
		perShort = ((SYS_TICK_RATE)/value) - 1;
		perLong = perShort + 1;
		timerError = SYS_TICK_RATE - (perLong * value);
		IEC0bits.T1IE = 1;		
	}
	else
	{
		// Set values for fixed period
		timerRate = SYS_TICK_RATE/value; 
		perShort = value - 1;
		perLong = perShort;
		timerError = 0;	
		IEC0bits.T1IE = 1;		
	}
	// Clear count
	errorAccumulator = 0;
	// Don't discard ticks
	tickCounter += TMR1;	
	// Start timer
	TMR1 = 0; 
	PR1 = perShort;
	// Unblock interrupts
	IFS0bits.T1IF = 0;		
	TIME_INTS_UNBLOCK();
	// Done
	return;			
}

#ifdef SYSTIME_ENABLE_RTC_TIMER	
#error "TODO: So far this was has not been required. N.b. The PIC24 only has one available timer."
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
// PIC24 internal rtcc hardware module specific code      //
////////////////////////////////////////////////////////////
#ifndef RTC_CAL_VALUE // Can be externally defined
#define RTC_CAL_VALUE 	0
#endif // 8bit signed, x2.03ppm
// Register block
typedef union {
	uint8_t field[8];
	uint16_t block[4];
}p24_rtc_format_t;
// Write unlock code
void p24_unlock_rtc(void) 
{
	__asm("disi #64");
	__asm("mov #0x55, w0");
	__asm("mov w0, NVMKEY");
	__asm("mov #0xAA, w0");
	__asm("mov w0, NVMKEY");
	__asm("bset RCFGCAL, #13");
	__asm("nop");
	__asm("nop");
}
// Standard bcd read 
uint8_t p24_from_bcd(uint8_t bcd) {return ((((bcd) & 0xf0) >> 4) * 10 + ((bcd) & 0x0f));}
// Standard bcd validate
uint8_t valid_bcd(uint8_t bcd){if(((bcd&0xF)>0x9)||((bcd>>4)>0x9))return 0;return 1;}
// Using 8bit multiplicative inverse (works upto #65)
uint8_t p24_to_bcd(uint8_t val) {uint16_t temp = (uint16_t)26 * val;return(uint8_t)(((temp & 0xFF00)>>(8-4)) + (((temp & 0xFF)*10)>>8));}
// Packing/unpacking
void pic24_time_to_block(DateTime_t* time, p24_rtc_format_t* block)
{
	block->field[0] = p24_to_bcd(time->seconds);
	block->field[1] = p24_to_bcd(time->minutes);
	block->field[2] = p24_to_bcd(time->hours);
	block->field[3] = 0; // Monday
	block->field[4] = p24_to_bcd(time->day);
	block->field[5] = p24_to_bcd(time->month);
	block->field[6] = p24_to_bcd(time->year);
	block->field[7] = 0;
}
void pic24_block_to_time(DateTime_t* time, p24_rtc_format_t* block)
{
	time->seconds = p24_from_bcd(block->field[0]); 
	time->minutes = p24_from_bcd(block->field[1]); 
	time->hours   = p24_from_bcd(block->field[2]); 
	time->day     = p24_from_bcd(block->field[4]); 
	time->month   = p24_from_bcd(block->field[5]); 
	time->year    = p24_from_bcd(block->field[6]); 
}
// Turn on secondary oscillator (not used)
inline void __attribute__((always_inline))pic24_sosc_on(void)
{
	if(!(OSCCON & 0x2))
	{	
		uint8_t oscconL = OSCCON | 0x2;
		__builtin_write_OSCCONL(oscconL);
	}
}
// Read register block
inline void __attribute__((always_inline))pic24_rtcc_read_raw(p24_rtc_format_t* blocks)
{
	uint16_t block0;	// Holder for block 0
	RCFGCAL &= 0xFCFF; 	// Clear block ptr
	do{
	block0 = RTCVAL;	// Read block 0 first
	RCFGCAL |= 0x0300;
	blocks->block[3] = RTCVAL;
	blocks->block[2] = RTCVAL;
	blocks->block[1] = RTCVAL;
	blocks->block[0] = RTCVAL;	
	}while(blocks->block[0] != block0);// roll over?
}
// Write register block
inline void __attribute__((always_inline))pic24_rtcc_write_raw(p24_rtc_format_t* blocks)
{
	RCFGCAL |= 0x0300;	
	RTCVAL = blocks->block[3];
	RTCVAL = blocks->block[2];
	RTCVAL = blocks->block[1];
	RTCVAL = blocks->block[0];	
}
// Write alarm block
inline void __attribute__((always_inline))pic24_alarm_write_raw(p24_rtc_format_t* blocks)
{
	ALCFGRPT |= 0x0300;	
	ALRMVAL = blocks->block[3];
	ALRMVAL = blocks->block[2];
	ALRMVAL = blocks->block[1];
	ALRMVAL = blocks->block[0];	
}
// Convert register block
inline void __attribute__((always_inline))pic24_rtcc_read(DateTime_t* time)
{
	// Perform safe read of block
	p24_rtc_format_t temp;
	pic24_rtcc_read_raw(&temp);
	pic24_block_to_time(time, &temp);
}
// Write the time
inline void __attribute__((always_inline))pic24_rtcc_write(DateTime_t* time)
{
	uint8_t yos = 0;
	p24_rtc_format_t temp;
	time->year = SysTimeShortenYear(time->year);
	if(time->year>50){time->year-=50;yos=0x50;}
	pic24_time_to_block(time, &temp);
	temp.field[6] += yos;
	p24_unlock_rtc();
#if 1	// [dgj] Think it should be writing to RTCC here (not alarm)?
	pic24_rtcc_write_raw(&temp);
#else
	pic24_alarm_write_raw(&temp);
#endif
	RCFGCALbits.RTCWREN = 0;
}
// Startup rtc
inline void __attribute__((always_inline))pic24_rtcc_init(void)
{
	uint8_t i;
	DateTime_t* time = &dateTime;
	p24_rtc_format_t temp;
	// Interrupt off
	IEC3bits.RTCIE = 0;	
	// Unlock
	do{p24_unlock_rtc();}
	while(!(RCFGCAL&0x2000));
	// Stop
	RCFGCAL &= 0x7FFF;
	// Read internal time
	pic24_rtcc_read_raw(&temp);
	// Convert it
	pic24_block_to_time(time, &temp);
	// Check values
	for(i=0;i<7;i++)if(!valid_bcd(temp.field[i]))break;
	if((i != 7) || (!SysTimeCheckTime(time)))
	{	// Default time
		memset(time,0,sizeof(dateTime));
		dateTime.day=dateTime.month=1;
		pic24_time_to_block(time, &temp);
		// Write back
		pic24_rtcc_write_raw(&temp);
	}
	// Set alarm
	ALCFGRPT = 0xC700;	
	pic24_alarm_write_raw(&temp);
	// Start rtc
	RCFGCAL = 0x8200 | RTC_CAL_VALUE;
	// Lock
	RCFGCALbits.RTCWREN = 0;
	// Interrupts on
	IPC15bits.RTCIP = RTC_INT_PRIORITY;
	IFS3bits.RTCIF = 0;
	IEC3bits.RTCIE = 1;
}
inline void __attribute__((always_inline))pic24_rtcc_off(void)
{
	// Interrupt off
	IEC3bits.RTCIE = 0;
	// Unlock
	p24_unlock_rtc();
	// Turn off
	RCFGCAL = 0x0000;
	// Lock
	RCFGCALbits.RTCWREN = 0;
}

//EOF
