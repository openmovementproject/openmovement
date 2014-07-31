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

// AX3 Main Code
// Dan Jackson, Karim Ladha, 2011-2012

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Peripherals/Nand.h"
#include "Peripherals/Accel.h"
#ifdef USE_GYRO
#include "Peripherals/Gyro.h"
#endif
#include "Peripherals/Rtc.h"
#include "USB/USB.h"
#include "USB/usb_function_msd.h"
#include "USB/usb_function_cdc.h"
#include "MDD File System/FSIO.h"
#include "Usb/USB_CDC_MSD.h"
#include "Ftl/FsFtl.h"
#include "Utils/FsUtils.h"
#include "Utils/Fifo.h"
#include "Utils/Util.h"
#include "Peripherals/Analog.h"
#include "Settings.h"
#include "Logger.h"
//#include <string.h>

// KL: 17-04-2014 Self test sequence
#include "AX3 SelfTest.h"


// Declarations
void RunAttached(void);
void RunLogging(void);
void TimedTasks(void);
void LedTasks(void);
void RunTestSequence(void);

// Interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
 	static unsigned int INTCON1val;
	LED_SET(LED_MAGENTA);
	INTCON1val = INTCON1;
	Nop();
	Nop();
	Nop();
	Nop();
	INTCON1 = 0;
    Reset();
}

// RTC
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
    RtcSwwdtIncrement();    // Increment software RTC, reset if overflow
    RtcTasks();
    //IFS3bits.RTCIF = 0;   // RtcTasks() services and clears the interrupt
}

// Accel interrupt 1
void __attribute__((interrupt,auto_psv)) _INT1Interrupt(void)
{
    LoggerAccelTasks();
	//IFS1bits.INT1IF = 0;  // DataTasks() services and clears the interrupt
}

// Accel interrupt 2
void __attribute__((interrupt,auto_psv)) _INT2Interrupt(void)
{
    LoggerAccelTasks();
	//IFS1bits.INT2IF = 0;  // DataTasks() services and clears the interrupt
}

#ifdef USE_GYRO
// Gyro interrupt 1
void __attribute__((interrupt,auto_psv)) _INT3Interrupt(void)
{
    LoggerGyroTasks();
	//IFS3bits.INT3IF = 0;  // DataTasks() services and clears the interrupt
}

// Gyro interrupt 2
void __attribute__((interrupt,auto_psv)) _INT4Interrupt(void)
{
    LoggerGyroTasks();
	//IFS3bits.INT4IF = 0;  // DataTasks() services and clears the interrupt
}
#endif

// TMR1
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
    if (RtcTimerTasks())
    {
        ; // TODO: Precisely-timed single sampling
    }
	//IFS0bits.T1IF = 0; // RtcTimerTasks() services and clears the interrupt
}

// CN
void __attribute__((interrupt, shadow, auto_psv)) _CNInterrupt(void)
{
	IFS1bits.CNIF = 0;
}

#include <p24FJ256GB106.h>


// Restart flag
static unsigned char restart = 0;
static unsigned char inactive = 0;
static unsigned short lastTime = 0;


