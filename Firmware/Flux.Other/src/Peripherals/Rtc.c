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

// Includes

// Microchip RTCC support
#if defined(__18CXX) || defined(__C30__) || defined(__PIC32MX__)
//#include <Rtcc.h>
#include "HardwareProfile.h"
#endif

#include "Peripherals/Rtc.h"

//Defines
// (Private) Checks whether a value is valid BCD - do not use this macro where evaluating the parameter has a side effect (e.g. a function call reading a value)
#define ValidBcd(_value) ((((_value) & 0xf0) <= 0x90) && (((_value) & 0x0f) <= 0x09))
// (Private) Convert a value from BCD - do not use this macro where evaluating the parameter has a side effect (e.g. a function call reading a value)
//static unsigned char FromBcd(unsigned char value) { return ((value & 0xf0) >> 4) * 10 + (value & 0x0f); }
#define FromBcd(_value) ((((_value) & 0xf0) >> 4) * 10 + ((_value) & 0x0f))
// (Private) Convert a value to BCD - do not use this macro where evaluating the parameter has a side effect (e.g. a function call reading a value)
//static unsigned char ToBcd(unsigned char value) { return ((value / 10) << 4) | (value % 10); }
#define ToBcd(_value) ((((_value) / 10) << 4) | ((_value) % 10))
// BCD Nibble to ASCII low and high nibbles - NUMERALS ONLY NOT HEX
#define my_BCDtoaL(_nibble)  ((_nibble&0xf)+'0')
#define my_BCDtoaH(_nibble)  ((_nibble>>4)+'0')

// Globals -- last time stored as two buffers so one won't be accessed while the other is overwritten
static unsigned char rtcLastTimeIndex = 0xff;
static rtcInternalTime rtcLastTime[2];
static char rtcString[23];
static const unsigned char maxDaysPerMonth[12+1] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
volatile unsigned short rtcTicksSeconds = 0;
unsigned short rtcRate = 0;
volatile signed short rtcTimerTotal = 0;

// KL: Adding patched for new processor - requires Rtcc.c for the device
#ifdef __PIC24FJ1024GB606__
#include "time.h"
extern void RTCC_LockOn(void);
    
#endif

#if (defined(RTC_SWWDT) || defined(RTC_SWWDT_TIMEOUT))
volatile unsigned int rtcSwwdtValue = 0;
#endif

// Checks that a BCD-coded internal time is valid
char RtcValidInternal(rtcInternalTime *v)
{
#ifdef __C30__
        if (   ValidBcd(v->f.year)
            && ValidBcd(v->f.mon)
            && ValidBcd(v->f.mday)
            && ValidBcd(v->f.hour)
            && ValidBcd(v->f.min)
            && ValidBcd(v->f.sec)
            && FromBcd(v->f.year) <= 99
            && FromBcd(v->f.mon) >= 1
            && FromBcd(v->f.mon) <= 12
            && FromBcd(v->f.mday) >= 1
            && FromBcd(v->f.mday) <= maxDaysPerMonth[FromBcd(v->f.mon)]
            && FromBcd(v->f.hour) <= 23
            && FromBcd(v->f.min) <= 59
            && FromBcd(v->f.sec) <= 59)
        {
            return 1;
        }
        return 0;
#elif defined __C32__
	    if (   ValidBcd(v->date.year)
            && ValidBcd(v->date.mon)
            && ValidBcd(v->date.mday)
            && ValidBcd(v->time.hour)
            && ValidBcd(v->time.min)
            && ValidBcd(v->time.sec)
            && FromBcd(v->date.year) <= 99
            && FromBcd(v->date.mon) >= 1
            && FromBcd(v->date.mon) <= 12
            && FromBcd(v->date.mday) >= 1
            && FromBcd(v->date.mday) <= maxDaysPerMonth[FromBcd(v->date.mon)]
            && FromBcd(v->time.hour) <= 23
            && FromBcd(v->time.min) <= 59
            && FromBcd(v->time.sec) <= 59)
        {
            return 1;
        }
        return 0;
#elif defined RTC_NO_INTERNAL_FORMAT
    return 1;
#else
	#error "Compiler?"
#endif
}


