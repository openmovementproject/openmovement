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
 *
 * CWA3.1 Karim Ladha 08/12/12
 */
 
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Peripherals/Nand.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Mag.h"
#include "Peripherals/Rtc.h"
#include "USB/USB.h"
#include "USB/usb_function_msd.h"
#include "USB/usb_function_cdc.h"
#include "USB/USB_CDC_MSD.h"
#include "MDD File System/FSIO.h"
#include "Ftl/FsFtl.h"
#include "Peripherals/Analog.h"
#include "Utils/Fifo.h"
#include "Settings.h"
#include "Logger.h"
#include "Util.h"
#include "BTtasks.h"
#include "GraphicsConfig.h"
#include "DisplayBasicFont.h"
#include "Graphics/Primitive.h"
#include "Graphics/DisplayDriver.h"
#include "myi2c.h"
#include "PIRULEN32PIX.h"
#include "Peripherals/bmp085.h"

// Prototypes
void RunAttached(void);
void TimedTasks(void);
void LedTasks(void);
extern void PrintTopDisplayLine(void);
extern void RunClockTasks(unsigned char forceOn);

// Globals
static unsigned char restart = 0;
static unsigned char inactive = 0;
static unsigned short lastTime = 0;


// Attached to USB
void RunAttached(void)
{
    // Enable peripherals
	RtcInterruptOn(0); // Keeps time upto date

    // Initialize sensors
    AccelStartup(ACCEL_RANGE_4G|ACCEL_RATE_100);
    GyroStartup();
	MagStartup(10);

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
			#define STREAM_RATE 10
			#define STREAM_INTERVAL (0x10000UL / STREAM_RATE)
			static unsigned long lastSampleTicks = 0;
            unsigned long now = RtcTicks();
            if (lastSampleTicks == 0) { lastSampleTicks = now; }
            if (now - lastSampleTicks > STREAM_INTERVAL)
            {
                accel_t accelSample;
				gyro_t gyroSample;
				mag_t magSample;
                lastSampleTicks += STREAM_INTERVAL;
                if (now - lastSampleTicks > 2 * STREAM_INTERVAL) { lastSampleTicks = now; } // not keeping up with sample rate
                
                AccelSingleSample(&accelSample);
				GyroSingleSample(&gyroSample);
				MagSingleSample(&magSample);
				printf("$ACCEL=%d,%d,%d\r\n", accelSample.x, accelSample.y, accelSample.z); 
				printf("$GYRO=%d,%d,%d\r\n", gyroSample.x, gyroSample.y, gyroSample.z);
				printf("$MAG=%d,%d,%d\r\n", magSample.x, magSample.y, magSample.z);
                USBCDCWait();
            }
        }
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

		// Lets display the clock value here on the screen
		RunClockTasks(0);
		
		// WDT
		RtcSwwdtReset();

        // Check and update inactivity
        inactive = FtlIncrementInactivity();

		// Read ADC and update battery status
        AdcSampleNow();
        if (adcResult.batt > BATT_CHARGE_FULL_USB && status.batteryFull<60) // Add extra minute of full detection to stop it latching prematurely
        {
            status.batteryFull++;
            if ((status.batteryFull >= 60) && (status.initialBattery != 0) && (status.initialBattery < BATT_CHARGE_MID_USB) )
            {
                // Increment battery health counter
                SettingsIncrementLogValue(LOG_VALUE_BATTERY);
            }
        }
		else
		{
			status.batteryFull = 0;
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
    if (status.ledOverride >= 0)
    {
        LED_SET(status.ledOverride);
		return;
    }
    else if (status.actionCountdown)
    {
        LED_SET(LED_RED);
    }
    else
    {
        if (inactive == 0)
        {
            if (status.batteryFull) {LED_SET(LED_GREEN);}       // full - flushed
            else                    {LED_SET(LED_YELLOW);}      // charging - flushed
        }
        else					
        {
            LED_SET(LED_RED);     								// unflushed
        }
    }
    return;
}



