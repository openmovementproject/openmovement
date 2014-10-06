/* 
 * Copyright (c) 2009-2013, Newcastle University, UK.
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
 
// Run attached (USB)

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Rtc.h"
#include "USB/USB.h"
#include "USB/usb_function_cdc.h"
#include "USB/USB_CDC_MSD.h"
#include "Analog.h"
#include "Utils/Fifo.h"
#include "Utils/Util.h"
#include "GraphicsConfig.h"
#include "Graphics/DisplayBasicFont.h"
#include "Graphics/Primitive.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/PIRULEN32PIX.h"
#include "Settings.h"
#include "Scales.h"

// GSM tranceiver
#include "Peripherals/Uart_vdma.h"
#include "TelitModule.h"
// Globals

// Prototypes
void RunAttached(void);
void TimedTasks(void);
void LedTasks(void);
extern void PrintTopDisplayLine(void);
extern void RunClockTasks(unsigned char forceOn);
extern char* FormNPRIMessage(void);

// Globals
static unsigned char restart = 0;
static unsigned short lastTime = 0;
unsigned char runGsmModule = FALSE;

// Attached to USB
void RunAttached(void)
{
    // Enable peripherals
	RtcInterruptOn(0); // Keeps time upto date

	AdcInit(); // Leave on for USB opperation
	AdcSampleNow();

	// Indicate new data
	DisplayInit();
	DisplayClear();
	Display_print_xy(" -= USB =-",0,2,2);

    CLOCK_PLL();	// PLL clock
    DelayMs(10); 	// Allow PLL to stabilise

	ScalesCommInit();

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
			// Recieve from USB. Process some commands and send some commands to GSM module
            char *line = (char*)_user_gets();	
            if (line != NULL)
            {
				#if 0 // Can redirect some of the GSM commands here
				if((line[0] == 'A'||line[0] == 'a')&&(line[1] == 'T'||line[1] == 't')) // AT command
				{
					unsigned short len = strlen(line); 	// Get length
					if (len<(64-2))
					{
						line[len] = '\r';line[len+1] = '\n';
						GsmSendData((line),(len+2));		// Send the line
						while(GsmTxBusy())
						{
							USBCDCWait();					// Wait for it to send
						}
					}
				}
                else
				#endif 
				SettingsCommand(line);
            }

			// Debugging - relay scales readings here
			if (ScalesDataReady()){
				if(!ScalesParseData())
					ScalesCommClear();
				else
				{
					char* msg;
					// If the data is OK - break out of this loop into next one
					printf("Scales data:\r\n");
					msg = FormNPRIMessage();
					printf("%s\r\n",msg);
					// Clear the receiver for next time if the data is currupt
					ScalesCommClear();
				}
			}
				
	
			// Run gsm state machine
			if (runGsmModule == TRUE)
			{
				#define MAX_TASK_INTERVAL (0x0000fffful / GSM_STATE_MACHINE_RATE_HZ)
				static unsigned long lastTaskTime = 0;
				unsigned long now = RtcTicks();
				unsigned long interval = now-lastTaskTime;
				if (interval > MAX_TASK_INTERVAL)
				{
					lastTaskTime = now;
					UpdateAdc();
					if(gsmState.machineState != GSM_OFF) // Don't call while off
						GsmStateMachine();
					PrintTopDisplayLine();
				}
			}
        }	

        TimedTasks();
		
    }

	#if defined(USB_INTERRUPT)
    USBDeviceDetach();
	#endif

    return;
}


// Timed tasks
void TimedTasks(void)
{
    if (lastTime != rtcTicksSeconds)
	//static unsigned int i;
	//if(!i++)
    {
        lastTime = rtcTicksSeconds;
		LedTasks();

		// WDT
		RtcSwwdtReset();

		// Read ADC and update battery status
        AdcSampleNow();
        if (adcResult.batt > BATT_CHARGE_FULL && status.batteryFull<60) // Add extra minute of full detection to stop it latching prematurely
        {
            status.batteryFull++;
        }
		else
		{
			status.batteryFull = 0;
		}

    }
    return;
}


// Led status while attached
void LedTasks(void)
{
    if (status.ledOverride >= 0)    {LED_SET(status.ledOverride);}
    else if (status.batteryFull) 	{LED_SET(LED_GREEN);}       // full - flushed
    else                    		{LED_SET(LED_YELLOW);}      // charging - flushed
    return;
}


