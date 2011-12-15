/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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
// Karim Ladha, Dan Jackson, 2011

// Includes
#include <Rtcc.h>
#include "HardwareProfile.h"
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
volatile unsigned short rtcTimer = 0, rtcTimerTotal = 0;


// Checks that a BCD-coded internal time is valid
char RtcValidInternal(rtcInternalTime *v)
{
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
}


// Initialize the RTC
void RtcStartup(void)
{
    rtcLastTimeIndex = 0;
    rtcLastTime[rtcLastTimeIndex].l[0] = 0; rtcLastTime[rtcLastTimeIndex].l[1] = 0;

    CLOCK_SOSCEN();
	RtccInitClock();

	RtccWrOn();
	mRtccOn();
	mRtccWrOff();

	// Check the time is a valid date
    mRtccWaitSync(); // don't read until safe to do so.
    RtccReadTimeDate(&rtcLastTime[rtcLastTimeIndex]);
    if (!RtcValidInternal(&rtcLastTime[rtcLastTimeIndex]))
    {
        RtcClear();
    }

    RtcInterruptOff();
}


void RtcInterruptOn(unsigned short newRate)
{
    // RTC and timer interrupts off
    IEC3bits.RTCIE = 0;
	IEC0bits.T1IE = 0;

    rtcRate = newRate;

	// Chime every 1 second to RTC int vector
	RtccWrOn();
	//mRtccOn();
	RtccWriteAlrmTimeDate(&rtcLastTime[rtcLastTimeIndex]);
	RtccSetChimeEnable(TRUE, TRUE);
	RtccSetAlarmRpt(RTCC_RPT_SEC, TRUE);
	RtccSetAlarmRptCount(1, TRUE); // Repeat once
	mRtccSetAlrmPtr(0b0000); //Interrupt will fire <0.5s and chime will continue at rate
	mRtccAlrmEnable();
	mRtccSetIntPriority(4);
	mRtccWrOff();

    // Setup timer
	T1CONbits.TSYNC= 1;	// Sync - for read/write alignment issue
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 1;// Timer on in idle mode
	T1CONbits.TON = 1;	// Timer on
    // Default PR is max. (2 seconds) -- we want precise time from TMR1, so we don't want overflows (we can saturate the reading to 0x7fff)
    PR1 = 0xffff;
    // Clear counter and precision timer
    rtcTicksSeconds = 0;
    TMR1 = 0;
    rtcTimer = 0;
    rtcTimerTotal = 0;

    // Wait until first RTC second elapses to ensure the sub-second timer is synchronized
    {
        char originalBusSense = USB_BUS_SENSE;
        // Should wake when timer flag elapses (up to 1 second), but will also exit after two seconds with TMR1, or if USB connection is toggled
        IFS0bits.T1IF = 0;
        IFS3bits.RTCIF = 0;
        while (!IFS3bits.RTCIF && !IFS0bits.T1IF && (USB_BUS_SENSE == originalBusSense))
        { 
        	LED_SET(BLUE); LED_SET(OFF);
        }
    }
    TMR1 = 0;       // Zero TMR1 for this new second boundary

    // Set up TMR1 PR for precise sampling
    if (rtcRate > 0)
    {
        PR1 = ((unsigned long)((32768L)/rtcRate)-1);		// Period
    }

    // Enable RTC interrupts
    TMR1 = 0x0000;
    IFS3bits.RTCIF = 0;
    IEC3bits.RTCIE = 1;

    // Enable TMR1 interrupts if exact rate required
    if (rtcRate > 0)
    {
        IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 1;
    }
}


void RtcInterruptOff(void)
{
    // Timer off and timer interrupts off
    T1CONbits.TON = 0;
    IEC0bits.T1IE = 0;
    IFS0bits.T1IF = 0;
    TMR1 = 0x0000;
    PR1 = 0xffff;

    // RTC interrupts off
    IEC3bits.RTCIE = 0;
    IFS3bits.RTCIF = 0;

    // Reset timers/counters
    rtcRate = 0;
    rtcTimer = 0;
    rtcTimerTotal = 0;
    rtcTicksSeconds = 0;
}

void RtcWrite(DateTime value)
{
    char oldRTCIE = IEC3bits.RTCIE;
    IEC3bits.RTCIE = 0;
    if (rtcLastTimeIndex != 0 && rtcLastTimeIndex != 1) { rtcLastTimeIndex = 0; }
    rtcLastTime[rtcLastTimeIndex] = *RtcToInternal(value);
	RtccWrOn();
	mRtccOn();
	RtccWriteTimeDate(&rtcLastTime[rtcLastTimeIndex], 1);
    TMR1 = 0;
    rtcTicksSeconds = 0;
	mRtccWrOff();
    IEC3bits.RTCIE = oldRTCIE;
}

void RtcClear(void)
{
    RtcWrite(DATETIME_MIN);
}


// Directly reads the RTC (WARNING: RTC interrupts must not be on)
void RtcRead(rtcInternalTime *time)
{
    // WARNING: Don't call this code while RTC interrupts are enabled
    mRtccWaitSync(); // don't read until safe to do so.
    RtccReadTimeDate(time);
}