// Main routine
int main(void)
{
    // Initialization
	InitIO();			// I/O pins
	CLOCK_SOSCEN(); 	// For the RTC
	WaitForPrecharge();	// ~0.5mA current

	// Peripherals - RTC and ADC always used
    LED_SET(LED_BLUE);  // Blue LED during startup
	CLOCK_INTOSC();     // 8 MHz
    // RTC and 'software WDT'
    RtcSwwdtReset();
    RtcStartup();
    RtcInterruptOn(0);  // Enable precise RTC (and software WDT)
    LED_SET(LED_BLUE);  // Blue LED during startup (set again as RTC sync strobes LED)
    // ADC
	AdcInit();
    AdcSampleWait();    // Ensure we have a valid battery level

    // Check the devices we have
    NandInitialize();
    NandVerifyDeviceId();
    AccelVerifyDeviceId();
#ifdef USE_GYRO
    GyroVerifyDeviceId();
#endif

	// Self test check - only actually executes on first run
	RunTestSequence();

    // If we haven't detected the NAND or accelerometer this could be the wrong firmware for this device (reset to bootloader)
    if (!nandPresent)
    {
        int i;
        for (i = 0; i < 5 * 3; i++) { LED_SET(LED_MAGENTA); DelayMs(111); LED_SET(LED_CYAN); DelayMs(111); LED_SET(LED_YELLOW); DelayMs(111); }
        #ifdef IGNORE_UNRECOGNIZED_PERIPHERALS
        if (!USB_BUS_SENSE)      // If IGNORE_UNRECOGNIZED_PERIPHERALS set, don't reset if connected to USB
        #endif
    	Reset();                // Reset
    }
    if (!accelPresent)
    {
        int i;
        for (i = 0; i < 5 * 3; i++) { LED_SET(LED_RED); DelayMs(111); LED_SET(LED_GREEN); DelayMs(111); LED_SET(LED_BLUE); DelayMs(111); }
        #ifdef IGNORE_UNRECOGNIZED_PERIPHERALS
        if (!USB_BUS_SENSE)      // If IGNORE_UNRECOGNIZED_PERIPHERALS set, don't reset if connected to USB
        #endif
    	Reset();                // Reset
    }

    // Read settings
    restart = 0;
    inactive = 0;
    SettingsInitialize();               // Initialize settings from ROM

LED_SET(LED_WHITE);         // White LED during later startup
    FtlStartup();                       // FTL & NAND startup
if(FSInit()!=TRUE)                          // Initialize the filesystem for reading
{
int i;
for (i = 0; i < 5 * 3; i++) { LED_SET(LED_RED); DelayMs(111); LED_SET(LED_GREEN); DelayMs(111); }
}
    SettingsReadFile(SETTINGS_FILE);    // Read settings from script
    // TODO: Make this "single binary file" mode optional on the settings just read
    LoggerReadMetadata(DEFAULT_FILE);   // Read settings from the logging binary file

    // Run as attached or logging
    if (USB_BUS_SENSE)
    {
        RunAttached();      // Run attached, returns when detatched
    }
    else
    {
        RunLogging();       // Run in logging mode, returns when attached
    }

	Reset();                // Reset
	return 0;
}


// Attached to USB
void RunAttached(void)
{
    // Clear the data capture buffer
    LoggerClear();

    // Initialize the ADXL
    AccelStartup(settings.sampleRate);
    //AccelEnableInterrupts(ACCEL_INT_SOURCE_WATERMARK | ACCEL_INT_SOURCE_OVERRUN | ACCEL_INT_SOURCE_DOUBLE_TAP, 0x00);
#ifdef USE_GYRO
    GyroStartup();
    //GyroStartupFifoInterrupts();
#endif

#if 1
FtlFlush(0);    // [dgj] Possible fix for 'format disk' bug
#endif
    CLOCK_PLL();	// HS PLL clock for the USB module 12MIPS
    DelayMs(2); 	// Allow PLL to stabilise

    fsftlUsbDiskMounted = status.diskMounted;
#if 1
FSInit();       // [dgj] Possible fix for 'format disk' bug
#endif
    MDD_MediaInitialize();  // MDD initialize

    USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
    #ifdef USB_INTERRUPT
    USBDeviceAttach();
    #endif

    while (USB_BUS_SENSE && restart != 1)
    {
        fsftlUsbDiskMounted = status.diskMounted;

        // Check bus status and service USB interrupts.
        #ifndef USB_INTERRUPT
        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
        #endif
        USBProcessIO();
        if ((USBGetDeviceState() >= CONFIGURED_STATE) && (USBIsDeviceSuspended() == FALSE))
        {
            const char *line = _user_gets();
            status.attached = 1;
            if (line != NULL)
            {
                status.stream = 0;                  // Disable streaming
                SettingsCommand(line, SETTINGS_USB);
            }

            // Stream accelerometer data
            if (status.stream)
            {
                #define STREAM_RATE 100
                #define STREAM_INTERVAL (0x10000UL / STREAM_RATE)
                static unsigned long lastSampleTicks = 0;
                unsigned long now = RtcTicks();
                if (lastSampleTicks == 0) { lastSampleTicks = now; }
                if (now - lastSampleTicks > STREAM_INTERVAL)
                {
                    accel_t accelSample;
                    lastSampleTicks += STREAM_INTERVAL;
                    if (now - lastSampleTicks > 2 * STREAM_INTERVAL) { lastSampleTicks = now; } // not keeping up with sample rate

                    AccelSingleSample(&accelSample);
#ifdef USE_GYRO
                    if (gyroPresent)
                    {
                        gyro_t gyroSample;
                        GyroSingleSample(&gyroSample);
                        printf("%d,%d,%d,%d,%d,%d\r\n", accelSample.x, accelSample.y, accelSample.z, gyroSample.x, gyroSample.y, gyroSample.z);
                    }
                    else
#endif
                    {
                        printf("%d,%d,%d\r\n", accelSample.x, accelSample.y, accelSample.z);
                    }
                    USBCDCWait();
                }

            }
        }
        else
        {
            status.attached = -1;
        }
        LedTasks();
        TimedTasks();
		

        // Experiment to see if this improves speed -- it doesn't seem to
        #ifdef FSFTL_READ_PREFETCH
		FsFtlPrefetch();
        #endif
    }
	#if defined(USB_INTERRUPT)
    USBDeviceDetach();
	#endif
    status.attached = -1;

    // Shutdown the FTL
    FtlShutdown();
    return;
}


