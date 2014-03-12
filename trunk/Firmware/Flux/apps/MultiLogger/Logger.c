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

// Unified Logger - Sensor Capture
// Dan Jackson, Karim Ladha

// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "TimeDelay.h"

#include "Analog.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Mag.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/Nand.h"
#include "Peripherals/bmp085.h"
#include "Peripherals/Prox.h"
#include "Bluetooth/Bluetooth.h"

#include "Utils/Fifo.h"
#include "Utils/DataStream.h"
#include "Utils/FileStream.h"

#include "Apps/MultiLogger/Logger.h"
#include "Apps/MultiLogger/Sampler.h"
#include "Apps/MultiLogger/Recorder.h"
#include "Apps/MultiLogger/Stream.h"
#include "Apps/MultiLogger/Settings.h"
#include "Utils/Util.h"
#include "Utils/FSUtils.h"
#include "MDD File System/FSIO.h"
#include "USB/USB_CDC_MSD.h"
#include "Graphics/Display Controller SSD1306.h"


#include "Apps/MultiLogger/RunClock.h"

//#define DEBUG_STATES


// Original connection state is recorded in main as a safety mechanism (timeout on connection change to force restart)
extern char usbAtStartup;


/*
// (Old) Stop logging conditions (values seen in log entries)
typedef enum
{
	STOP_NONE       = 0,    // No reason to stop
    STOP_CONNECTION = 1,    // Connection state changed, forced a restart
    STOP_BATTERY    = 2,    // Stopped as battery too low
    STOP_INTERVAL   = 3,    // Stopped as no interval set or outside interval
    STOP_MEMORY     = 4,    // Stopped as memory full
    STOP_RESTART    = 9,    // Restart requested
//	STOP_NONE                   = 0,    // (not seen)
//	STOP_INTERVAL               = 1,    // Blue 0 ..
//	STOP_SAMPLE_LIMIT           = 2,    // Blue 1 -.-.
//	STOP_DISK_FULL              = 2,    // Blue 1 -.-. (same as sample limit)
//	NOT_STARTED_NO_INTERVAL     = 3,    // Blue 2 --.--.
//	NOT_STARTED_AFTER_INTERVAL  = 4,    // Blue 3 ---.---.
//	NOT_STARTED_SAMPLE_LIMIT    = 5,    // Blue 4 ----.----.
//	NOT_STARTED_DISK_FULL       = 5,    // Blue 4 ----.----. (same as sample limit)
//	NOT_STARTED_WAIT_USB        = 6,    // (not seen)
//	STOP_USB                    = 7,    // (not seen)
//	NOT_STARTED_INITIAL_BATTERY = 8,    // Red 0 ..
//	NOT_STARTED_WAIT_BATTERY    = 9,    // Red 1 -.-.
//	STOP_BATTERY                = 10,   // Red 2 --.--.
//	NOT_STARTED_FILE_OPEN       = 11,   // Red 3 ---.---.
//	STOP_LOGGING_WRITE_ERR      = 12,   // Red 4 ----.----. (not seen, restarted)
//	STOP_LOGGING_SAMPLE_ERR     = 13,   // Red 5 -----.-----. (not seen, restarted)
} stop_t;
*/

