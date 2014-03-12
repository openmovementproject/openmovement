/*
Timer1.h emulates the most basic RTC functions and allows 
a time base using the T1 oscillator to be generated, P24 only
*/
#ifndef TIMER1_H
#define TIMER1_H
#include "GenericTypeDefs.h"

extern volatile DWORD_VAL timerTicks;

void TimerStartup(void);
void TimerInterruptOn(unsigned short newRate);
void TimerInterruptOff(void);
unsigned char TimerTasks(void);

// Get current tick
unsigned long TimerTicks(void);

// Create similar functionality to RTC driver
#define rtcTicksSeconds			(timerTicks.w[1])
#define RtcInterruptOn(_rate_) 	TimerInterruptOn(_rate_)

typedef unsigned char TIMER_IPL_shadow_t;
#define TIMER_INTS_DISABLE()	{IPLshadow = SRbits.IPL; if(SRbits.IPL<T1_INT_PRIORITY)SRbits.IPL=T1_INT_PRIORITY;}
#define TIMER_INTS_ENABLE()		{SRbits.IPL = IPLshadow;}

#if defined(RTC_SWWDT_TIMEOUT) && (RTC_SWWDT_TIMEOUT>0)
	extern volatile unsigned int SwwdtValue;
	// These functions are actually defines to ensure they're actually inline
	//#warning "SW WDT ENABLED"
	#define SwwdtReset() { SwwdtValue = 0; }
	#define SwwdtIncrement() { if (++SwwdtValue >= RTC_SWWDT_TIMEOUT) { LED_SET(LED_MAGENTA); DelayMs(1000); Reset(); } }
#else
	#define SwwdtReset() 	{}
	#define SwwdtIncrement() {}
#endif

#endif
//EOF