// Timed tasks
void TimedTasks(void)
{
    // 1 Hz update
    if (lastTime != rtcTicksSeconds)
    {
        lastTime = rtcTicksSeconds;

        // Increment timer and toggle bit on overflow
        inactive = FtlIncrementInactivity();
        AdcSampleNow();
        if (adcResult.batt > BATT_CHARGE_FULL_USB && status.batteryFull < BATT_FULL_INTERVAL)
        {
            status.batteryFull++;
            if (status.batteryFull >= BATT_FULL_INTERVAL)
            {
                if (status.initialBattery != 0 && status.initialBattery < BATT_CHARGE_MID_USB)
                {
                    // Increment battery health counter
                    SettingsIncrementLogValue(LOG_VALUE_BATTERY);
                }
            }
        }

        if (inactive > 3)
        {
            FtlFlush(1);	// Inactivity time out on scratch hold
        }

        if (status.actionCountdown > 0)
        {
            status.actionCountdown--;
            if (status.actionCountdown == 0)
            {
                if (SettingsAction(status.actionFlags))
                {
                    restart = 1;
                }
            }
        }

        // Reset SW-WDT
        RtcSwwdtReset();
    }
    return;
}


// Led status while attached
void LedTasks(void)
{
    static unsigned int LEDTimer;
    static BOOL LEDtoggle;
//    static unsigned short inactive = 0;

    if (++LEDTimer == 0) { LEDtoggle = !LEDtoggle; }

    if (status.attached > 0)
    {
        if (status.actionCountdown)
        {
            if (((unsigned char)(LEDTimer)) < ((LEDTimer) >> 8)) { LED_SET(LEDtoggle ? LED_RED : LED_OFF); } else { LED_SET(LEDtoggle ? LED_RED : LED_OFF); }
        }
        else if (status.ledOverride >= 0)
        {
            LED_SET(status.ledOverride);
        }
        else
        {
            char c0, c1;

            if (inactive == 0)
            {
                if (status.batteryFull >= BATT_FULL_INTERVAL) { c0 = LED_OFF; c1 = LED_WHITE; }       // full - flushed
                else                    { c0 = LED_OFF; c1 = LED_YELLOW; }      // charging - flushed
            }
            else						// Red breath
            {
                if (status.batteryFull >= BATT_FULL_INTERVAL) { c0 = LED_RED; c1 = LED_WHITE; }       // full - unflushed
                else                    { c0 = LED_RED; c1 = LED_YELLOW; }      // charging - unflushed
            }
            if (((unsigned char)(LEDTimer)) < ((LEDTimer) >> 8)) { LED_SET(LEDtoggle ? c1 : c0); } else { LED_SET(LEDtoggle ? c0 : c1); }
        }
    }
    else
    {
        if (status.batteryFull) { LED_SET(LED_GREEN); }       // full - not enumerated
        else                    { LED_SET(LED_YELLOW); }      // charging - not enumarated (could change to red if yellow-green contrast not strong enough)
    }
    return;
}