void DebugTasks(void)
{
#ifdef DEBUG_STATES
    if (USB_BUS_SENSE)
    {
        // Status
        static StateConnection lastConnection = 0xff;
        static StateBattery lastBattery = 0xff;
        static StateWireless lastWireless = 0xff;
        static StateDrive lastDrive = 0xff;
        // Sampler
        static StateSampling lastSampling = 0xff;
        // Logger
        static StateInterval lastInterval = 0xff;
        static StateRecording lastRecording = 0xff;
        static StateSchedule lastSchedule = 0xff;
        static StateStream lastStream = 0xff;
        // Inactivity
        static unsigned short lastInactivity = 0xffff;

        if (status.connection != lastConnection)
        {
            const char *val = "?";
            lastConnection = status.connection;
            if (status.connection == CONNECTION_NONE) { val = "NONE"; }
            if (status.connection == CONNECTION_POWER) { val = "POWER"; }
            if (status.connection == CONNECTION_COMPUTER) { val = "COMPUTER"; }
            printf("CONNECTION: %s\r\n", val);
        }

        if (status.battery != lastBattery)
        {
            const char *val = "?";
            lastBattery = status.battery;
            if (status.battery == BATTERY_NORMAL) { val = "NORMAL"; }
            if (status.battery == BATTERY_WARNING) { val = "WARNING"; }
            if (status.battery == BATTERY_EMPTY) { val = "EMPTY"; }
            if (status.battery == BATTERY_FULL) { val = "FULL"; }
            if (status.battery == BATTERY_CHARGING) { val = "CHARGING"; }
            printf("BATTERY: %s\r\n", val);
        }

        if (status.wireless != lastWireless)
        {
            const char *val = "?";
            lastWireless = status.wireless;
            if (status.wireless == WIRELESS_OFF) { val = "OFF"; }
            if (status.wireless == WIRELESS_DISCOVERABLE) { val = "DISCOVERABLE"; }
            if (status.wireless == WIRELESS_CONNECTED) { val = "CONNECTED"; }
            printf("WIRELESS: %s\r\n", val);
        }

        if (status.drive != lastDrive)
        {
            const char *val = "?";
            lastDrive = status.drive;
            if (status.drive == DRIVE_ERROR) { val = "ERROR"; }
            if (status.drive == DRIVE_MOUNTED) { val = "MOUNTED"; }
            if (status.drive == DRIVE_OWNED) { val = "OWNED"; }
            if (status.drive == DRIVE_OWNED_FULL) { val = "OWNED_FULL"; }
            if (status.drive == DRIVE_UNOWNED) { val = "UNOWNED"; }
            printf("DRIVE: %s\r\n", val);
        }

        if (logger.sampling != lastSampling)
        {
            const char *val = "?";
            lastSampling = logger.sampling;
            if (logger.sampling == SAMPLING_OFF) { val = "OFF"; }
            if (logger.sampling == SAMPLING_ON) { val = "ON"; }
            printf("SAMPLING: %s\r\n", val);
        }

        if (logger.interval != lastInterval)
        {
            const char *val = "?";
            lastInterval = logger.interval;
            if (logger.interval == INTERVAL_AFTER) { val = "AFTER"; }
            if (logger.interval == INTERVAL_ALWAYS) { val = "ALWAYS"; }
            if (logger.interval == INTERVAL_NONE) { val = "NONE"; }
            if (logger.interval == INTERVAL_WAITING) { val = "WAITING"; }
            if (logger.interval == INTERVAL_WITHIN) { val = "WITHIN"; }
            printf("INTERVAL: %s\r\n", val);
        }

        if (logger.recording != lastRecording)
        {
            const char *val = "?";
            lastRecording = logger.recording;
            if (logger.recording == RECORDING_OFF) { val = "OFF"; }
            if (logger.recording == RECORDING_ON) { val = "ON"; }
            printf("RECORDING: %s\r\n", val);
        }

        if (logger.schedule != lastSchedule)
        {
            const char *val = "?";
            lastSchedule = logger.schedule;
            if (logger.schedule == SCHEDULE_ALWAYS) { val = "ALWAYS"; }
            if (logger.schedule == SCHEDULE_INTERVAL) { val = "INTERVAL"; }
            if (logger.schedule == SCHEDULE_NEVER) { val = "NEVER"; }
            printf("SCHEDULE: %s\r\n", val);
        }

        if (logger.stream != lastStream)
        {
            const char *val = "?";
            lastStream = logger.stream;
            if (logger.stream == STREAM_NONE) { val = "NONE"; }
            if (logger.stream == STREAM_WIRELESS) { val = "WIRELESS"; }
            if (logger.stream == STREAM_USB) { val = "USB"; }
            if (logger.stream == STREAM_BOTH) { val = "BOTH"; }
            printf("STREAM: %s\r\n", val);
        }

        if (status.inactivity != lastInactivity)
        {
            if (status.inactivity < lastInactivity)
            {
                printf("!");
            }
            lastInactivity = status.inactivity;
        }

    }
#endif
}


