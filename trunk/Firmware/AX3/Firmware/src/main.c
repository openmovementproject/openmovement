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

// AX3 Main Code
// Dan Jackson, Karim Ladha, 2011

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
#include "USB_CDC_MSD.h"
#include "MDD File System/FSIO.h"
#include "Ftl/FsFtl.h"
#include "Analogue.h"
#include "Settings.h"
#include "Logger.h"
#include "Data.h"
#include "Util.h"
//#include <string.h>



// Declarations
void RunAttached(void);
void RunLogging(void);
void TimedTasks(void);
void LedTasks(void);


// Interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
 	static unsigned int INTCON1val;
	LED_SET(MAGENTA);
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
    RtcTasks();
    //IFS3bits.RTCIF = 0;   // RtcTasks() services and clears the interrupt
}

// Accel interrupt 1
void __attribute__((interrupt,auto_psv)) _INT1Interrupt(void)
{
    DataTasks();
	//IFS1bits.INT1IF = 0;  // DataTasks() services and clears the interrupt
}

// Accel interrupt 2
void __attribute__((interrupt,auto_psv)) _INT2Interrupt(void)
{
    DataTasks();
	//IFS1bits.INT2IF = 0;  // DataTasks() services and clears the interrupt
}

// TMR1
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
    if (RtcTimerTasks())
    {
        ; // TODO: Precisely-timed single sampling
    }
	//IFS0bits.T1IF = 0; // RtcTimerTasks() services and clears the interrupt
}


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
	CLOCK_INTOSC();     // 8 MHz
	LED_SET(CYAN);
    RtcStartup();
	InitADCOn();
    SampleADC();                    // Ensure we have a valid battery level

    // Read settings
    LED_SET(GREEN);
    restart = 0;
    inactive = 0;
    SettingsInitialize();           // Initialize settings from ROM
    FtlStartup();                   // FTL & NAND startup
    FSInit();                       // Initialize the filesystem for reading
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
    // Enable peripherals
	RtcInterruptOn(0);

    // Clear the data capture buffer
    DataClear();

    // Initialize the ADXL and enable ADXL interrupts
    AccelStartup(settings.sampleRate);
    AccelEnableInterrupts(ACCEL_INT_SOURCE_WATERMARK | ACCEL_INT_SOURCE_OVERRUN | ACCEL_INT_SOURCE_DOUBLE_TAP, 0x00);

    CLOCK_PLL();	// HS PLL clock for the USB module 12MIPS
    DelayMs(1); 	// Allow PLL to stabilise

    fsftlUsbDiskMounted = status.diskMounted;
    MDD_MediaInitialize();  // MDD initialize

    USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
    #ifdef USB_INTERRUPT
    USBDeviceAttach();
    #endif

    while(USB_BUS_SENSE && restart != 1)
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
        }
        else
        {
            status.attached = -1;
        }
        LedTasks();
        TimedTasks();
		
        // Stream accelerometer data
        if (status.stream)
        {
            unsigned short samples;
            samples = DataLength();
            while (samples--)
            {
                DataType value;
                DataPop(&value, 1);
                printf("%d,%d,%d\r\n", value.x, value.y, value.z);
            }
        }

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
    if (lastTime != rtcTicksSeconds)
    {
        lastTime = rtcTicksSeconds;

        // Increment timer and toggle bit on overflow
        inactive = FtlIncrementInactivity();
        SampleADC_noLDR();
        if (ADCresult[ADC_BATTERY] > batt_full_charge_with_USB && !status.batteryFull)
        {
            status.batteryFull = 1;
            if (status.initialBattery != 0 && status.initialBattery < batt_mid_charge_with_USB)
            {
                // Increment battery health counter
                SettingsIncrementLogValue(LOG_VALUE_BATTERY);
            }
        }

        // TODO: Change to be time-based
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

    if (status.actionCountdown)
    {
        if (((unsigned char)(LEDTimer)) < ((LEDTimer) >> 8)) { LED_SET(LEDtoggle ? RED : OFF); } else { LED_SET(LEDtoggle ? RED : OFF); }
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
            if (status.batteryFull) { c0 = OFF; c1 = WHITE; }       // full - flushed
            else                    { c0 = OFF; c1 = YELLOW; }      // charging - flushed
        }
        else						// Red breath
        {
            if (status.batteryFull) { c0 = RED; c1 = WHITE; }       // full - unflushed
            else                    { c0 = RED; c1 = YELLOW; }      // charging - unflushed
        }
        if (((unsigned char)(LEDTimer)) < ((LEDTimer) >> 8)) { LED_SET(LEDtoggle ? c1 : c0); } else { LED_SET(LEDtoggle ? c0 : c1); }
    }
    return;
}