// Stop logging conditions (values seen in log entries)
typedef enum
{ 
	STOP_NONE                   = 0,    // (not seen)
	STOP_INTERVAL               = 1,    // Blue 0 ..
	STOP_SAMPLE_LIMIT           = 2,    // Blue 1 -.-.
	STOP_DISK_FULL              = 2,    // Blue 1 -.-. (same as sample limit)
	NOT_STARTED_NO_INTERVAL     = 3,    // Blue 2 --.--.
	NOT_STARTED_AFTER_INTERVAL  = 4,    // Blue 3 ---.---.
	NOT_STARTED_SAMPLE_LIMIT    = 5,    // Blue 4 ----.----.
	NOT_STARTED_DISK_FULL       = 5,    // Blue 4 ----.----. (same as sample limit)
	NOT_STARTED_WAIT_USB        = 6,    // (not seen)
	STOP_USB                    = 7,    // (not seen)
	NOT_STARTED_INITIAL_BATTERY = 8,    // Red 0 ..
	NOT_STARTED_WAIT_BATTERY    = 9,    // Red 1 -.-.
	STOP_BATTERY                = 10,   // Red 2 --.--.
	NOT_STARTED_FILE_OPEN       = 11,   // Red 3 ---.---.
	STOP_LOGGING_WRITE_ERR      = 12,   // Red 4 ----.----. (not seen, restarted)
	STOP_LOGGING_SAMPLE_ERR     = 13,   // Red 5 -----.-----. (not seen, restarted)
} StopCondition;

// Log entry strings
const char *stopConditionString[] =
{
	"STOP_NONE",                   // 0
	"STOP_INTERVAL",               // 1
	"STOP_SAMPLE_LIMIT",           // 2 (or STOP_DISK_FULL)
	"NOT_STARTED_NO_INTERVAL",     // 3
	"NOT_STARTED_AFTER_INTERVAL",  // 4
	"NOT_STARTED_SAMPLE_LIMIT",    // 5 (or NOT_STARTED_DISK_FULL)
	"NOT_STARTED_WAIT_USB",        // 6
	"STOP_USB",                    // 7
	"NOT_STARTED_INITIAL_BATTERY", // 8
	"NOT_STARTED_WAIT_BATTERY",    // 9
	"STOP_BATTERY",                // 10
	"NOT_STARTED_FILE_OPEN",       // 11
	"STOP_LOGGING_WRITE_ERR",      // 12
	"STOP_LOGGING_SAMPLE_ERR",     // 13
};

// Flash codes
const char stopFlashCode[] =
{
//     0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13
//   off,  B:0,  B:1,  B:2,  B:3,  B:4,  B:5,  B:6,  R:0,  R:1,  R:2,  R:3,  R:4,  R:5
    0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45
};