// Initialize the RTC
void RtcStartup(void)
{
	IEC0bits.T1IE = 0;
	IEC3bits.RTCIE = 0;

    rtcLastTimeIndex = 0;
    rtcLastTime[rtcLastTimeIndex].l[0] = 0; 
	rtcLastTime[rtcLastTimeIndex].l[1] = 0;
#ifdef __PIC24FJ1024GB606__ // KL: From code configurator
   CLOCK_SOSCEN();
   RTCCON1Lbits.RTCEN = 0; 
// KL Alarm 'chime' per second
RTCCON1H = 0xC101;   
   // PWCPS 1:1; PS 1:1; CLKSEL SOSC; FDIV 0; 
   RTCCON2L = 0x0000;
   // DIV 16384 -> 2Hz clock ; 
   RTCCON2H = 16383;
   // PWCSTAB 0; PWCSAMP 0; 
   RTCCON3L = 0x0000;
   // RTCEN enabled; OUTSEL Alarm Event; PWCPOE disabled; PWCEN disabled; WRLOCK disabled; PWCPOL disabled; TSAEN disabled; RTCOE disabled; 
   RTCCON1L = 0x8000; 
   RTCC_LockOn();    
   RtccReadTimeDate(&rtcLastTime[rtcLastTimeIndex]);
#elif defined (__C30__)
    CLOCK_SOSCEN();
	RtccInitClock();
	RtccWrOn();
	mRtccOn();
	mRtccWrOff();
    RtccReadTimeDate(&rtcLastTime[rtcLastTimeIndex]);
#elif defined(__C32__)
	CLOCK_SOSCEN();
  	RtccInit();
	RtccEnable();
	RtccGetTimeDate(&rtcLastTime[rtcLastTimeIndex].time,&rtcLastTime[rtcLastTimeIndex].date);
#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif
	// Check the time is a valid date
    if (!RtcValidInternal(&rtcLastTime[rtcLastTimeIndex]))
    {
        RtcClear();
    }

	RtcSwwdtReset();

    // Setup timer
	T1CONbits.TSYNC = 0;// Sync - for read/write alignment issue
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 0;// Timer on in idle mode
	PR1 = 0xFFFF;		// Timer period of 2 seconds
	TMR1 = 0;			// Clear timer
	T1CONbits.TON = 1;	// Timer on

    // Wait until first RTC second elapses to ensure the sub-second timer is synchronized
    {
        char originalBusSense = USB_BUS_SENSE;
        // Should wake when timer flag elapses (up to 1 second), but will also exit after two seconds with TMR1, or if USB connection is toggled
        IFS0bits.T1IF = 0;
        IFS3bits.RTCIF = 0;

		// FIX by KL, speeds up RTC startup
#if !defined(__PIC24FJ64GB002__) && !defined(__PIC24FJ64GB004__) && !defined(__dsPIC33EP256MU806__)
		OSCCONBITS OSCCONBITS_backup = OSCCONbits;
		OSCCONBITS OSCCONBITS_copy = OSCCONbits;
		asm("DISI #0x3FFF");
		OSCCONBITS_copy.NOSC = 0b101;		
		OSCCONBITS_copy.OSWEN = 1;
		__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );
		__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );
		asm("DISI #0");	/*Internal RC oscillator*/
		while (OSCCONbits.OSWEN == 1);
#endif
		int timeout = 3000; // Timeout to stop lock up
        while (!IFS3bits.RTCIF && (USB_BUS_SENSE == originalBusSense) && timeout--)
        { 
        	LED_SET(LED_BLUE); LED_SET(LED_OFF);
        }

		TMR1 = 0; // If the RTC started successfully, timer1 has the fractional count
		// To maintain an accurate factional count the RTC interrupts must be on. 
		rtcTicksSeconds = 0;
#if !defined(__PIC24FJ64GB002__) && !defined(__PIC24FJ64GB004__)&& !defined(__dsPIC33EP256MU806__)
		asm("DISI #0x3FFF");
		OSCCONBITS_backup.OSWEN = 1;
		__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_backup)[1] );
		__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_backup)[0] );
		asm("DISI #0");
		while (OSCCONbits.OSWEN == 1);
#endif

    }

}