void StateSetInterval(StateInterval newInterval)
{
    //if (logger.interval == newInterval) { return; }     // Only interested in state changes
   
    // If under scheduled recording start/stop...
    if (logger.schedule == SCHEDULE_INTERVAL && status.connection == CONNECTION_NONE)       // Only use scheduled recording while disconnected
    {
        if (newInterval == INTERVAL_WITHIN)             // check new interval
        {
            // Inside interval, recording should be on
            StateSetRecording(RECORDING_ON, RECORD_STOP_REASON_NONE);
        }
        else if (newInterval != INTERVAL_WITHIN)    // check old interval
        {
            // Outside interval, recording should be off
            StateSetRecording(RECORDING_OFF, RECORD_STOP_REASON_INTERVAL);
        }
    }
    else
    {
        if (logger.schedule == SCHEDULE_ALWAYS)
        {
            // On
            StateSetRecording(RECORDING_ON, RECORD_STOP_REASON_NONE);
        }
        else
        {
            // Off
            StateSetRecording(RECORDING_OFF, RECORD_STOP_REASON_NONE);
        }
    }
    logger.interval = newInterval;
}



// Connection tasks
char UsbConnectionTasks(void)
{
    char connected = USB_BUS_SENSE;

    // Connection
    if (connected)
    {
        // We have a new connection
        if (status.connection == CONNECTION_NONE)
        {
            status.connection = CONNECTION_POWER;

// TEST: Immediately mount the drive if powered and device unlocked
if (status.lockCode == 0)
{
    StateSetDrive(DRIVE_MOUNTED, 0, RECORD_STOP_REASON_CONNECTED);
}
            fsftlUsbDiskMounted = (status.drive == DRIVE_MOUNTED);
            
            // Switch clock
			if(OSCCONbits.COSC != 0b001)
			{
            	DelayMs(10); 	// Allow Vdd to settle (if just started)
				if(ReturnWhenSafeToClockSwitch())
            	{
					CLOCK_PLL();// HS PLL clock for the USB module 12MIPS
					BluetoothAdaptToClockSwitch();
				}	
				else
				{
					StateSetWireless(WIRELESS_OFF);
					CLOCK_PLL();// HS PLL clock for the USB module 12MIPS
				}

            	DelayMs(10); 	// Allow PLL to stabilise
			}

            MDD_MediaInitialize();  // MDD initialize

            USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
            #ifdef USB_INTERRUPT
            USBDeviceAttach();
            #endif
        }

        // Check if it's an active USB connection
        if ((USBGetDeviceState() >= CONFIGURED_STATE) && (USBIsDeviceSuspended() == FALSE))
        {
            // POWER -> COMPUTER
            if (status.connection == CONNECTION_POWER)
            {
                status.connection = CONNECTION_COMPUTER;
                // Mount the drive if connected and device unlocked
                if (USB_BUS_SENSE && status.lockCode == 0)
                {
                    StateSetDrive(DRIVE_MOUNTED, 0, RECORD_STOP_REASON_CONNECTED);
                }
            }
        }
        else
        {
            // COMPUTER -> POWER
            if (status.connection == CONNECTION_COMPUTER)
            {
                status.connection = CONNECTION_POWER;
                // If the drive is mounted, unmount it
                if (status.drive == DRIVE_MOUNTED)
                {
                    StateSetDrive(DRIVE_UNOWNED, 0, 0);
                }
            }
        }

        // Keep OS aware of drive mounted status
        fsftlUsbDiskMounted = (status.drive == DRIVE_MOUNTED);

        // Check bus status and service USB interrupts.
        #ifndef USB_INTERRUPT
        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
        #endif

        USBProcessIO();

        // Process any incoming line
        if (status.connection == CONNECTION_COMPUTER)
        {
	        int maxline = 20;
            while (maxline--)
            {
	            char *line_usb = (char*)_user_gets();
	            if (line_usb == NULL) { break; }
	            SettingsCommand(line_usb, SETTINGS_USB);
           }
        }
    }
    else
    {
        // COMPUTER|POWER -> NONE
        if (status.connection != CONNECTION_NONE)
        {
            status.connection = CONNECTION_NONE;
            // If the drive is mounted, unmount it
            if (status.drive == DRIVE_MOUNTED)
            {
                StateSetDrive(DRIVE_MOUNTED, 0, 0);
            }

            #if defined(USB_INTERRUPT)
            USBDeviceDetach();
            #endif

            // Switch clock
			if(OSCCONbits.COSC != 0b111)
			{
				if (ReturnWhenSafeToClockSwitch())
				{
            		CLOCK_INTOSC();	// Switch clock
					BluetoothAdaptToClockSwitch();
				}
				else
				{
					StateSetWireless(WIRELESS_OFF);
					CLOCK_INTOSC();
				}
				
			}
        }
    }

    // Happy while the connection state is the same
    return (connected != usbAtStartup);
}


