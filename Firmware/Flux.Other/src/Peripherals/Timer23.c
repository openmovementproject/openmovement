// KL: Written to provide a system tick count and periodic callback function
/*
Timer2/3.h emulates the most basic RTC functions and allows 
a time base using the T2/3 timer to be generated, P24/33 only
*/
/*	At 48MIPS + No prescaler yeilds
		64 bit rollover is after 12 thousand years
		32 bit rollover is after 89.5 seconds
		16 bit rollover is after 1.36 milliseconds 
*/ 
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "Peripherals/Timer23.h"
#include "HardwareProfile.h"

// Definitions required - try to avoid inefficient divides by > 16bits
#ifndef TICKS_TO_SEC
	#define TICKS_TO_SEC(_ticks) (((long long)_ticks)/((long)GetInstructionClock()))
#endif
#ifndef TICKS_TO_MS
	#define TICKS_TO_MS(_ticks) (((long long)1000 * _ticks)/((long)GetInstructionClock()))
#endif
#ifndef TIMER23_TICK_PRIORITY
	#define TIMER23_TICK_PRIORITY 4
#endif

// Globals
static TickCB_t tickCB = NULL;

// Private Prototypes
static void GetTickCopy(void);

// Globals
QWORD_VAL ticksSaved = {0};				// Safe copy of the tick grabbed
volatile QWORD_VAL timerTicks = {0}; 	// 64 bit tick count
volatile unsigned int SwwdtValue = 0;	// For RTC functions if used
static unsigned long timerError = 0;	// Since the rate is not normally exact, it needs compensating
static unsigned long errorAccumulator = 0;
static unsigned long timerRate = 1;
static unsigned long prH = 0xffffffff,prL=0xffffffff;
static unsigned long timerPeriod = 0xffffffff;

// Interrupt
void __attribute__((interrupt,auto_psv)) _T3Interrupt(void)
{
	// Clear flag
	IFS0bits.T3IF = 0;			// Clear isr flag
	// Main counter
	timerTicks.Val += timerPeriod+1;
	// Error correction
	errorAccumulator += timerError;	// Accumulate the error 
	if (errorAccumulator >= timerRate)
	{
		errorAccumulator -= timerRate;
		if(timerPeriod != prH) 
		{
			timerPeriod = prH;
			PR3 = timerPeriod >> 16;		
			PR2 = timerPeriod & 0xffff;	
		}
	}
	else
	{
		if(timerPeriod != prL) 
		{
			timerPeriod = prL;
			PR3 = timerPeriod >> 16;		
			PR2 = timerPeriod & 0xffff;	
		}			
	}
	// Callback
	if(tickCB!=NULL)tickCB();
}

// Source
void TimerSetCB(TickCB_t cb)
{
	tickCB = cb;
}

void TimerIsrDisable(void)
{
	IEC0bits.T3IE = 0;
}

void TimerIsrEnable(void)
{
	IEC0bits.T3IE = 1;
}

unsigned long TimerSeconds(void)
{
	GetTickCopy();
	return TICKS_TO_SEC(ticksSaved.Val);
}

unsigned long long TimerMilliSeconds(void)
{
	GetTickCopy();
	return TICKS_TO_MS(ticksSaved.Val);
}

unsigned short TimerGet16(void)
{
	GetTickCopy();
	return ticksSaved.w[0];
}

unsigned long TimerGet32(void)
{
	GetTickCopy();
	return ticksSaved.d[0];
}

unsigned long long TimerGet64(void)
{
	GetTickCopy();
	return ticksSaved.Val;
}

void TimerStartup(void)
{
	// Ints off
	IEC0bits.T3IE = 0;	// Disable isr
	T2CON = 0x0008; 	// 32bit mode
	T3CON = 0x0000; 	// Ignored reg
	// Init vars
	timerRate = 1; 
	prL = prH = 0xffffffff;
    // Setup timer for 1 Hz
	timerPeriod = GetInstructionClock() - 1;
	PR3 = timerPeriod >> 16;	// Set period	
	PR2 = timerPeriod & 0xffff;	// Set period
	TMR3 = 0;				// Clear count
	TMR2 = 0;				// Clear count
	timerTicks.Val = 0; 	// Clear second count
	errorAccumulator = 0;	// Clear value here
	timerError = 0;			// Exactly 1 Hz
	// Interrupts
	IPC2bits.T3IP = TIMER23_TICK_PRIORITY; // Int priority
	IFS0bits.T3IF = 0;	// Flag cleared
	IEC0bits.T3IE = 1;	// Enable isr
	T2CONbits.TON = 1;	// Start timer
	// Done.. timer is running if clocked
}

void TimerInterruptOn(unsigned short newRate)
{
    // Timer interrupts off
	IEC0bits.T3IE = 0;
	// Prevent div by zero
    if (newRate == 0)newRate=1;
	timerRate = newRate;
    // Clear counter and set precision timer
    prL = timerPeriod = (((GetInstructionClock())/newRate)-1);
	prH = prL+1;
	timerError = GetInstructionClock() - ((timerPeriod+1) * newRate);// Error in ticks/second
	errorAccumulator = 0;
	// Set new period, clear timer
	PR3 = timerPeriod >> 16;	// Set period	
	PR2 = timerPeriod & 0xffff;	// Set period
	TMR3 = 0;				// Clear count
	TMR2 = 0;				// Clear count	
    // Enable RTC and timer interrupts	
	IEC0bits.T3IE = 1;	
}

void TimerInterruptOff(void)
{
    IEC0bits.T3IE = 0;	// Stop interrupt
    T2CONbits.TON = 0;	// Stop timer
}

static void GetTickCopy(void)
{
	static unsigned char isrPend;

	// Pause isr
	IEC0bits.T3IE = 0; 				
	Nop();

	/* 	If isr is not pending, read timer 2, this latches timer 3.
		Now check if it has overflowed, if so, a re-read is needed
		and the overflow will have to be added as the isr is blocked
	*/	
	isrPend = IFS0bits.T3IF;	
	ticksSaved.w[0] = TMR2;
	if(isrPend != IFS0bits.T3IF)
	{
		isrPend = TRUE;
		ticksSaved.w[0] = TMR2;
	}

	ticksSaved.w[1] = TMR3; 
	ticksSaved.d[1] = 0;
	ticksSaved.Val += timerTicks.Val;
	
	if(isrPend)
	{
		ticksSaved.Val += timerPeriod + 1;
	}

	// Re-enable isr
	IEC0bits.T3IE = 1; 
	return;						
}