inline void RtcTasks(void)
{
    // Check interrupt flag
    if (IFS3bits.RTCIF)
    {
        unsigned char nextIndex;

    	mRtcc_Clear_Intr_Status_Bit;

        if (!rtcLastTimeIndex) { nextIndex = 1; }
        else { nextIndex = 0; }

        mRtccWaitSync(); // don't read until safe to do so (worst case 1ms, but probably not after 1 sec chime?)

        // Need DISI so that TMR1 will always match cached timestamp in any higher priority interrupts
        asm("DISI #0x3FF"); // Stop interrupts
        RtccReadTimeDate(&rtcLastTime[nextIndex]);  // read into next buffer (nobody should see a partial-overwrite)
        TMR1 = 0; // Clear TMR1 so it is synced to the RTC
        rtcLastTimeIndex = nextIndex;   // Save current index
        rtcTicksSeconds++;  // Increment second counter
        rtcTimer = 0;       // Zero sample timer counter for a new second
        rtcTimerTotal = 0;  // Zero total timer value for a new second
        asm("DISI #0"); // Enable interrupts

    }
}

DateTime RtcNow(void)
{
    // If interrupts aren't on, update the time now (if they are on, the time and fractional time is kept up to date in the interrupt handler)
    if (!IEC3bits.RTCIE)
    {
        unsigned char nextIndex;
        if (!rtcLastTimeIndex) { nextIndex = 1; } else { nextIndex = 0; }
        mRtccWaitSync(); // don't read until safe to do so (worst case 1ms, but probably not after 1 sec chime?)
        RtccReadTimeDate(&rtcLastTime[nextIndex]);  // read into next buffer (nobody should see a partial-overwrite)
        rtcLastTimeIndex = nextIndex;   // Save current index
    }

    // Read from double-buffered data protects from overwrite by interrupt
    if (rtcLastTimeIndex >= 2) { return NULL; }
    return RtcFromInternal(&rtcLastTime[rtcLastTimeIndex]);
}


DateTime RtcNowFractional(unsigned short *fractional)
{
    unsigned short f;
    unsigned long t;

    // Save and disable interrupt state
    char oldRTCIE = IEC3bits.RTCIE, oldT1IE = IEC0bits.T1IE;
    IEC3bits.RTCIE = 0;
    IEC0bits.T1IE = 0;

    // Get time
    t = RtcNow();

    // Get fractional part
    f = TMR1;                                           // acquire
    if (rtcRate == 0)
    {
        if (f > 0x7fff) { f = 0x7fff; }                 // saturate
    }
    else
    {
        if (rtcTimerTotal <= 0x7fff && 0x7fff - rtcTimerTotal >= t) { f += rtcTimerTotal; }
        else { f = 0x7fff; }                            // saturate
    }
    f <<= 1;                                            // scale

    // Output fractional part
    *fractional = f;

    // Restore interrupt state
    IEC3bits.RTCIE = oldRTCIE;
    IEC0bits.T1IE = oldT1IE;

    return t;
}


// Reads the current second count since start (wraps every 18.2 hours) as a 16.16 fixed point number
unsigned long RtcTicks(void)
{
    unsigned short f;
    unsigned long ticks;

    // Save and disable interrupt state
    char oldRTCIE = IEC3bits.RTCIE, oldT1IE = IEC0bits.T1IE;
    IEC3bits.RTCIE = 0;
    IEC0bits.T1IE = 0;

    // Whole-second count as top word
    ticks = ((unsigned long)rtcTicksSeconds) << 16;
    
    // Get fractional part
    f = TMR1;                                           // acquire
    if (rtcRate == 0)
    {
        if (f > 0x7fff) { f = 0x7fff; }                 // saturate
    }
    else
    {
        if (rtcTimerTotal <= 0x7fff && 0x7fff - rtcTimerTotal >= f) { f += rtcTimerTotal; }
        else { f = 0x7fff; }                            // saturate
    }
    f <<= 1;                                            // scale

    // Add fractional part
    ticks |= f;                                         // add as low word

    // Restore interrupt state
    IEC3bits.RTCIE = oldRTCIE;
    IEC0bits.T1IE = oldT1IE;

    return ticks;
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
	// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
	unsigned char year = FromBcd(time->f.year);
	unsigned char mon = FromBcd(time->f.mon);
	unsigned char mday = FromBcd(time->f.mday);
	unsigned char hour = FromBcd(time->f.hour);
	unsigned char min = FromBcd(time->f.min);
	unsigned char sec = FromBcd(time->f.sec);

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
    retval.f.mday = ToBcd(DATETIME_DAY(value));
    retval.f.mon  = ToBcd(DATETIME_MONTH(value));
    retval.f.year = ToBcd(DATETIME_YEAR(value));
    retval.f.hour = ToBcd(DATETIME_HOURS(value));
    retval.f.min  = ToBcd(DATETIME_MINUTES(value));
    retval.f.sec  = ToBcd(DATETIME_SECONDS(value));
    return &retval;
}



// TMR1 tasks
inline char RtcTimerTasks(void)
{
    if (IFS0bits.T1IF)
    {
        IFS0bits.T1IF = 0;

        if (rtcTimer < rtcRate)
        {
            rtcTimer++;
            rtcTimerTotal += PR1;
            return 1;
        }
    }
    return 0;
}