// Led status
void __attribute__((weak))LedTasks(void)
{
    static unsigned short time_since_disk_activity = 0;
    static unsigned char lastTime = 0xff;
    unsigned char newSeconds, secondElapsed = 0;;
    char holdUp = 0;

	newSeconds = (unsigned char) rtcTicksSeconds;
    if (lastTime != newSeconds)
    {
        lastTime = rtcTicksSeconds;
		secondElapsed = 1;
        
        // Check and update inactivity
        if (status.drive == DRIVE_MOUNTED)
        {
            time_since_disk_activity = FtlIncrementInactivity(); 				// Returns seconds of inactivity (0 if no activity since last flush);
			if ((time_since_disk_activity > 3)&& (time_since_disk_activity!=0))	// Zero means it has not been active since last flush
            {
                FtlFlush(1);                        // Inactivity time out
            }
        } 
		else 
		{ 		
			time_since_disk_activity = 0; 
		}
    }

	// Over ridden
    if (status.ledOverride >= 0)
    {
        LED_SET(status.ledOverride);
		return;
    }
	// Not over ridden - once per second
    else if (secondElapsed)
    {
		// Powered
		if (status.connection >= CONNECTION_POWER) // Charging, maybe on USB
		{	
			// Red LED - FTL busy
			if(time_since_disk_activity !=0)
			{
				LED_SET(LED_RED);
			}
			// Green LED
			else if (status.battery == BATTERY_FULL )	
			{
				LED_SET(LED_GREEN);
			}
			// Yellow LED
			else if(status.battery == BATTERY_CHARGING )
			{
				LED_SET(LED_YELLOW);
			}
			// Blue LED - bluetooth
			if (status.wireless == WIRELESS_CONNECTED)
			{
				LED_B = 1;
			}
			else if (status.wireless == WIRELESS_DISCOVERABLE)
			{
				LED_B = (newSeconds & 1);
			}
		}
		// Not powered
		// Wireless streaming (and or logging )
		else if ((logger.stream == STREAM_WIRELESS) && (status.wireless == WIRELESS_CONNECTED))
		{
			if (settings.debuggingInfo > 0)
			{
				if (logger.recording == RECORDING_ON) 	LED_B = LED_G = (newSeconds & 1); // Logging as well
				else 									LED_B = (newSeconds & 1);			// Streaming only
			}	
		}
		// Logging, with or without wireless
		else if (logger.recording == RECORDING_ON)
		{
			if (settings.debuggingInfo > 0)
			{
				if ((settings.debuggingInfo > 1) || (logger.debugFlashCount))
				{
					// Decrement flash counter
					if (logger.debugFlashCount)logger.debugFlashCount--;

					// Discoverable (and present)
					if(status.wireless >= WIRELESS_DISCOVERABLE )	{LED_SET(LED_CYAN);}
					// Not discoverable
					else											{LED_SET(LED_GREEN);}

					// Holdup 
					holdUp = 1;
				}
			}
		}
		// Not logging or streaming
		// Discoverable?
		else if (status.discoverable >= DISCOVERABLE_ALWAYS)
		{
			LED_SET(LED_BLUE);
			holdUp = 1;
		}
		// Connected?
		else if (status.wireless == WIRELESS_CONNECTED)
		{
			LED_SET(LED_BLUE);
		}
		// Waiting to log
		else if (logger.interval == INTERVAL_WAITING)
		{ 
			if (settings.debuggingInfo > 0)
			{
				// Green led flash
				if(newSeconds & 1)			{LED_SET(LED_YELLOW);holdUp = 1;}
				else						{LED_SET(LED_OFF); }
			}
		}
		// Low battery
		else if (status.battery == BATTERY_WARNING) 
		{ 
			// Battery warning: flash red (will flash as off while asleep when disconnected)
			if (settings.debuggingInfo > 0)
			{
				if(!(newSeconds & 1))		{LED_SET(LED_RED);holdUp = 1;}
				else						{LED_SET(LED_OFF); }
			}
		}
		// Nothing
		else
		{
			LED_SET(LED_OFF);
		}
	}
    
	// Wait for led to be minimally visible  
	if (holdUp) { DelayMs(1); }		// TODO: More power-efficient way of delaying to make the flash visible

    return;
}