void RtcInterruptOn(unsigned short newRate)
{
	RTC_IPL_shadow_t IPLshadow;

    // RTC and timer interrupts off
	RTC_INTS_DISABLE();

#ifdef __C30__

	IPC15bits.RTCIP = RTC_INT_PRIORITY;
	IPC0bits.T1IP = T1_INT_PRIORITY;

    rtcRate = newRate;
#ifdef __PIC24FJ1024GB606__
    // Not needed, chine is already enabled with ISR disabled
#else
	// Chime every 1 second to RTC int vector
	RtccWrOn();
	//mRtccOn();
	RtccWriteAlrmTimeDate(&rtcLastTime[rtcLastTimeIndex]);
    // Interrupt will fire at next second, chime will continue at 1 second intervals
	RtccSetChimeEnable(TRUE, TRUE);
	RtccSetAlarmRpt(RTCC_RPT_SEC, TRUE);
	RtccSetAlarmRptCount(1, TRUE);  // Repeat once
	//mRtccSetAlrmPtr(0b0000);
	mRtccAlrmEnable();
	mRtccWrOff();
#endif
    if (rtcRate == 0)
    {
		PR1 = 0xffff;
		TMR1 += rtcTimerTotal;
		rtcTimerTotal = 0;
	    // Enable RTC interrupts
	    IFS3bits.RTCIF = 0;
	    IEC3bits.RTCIE = 1; // Just enable RTC chime ints
	}
	else
	{
	    // Clear counter and precision timer
	    PR1 = ((unsigned long)((32768L)/rtcRate)-1);		// Period
	    rtcTimerTotal += TMR1;  // This never increments if the timer1 isr is never called
		TMR1 = 0;
	    // Enable RTC and timer interrupts	
		IFS0bits.T1IF = 0;
		IFS3bits.RTCIF = 0;
	    IEC3bits.RTCIE = 1;
		IEC0bits.T1IE = 1;
	}	

	RTC_INTS_ENABLE();
	return;

#elif defined(__C32__)

#error "THIS IS NOW BROKEN, use c30 case to fix"
    rtcRate = newRate;
	// Chime every 1 second to RTC int vector
	RtccSetAlarmTimeDate(rtcLastTime[rtcLastTimeIndex].l[0], rtcLastTime[rtcLastTimeIndex].l[1]);
	RtccSetAlarmRptCount(0);
	RtccSetAlarmRpt(RTCC_RPT_SEC);
	RtccChimeEnable();
	RtccAlarmEnable();

    // Setup timer
	T1CONbits.TON = 0;	// Timer off
	IPC1bits.T1IP = 6;	// High priority
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 0;	// Int off 
    
	T1CONbits.TSYNC = 0;	// The timer will not clock if synced in sleep! :( - this means reads to tmr1 need two attempts
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 0;// 0 = timer not stopped in idle mode
    // Default PR is max. (2 seconds) -- we want precise time from TMR1, so we don't want overflows (we can saturate the reading to 0x7fff)
    PR1 = 0xffff;
    TMR1 = 0x0000;                  // Reset timer
	T1CONbits.TON = 1;	// Timer on

    // Clear counter and precision timer
    rtcTicksSeconds = 0;
    TMR1 = 0;
    rtcTimerTotal = 0;  // This never increments if the timer1 isr is never called

    // Wait until first RTC second elapses to ensure the sub-second timer is synchronized
    {
        char originalBusSense = USB_BUS_SENSE;
        // Should wake when timer flag elapses (up to 1 second), but will also exit after two seconds with TMR1, or if USB connection is toggled
        IFS0CLR = _IFS0_T1IF_MASK;
        IFS1CLR = _IFS1_RTCCIF_MASK;
        while (!IFS1bits.RTCCIF && !IFS0bits.T1IF && (USB_BUS_SENSE == originalBusSense))
        { 
        	LED_SET(LED_BLUE); LED_SET(LED_OFF);
        }
    }


    // Set up TMR1 PR for precise operation at designated frequency if needed - independant to the rtcc
    if (rtcRate > 0)
    {
        PR1 = ((unsigned long)((32768L)/rtcRate)-1);		// Period
    }

    // Enable RTC interrupts
    TMR1 = 0x0000;
	IPC8bits.RTCCIP = RTC_INT_PRIORITY;
    IFS1CLR = _IFS1_RTCCIF_MASK;

    // Enable TMR1 interrupts if exact rate is required
	IPC1bits.T1IP = T1_INT_PRIORITY;
    IFS0CLR = _IFS0_T1IF_MASK;

    if (rtcRate > 0)
    {
    	IEC0SET = _IEC0_T1IE_MASK;	// Start vectoring
    }
    IEC1SET = _IEC1_RTCCIE_MASK;

 	INTEnableSystemMultiVectoredInt(); 
	RTC_INTS_ENABLE();
#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif

}

void RtcInterruptOff(void)
{
#ifdef __C30__
    IEC3bits.RTCIE = 0;
    IEC0bits.T1IE = 0;

    // Timer off and timer interrupts off
    T1CONbits.TON = 0;
    IFS0bits.T1IF = 0;
    TMR1 = 0x0000;
    PR1 = 0xffff;

    // RTC interrupts off
    IEC3bits.RTCIE = 0;

#elif defined(__C32__)

	IFS1CLR = _IFS1_RTCCIF_MASK;
	IFS0CLR = _IFS0_T1IF_MASK;
	TMR1CLR = 0xFFFFFFFF;

#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif

    // Reset timers/counters
    rtcRate = 0;
    rtcTimerTotal = 0;
    rtcTicksSeconds = 0;
}