// Stop logging conditions (values seen in log entries)
typedef enum
{ 
	STOP_NONE                   = 0,    // (not seen)
	STOP_INTERVAL               = 1,    // Blue 0 ..
	STOP_SAMPLE_LIMIT           = 2,    // Blue 1 -.-.
	NOT_STARTED_NO_INTERVAL     = 3,    // Blue 2 --.--.
	NOT_STARTED_AFTER_INTERVAL  = 4,    // Blue 3 ---.---.
	NOT_STARTED_SAMPLE_LIMIT    = 5,    // Blue 4 ----.----.
	NOT_STARTED_WAIT_USB        = 6,    // (not seen)
	STOP_USB                    = 7,    // (not seen)
	NOT_STARTED_INITIAL_BATTERY = 8,    // Red 0 ..
	NOT_STARTED_WAIT_BATTERY    = 9,    // Red 1 -.-.
	STOP_BATTERY                = 10,   // Red 2 --.--.
	NOT_STARTED_FILE_OPEN       = 11,   // Red 3 ---.---.
	STOP_LOGGING_ERROR          = 12,   // Red 4 ----.----.
} StopCondition;

// Log entry strings
const char *stopConditionString[] =
{
	"STOP_NONE",                   // 0
	"STOP_INTERVAL",               // 1
	"STOP_SAMPLE_LIMIT",           // 2
	"NOT_STARTED_NO_INTERVAL",     // 3
	"NOT_STARTED_AFTER_INTERVAL",  // 4
	"NOT_STARTED_SAMPLE_LIMIT",    // 5
	"NOT_STARTED_WAIT_USB",        // 6
	"STOP_USB",                    // 7
	"NOT_STARTED_INITIAL_BATTERY", // 8
	"NOT_STARTED_WAIT_BATTERY",    // 9
	"STOP_BATTERY",                // 10
	"NOT_STARTED_FILE_OPEN",       // 11
	"STOP_LOGGING_ERROR",          // 12
};

// Flash codes
const char stopFlashCode[] =
{
//     0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12
//   off,  B:0,  B:1,  B:2,  B:3,  B:4,  B:5,  B:6,  R:0,  R:1,  R:2,  R:3,  R:4
    0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x40, 0x41, 0x42, 0x42, 0x44
};