// Battery tasks
void BatteryTasks(void)
{
    static unsigned short lastTime = 0xffff;
    if (lastTime == rtcTicksSeconds) { return; }        // Update 1Hz
    lastTime = rtcTicksSeconds;

    // Update battery status (battery reading updated in timer interrupt)
    if (status.connection == CONNECTION_NONE)
    {
        status.battery = BATTERY_NORMAL;

        // Check if battery too low...
        if (adcResult.batt < BATT_CHARGE_WARNING)
        {
			status.battery = BATTERY_WARNING;
        }
        
        // Check if battery too low...
        if (adcResult.batt < BATT_CHARGE_MIN_SAFE)
        {
            if (status.batteryLowTimer < BATT_EMPTY_INTERVAL) { status.batteryLowTimer++; }
            if (status.batteryLowTimer >= BATT_EMPTY_INTERVAL)
            {
                status.battery = BATTERY_EMPTY;
            }
        }
        else
        {
            status.batteryLowTimer = 0;
        }
        status.batteryFullTimer = 0;
    }
    else
    {
        status.batteryLowTimer = 0;
        if (status.battery == BATTERY_EMPTY || status.battery == BATTERY_WARNING || status.battery == BATTERY_NORMAL)
        {
            status.battery = BATTERY_CHARGING;
            status.batteryFullTimer = 0;
        }
        if (status.battery != BATTERY_FULL && adcResult.batt > BATT_CHARGE_FULL_USB) // Add extra minute of full detection to stop it latching prematurely
        {
            status.batteryFullTimer++;       // 1-60
            if (status.batteryFullTimer >= BATT_FULL_INTERVAL)
            {
                status.battery = BATTERY_FULL;      // while connected, this will latch
                if ((status.initialBattery != 0) && (status.initialBattery < BATT_CHARGE_MID_USB))
                {
                    // Increment battery health counter
                    SettingsIncrementLogValue(LOG_VALUE_BATTERY);
                    status.initialBattery = 0;      // just make certain this doesn't trip again
                }
            }
        }
    }
    return;
}


char ActionTasks(void)
{
    static unsigned short lastTime = 0xffff;
    if (lastTime == rtcTicksSeconds) { return 0; }        // Update 1Hz    
    lastTime = rtcTicksSeconds;

    if (status.actionCountdown > 0)
    {
        status.actionCountdown--;
        if (status.actionCountdown == 0)
        {
            if (SettingsAction(status.actionFlags))
            {
                return 1; // Restart!
            }
        }
    }
    return 0;
}


void StateSetWireless(StateWireless newState)
{
    if (!btPresent) { status.wireless = WIRELESS_OFF; return; }

    // If the wireless is off and we should be discoverable, turn it on
    if (status.wireless <= WIRELESS_OFF && newState != WIRELESS_OFF)
    {
		if (BluetoothInit(NULL, 0xffff, 0)) {;} 						// If successful
		else {BluetoothOff(); status.wireless = WIRELESS_OFF;return;}	// If failed, force off
    }

    // If the wireless is on (but not connected) and it shouldn't be discoverable, turn it off
    if (status.wireless != WIRELESS_OFF && newState == WIRELESS_OFF)
    {
        BluetoothOff();
    }

    // Set the state to what the caller requested (have to trust DISCOVERABLE <-> CONNECTED transitions are correct)
    status.wireless = newState;
}


