/*
Timer1.h emulates the most basic RTC functions and allows 
a time base using the T1 oscillator to be generated, P24 only
*/

// Implementation-specific
#include "Compiler.h"
#include "HardwareProfile.h"
#include "Peripherals/Timer1.h"

// Globals
volatile DWORD_VAL timerTicks = {0}; // 16.16 unsigned long tick count
volatile unsigned int SwwdtValue = 0;
static unsigned short timerError = 0;	// Since the rate is not normally exact, it needs compensating
static unsigned short errorAccumulator = 0;
static unsigned short t1rate = 1;
static unsigned short pr1H = 0xffff,pr1L=0xffff;

// Initialization
void TimerStartup(void)
{
	// Ints off
	IEC0bits.T1IE = 0;
	// Timer osc enable
    CLOCK_SOSCEN();
	// Clear swwdt
	SwwdtReset();
	// Init vars
	t1rate = 1; pr1L = pr1H = 0xffff;
    // Setup timer
	T1CONbits.TSYNC = 0;// No sync
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 0;// Timer on in idle mode
	PR1 = 0xFFFF;		// Timer period of 2 seconds
	TMR1 = 0;			// Clear timer
	T1CONbits.TON = 1;	// Timer on
	timerTicks.Val = 0; // Clear second count
	errorAccumulator = 0;	// Clear value here
	timerError = 0;			// No interrupt so no error
	// Done.. timer is running if clocked
}


void TimerInterruptOn(unsigned short newRate)
{
    // Timer interrupts off, block interrupts using T1 resources too
	TIMER_IPL_shadow_t IPLshadow;
	TIMER_INTS_DISABLE(); 
	// Set priority
	IPC0bits.T1IP = T1_INT_PRIORITY;
	// Prevent div by zero
    if (newRate == 0)newRate=1;
	t1rate = newRate;
    // Clear counter and set precision timer
    pr1L = PR1 = (((32768)/newRate)-1);			// Period
	pr1H = pr1L+1;
	timerError = 32768 - ((PR1+1) * newRate);	// Error in ticks/32768
	errorAccumulator = 0;
	if(TMR1 > PR1) TMR1 = 0;					// If we will miss the next interrupt, clear count
    // Enable RTC and timer interrupts	
	IEC0bits.T1IE = 1;
	// Turn on ints, may vector immediately
	TIMER_INTS_ENABLE();
}

void TimerInterruptOff(void)
{
    IEC0bits.T1IE = 0;
    T1CONbits.TON = 0;
    IFS0bits.T1IF = 0;
    TMR1 = 0x0000;
    PR1 = 0xffff;
}

unsigned char TimerTasks(void)
{
	unsigned char retval = 0; 		// b0 is second elapsed flag, b1 is timer period flag
    if (IFS0bits.T1IF)
    {
		// Clear flag
        IFS0bits.T1IF = 0;			// Clear isr flag
		// Main counter
		timerTicks.w[0] >>= 1; 		// Shift ticks right, leave seconds, top bit = 0
		timerTicks.w[0] += (PR1+1);	// Add new ticks (exact value), see if the number overflows
		if (timerTicks.w[0]&0x8000) // See if it overflowed (full second / 32768 ticks)
		{
			timerTicks.w[1]++;		// Increment seconds
			retval = 1;				// Set flag for second overflow
		}
		timerTicks.w[0] <<= 1;		// Shift ticks back so timerTicks is still valid

		// Error correction
		errorAccumulator += timerError;	// Accumulate the error 
		if (errorAccumulator >= t1rate)
		{
			errorAccumulator -= t1rate;
			if(PR1 != pr1H) PR1 = pr1H;
		}
		else
		{
			if(PR1 != pr1L) PR1 = pr1L;	
		}
	}
	return retval;
}

unsigned long TimerTicks(void)
{
	unsigned short tmr1 = TMR1;
	DWORD_VAL ticks = timerTicks;	// Latest
	// If interrupt occurs here then ticks + TMR1 will not be valid (short by PR1+1 << 1) 
	if(tmr1 > TMR1) return 	(ticks.Val + ((PR1+1)<<1)); // It just over
	else 			return	(ticks.Val + (tmr1<<1));
}	

//EOF
