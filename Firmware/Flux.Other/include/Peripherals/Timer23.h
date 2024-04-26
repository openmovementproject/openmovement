// KL: Written to provide a system tick count from the main clock and accurate periodic callback function
// Uses resources timer 2 and 3 simultaneously as a 32bit timer based on system clock.
// The main limitations are: Timer does not advance in sleep. At 48MIPS, the 32bit count rolls over in 90s.
// The 64bit counter yields 12000 years at 48MIPS.

#ifndef _TICK_H_
#define _TICK_H_

/* Definitions required - rty to avoid inefficient divides by > 16bits
	TICKS_TO_SEC - Value of divisor to convert ticks to seconds. I.e. Instruction clock
	TICKS_TO_MS - Value of divisor to convert ticks to milliseconds. I.e. Instruction clock/1000
	TIMER23_TICK_PRIORITY - Value to assign the interrupt priority of the timer interrupt
*/

// Types
typedef void (*TickCB_t)(void);

// Prototypes
void TimerStartup(void);

void TimerInterruptOn(unsigned short newRate);

void TimerIsrDisable(void);

void TimerIsrEnable(void);

void TimerInterruptOff(void);

void TimerSetCB(TickCB_t cb);

unsigned short TimerGet16(void);

unsigned long TimerGet32(void);

unsigned long long TimerGet64(void);

unsigned long TimerSeconds(void);

unsigned long long TimerMilliSeconds(void);

#endif