void RtcWrite(DateTime value)
{
	RTC_IPL_shadow_t IPLshadow;
	RTC_INTS_DISABLE();
#ifdef __PIC24FJ1024GB606__
    // KL: To prevent having to write my own driver for new hardware
    struct tm stupidFormatTime;
    stupidFormatTime.tm_sec =     DATETIME_SECONDS(value);
    stupidFormatTime.tm_min =     DATETIME_MINUTES(value); 
    stupidFormatTime.tm_hour =    DATETIME_HOURS(value);
    stupidFormatTime.tm_mday =    DATETIME_DAY(value);
    stupidFormatTime.tm_mon =     DATETIME_MONTH(value); 
    stupidFormatTime.tm_year =    DATETIME_YEAR(value);
    stupidFormatTime.tm_wday =    0;
    stupidFormatTime.tm_yday =    0;
    stupidFormatTime.tm_isdst =   0;
    RTCC_TimeSet(&stupidFormatTime);
#elif defined (__C30__)
    if (rtcLastTimeIndex != 0 && rtcLastTimeIndex != 1) { rtcLastTimeIndex = 0; }
    rtcLastTime[rtcLastTimeIndex] = *RtcToInternal(value);
	RtccWrOn();
	mRtccOn();
	RtccWriteTimeDate(&rtcLastTime[rtcLastTimeIndex], 1);
    TMR1 = 0;
	rtcTimerTotal = 0;
    rtcTicksSeconds = 0;
	mRtccWrOff();
#elif defined(__C32__)
    if (rtcLastTimeIndex != 0 && rtcLastTimeIndex != 1) { rtcLastTimeIndex = 0; }
    rtcLastTime[rtcLastTimeIndex] = *RtcToInternal(value);
	RtccSetTimeDate(rtcLastTime[rtcLastTimeIndex].l[0],rtcLastTime[rtcLastTimeIndex].l[1]);
    TMR1 = 0;
	rtcTimerTotal = 0;
    rtcTicksSeconds = 0;
#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif
	RTC_INTS_ENABLE();
}

inline void RtcTasks(void)
{
	RTC_IPL_shadow_t IPLshadow;
	unsigned short t1read;
#ifdef __C30__
    // Check interrupt flag
    if (IFS3bits.RTCIF)
    {
        unsigned char nextIndex;
    	IFS3bits.RTCIF = 0;

        RTC_INTS_DISABLE();             // Stop interrupts

        if (!rtcLastTimeIndex) { nextIndex = 1; }
        else { nextIndex = 0; }

        // Directly reads the RTC (WARNING: RTC interrupts must not be on)
        RtcRead(&rtcLastTime[nextIndex]);   // Read into next buffer (nobody should see a partial-overwrite)
        rtcLastTimeIndex = nextIndex;   // Save current index
        rtcTicksSeconds++;              // Increment second counter

		if (rtcRate == 0) // Indicates timer1 period is unused
		{
			#ifdef __dsPIC33E__
			T1CONbits.TON = 0;
			TMR1 = 0; // Clear the timer
			T1CONbits.TON = 1;
			#else
			TMR1 = 0; // Clear the timer
			#endif
		}
		else
		{
			// Read TMR1 whilst protecting agains overflows
			t1read = TMR1; /*But it could have just overflowed and reset tmr1...*/
			if (IFS0bits.T1IF)	{t1read = (PR1+1); } 
			// Effectively subtract the current value from the accumulator,
			// if overflowed then the T1ISR will add PR1 to the accumulator as soon as this funct returns
			rtcTimerTotal = -t1read;
		}
        RTC_INTS_ENABLE();             // Enable interrupts
    }
#elif defined(__C32__)
    // Check interrupt flag
    if (IFS1bits.RTCCIF)
    {
        unsigned char nextIndex;
    	IFS1CLR = _IFS1_RTCCIF_MASK;

        RTC_INTS_DISABLE();             // Stop interrupts

        if (!rtcLastTimeIndex) { nextIndex = 1; }
        else { nextIndex = 0; }

        // Directly reads the RTC (WARNING: RTC interrupts must not be on)
        RtcRead(&rtcLastTime[nextIndex]);   // Read into next buffer (nobody should see a partial-overwrite)
        rtcLastTimeIndex = nextIndex;   // Save current index
        rtcTicksSeconds++;              // Increment second counter

		if (rtcRate == 0) // Indicates timer1 period is unused
		{
			TMR1 = 0; // Clear the timer
		}
		else
		{
			unsigned short t1readA, t1readB;
			// Read TMR1 whilst protecting agains overflows
			t1readA = TMR1; t1readB = TMR1;/*But it could have just overflowed and reset tmr1...*/
			if (t1readA != t1readB) t1readA = t1readB; // Protect from overflow
			if (IFS0bits.T1IF)	{t1readA = (PR1+1); } 
			// Effectively subtract the current value from the accumulator,
			// if overflowed then the T1ISR will add PR1 to the accumulator as soon as this funct returns
			rtcTimerTotal = -t1readA;
		}
        RTC_INTS_ENABLE();             // Enable interrupts
    }
#elif defined RTC_NO_INTERNAL_FORMAT
    t1read; // ignore
#else
#error "Compiler?"
#endif
}

