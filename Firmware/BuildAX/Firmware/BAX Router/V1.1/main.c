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

// BuildAX Router
// Karim Ladha, 2013-2014

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "TaskScheduler.h"
#include "Settings.h"
#include "Analog.h"
#include "Peripherals/Eeprom.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/ExRtc.h"
#include "Peripherals/myi2c.h"
#include "Peripherals/Si443x.h"
#include "USB/USB.h"
#include "USB/USB_CDC_MSD.h"
#ifdef USE_FAT_FS
	#include "FatFs/FatFsIo.h"
#else
	#include "MDD File System/FSIO.h"
#endif

#include "Utils/Util.h"
#include "Data.h"
#include "BaxRx.h"
#include "MiWiRx.h"

#include "TcpIpTasks.h"
#include "TinyHTTP.h"
#include "FSConfig.h"

// Debug setting
//	#define DEBUG_ON
#include "Debug.h"

// Prototypes
void TimedTasks(void);
void LedTasks(void);
void ButtonTasks(void);
void USBCdcTasks(void);
void SystemCheck(void);
void SaveLogFile(void);
void StartNewLogFile(void);
void CheckLogFileLen(void);
void BaxLinkPktDetected (BaxPacket_t* pkt);

// Stopwatch
extern unsigned long cyclecount;
void inline CycleTimerStart(void);
void inline CycleTimerOff(void);
unsigned long inline CycleTimerGet(void);

// Globals
unsigned short ledOveride=0, ledOverideTimer=0;

// Interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
 	static unsigned int INTCON1val;
	LED_SET(LED_ALL);
	INTCON1val = INTCON1;
	INTCON1 = 0;
#ifndef __DEBUG
    Reset();
#else
	while (!BUTTON)
	{
		LED_SET(LED_ALL);
		DelayMs(250);
		LED_SET(LED_OFF);
		DelayMs(250);
	}
	Nop();
	Nop();
	#warning "Default interrupt does not reset"
#endif
}

// RTC
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks();
	RtcSwwdtIncrement();    // Increment software RTC, reset if overflow
}
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void)
{
	RtcTimerTasks();
	TimedTasks();			// Flash LEDs, update ADC
}
void __attribute__((interrupt,auto_psv)) Si44Int(void)
{
	Si44RadioIrqHandler();	// Radio isr
	BaxReadPacket();		// Data tasks
}
void __attribute__((interrupt,auto_psv)) MRFInt(void)
{
	MRFIsr(); 				// Radio isr
	MiWiRxReadPacket();		// Data tasks
}

