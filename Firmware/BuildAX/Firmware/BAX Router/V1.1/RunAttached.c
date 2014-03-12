/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// BuildAX Router - Run Attached
// Karim Ladha, 2013-2014

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#ifdef HAS_PROX
#include "Peripherals/Prox.h"
#endif
#include "Peripherals/Rtc.h"
#include "USB/USB.h"
#include "USB/usb_function_msd.h"
#include "USB/usb_function_cdc.h"
#include "USB/USB_CDC_MSD.h"
#include "MDD File System/FSIO.h"
#include "Analog.h"
#include "Utils/Fifo.h"
#include "Settings.h"
#include "Logger.h"
#include "Utils/Util.h"
#include "GraphicsConfig.h"
#include "Graphics/DisplayBasicFont.h"
#include "Graphics/Primitive.h"
#include "Graphics/DisplayDriver.h"
#include "Peripherals/myi2c.h"
#include "Graphics/PIRULEN32PIX.h"

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
	// Do this first to give the card time to start up
	SD_ENABLE(); 		// Turn on SD card

    // Enable peripherals
	RtcInterruptOn(0);  // Keeps time upto date

	LED_SET(LED_WHITE);
    CLOCK_PLL();		// PLL clock

    // Initialize sensors
    // Check if we have an accelerometer
    AccelVerifyDeviceId();
    // Check if we have a gyro
	#ifdef USE_GYRO 
    GyroVerifyDeviceId();
	#endif
	#ifdef HAS_PROX
	// Check for prox
	ProxVerifyDeviceId();
	ProxStartup();
	#endif

    // Initialize sensors
    AccelStartup(ACCEL_RANGE_4G|ACCEL_RATE_100);
	#ifdef USE_GYRO 
	GyroStartup();
	#endif
	AdcInit();

	#ifndef NO_DISPLAY
	DisplayClear();
	Display_print_xy(" <= USB =>",0,2,2);
	#endif
	
	status.diskMounted = (status.lockCode == 0x0000) ? 1 : 0;
	status.stream = 0;

//   MDD_MediaInitialize();  // KL FIX: The SD card is re-inited in the usb framework which causes a lockup in some cases

	// Power up module if off
	PMD4bits.USB1MD = 0;

    USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
    #ifdef USB_INTERRUPT
    USBDeviceAttach();
    #endif

    while(USB_BUS_SENSE && restart != 1)
    {
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
                status.stream = 0;  // Disable streaming
                SettingsCommand(line, SETTINGS_USB);
            }
        }
        else
        {
            status.attached = -1;
        }
	
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
				#ifdef USE_GYRO 
				gyro_t gyroSample;
				#endif

				extern unsigned char scratchBuffer[];
				char * ptr = (char *)scratchBuffer;
				unsigned short len;

                lastSampleTicks += STREAM_INTERVAL;
                if (now - lastSampleTicks > 2 * STREAM_INTERVAL) { lastSampleTicks = now; } // not keeping up with sample rate
                
#ifdef HAS_PROX
				// Sample sensors
				if(ProxSampleReady())
				{
					ProxReadSample();
					ProxStartSample();
				}
#endif
                AccelSingleSample(&accelSample);
				#ifdef USE_GYRO
				GyroSingleSample(&gyroSample);
				#endif
			
				// Write ascii to scratch buffer
				ptr = (char *)scratchBuffer;
				ptr += sprintf(ptr, "\f$ACCEL=%d,%d,%d\r\n", accelSample.x, accelSample.y, accelSample.z);
				#ifdef USE_GYRO 
				ptr += sprintf(ptr, "$GYRO=%d,%d,%d\r\n", gyroSample.x, gyroSample.y, gyroSample.z);
				#endif
				#ifdef HAS_PROX
				ptr += sprintf(ptr, "$PROX=%d\r\n", prox.proximity);
				ptr += sprintf(ptr, "$LIGHT=%d\r\n", prox.light);
				#endif
				len = (unsigned short)((void*)ptr - (void*)scratchBuffer);

				// Stream over USB
				if ((status.stream) && status.attached == 1) { usb_write(scratchBuffer, len); }
            }
        }
    }
	#if defined(USB_INTERRUPT)
    USBDeviceDetach();
	#endif
    status.attached = -1;

    return;
}


// Timed tasks
void TimedTasks(void)
{
    if (lastTime != rtcTicksSeconds)
    {
        lastTime = rtcTicksSeconds;
		LedTasks();

		// Lets display the clock value here on the screen
		#ifndef NO_DISPLAY
		RunClockTasks(0);
		#endif
		
		// WDT
		RtcSwwdtReset();

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
//            FtlFlush(1);	// Inactivity time out on scratch hold
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