// TMR1 tasks
inline void RtcTimerTasks(void)
{
#ifdef __C30__
	// By this timer TMR1 is already zero
    if (IFS0bits.T1IF)
    {
        IFS0bits.T1IF = 0;
		if  (rtcRate != 0)	rtcTimerTotal += (PR1+1);
	}
#elif defined(__C32__)
	// By this timer TMR1 is already zero
    if (IFS0bits.T1IF)
    {
        IFS0CLR = _IFS0_T1IF_MASK;
		if  (rtcRate != 0)	rtcTimerTotal += (PR1+1);
	}
#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif
}

DateTime RtcNow(void)
{
#if defined (__C30__)

    // If interrupts aren't on, update the time now (if they are on, the time and fractional time is kept up to date in the interrupt handler)
    if (!IEC3bits.RTCIE)
    {
        unsigned char nextIndex;
        if (!rtcLastTimeIndex) { nextIndex = 1; } else { nextIndex = 0; }
        RtcRead(&rtcLastTime[nextIndex]);  // read into next buffer (nobody should see a partial-overwrite)
        rtcLastTimeIndex = nextIndex;   // Save current index
    }

#elif defined(__C32__)

    // If interrupts aren't on, update the time now (if they are on, the time and fractional time is kept up to date in the interrupt handler)
    if (!IEC1bits.RTCCIE)
    {
        unsigned char nextIndex;
        if (!rtcLastTimeIndex) { nextIndex = 1; } else { nextIndex = 0; }
        RtcRead(&rtcLastTime[nextIndex]);  // read into next buffer (nobody should see a partial-overwrite)
        rtcLastTimeIndex = nextIndex;   // Save current index
    }

#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif

    // Read from double-buffered data protects from overwrite by interrupt
    if (rtcLastTimeIndex >= 2) { return 0; }
    return RtcFromInternal(&rtcLastTime[rtcLastTimeIndex]);
}


DateTime RtcNowFractional(unsigned short *fractional)
{
    unsigned short fraction;
	unsigned short t1read;
    unsigned long time = 0;
	RTC_IPL_shadow_t IPLshadow;

    // Save and disable interrupt state
	RTC_INTS_DISABLE();
	
	// Grab the milliseconds first for better accuracy
	#ifdef __C30__
		/*	If timer1 is off and rtc rate is zero, timer1 is reset in the rtc interrupt
			hence tmr1 is the fractional part. 
			If timer1 is on, rtcrate != 0, we need to use the new method as used below.
		*/
		if (IEC0bits.T1IE)
		{
			fraction = rtcTimerTotal; // None volatile whilst in the isr if at same priority
			t1read = TMR1; /*But it could have just overflowed and reset tmr1...*/
			if (IFS0bits.T1IF)	{t1read = (PR1+1); } 
		}
		else
		{
			t1read = TMR1; /* Timer1 is off, fraction is always tmr1, rtc clears count*/
			fraction = 0; 
		}

	#elif defined (__C32__)
	{
		unsigned short t1readA, t1readB;
		fraction = rtcTimerTotal; // None volatile whilst in the isr if at same priority
		// Read TMR1 whilst protecting agains overflows
		t1readA = TMR1; t1readB = TMR1;/*But it could have just overflowed and reset tmr1...*/
		if (t1readA != t1readB) t1read = t1readB; // Protect from overflow
		else t1read = t1readA;
		if (IFS0bits.T1IF)	{t1read = (PR1+1); } 
	}
    #elif defined RTC_NO_INTERNAL_FORMAT
        t1read = 0;
    #else
        #error "Compiler?"
	#endif

    // Read from double-buffered data protected from overwrite by interrupt disable
    if (rtcLastTimeIndex < 2) {time = RtcFromInternal(&rtcLastTime[rtcLastTimeIndex]);}

    // Restore interrupt state
	RTC_INTS_ENABLE();

    // Get fractional part
	fraction += t1read;							// Add up the total and latched timer value
	if (fraction > 0x7fff) fraction = 0x7fff;	// saturate. Small error here but thats ok
    fraction <<= 1;                             // scale

    // Output fractional part...
    *fractional = fraction;
	// ...and return the time.
    return time;
}