// Main routine
int main(void)
{
	unsigned short i, rcon = RCON;
	RCON = 0;

    // Initialization
	InitIO();			// I/O pins
	POWER_LED = 1; 		// Indicate powered
	CLOCK_INTOSC();		// Oscillator
	myI2Cclear();		// Once
	InitI2C();			// Once
    UpdateAdc();		// Get ADC values

	// LED sequence
	for(i=0;i<8;i++)
	{
		LED_SET((0b1<<i));
		DelayMs(100);
	}
	POWER_LED = 1;

	// Start int/ext RTC, eeprom  and the ISR + SWWDT
	I2C_eeprom_present(); // See if we have an eeprom
	ExRtcVerifyDeviceId(); 	// Check for external RTC
	RtcStartup();			// Internal RTC startup
	ExRtcStartup();			// External RTC startup
	settings.start_time = ExRtcReadTime(); 			// Read ext RTC for start time;
	if (settings.start_time != DATETIME_INVALID)	// If ext RTC had a valid time
	{
		RtcWrite(settings.start_time);				// Use it to set the internal RTC
	}
	else 
	{
		settings.start_time = RtcNow();				// Read the internal RTC instead
		ExRtcWriteTime(settings.start_time);		// The int RTC resets to DATETIME_MIN if invalid too
	}
	RtcInterruptOn(TIMED_TASKS_RATE); 				// RTC on + Timer1 isr on 10 HZ

	// Switch to high performance clock, check SD card, init FS & read new settings from disk / eeprom
	CLOCK_XTAL_PLL();		// Oscillator
	SettingsInitialize();	// Settings

// Debug setting overide
#ifdef __DEBUG
settings.usb_stream = STATE_ON;
settings.usb_stream_mode = 0;
settings.telnet_stream = STATE_ON;
settings.telnet_stream_mode = 0;
settings.udp_stream = STATE_ON;
settings.udp_stream_mode = 0;
settings.settings_source = 3;
status.usb_disk_mounted = 0;
status.usb_disk_wp = 0;
#endif

	// Data sources
	InitDataList();

	// Init scheduler
	SchedulerInit();
	SchedulerAddTask(settings.save_interval, settings.save_interval, TASK_RPT_OFFSET_OFFSET, SaveLogFile, NULL);
	SchedulerAddTask(settings.save_interval, settings.max_file_time, TASK_RPT_OFFSET_TIME, StartNewLogFile, NULL);
	SchedulerAddTask(DEFAULT_FILE_CHECK_INTERVAL, DEFAULT_FILE_CHECK_INTERVAL, TASK_RPT_OFFSET_OFFSET, CheckLogFileLen, NULL);
	SchedulerAddTask(0, 0, TASK_RPT_OS_ONE_HZ, ButtonTasks, NULL);
	SchedulerAddTask(5ul, 60ul, TASK_RPT_OFFSET_OFFSET, BaxRxTasks, NULL);
	SchedulerAddTask(5ul, MIWI_KEEP_ALIVE_SEC, TASK_RPT_OFFSET_OFFSET, MiWiRxTasks, NULL);
	SchedulerAddTask(0, 0, TASK_RPT_OS_ONE_HZ, SystemCheck, NULL);

	// Log reset to error file
	DBG_log_error("Reset: %04X\r\n",(unsigned short)rcon);

    USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
    #ifdef USB_INTERRUPT
    USBDeviceAttach();
    #endif

    while(USB_BUS_SENSE) // Really - while(the device has a power source!)
    {
        // Usb stack
        #ifndef USB_INTERRUPT
        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
        #endif
        USBProcessIO();	
		// USB state/Serial input
		USBCdcTasks();

		// TCPIP tasks
		TcipipTasks();

		// Read data elements - all open streams and the comand/error streams
		ReadDataElements(GetOpenStreams() | CMD_ERR_STREAMS);

		// Scheduler
		if(SchedulerTasks())
		{
			// SWWDT
			RtcSwwdtReset();
		
		}// 1 Hz tasks

    } // while(USB_BUS_SENSE)

	#if defined(USB_INTERRUPT)
    USBDeviceDetach();
	#endif

	Reset();                // Reset
	return 0;
}

// Timed tasks - IN ISR!!!
void TimedTasks(void)
{
	static unsigned char prescaler = 0;

	// LED tasks at full update rate
	LedTasks();

	// Tasks at 1Hz
	if(prescaler++ >= TIMED_TASKS_RATE)
	{
		prescaler = 0;

		// Read ADC
	    UpdateAdc();
		
		// Add other 1Hz tasks		
	}
    return;
}