// Logging mode
void RunLogging(void)
{
    const char *filename = DEFAULT_FILE;
    StopCondition stopCondition = 0;

    InitADCOn();
    SampleADC();

    SettingsIncrementLogValue(LOG_VALUE_RESTART);   // Increment restart counter

    if (ADCresult[ADC_BATTERY] < MINIMUM_SAFE_BATTERY_RUNNING_VOLTAGE)
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
                // Don't always perform the battery amd RTC checks
                for (i = 0; i < 5; i++)
                {
                    // Exit condition: USB connection
                    if (USB_BUS_SENSE) { stopCondition = NOT_STARTED_WAIT_USB; restart = 1; break; }

                    if (settings.debuggingInfo >= 1 || status.debugFlashCount > 0) { LED_SET(YELLOW); if (status.debugFlashCount > 0) status.debugFlashCount--; Delay10us(5); }
                    LED_SET(OFF);

                    SystemPwrSave(WAKE_ON_WDT|WAKE_ON_USB|DONT_RESTORE_PERIPHERALS|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
                }
                if (stopCondition) { break; }

                //Delay10us(10);
                InitADCOn();
                SampleADC_noLDR();

                // Exit condition: Battery too low
                if (ADCresult[ADC_BATTERY] < MINIMUM_SAFE_BATTERY_RUNNING_VOLTAGE) { stopCondition = NOT_STARTED_WAIT_BATTERY; break; }

                // Exit condition: Start logging
                if (RtcNow() >= settings.loggingStartTime) { break; }
            }
            LED_SET(OFF);
        }

        // If battery not too low and USB not connected, start logging (we are after any delayed start time)
        if (restart != 1 && !stopCondition)
        {
            if (!LoggerStart(filename))
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
	                // Enable precise RTC
	                RtcInterruptOn(0);
	
	                // Clear the data capture buffer
	                DataClear();
	
	                // Initialize the ADXL and enable ADXL interrupts
	                AccelStartup(settings.sampleRate);
	                AccelEnableInterrupts(ACCEL_INT_SOURCE_WATERMARK | ACCEL_INT_SOURCE_OVERRUN | ACCEL_INT_SOURCE_DOUBLE_TAP, 0x00);
	
	                // Logging loop
	                while (restart != 1 && !stopCondition)
	                {
	                    unsigned short now;
	
	                    // Exit condition: USB connection
	                    if (USB_BUS_SENSE) { stopCondition = STOP_USB; restart = 1; break; }
	
	                    // Get current time
	                    now = RtcSeconds();
	
	                    // Write sector (if enough data)
	                    if (LoggerWrite())
	                    {
	                        // We successfully wrote something, update 'last written' status
	                        status.lastWrittenTicks = now;
	
	                        // Check if battery too low...
	                        InitADCOn(); SampleADC_noLDR(); // (ADC was just updated by logger anyway)
	                        if (ADCresult[ADC_BATTERY] < MINIMUM_SAFE_BATTERY_RUNNING_VOLTAGE)
	                        {
	                            // Get a second opinion to be certain and, if so, stop logging
	                            InitADCOn(); SampleADC();
	                            if (ADCresult[ADC_BATTERY] < MINIMUM_SAFE_BATTERY_RUNNING_VOLTAGE) { stopCondition = STOP_BATTERY; break; }
	                        }
	
	                        // If written required number of samples, stop logging
	                        if (settings.maximumSamples != 0 && status.sampleCount >= settings.maximumSamples) { stopCondition = STOP_SAMPLE_LIMIT; break; }
	
	                        // If after logging end time, stop logging
	                        if (RtcNow() > settings.loggingEndTime) { stopCondition = STOP_INTERVAL; break; }
	                    }
	
	                    // Status monitor
	                    if (status.lastWrittenTicks == 0x0000) { status.lastWrittenTicks = now; }
	
	                    // If not been able to write for 15 seconds, there's a Filesystem/Ftl/NAND problem, stop logging
	                    if (now - status.lastWrittenTicks > 15) { stopCondition = STOP_LOGGING_ERROR; break; }
	
	                    // Sleep until ADXL INT1, RTC, USB or WDT
	                    SystemPwrSave(WAKE_ON_RTC|WAKE_ON_WDT|WAKE_ON_USB|WAKE_ON_ADXL1|WAKE_ON_TIMER1|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
	                    //Sleep();
	                    //__builtin_nop();
	                }
				}
	
                LoggerStop();
            }
        }
    }

    // Blue while powering down (lowest power)
    LED_SET(BLUE);

    // Shutdown peripherals
    AccelStandby();
    RtcInterruptOff();
    FtlShutdown();

    // Add log entry for stop reason
    SettingsAddLogEntry(LOG_CATEGORY_STOP | (unsigned int)stopCondition, RtcNow(), stopConditionString[stopCondition]);

    // Turn off LEDs
    LED_SET(OFF);

    // Sleep if not connected (and not restarting)
    if (restart != 1)
    {
        unsigned char led, countReset, countdown;
        led = (stopFlashCode[stopCondition] >> 4);
        countReset = stopFlashCode[stopCondition] & 0x0f;
        countdown = countReset;
        CLOCK_INTOSC(); // Lower power
        
        // Strobes LED every few seconds until connected
        while (restart != 1)
        {
            if (USB_BUS_SENSE) { restart = 1; break; }
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
                LED_SET(OFF);
            }
        }
    }

    return;
}