// Reads the current second count since start (wraps every 18.2 hours) as a 16.16 fixed point number
unsigned long RtcTicks(void)
{
    unsigned short fraction;
	unsigned short t1read;
    unsigned long ticks;
	RTC_IPL_shadow_t IPLshadow;

    // Save and disable interrupt state
	RTC_INTS_DISABLE();

	// Grab the milliseconds first for better accuracy
	fraction = rtcTimerTotal; // None volatile whilst in the isr if at same priority

	// Grab the milliseconds first for better accuracy
	#ifdef __C30__
		t1read = TMR1; /*But it could have just overflowed and reset tmr1...*/
		if ((IFS0bits.T1IF)&&(IEC0bits.T1IE))	{t1read = (PR1+1); } 
	#elif defined (__C32__)
	{
		unsigned short t1readA, t1readB;
		// Read TMR1 whilst protecting agains overflows
		t1readA = TMR1; t1readB = TMR1;/*But it could have just overflowed and reset tmr1...*/
		if (t1readA != t1readB) t1read = t1readB; // Protect from overflow
		else t1read = t1readA;
		if (IFS0bits.T1IF)	{t1read = (PR1+1); } 
	}
    #elif defined RTC_NO_INTERNAL_FORMAT
        t1read = 0;
	#endif

    // Whole-second count as top word
    ticks = ((unsigned long)rtcTicksSeconds) << 16;

	// Re-enable the ints
	RTC_INTS_ENABLE();
	
    // Get fractional part
	fraction += t1read;							// Add up the total and latched timer value
	if (fraction > 0x7fff) fraction = 0x7fff;	// saturate. Small error here but thats ok
    fraction <<= 1;                                              // scale

    // Add fractional part
    ticks |= fraction;                          // add as low word

    return ticks;
}

DateTime RtcCalcOffset(DateTime start, signed long seconds)
{
	// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
	const unsigned char shifts[3]	=	{0,	6,12};				// Bit shifts
	const unsigned char masks[3]	=	{0x3f,0x3f,0x1f};		// Field masks
	const unsigned char divisors[3] =  {60,60,24};				// Division table
	const unsigned char daysPerMonth[13] = {31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	DateTime retval = 0;										// Our return val, zero
	unsigned char i, backwards;
	signed short remainingDays;
	/*
		This is not easy to do efficiently since every field could roll over.
		The documentation says 32/16 (un)signed division is 19 cycles and HW 
		supported on dsPIC and PIC24. We will take advantage of this. 
	*/
	// Early out
	if(seconds == 0) 		return start;
	else if (seconds > 0) 	backwards = 0;
	else // Backwards operation
	{
		// Find the second wise *forward* time equivalent required
		/* 
			24hx60mx60s = 86400 = 675 * 2^7. A/86400 == (A/675)>>7 , so to find
			the remainder of A/86400 we can use R = ((675*N/A)&(1-2^7)+N%A)
		*/
		unsigned long temp32;							// Temporary 32bit var, abs secs
		unsigned short temp16 = 675;					// Using 32bit/16bit divide
		seconds = -seconds; 							// abs value
		temp32 = seconds/temp16; 						// made explicit to allow
		temp16 = seconds%temp16;						// the compiler to optimise
		temp32 = (temp32%0x7f)*675 + temp16;			// Get remainder of seconds/day											
		seconds =  (24ul*3600)+seconds-temp32-temp32;	// correct offset to add
		backwards = 1;									// set backwards flag 
	}

	// Find and set sec,min,hours,day fields
 	for(i=0;i<3;i++)
	{
		unsigned long temp = seconds;			// Current value sec/min/hours
		unsigned short divisor = divisors[i];	// Current divisor
		unsigned char field;					// The calculated field
		temp += (start>>(shifts[i]))&masks[i];	// Add field from the start time
		if(temp >= divisor)						// Fast short circuit
		{
			seconds =  temp / divisor;			// Divide (explicit for compiler optimiser)
			field = temp % divisor;				// Carry (explicit for compiler optimiser)
		}
		else
		{
			seconds = 0;						// Faster than divide
			field = temp;						// Set field 		
		}
		retval|=(unsigned long)field<<(shifts[i]);	// Set new value in retval
	}												// Seconds val is *days* if non zero
	remainingDays = seconds;					// Number of full days advanced forward 
	if(backwards)								// For reverse we have to correct rollover
	{
		// Check we didn't cross 00:00:00 time in roll back (o/p < i/p)
		if(retval < (start & 0x1ffff)) 	remainingDays--; // Subtract the extra day
		else 							remainingDays++; // Add the extra day	
		remainingDays = -remainingDays;
	}

	// Add on extra days (stored in seconds)
	if(remainingDays == 0)
	{
		// Set same date, done....
		retval |= (start&0xfffe0000);
	}
	else // We are adding/subtracting days too, iteratively add to moth date
	{
		unsigned char year = DATETIME_YEAR(start);			// The year we are in
		unsigned char month = DATETIME_MONTH(start);		// The month we are in
		unsigned char days = DATETIME_DAY(start);			// The day we are in
		remainingDays+=days;								// Find new date (+ or -)
		
		for(;;)
		{
			// Evaluate each of these per iteration
			unsigned char maxDays = daysPerMonth[month-backwards];// Find max days in month (or previous)
			unsigned char isLeap = (!(year & 3))?1:0;			// Is it a leap year?
			if(((month-backwards) == 2)&& isLeap)maxDays++; 	// Account for feb 29th too

			if (remainingDays > maxDays) 						// If it is outside the month
			{
				remainingDays -= maxDays;						// Subtract the days in this month 
				month++;										// Go into next month
				if(month > 12)									// If we are outside the year	
				{
					month = 1;									// Go back to January
					year++;										// Go to next year
				}	
			}
			else if (remainingDays < 1) 						// If it is outside the month backwards
			{
				remainingDays += maxDays;						// Remove the days in the backwards month
				month--;										// Go into previous month
				if(month < 1)									// If we went past January	
				{
					month = 12;									// Go back to December
					year--;										// Go back a year
				}	
			}
			else												
			{
				days = remainingDays;							// Set the value
				break;											// Exit
			}
		}
		// Re-assemble the date in place
		retval |=(((unsigned long)(days) & 0x1f) << 17);	
		retval |=(((unsigned long)(month) & 0x0f) << 22);
		retval |=(((unsigned long)(year % 100) & 0x3f) << 26);
	}
	// Return
	return retval;
}

/*
void SetupAlarm(void)
{
	RtccWrOn();
	RtccWriteAlrmTimeDate(&rtcTime);
	RtccSetChimeEnable(FALSE,TRUE);
	//RtccSetAlarmRpt(RTCC_RPT_HALF_SEC, TRUE);
	RtccSetAlarmRpt(RTCC_RPT_SEC, TRUE);
	RtccSetAlarmRptCount(1, TRUE); // Repeat once 
	mRtccSetAlrmPtr(0b0110); // How many fields need to match for alarm to occur - HH:MM:SS, p261 datasheet
	mRtccAlrmEnable();
	mRtccSetIntPriority(4);	
	mRtccWrOff();
}

void SetupChimeOn (void) // Note: Chime enable just lets the alarm keep repeating indefinitely, still need one alarm to start it.
{
	RtccWrOn();
	RtccWriteAlrmTimeDate(&rtcTime);
	RtccSetChimeEnable(TRUE,TRUE);
	RtccSetAlarmRpt(RTCC_RPT_SEC, TRUE);
	RtccSetAlarmRptCount(1, TRUE); // Repeat once 
	mRtccSetAlrmPtr(0b0000); //Interrupt will fire <0.5s and chime will continue at rate
	mRtccAlrmEnable();
	mRtccSetIntPriority(4);	
	mRtccWrOff();
}

void SetupChimeOff (void) 
{
	RtccWrOn();
	RtccSetChimeEnable(FALSE,TRUE);
	mRtccWrOff();
}
*/


// Convert a date/time number from a string ("YY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
DateTime RtcFromString(const char *value)
{
    int index;
    unsigned int v;
    unsigned char year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    const char *c;

	if (value[0] == '\0') { return 0; }
	if (value[0] == '0' && value[1] == '\0') { return 0; }
	if (value[0] == '-') { return 0xffffffff; }
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
            (day >= 1 && day <= maxDaysPerMonth[month]) &&
            (hours <= 23) &&
            (minutes <= 59) &&
            (seconds <= 59)
        )
    {
		if (month == 2 && day == 29 && (year & 3) != 0) { month = 3; day = 1; }	// Turn 29-Feb in non-leap years into 1-Mar
        return DATETIME_FROM_YMDHMS(year, month, day, hours, minutes, seconds);
    }
    else
    {
        return 0;
    }
}