// Logging mode
void RunLogging(void)
{
    const char *filename = DEFAULT_FILE;
    StopCondition stopCondition = 0;

    AdcInit();
    AdcSampleWait();

    SettingsIncrementLogValue(LOG_VALUE_RESTART);   // Increment restart counter

    if (adcResult.batt < BATT_CHARGE_MIN_SAFE)
    {
        // Battery level insufficient to start logging -- error
        stopCondition = NOT_STARTED_INITIAL_BATTERY;
    }
    else if (settings.loggingStartTime >= settings.loggingEndTime)
    {
        // We have a no defined logging interval (not even 'infinite') -- don't log
        stopCondition = NOT_STARTED_NO_INTERVAL;   // not an error
    }
    else if (settings.loggingEndTime > settings.loggingStartTime && RtcNow() >= settings.loggingEndTime)
    {
        // We have a set finish time and we are after that time -- don't log
        stopCondition = NOT_STARTED_AFTER_INTERVAL;   // not an error
    }
    else
    {
        // If we have a delayed start and we are before that time -- wait
        if (settings.loggingStartTime < settings.loggingEndTime && RtcNow() < settings.loggingStartTime)
        {
            // Strobes LED every few seconds until after start time or USB-connected
            status.debugFlashCount = 5;
            while (restart != 1 && !stopCondition)
            {
                unsigned int i;

                // Reset SW-WDT
                RtcSwwdtReset();

                // Don't always perform the battery amd RTC checks
                for (i = 0; i < 5; i++)
                {
                    // Exit condition: USB connection
                    if (USB_BUS_SENSE) { stopCondition = NOT_STARTED_WAIT_USB; restart = 1; break; }

                    if (settings.debuggingInfo >= 1 || status.debugFlashCount > 0) { LED_SET(LED_YELLOW); if (status.debugFlashCount > 0) status.debugFlashCount--; Delay10us(5); }
                    LED_SET(LED_OFF);

                    SystemPwrSave(WAKE_ON_WDT|WAKE_ON_USB|DONT_RESTORE_PERIPHERALS|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
                }
                if (stopCondition) { break; }

                //Delay10us(10);
                AdcInit();
                AdcSampleNow();

                // Exit condition: Battery too low
                if (adcResult.batt < BATT_CHARGE_MIN_SAFE) { stopCondition = NOT_STARTED_WAIT_BATTERY; break; }

                // Exit condition: Start logging
                if (RtcNow() >= settings.loggingStartTime) { break; }
            }
            LED_SET(LED_OFF);
        }

        // If battery not too low and USB not connected, start logging (we are after any delayed start time)
        if (restart != 1 && !stopCondition)
        {
            if (FSDiskFree() == 0)
            {
	            stopCondition = NOT_STARTED_DISK_FULL;
            }
            else if (!LoggerStart(filename))
            {
	            stopCondition = NOT_STARTED_FILE_OPEN;
            }
            else
            {
                // If written required number of samples, stop logging
                if (settings.maximumSamples != 0 && status.sampleCount >= settings.maximumSamples)
                { 
                	stopCondition = NOT_STARTED_SAMPLE_LIMIT; 		// not an error
                }
                else
                {
                    int failCounter = 0;

#ifdef HIGH_SPEED_USES_PLL
#warning "HIGH_SPEED_USES_PLL not fully tested."
					// [High-speed] If sampling at high rate, switch to PLL
					char isHighSpeed = 0;
					if (ACCEL_FREQUENCY_FOR_RATE(settings.sampleRate) >= 1600) { isHighSpeed = 1; }
					if (isHighSpeed) { CLOCK_PLL();	}
#endif

	                // Clear the data capture buffer
	                LoggerClear();
	
	                // Initialize the ADXL and enable ADXL interrupts
	                AccelStartup(settings.sampleRate);
	                AccelEnableInterrupts(ACCEL_INT_SOURCE_WATERMARK | ACCEL_INT_SOURCE_OVERRUN, 0x00);	/* ACCEL_INT_SOURCE_DOUBLE_TAP removed */
#ifdef USE_GYRO
                    GyroStartupFifoInterrupts();
#endif
	
	                // Logging loop
	                while (restart != 1 && !stopCondition)
	                {
	                    unsigned short now;
                        short result;

	                    // Exit condition: USB connection
	                    if (USB_BUS_SENSE) { stopCondition = STOP_USB; restart = 1; break; }
	
	                    // Get current time
	                    now = RtcSeconds();
	
	                    // Write sector (if enough data)
                        result = LoggerWrite();
	                    if (result)
	                    {
                            // We successfully sampled
                            status.lastSampledTicks = now;

                            // We successfully wrote something, update 'last written' status
                            if (result > 0)
                            {
                                status.lastWrittenTicks = now;

                                // Reset SW-WDT
                                RtcSwwdtReset();
                            }
	
	                        // Check if battery too low...
	                        AdcInit(); AdcSampleNow(); // (ADC was just updated by logger anyway)
	                        if (adcResult.batt < BATT_CHARGE_MIN_SAFE)
	                        {
	                            // Get a second opinion to be certain and, if so, stop logging
	                            AdcInit(); AdcSampleNow();
	                            if (adcResult.batt < BATT_CHARGE_MIN_SAFE) { stopCondition = STOP_BATTERY; break; }
	                        }
	
	                        // If written required number of samples, stop logging
	                        if (settings.maximumSamples != 0 && status.sampleCount >= settings.maximumSamples) { stopCondition = STOP_SAMPLE_LIMIT; break; }
	
	                        // If after logging end time, stop logging
	                        if (RtcNow() > settings.loggingEndTime) { stopCondition = STOP_INTERVAL; break; }
	                    }
	
	                    // Status monitor
	                    if (status.lastSampledTicks == 0x0000) { status.lastSampledTicks = now; }
	                    if (status.lastWrittenTicks == 0x0000) { status.lastWrittenTicks = now; }
	
                        if (now - status.lastSampledTicks > 15)
                        {
                            // If not been sampled enough for a sector for 15 seconds, there's an ADXL problem, log the error and restart
                            failCounter++;
                            if (failCounter > 5)
                            {
                                stopCondition = STOP_LOGGING_SAMPLE_ERR;
                                restart = 1;
                                break;
                            }
                        }
                        else if (now - status.lastWrittenTicks > 30)
                        {
                            // If not been able to successfully write for 30 seconds, there may be a Filesystem/Ftl/NAND problem, stop logging
                            failCounter++;
                            if (failCounter > 5)
                            {
                                // If we detected a write problem and the disk is full, this is a valid stop reason, otherwise it's a write error
                                if (FSDiskFree() == 0)
                                {
            	                    stopCondition = STOP_DISK_FULL;
                                }
                                else
                                {
                                    stopCondition = STOP_LOGGING_WRITE_ERR;
                                    restart = 1;
                                }
                                break;
                            }
                        }
                        else { failCounter = 0; }
	
#ifdef HIGH_SPEED_USES_PLL
// [High-speed] No sleep if sampling at high rate
if (!isHighSpeed)
{
#endif

	                    // Sleep until ADXL INT1, RTC, USB or WDT
#ifdef USE_GYRO
	                    SystemPwrSave(WAKE_ON_RTC|WAKE_ON_WDT|WAKE_ON_USB|WAKE_ON_ADXL1|WAKE_ON_GYRO2|WAKE_ON_TIMER1|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
#else
	                    SystemPwrSave(WAKE_ON_RTC|WAKE_ON_WDT|WAKE_ON_USB|WAKE_ON_ADXL1|WAKE_ON_TIMER1|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
#endif
	                    //Sleep();
	                    //__builtin_nop();
#ifdef HIGH_SPEED_USES_PLL
}
#endif
	                    
	                    
	                }

#ifdef HIGH_SPEED_USES_PLL
					// [High-speed] Lower clock rate
					if (isHighSpeed) { CLOCK_INTOSC();}
#endif

				}
	
                LoggerStop();
            }
        }
    }

    // Blue while powering down (lowest power)
    LED_SET(LED_BLUE);

    // Shutdown peripherals
    AccelStandby();
#ifdef USE_GYRO
    GyroStandby();
#endif
    RtcInterruptOff();
    FtlShutdown();

    // Add log entry for stop reason
    SettingsAddLogEntry(LOG_CATEGORY_STOP | (unsigned int)stopCondition, RtcNow(), stopConditionString[stopCondition]);

    // Turn off LEDs
    LED_SET(LED_OFF);

    // Sleep if not connected (and not restarting)
    if (restart != 1)
    {
        unsigned char led, countReset, countdown;
        led = (stopFlashCode[stopCondition] >> 4);
        countReset = stopFlashCode[stopCondition] & 0x0f;
        countdown = countReset;
        if (settings.debuggingInfo == 0xff) { led = 0; }
        CLOCK_INTOSC(); // Lower power
        
        // Strobes LED every few seconds until connected
        while (restart != 1)
        {
            if (USB_BUS_SENSE) { restart = 1; break; }

            // Reset SW-WDT
            RtcSwwdtReset();

// NOTE: RTC interrupts are now off, this assumes a WDT interval of 1 second.
            SystemPwrSave(WAKE_ON_WDT|LOWER_PWR_SLOWER_WAKE|WAKE_ON_USB|ADC_POWER_DOWN|ACCEL_POWER_DOWN|GYRO_POWER_DOWN|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
            //SystemPwrSave(WAKE_ON_WDT|WAKE_ON_BUTTON|WAKE_ON_USB|DONT_RESTORE_PERIPHERALS|ADC_POWER_DOWN|ACCEL_POWER_DOWN|GYRO_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS);

            if (led)
            {
                LED_SET(led);
                if (countdown > 0)
                {
                	Delay10us(15);
                	countdown--;
                }
                else
                {
                    Delay10us(3);
                    countdown = countReset;
                }
                LED_SET(LED_OFF);
            }
        }
    }

    return;
}