// IN ISR
void LedTasks(void)
{

	// LED overide
	if(ledOverideTimer) 
	{
		static unsigned char toggle = 0;
		ledOverideTimer--;
		// Allow flashing
		if(toggle == 0)	{toggle = 1;	LED_SET(ledOveride);}
		else 			{toggle = 0; 	LED_SET(ledOveride>>8);}
		return;
	}

	// Power LED always on
	POWER_LED = 1; 

	// USB/CDC LED
	if(status.usb_state >= INITIALISED)
		{CDC_LED = 1;}
	else 
		{CDC_LED = 0;}	

	// Ethernet
	if(status.ethernet_state == ACTIVE)
		{TCP_LED = 1;}
	else
		{TCP_LED = 0;}	

	// SD card LED usb mounted
	if(!SD_CD && status.usb_disk_mounted) 
		{SD_CARD_MOUNT_LED = 1;}
	else
		{SD_CARD_MOUNT_LED = 0;}

	// File logging LED
	if (!SD_CD && status.file_state == ACTIVE) 
		{FILE_LED = 1;}
	else
		{FILE_LED = 0;}

	// Si44 radio
	if(	status.radio_state == ACTIVE)
		{SI44_LED = 1;}
	else
		{SI44_LED = 0;}

	// Telnet
	if(status.telnet_mask != 0)
		{TEL_LED = 1;}
	else
		{TEL_LED = 0;}
		
	return;
}

// Firmware controlled media detect for USB MSD stack
unsigned char Usb_Sd_Card_Is_Available (void)
{
	SD_CARD_MOUNT_LED = !SD_CARD_MOUNT_LED; // Flicker LED to show activity
	return (!SD_CD & status.usb_disk_mounted);
}
unsigned char Usb_Sd_Card_Is_Protected (void)
{
	SD_CARD_MOUNT_LED = !SD_CARD_MOUNT_LED; // Flicker LED to show activity
	return (SD_CD | (!status.usb_disk_mounted) | (status.usb_disk_wp));
}
void USBCdcTasks(void)
{		
	if ((USBGetDeviceState() >= CONFIGURED_STATE) && (USBIsDeviceSuspended() == FALSE))
	{
		status.usb_state = INITIALISED; 								// Connected
		if(settings.usb_stream == STATE_ON)status.usb_state = ACTIVE;	// Data on
	
		// Commands - added as data element
		const char *line = _user_gets();
		if (line != NULL)
		{
			AddDataElement(TYPE_TEXT_ELEMENT,DATA_DEST_ALL, strlen(line), CMD_CDC_FLAG , (void*)line); 	// Add element
		}
	}
	else
	{
		status.usb_state = PRESENT; // Powered only
	}
}


// Button tasks
void ButtonTasks(void)
{
	static unsigned short buttonTimer = 0;
	// Button hold tasks
	if(BUTTON && buttonTimer == 0)
	{
		buttonTimer = 1;			// On press 
		ledOverideTimer = 10;		// 1 sec 
		ledOveride = 0x55AA;		// All LEDs toggling
		// Enable device discovery
		BaxSetDiscoveryCB(BaxLinkPktDetected);
		// Cancel device discovery after timeout
		ScheduleTaskRemove(BaxSetDiscoveryCB);
		SchedulerAddTask(300ul, 0, TASK_OFFSET_ONCE|TASK_CB_USE_ARG, BaxSetDiscoveryCB, NULL);
	}
	else if (!BUTTON) 
	{
		buttonTimer = 0;
		if(USBGetDeviceState() == DETACHED_STATE)
		{
	    	USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
	   		#ifdef USB_INTERRUPT
	    	USBDeviceAttach();
	    	#endif
	 	}   
	}
	else buttonTimer++; // Increment timer

	// Check for hold events
	if (buttonTimer >= 11)
	{
		buttonTimer = 12;					// Clamp to prevent wrap
	}
	else if(buttonTimer == 10)				// 10 sec hold - factory reset
	{
		ledOverideTimer = 10; 				// 1 seconds 
		ledOveride=0x00ff;					// All LEDs flashing
		SettingsAction(ACTION_SAVE); 		// Save files 
		SettingsAction(ACTION_SETTINGS_WIPE);// Reset all settings
		SettingsInitialize();				// Load these defaults
	}
	else if(buttonTimer == 5)				// 5 sec hold
	{	
		// Un/Mount USB+disk
		status.radio_state = OFF;			// Reser radio
		#ifdef USB_INTERRUPT
		USBDeviceDetach();					// Detatch radio
		#endif
		ledOverideTimer = 10;				// 1 sec 
		ledOveride = 0x0770;				// Alternate leds
		status.usb_disk_mounted = !status.usb_disk_mounted;
		status.usb_disk_wp = (status.file_state == ACTIVE);	
	}
	return;
}