// Convert a date/time number to a string ("yyYY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
const char *RtcToString(DateTime value)
{
    // "yyYY/MM/DD,HH:MM:SS+00"
    char *c = rtcString;
    unsigned int v;
	if (value < DATETIME_MIN) { *c++ = '0'; *c++ = '\0'; }				// "0"
	else if (value > DATETIME_MAX) { *c++ = '-'; *c++ = '1'; *c++ = '\0'; }	// "-1"
	else
	{
	    v = 2000 + DATETIME_YEAR(value); *c++ = '0' + ((v / 1000) % 10); *c++ = '0' + ((v / 100) % 10); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	    v = DATETIME_MONTH(value);       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	    v = DATETIME_DAY(value);         *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ',';
	    v = DATETIME_HOURS(value);       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = DATETIME_MINUTES(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = DATETIME_SECONDS(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10);
	    //*c++ = '+'; *c++ = '0'; *c++ = '0';
	    *c++ = '\0';
	}
    return rtcString;
}


// Convert a date/time from the implementation-specific representation
DateTime RtcFromInternal(rtcInternalTime *time)
{
	#ifdef __C30__
	// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
	unsigned char year = FromBcd(time->f.year);
	unsigned char mon = FromBcd(time->f.mon);
	unsigned char mday = FromBcd(time->f.mday);
	unsigned char hour = FromBcd(time->f.hour);
	unsigned char min = FromBcd(time->f.min);
	unsigned char sec = FromBcd(time->f.sec);
	#elif defined(__C32__)
	// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
	unsigned char year = FromBcd(time->date.year);
	unsigned char mon = FromBcd(time->date.mon);
	unsigned char mday = FromBcd(time->date.mday);
	unsigned char hour = FromBcd(time->time.hour);
	unsigned char min = FromBcd(time->time.min);
	unsigned char sec = FromBcd(time->time.sec);
    #elif defined RTC_NO_INTERNAL_FORMAT
    unsigned char year = 0, mon = 1, mday = 1, hour = 0, min = 0, sec = 0;
	#else
	#error "Compiler?"
	#endif

	unsigned long retval = sec;
	retval |= ((unsigned long)year)<<26;
	retval |= ((unsigned long)mon)<<22;
	retval |= ((unsigned long)mday)<<17;
	retval |= ((unsigned long)hour)<<12;
	retval |= ((unsigned long)min)<<6;

	return retval;
}


// Convert a date/time to the implementation-specific representation
rtcInternalTime *RtcToInternal(DateTime value)
{
    static rtcInternalTime retval;
    retval.l[0] = 0; retval.l[1] = 0;
#ifdef __C30__
    retval.f.mday = ToBcd(DATETIME_DAY(value));
    retval.f.mon  = ToBcd(DATETIME_MONTH(value));
    retval.f.year = ToBcd(DATETIME_YEAR(value));
    retval.f.hour = ToBcd(DATETIME_HOURS(value));
    retval.f.min  = ToBcd(DATETIME_MINUTES(value));
    retval.f.sec  = ToBcd(DATETIME_SECONDS(value));
#elif defined(__C32__)
    retval.date.mday = ToBcd(DATETIME_DAY(value));
    retval.date.mon  = ToBcd(DATETIME_MONTH(value));
    retval.date.year = ToBcd(DATETIME_YEAR(value));
    retval.time.hour = ToBcd(DATETIME_HOURS(value));
    retval.time.min  = ToBcd(DATETIME_MINUTES(value));
    retval.time.sec  = ToBcd(DATETIME_SECONDS(value));
#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif

    return &retval;
}

// Calibrate the RCT to speed up (+ive value) or slow down (-ive value) - send signed int in ppm +/- 258 ppm (PIC32 +/-253)
void RtcCal(signed short ppm)
{
#ifdef __PIC24FJ1024GB606__
    // KL: New hardware - function excluded since its never used anyway
        
#elif defined (__C30__)
	// RCFGCAL adjustments fix +/- 4 pulses per minute x the cal value
	// 4 pulses per minute = 2.0345 ppm, we will fix this based on one divde and offset
	// If the ppm adjust value is greater than +/-58, we subtract (or add) an extra 1 to the value to compensate

	// Grab sign
	signed char 	sign = 1;
	if (ppm<0) 		sign = -1;
	// Clamp to +/-258
	if ((ppm > 258)||(ppm <-258)) {(sign==1)? (ppm=258) : (ppm=-258);}
	// Compensate for inexact ppm - cal conversion
	ppm -= (ppm/58);
	// Scale to multiples of 2ppm 
	ppm>>=1;
	RCFGCALbits.CAL = (signed char)(ppm&0xff);

#elif defined(__C32__)
	// RTCCONbits.CAL adjustments fix +/- 1 pulses per minute x the cal value
	// 1 pulses per minute = 0.5086 ppm, we will fix this based on one divde and offset
	// If the ppm adjust value is greater than +/-30, we subtract (or add) an extra 1 to the value to compensate
	// i.e. cal = ppm/0.5086, cal = ppm*1.966 = ppm*2 + ppm*(-0.0338) ~= ppm*2 - ppm/30

	// Grab sign
	signed short	correction;
	signed char 	sign = 1;
	if (ppm<0) 		sign = -1;
	// Clamp 
	if ((ppm > 253)||(ppm <-253)) {(sign==1)? (ppm=253) : (ppm=-253);}
	// Compensate for inexact ppm - cal conversion
	correction = (ppm/30); // +/-2
	// Scale to multiples of 0.5ppm + correct
	ppm<<=1;
	ppm -= correction;

	RtccSetCalibration(ppm&0x1ff);

#elif defined RTC_NO_INTERNAL_FORMAT
    // ignore
#else
#error "Compiler?"
#endif

	return;	
}
#ifdef __C32__
// Directly reads the RTC (WARNING: RTC interrupts must not be on)
void RtcRead(rtcInternalTime *time)
{
	RtccGetTimeDate((rtccTime*)&time->l[0], (rtccDate*)&time->l[1]);
}
#endif
//EOF