void WirelessTasks(void)
{
    char connection = FALSE;
    char wantDiscoverable;

    if (!btPresent) { status.wireless = WIRELESS_OFF; return; }

	// Must call this all the time regardless of whether connected
    BluetoothSerialIO();

    // Should we be discoverable?
    #define DISCOVERABLE_TIME (1 * 60)

	// KL: Added bluetooth always on on USB attached
	#ifdef BLUETOOTH_ALWAYS_ON_IN_USB_MODE
    wantDiscoverable = ( USB_BUS_SENSE || status.discoverable == DISCOVERABLE_ALWAYS || (status.discoverable == DISCOVERABLE_AUTO && (status.inactivity < DISCOVERABLE_TIME)));
    #else
	wantDiscoverable = (status.discoverable == DISCOVERABLE_ALWAYS || (status.discoverable == DISCOVERABLE_AUTO && (status.inactivity < DISCOVERABLE_TIME )));
	#endif
	    
    // If the wireless is off and we should be discoverable, turn it on
    if (status.wireless == WIRELESS_OFF && wantDiscoverable)
    {
		BluetoothCommClear();
        StateSetWireless(WIRELESS_DISCOVERABLE);
    }
    
    // If the wireless is on (but not connected) and it shouldn't be discoverable, turn it off
    if (status.wireless == WIRELESS_DISCOVERABLE && !wantDiscoverable)
    {
		BluetoothCommClear();
        StateSetWireless(WIRELESS_OFF);
    }

    // Get connection status
	if (BluetoothGetState() == BT_CONNECTED)  connection = TRUE;
    
    // If the wireless is discoverable and now connected, set the state to connected
    if (status.wireless == WIRELESS_DISCOVERABLE && connection)
    {
        StateSetWireless(WIRELESS_CONNECTED);
    }
    
    // If the wireless state is connected, but no longer physically connected, set the state to discoverable
    if (status.wireless == WIRELESS_CONNECTED && !connection)
    {
		BluetoothCommClear();
        StateSetWireless(WIRELESS_DISCOVERABLE);
        status.inactivity = 0;       // reset inactivity timer
    }

    // Update I/O tasks
    if (status.wireless >= WIRELESS_DISCOVERABLE)
    {
        {
	        int maxline = 50;
            static char *line_bt;
            while (maxline--)
            {
	            line_bt = (char*)Bluetooth_gets();
	            if (line_bt == NULL) { break; }
                SettingsCommand(line_bt, SETTINGS_ALTERNATE);
	        }
        }
    }

}


void IntervalTasks(void)
{
    DateTime now;
    StateInterval newInterval;
    static unsigned short lastTime = 0xffff;
    if (lastTime == rtcTicksSeconds) { return; }  // Update 1 Hz
    lastTime = rtcTicksSeconds;

    now = RtcNow();

    if (settings.loggingStartTime >= settings.loggingEndTime) { newInterval = INTERVAL_NONE; }
    else if (now >= settings.loggingEndTime)   { newInterval = INTERVAL_AFTER; }
    else if (now <  settings.loggingStartTime) { newInterval = INTERVAL_WAITING; }
    else if (now >= settings.loggingStartTime) { newInterval = INTERVAL_WITHIN; }
    else { newInterval = INTERVAL_WAITING; } // (all cases already handled)

    StateSetInterval(newInterval);
}


const char *StopReasonToString(unsigned short reason)
{
	switch (reason)
	{
    	case RECORD_STOP_REASON_NONE:          return "NONE";
    	case RECORD_STOP_REASON_INTERVAL:      return "INTERVAL";
    	case RECORD_STOP_REASON_COMMAND:       return "COMMAND";
    	case RECORD_STOP_REASON_CONNECTED:     return "CONNECTED";
    	case RECORD_STOP_REASON_BATTERY:       return "BATTERY";
    	case RECORD_STOP_REASON_WRITE_ERROR:   return "WRITE_ERROR";
    	case RECORD_STOP_REASON_MEASURE_ERROR: return "MEASURE_ERROR";
		default: return "UNKNOWN";
	}	
}


#ifdef APPLICATION_UPDATE
extern void APPLICATION_UPDATE(void);
#endif	