void BaxLinkPktDetected (BaxPacket_t* pkt)
{
	if(pkt==NULL)
	{
		BaxSetDiscoveryCB(NULL);
		return;
	}
	// Add new key to list
	BaxKey_t new;
	new.address = pkt->address;
	memcpy(&new.b[4],&pkt->b[6],AES_BLOCK_SIZE);
	BaxAddKeys(&new, 1);
	return;
}

// System tasks
void SystemCheck(void)
{
	// Setup radios (if off)
	if(status.radio_state == OFF)	BaxRxInit();
	if(status.mrf_state == OFF)		MiWiRxInit();

	// Support hot plugging SD (with data loss if files open)
	if (SD_CD) // No card, set state
	{
		if(status.file_state >= INITIALISED)
		{
			if(status.output_bin_file)
				FSfclose(status.output_bin_file);
			if(status.output_text_file)
				FSfclose(status.output_text_file);
			if(status.error_file)
				FSfclose(status.error_file);
			status.file_state = NOT_PRESENT;
		}
	}
	else 
	{
		if(status.file_state == NOT_PRESENT)
			SettingsInitialize(); // Remount card (load settings.ini) 
	}

}

void SaveLogFile(void)
{
	// Save the file
	SettingsAction(ACTION_SAVE);
}

void StartNewLogFile(void)
{
	// Try open a log file (will use loaded settings)
	SettingsAction(ACTION_START_FILE);	// Try start new file
	if(status.file_state == ACTIVE) 	// If logging to disk
	{
		// Always set session number and time on open
		status.usb_disk_wp = 1; 
		settings.session_number++;
		status.session_number = settings.session_number;
		status.session_start_time = settings.start_time;
		// Save setting number - so on reset it starts another file
		SettingsSave();	
	}
}
void CheckLogFileLen(void)
{
	// Check our o/p file lengths
	if(	(FSFileSize(status.output_text_file) >= settings.max_file_size)||
		(FSFileSize(status.output_bin_file) >= settings.max_file_size))
	{
		// Open a log file (will use loaded settings)
		SettingsAction(ACTION_START_FILE);	// Try start new file
		if(status.file_state == ACTIVE) 	// If logging to disk
		{
			// Always set session number and time on open
			status.usb_disk_wp = 1; 
			settings.session_number++;
			status.session_number = settings.session_number;
			status.session_start_time = settings.start_time;
			// Save setting number - so on reset it starts another file
			SettingsSave();	
		}
	}
}

#ifdef __DEBUG
unsigned long cyclecount;
// CPU stop watch, upto 2^32 cycles + overflow detect
inline void CycleTimerStart(void)
{
	T4CONbits.TON = 0;// Stop timer
	TMR4 = 0;		// Clear count
	PR4 = 0xffff;	// Set period
	TMR5 = 0;		// Clear count
	PR5 = 0xffff;	// Set period
	T4CON = 0x0008; // no prescaler
	T5CON = 0x0000; // no prescaler
	IFS1bits.T5IF = 0;// Clear overflow flag
	T4CONbits.TON = 1;// Start timer
}
inline void CycleTimerOff(void)
{
	T4CONbits.TON = 0;// Stop timer
}
inline unsigned long CycleTimerGet(void)
{
	union{
	unsigned short w[2];
	unsigned long l;
	}time;
	time.w[0] = TMR4;
	time.w[1] = TMR5;
	if(!IFS1bits.T5IF)cyclecount = time.l;
	else cyclecount = 0xffffffff;
	return cyclecount;
}
#endif

//EOF