// Logger tasks
void RunLogging(void)
{
    record_stop_reason_t stop = RECORD_STOP_REASON_NONE;
    unsigned short changeLoops;

    // Check peripherals
LED_SET(LED_CYAN);
	NandInitialize(); // Or device id will fail
	NandVerifyDeviceId();
    AccelVerifyDeviceId();
    GyroVerifyDeviceId();
    MagVerifyDeviceId();
    AltVerifyDeviceId();
	ProxVerifyDeviceId();
    
#ifdef TEST_FOR_FAILED_DEVICES
	Nop();
	Nop();
	// Missing peripheral flash
	{
		char problemCode = 0;
		char b;
		if (!nandPresent)  { problemCode |= 0x01; }
		if (!accelPresent) { problemCode |= 0x02; }
		if (!gyroPresent)  { problemCode |= 0x04; }
		if (!magPresent)   { problemCode |= 0x08; }
		if (!altPresent)   { problemCode |= 0x10; }
		if (!proxPresent)   { problemCode |= 0x20; }
		if (problemCode != 0)
		{
			LED_SET(LED_BLUE);
			DelayMs(500);
			for (b = 1; b <= 5; b++)
			{
				if (problemCode & (1 << b))
				{
					LED_SET(LED_WHITE); DelayMs(100); LED_SET(LED_BLUE); DelayMs(900); 
				}	
				else
				{
					LED_SET(LED_WHITE); DelayMs(700); LED_SET(LED_BLUE); DelayMs(300); 
				}	
			}
			DelayMs(500);
		}	
	}
#endif


    // Startup
LED_SET(LED_YELLOW);
    SettingsInitialize();           // Initialize settings from ROM
    SamplerInit();                  // Initialize sampler states (and peripherals)
    RecorderInit();                 // Initialize recorder states (must be after sampler init for data structures to be init'd properly)
LED_SET(LED_GREEN);
    FtlStartup();                   // FTL & NAND startup

    // Try to own the drive so we can read the configuration
    StateSetDrive(DRIVE_OWNED, 0, 0);

    // If we own the drive, read the configuration
    if (status.drive == DRIVE_OWNED || status.drive == DRIVE_OWNED_FULL)
    {
LED_SET(LED_MAGENTA);
        SettingsReadFile(SETTINGS_FILE);                                // Read commands from the script
        // Find the current logging data file
        if (LoggerFindCurrentFile(status.filespec, logger.filename, NULL) >= 0)
        {
            LoggerReadMetadata(logger.filename);                        // Read settings from the logging binary file
        }
    }
LED_SET(LED_OFF);

LED_SET(LED_BLUE);

	// For higher performance - run in pll mode by setting pll on here before btinit()
	// Blutooth initialised and turned off is not required - btPresent bit set by init() if found
	if(BluetoothInit(BLUETOOTH_NAME, settings.deviceId, BLUETOOTH_COD))	{status.wireless = WIRELESS_DISCOVERABLE;}
	else 																{status.wireless = WIRELESS_OFF;}
	#ifdef BLUETOOTH_ALWAYS_ON_IN_USB_MODE
	if (!USB_BUS_SENSE && status.discoverable < DISCOVERABLE_ALWAYS) {BluetoothOff(); status.wireless = WIRELESS_OFF;} // Leave on if attached 
	#else
	if (status.discoverable < DISCOVERABLE_ALWAYS)	{BluetoothOff(); status.wireless = WIRELESS_OFF;}
	#endif
LED_SET(LED_OFF);

	// Run first
    BatteryTasks();

    // Loop -- the state space should take care of exiting, but as a backup plan, limit the number iterations once the connectivity has changed
    for (changeLoops = 20; stop == RECORD_STOP_REASON_NONE && ((USB_BUS_SENSE == usbAtStartup) || changeLoops--); )
    {

        DebugTasks();

        // USB Connection status and tasks - called first so status.connection is always valid
        if (UsbConnectionTasks()) { stop = RECORD_STOP_REASON_CONNECTED; break; }

        // Battery status
        BatteryTasks();

#ifdef APPLICATION_UPDATE
		APPLICATION_UPDATE();
#endif	

        // Exit if disconnected and battery too low
        if (status.connection == CONNECTION_NONE && status.battery == BATTERY_EMPTY) { stop = RECORD_STOP_REASON_BATTERY; break; }

        // Bluetooth status
        WirelessTasks();

        // Update recording interval
        IntervalTasks();

        // Update sampling status/perform tasks
        SamplerTasks();

        // Update recording status/perform tasks
        RecordingTasks();

        // Delayed action commands
        if (ActionTasks()) { stop = RECORD_STOP_REASON_COMMAND; break; }   // Restart

        // If the drive is un-owned, try to take ownership
        if (status.drive == DRIVE_UNOWNED) { StateSetDrive(DRIVE_OWNED, 0, 0); }

        // Exit if disconnected, wireless off, never discoverable and (never recording, or no recording interval, or after recording interval)
        if (status.connection == CONNECTION_NONE && status.wireless == WIRELESS_OFF && status.discoverable == DISCOVERABLE_NEVER && (logger.schedule == SCHEDULE_NEVER || logger.interval == INTERVAL_NONE || logger.interval == INTERVAL_AFTER)) { stop = RECORD_STOP_REASON_INTERVAL; break; }

        // Exit if disconnected, wireless off, never discoverable and drive full
        if (status.connection == CONNECTION_NONE && status.wireless == WIRELESS_OFF && status.discoverable == DISCOVERABLE_NEVER && (status.drive == DRIVE_OWNED_FULL || status.drive == DRIVE_ERROR)) { stop = RECORD_STOP_REASON_WRITE_ERROR; break; }

		// Reset WDT while in this loop
        RtcSwwdtReset();
        
		// Sleep states.....If we're able to sleep (no wired connection)
	    if (status.connection == CONNECTION_NONE ) 
	    {
			// LED off unless over ridden
			if(status.ledOverride == -1)LED_SET(LED_OFF);
#ifndef BT_NEVER_SLEEP
			switch (BluetoothGetPowerState()){
				case (BT_STANDBY)	:
				case (BT_SLEEP)	:	SystemPwrSave(WAKE_ON_WDT|LOWER_PWR_SLOWER_WAKE);
									break;
				case (BT_IDLE)	:	Idle();
									break;
				case (BT_ACTIVE):	break;
			}
#endif			
		 } // Woken  (if was in idle or sleep)

#ifdef HAVE_DISPLAY
		// Clock tasks at 1Hz or on orientation change
		{
			static unsigned short last = 0;
			signed char checkForGensture = TestForWatchGesture();
			signed char checkForButton = ButtonTasks();		// Button tasks
		

			if ((RtcSeconds() != last) || (checkForGensture != 0) || checkForButton == 1)
			{
				last = RtcSeconds();
				if (USB_BUS_SENSE || bluetoothState >= BT_CONNECTED)RunClockTasks(1); // Clock on 
				else if (checkForButton)							RunClockTasks(5); // Turn on clock for 5 seconds
				else if (checkForGensture==1)						RunClockTasks(5); // Turn on clock for 5 seconds
				else if (checkForGensture==-1)						RunClockTasks(-1);// Turn off clock
				else												RunClockTasks(0); // Count down to display off
			}
		}
#endif


		// LED Update
        LedTasks();

    }// Finished main loop - clean up and restart (unless battery is very low)

// Clean-up code

    // Stop wireless
    StateSetWireless(WIRELESS_OFF);

	// Switch back to normal clock if PLL on
	if (OSCCONbits.NOSC == 0b001) CLOCK_INTOSC();   // KL added

    // Stop recording
    StateSetRecording(RECORDING_OFF, stop);

    // Stop streaming
    StateSetStream(STREAM_NONE);

    // Stop sampling
    StateSetSampling(SAMPLING_OFF, 1);

    // Fully stop the accelerometer (as stopping sampling will leave the activity interrupts on)
    AccelStandby();

	// Shut off display 
	DisplayClear();

    // Just be sure these are off
    GyroStandby();
    BluetoothOff();

	SettingsAddLogEntry(LOG_CATEGORY_STOP | stop, RtcNow(), StopReasonToString(stop));

    // Block and wait for USB connection, unless
    if (stop != RECORD_STOP_REASON_COMMAND && stop != RECORD_STOP_REASON_CONNECTED)
    {
        // Shutdown and sleep until connected
        //    Shutdown();
        //    SleepUntilConnection();

        // TODO: Restart flag in settings.h
        while (USB_BUS_SENSE == usbAtStartup)
        {
            LED_SET(LED_OFF);

            SystemPwrSave(WAKE_ON_WDT|LOWER_PWR_SLOWER_WAKE);
            RtcSwwdtReset();
            
			// Flash red when stopped for an error reason -- e.g. low battery / disk full
			if (stop == RECORD_STOP_REASON_BATTERY || stop == RECORD_STOP_REASON_WRITE_ERROR || stop == RECORD_STOP_REASON_MEASURE_ERROR)
			{
				unsigned int i;
				LED_SET(LED_RED);
				for (i = 2500; i; i--) { Nop(); }
			}	
//if (settings.debuggingInfo >= 3 && stop == RECORD_STOP_REASON_INTERVAL) { unsigned int i; LED_SET(LED_YELLOW); for (i = 2500; i; i--) { Nop(); } }	

        }
    }

    return;
}

signed char __attribute__((weak)) ButtonTasks(void)
{
	return 0;
}
//EOF

