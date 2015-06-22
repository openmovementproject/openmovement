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
// Karim Ladha 2015
// AX9 project

// Includes
#include <stdint.h>
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Analog.h"
#include "Utils/Util.h"
#include "Settings.h"
#include "Peripherals/SysTime.h"
#include "Peripherals/Nand.h"
#include "Peripherals/bmp085.h"
#include "Peripherals/MultiSensor.h"
#include "Peripherals/Nvm.h"
#include "SamplerStreamer.h"
#include "att.h" 
#include "bt-app.h"
#include "bt-serial.h"
#include "btconfig.h"	// Settings
#include "btprofile.h"	// Device attributes

#ifdef USE_FAT_FS
	#include "FatFs/FatFsIo.h"
#else
	#include "MDD File System/FSIO.h"
#endif
#include "Ftl/Ftl.h"
#include "AsyncDiskIO.h"
#include "USB/USB_CDC_MSD.h"

/* 
Development road map and to do list:

-Make 'Add calibration values to NVM' command
-Use NVM calibration to convert analogue sensors
-Create thermistor to degrees conversion
-Add light sensor to data streams

-Bluetooth HCI revisions to use scripts
-SDP module completion and testing
-Optimise transfer speeds
-Add time synchronisation
-Add master connection to Bluetooth HR monitor
-Add obex protocol and service

-Nand flash driver for multiple devices on alternative chip selects
-Adaptive disk size for alternate nand chips
-Test 1GB spansion chipset performance

-Add fifo sampling mode for accel/gyro sensors
-Allow mixed mode sampling behaviour
-Add logger code

-Create multi-stream module for stream/log co-existence
-Implement 'sector pool' logger method for asynchronous disk
-Make reliable dual-mode streamer+logger device

*/

// Types
typedef enum {
STOP_HIBERNATE,
STOP_LOW_BATT,
STOP_BLUETOOTH,
STOP_SENSORS,
} stopReson_t;

// Prototypes
void SystemTasks(void);
void UsbTasks(void);
void BluetoothTasks(void);
void RunStreamer(void);
void LowFreqTasks(void);
void SleepUntilWoken(void);
void CheckForLowBatt(void);
void StopCode(unsigned short reason);
void RunTestSequence(unsigned short reason);

// Globals
unsigned long pressure = 0;
signed short temperature = 0;
unsigned short battmv = 0;

void AccelInactivityMonitor(void)
{
	// Inactivity
	if ((status.accelIntPending == TRUE) && AccelReadIntSource())
	{
		status.accelIntPending = 0;
	}
	else 
	{	
		status.inactivity++;
	}
}
// Accel ints
void __attribute__((interrupt, shadow, auto_psv)) ACCEL_INT1_VECTOR(void)
{
	status.accelIntPending = TRUE;
	status.inactivity = 0;
	ACCEL_INT1_IF = 0;	
}

// Main routine
int main(void)
{
	// Entry point
	SystemInit();

while(1)
{
LED_SET(LED_GREEN);
SysTimeDelayMs(1000);

LED_SET(LED_RED);
SysTimeDelayMs(1000);
}

	// Set startup colour
	LED_SET(LED_RED);	

	// Memory and file system
	NandInitialize(); 		// Nand initialise
	NandVerifyDeviceId();	// Check nand device type
	FtlStartup();			// Initialise ftl
	FSInit();				// Mount disk

	// Get valid ADC readings for all analogue sensors
	UpdateAdcFull();		// We must have a valid battery level

	// Query I2C bus 1 and 2 sensors and eeprom
	AltVerifyDeviceId();	// Altimeter
	MultiVerifyDeviceId();	// Accel, Gyro, Mag
	MultiStandby();			// Initial state is all off
	
	// Check for unresponsive devices
	if(	((multiPresent.all != 0x7) 	|| 	// Test all sensors 
		(altPresent != TRUE)		||	// Test altimeter
		(nandPresent != TRUE))		&&	// Test nand
		(status.sleepMode != 5)) 		// Continue if in test mode 5 (self test)
	{
		#ifndef __DEBUG
		StopCode(STOP_SENSORS); 
		#endif
	}

	// Sampler and ints fifo initialise 	
	SamplerInitOff(); 			// Turn off everything

	// Read non-volatile settings
	#ifdef __DEBUG
		SettingsInitialize(1);	// Reset settings to defaults in debug mode
	#else
		SettingsInitialize(0);	// Read from nvm
	#endif

	// Check battery - wait for usb attach if discharged
	if (adcResult.batt < BATT_CHARGE_MIN_SAFE)
		StopCode(STOP_LOW_BATT);// Can't continue on empty battery

	// Connect if not hibernating
	LED_SET(LED_YELLOW);				// App colour
	if (status.sleepMode != 4) 			// Sleep mode 4 = hibernate now, skip over
		RunStreamer();					// otherwise run main application

	// Turn off all peripherals and radio
	BtAppStop();
	MultiStandby();

	// Hibernate mode set? Stop here
	if((status.sleepMode == 2) || (status.sleepMode == 3) || (status.sleepMode == 4) )
		StopCode(STOP_HIBERNATE);// Hibernate until usb change or wake event e.g orientation change

	// Woke from hibernation
	if((USB_BUS_SENSE)&&(status.sleepMode == 4)) 		
	{								// If in hibernate mode and usb woken
		settings.sleepMode =  1; 	// Set normally discoverable 
		SettingsSave();				// and save new settings
	}

	// Can occur if user sets this mode by accident
	if(status.sleepMode == 5)	
	{
		settings.sleepMode =  4; 	// Normal hibernate
		SettingsSave();				// Save new settings
	}

	// Reset device
	Reset();                
	return 0;
}

void RunStreamer(void)
{
	unsigned long ticks_lf = 0;

	// USB mode? Sampled once to avoid mismatch
	status.usbAtStart = USB_BUS_SENSE;

	// Set clock speed here	
	if (settings.highPerformance){CLOCK_PLL();}

	// Turn on rtc
    SysTimeInit();			// Setup timer to run

	// We have now tested the hardware - done
	if (status.sleepMode == 5)
		{StopCode(STOP_HIBERNATE);}

	// Reset inactivity monitoring
	status.inactivity = 0;
	status.runMain = TRUE;
	settings.runBt = FALSE;

	while(	(status.runMain == TRUE) &&
			(status.inactivity < settings.inactivityThreshold)) 
	{
		unsigned long now;

		// System tasks
		SystemTasks();

		// Bluetooth stack 
		BluetoothTasks();

		// Usb state handler
		UsbTasks();
	
		now = SysTimeTicks();
		if((now - ticks_lf) > LOW_FREQ_TASKS_TICKS)
		{
			ticks_lf = now; 
			LowFreqTasks();

			// Block inactivity timeout for sleep modes other than 2 and 3
			if ((status.sleepMode != 2)&&(status.sleepMode != 3)) 
				status.inactivity = 0;
		}

		if(status.streaming)
		{
			// Trys to send outgoing data from data buffer
			StreamerTasks();
		}

		// Try and go to lower power if possible
		if(status.ledOverride == -1) LED_SET(LED_OFF);	
		// 0 == no sleep / usb, 1 == discoverable
		if(status.sleepMode > 0)	
		{
			HciPhySafeSleep();
		}
		#ifdef __DEBUG
			LED_B = LED_TURN_ON;
		#endif
	} // Stopped

	// Wait for current activity to stop before returning
	status.streaming = FALSE;
	SamplerInitOff();
	USBCDCWait();			// Wait for serial output pending
	BluetoothSerialWait();  // Wait for pointers to clear

	return;
}

void SystemTasks(void)
{
	// System clock setting
	if (!OSC_PLL_ON() && settings.highPerformance && settings.doStream)
	{
		// Need clock switch first
		HciPhyPause();
		CLOCK_PLL();
		HciPhyAdaptClock(); 
		HciPhyPlay();
	}
	else if (OSC_PLL_ON() && !USB_BUS_SENSE)
	{
		// Wait until safe
		HciPhyPause();
		CLOCK_INTOSC();
		HciPhyAdaptClock(); 
		HciPhyPlay();
	}
	
	// Streamer status
	if(!status.streaming && settings.doStream && ((status.btState == STATE_ACTIVE)||(status.usbState == STATE_ACTIVE)))
	{
		SamplerInitOn(); // Start sampler interrupt and setup sensors
		status.streaming = TRUE;
	}
	else if (status.streaming && (settings.doStream == FALSE || (status.usbState != STATE_ACTIVE && status.btState != STATE_ACTIVE)))
	{
		SamplerInitOff();// Turn off sensors and interrupt 
		status.streaming = FALSE;
	}

	// Sleep mode latch
	if(status.sleepMode != settings.sleepMode)
	{
		// Reasons not to adopt the set sleep mode
		if(	(status.usbState != STATE_OFF) ||
			(settings.highPerformance == TRUE))
		{
			// Ignore sleep mode
		}
		else
		{
			// Adopt sleep mode
			status.sleepMode = settings.sleepMode;
		}
	}	
}

void UsbTasks(void)
{
	// Latch state if off or unknown
	if((status.usbState == STATE_UNKNOWN)||(status.usbState == STATE_OFF))
	{
		if(USB_BUS_SENSE)
		{
			status.sleepMode = 0; // On connect, prevent sleep
			status.usbState	= TRUE;	
		}
	}

	// Check for detach event
	if((status.usbState >= STATE_ON)&&(USB_BUS_SENSE != TRUE))
	{
		if(status.usbState >= STATE_ENABLED)
		{
			#if defined(USB_INTERRUPT)
		    USBDeviceDetach();
			#endif
		}
		status.sleepMode = settings.sleepMode; // On disconnect
		status.usbState = STATE_OFF;
	}

	if(status.usbAtStart != USB_BUS_SENSE)
	{
		if(status.usbAtStart == FALSE)
			status.runMain = FALSE; 			// On connect but previously not connected, Exit main loop and reset
		status.usbAtStart = USB_BUS_SENSE; 
	}

	// If on but inactive, start stack
	if(status.usbState == STATE_ON)
	{
		if(!OSC_PLL_ON())
		{
			HciPhyPause();
			CLOCK_PLL();
			DelayMs(5);
			HciPhyAdaptClock(); 
			HciPhyPlay();
		}
		USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
		#ifdef USB_INTERRUPT
		USBDeviceAttach();
		#endif
		status.usbState = STATE_ENABLED;
	}
	
	// If enabled, run stack tasks
	if(status.usbState >= STATE_ENABLED)
	{
        #ifndef USB_INTERRUPT
        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
        #endif
        USBProcessIO();	
		AsyncDiskIOTasks();

		if ((USBGetDeviceState() >= CONFIGURED_STATE) && (USBIsDeviceSuspended() == FALSE))
		{
            const char *line = _user_gets();
			status.usbState = STATE_ACTIVE;
            if (line != NULL)
            {	
				// Command colour
				LED_SET(LED_GREEN);
// Debug
if(settings.runBt == FALSE)
	settings.runBt = TRUE;

				// If streaming, first turn off stream and sensors
				if(status.streaming)
				{
					status.streaming = FALSE;
					SamplerInitOff();
					USBCDCWait();
				}
				// Redirect write handler to bluetooth
				stdout->_flag |= _IOBINARY;
				writeHandler = usb_write;
                SettingsCommand(line);
if(0){
DateTime_t time;
EpochTime_t now;
uint16_t ticks;
now = SysTimeEpochTicks(&ticks);
SysTimeRead(&time);
printf("\r\nEpoch: %010lu:%05u",now,ticks);
printf("\r\nTime: %02u/%02u/%02u,%02u:%02u:%02u",time.year,time.month,time.day,time.hours,time.minutes,time.seconds);
}
            }
		}	
	}
}

void BluetoothTasks(void)
{
	// Early out if off
	if(settings.runBt == FALSE || status.btState == STATE_ERROR) return;

	// If uninited or off when supposed to be on
	if((status.btState == STATE_UNKNOWN)||(status.btState == STATE_OFF && settings.runBt == TRUE))
	{
		// Init bluetooth
		if(!BtAppStart())
		{
			status.btState = STATE_ERROR;
		}
		else
		{
			status.btState = STATE_ENABLED;
		}
	}

	// Check bt is running
	if(status.btState < STATE_ON) return;

	// If on but inactive, start stack
	if(status.btState >= STATE_ON)
	{
		// Connected?
		if((bluetoothState >= BT_CONNECTED)&&(status.btState != STATE_ACTIVE))
		{
			status.btState = STATE_ACTIVE;
		}
		else if ((bluetoothState < BT_CONNECTED) && (status.btState == STATE_ACTIVE))
		{
			status.ledOverride = -1;
			status.btState = STATE_ON;
		}

		// Check for serial inputs
        if (bluetoothState == BT_CONNECTED)
        {
            const char *line;
			// Stack Tasks / data transfer
			BluetoothSerialIO();
			// Check for command
			line = Bluetooth_gets();
			// Handle command
            if (line != NULL)
            {
				// Command colour
				LED_SET(LED_GREEN);
				// If streaming, first turn off stream and sensors
				if(status.streaming)
				{
					status.streaming = FALSE;
					SamplerInitOff();
					BluetoothSerialWait(); // Wait for pointers to clear
				}
				// Redirect write handler to bluetooth
				stdout->_flag |= _IOBINARY;
				writeHandler = Bluetooth_write;
                SettingsCommand(line);
            }
        }
	}
}


// Led status while attached - IPL0 (main scope)
void LowFreqTasks(void)
{
	uint16_t inactive;
	// LED tasks
	if(status.ledOverride>=0)
	{
		LED_SET(status.ledOverride);
	}
	else if ((status.streaming)&&(settings.sampleRate))
	{
		// Nothing - blue LED set from T1 sample timer
	}
	else
	{
		switch (bluetoothState){
			case (BT_OFF):					
			case (BT_STARTING):				
			case (BT_FAIL):
			case (BT_CONFIGURED):			{LED_SET(LED_YELLOW);break;}
			case (BT_CONNECTED):				
			case (BT_REQUEST):				{LED_SET(LED_BLUE);break;}
		}
	}

	// File system flush
	inactive = FtlIncrementInactivity();
	if (inactive > FTL_INACTIVE_FLUSH_SECONDS)
	{
		FtlFlush(1);// Save ftl state
	}	

	// Barometer/Altimeter temp pressure alternate sampling
	static unsigned char toggle = 0;
	if (bluetoothState == BT_CONNECTED) // Otherwise we don't need the altimeter data
	{
		if (++toggle&0b1)
		{
			if(BMP085_read_ut())
			{
				BMP085_Initiate_up_conversion();
				status.temperature = BMP085_calc_temperature();
			}
		}
		else                                        
		{
			if (BMP085_read_up())					
			{											
				BMP085_Initiate_ut_conversion();	
				status.pressure = BMP085_calc_pressure();
				status.newXdata = TRUE; // Indicate we have new extended data
				#ifndef DISABLE_LE_MODES
				if(status.streaming)
				{
					metaDataOut.attCfg = 0;
					metaDataOut.attLen = 8;
					metaDataOut.attData = (void*)&status.pressure; // Start of struct	
					metaDataOut.dataFlag = TRUE; // Will send notification/indication as configured	
				}
				#endif	
			}
		}
	}

	// Sample battery level
	UpdateAdc();
	status.battmv = AdcToMillivolt(adcResult.batt);

	// Connection request timeout
	{
		static unsigned short requestTimeout = 0;
		if((bluetoothState == BT_REQUEST)&&(requestTimeout >= 20))bluetoothState = BT_CONFIGURED;
		else if (bluetoothState == BT_REQUEST)requestTimeout++;
		else requestTimeout = 0;
	}
}

// Stop code returns only if set to hibernate (errors reset)
void StopCode(unsigned short reason)
{
	// Never stay stopped forever if usb attached (~40seconds)
	unsigned short usbExitTimer = 10;

	// Lowest power
	BtAppStop();		// Power off bluetooth
	SamplerInitOff(); 	// Will leave accel on if settings.accelInts is non zero
	SysTimeClearAllCB();
	CLOCK_INTOSC();		// Incase at pll clock

	// Latch current usb sense
	status.usbAtStart = USB_BUS_SENSE;
	
	// First check if we are supposed to run test mode (sleep mode 5)
	if(status.sleepMode == 5)
	{
		// Clear the test mode
		settings.sleepMode =  4; 	// Normal hibernate
		SettingsSave();				// Save new settings
		// Some indicator of running test
		LED_SET(LED_WHITE);
		// Run test, pass reason on
		RunTestSequence(reason);
		// If alls well, return to bootloader if still connected
		if((reason == STOP_HIBERNATE)&&(USB_BUS_SENSE)) 	
				Reset();// Still on bootloader probably, just exit
		status.usbAtStart = USB_BUS_SENSE;
		// Otherwise - indicate error (user should detach within the 40s timeout)	
	}

	// Hibernate mode
	if(reason == STOP_HIBERNATE) 	
	{
		LED_SET(LED_OFF);
		if((status.sleepMode == 2)||(status.sleepMode == 3))
		{
			DelayMs(1000);			// The accelerometer will interrupt just after startup regardless
			AccelReadIntSource();	// So clear it here, then set activity
		}
		status.inactivity = 1;		// This will be cleared by any accel interrupts
	}

	while (status.usbAtStart == USB_BUS_SENSE)			
	{				// Wait for USB to change state to exit (hibernate exits automatically)		
		if(USB_BUS_SENSE && --usbExitTimer<=0)
			break;					// After ~40 secs we exit if USB attached

		UpdateAdc(); 	// Update battery level value and check
		if(adcResult.batt < BATT_CHARGE_MIN_SAFE)
			reason = STOP_LOW_BATT; 

		// Led flashing switch
		switch (reason) {
			case STOP_HIBERNATE : {// No error, just hibernate (could wake on activity)
				LED_SET(LED_GREEN);
				DelayMs(1);
				LED_SET(LED_OFF);
				if(!status.inactivity)	// Accel movement in hibernate
					return;
				break;
			}
			case STOP_LOW_BATT : {// Battery low
				LED_SET(LED_RED);
				DelayMs(1);
				LED_SET(LED_OFF);
				break;
			}
			case STOP_BLUETOOTH : {// Bluetooth error
				LED_SET(LED_MAGENTA);
				DelayMs(1);
				LED_SET(LED_OFF);
				break;
			}
			case STOP_SENSORS : {// Peripheral error
				LED_SET(LED_CYAN);
				DelayMs(1);
				LED_SET(LED_OFF);
				break;
			}
			default : break;
		}	
//RtcSwwdtReset();
		SystemPwrSave(WAKE_ON_WDT|LOWER_PWR_SLOWER_WAKE); // 4s cycle depending on config words
	}
	// Dont return for errors
	if(reason == STOP_HIBERNATE)
	{	
		if((USB_BUS_SENSE) && (status.sleepMode == 4))// Stop hibernating, USB attached
		{
			settings.sleepMode =  1; 	// Normal mode
			SettingsSave();				// Save new settings
		}
		return;
	}
	Reset();
}

// SELF TEST FUNCTIONALITY - EEPROM ADDRESS AND LEN SET IN HW PROFILE
static unsigned short selfTestOffset = 0;
static void NvmSelfTestWrite (const void *buffer, unsigned int len)
{
	unsigned char* dest = streamerOutBuffer + selfTestOffset;
	unsigned short remaining = SELF_TEST_SIZE - selfTestOffset;
	if(remaining < len) return;
	memcpy(dest,(void*)buffer,len);
	selfTestOffset += len;
	return;
}
static void NvmSelfTestWriteStart(void)
{
	memset(streamerOutBuffer,0,SELF_TEST_SIZE+1);	// For null
	stdout->_flag |= _IOBINARY;						// Set binary flag
	writeHandler = NvmSelfTestWrite; 				// Divert printf
	selfTestOffset = 0;
}
static void NvmSelfTestWriteEnd(void)
{
	WriteNvm(SELF_TEST_RESULTS, streamerOutBuffer, selfTestOffset+1); // For null
}

void RunTestSequence(unsigned short reason)
{
	// Start file entry
	NvmSelfTestWriteStart();

	// Self test format version
printf("\fSELF TEST FORMAT: 1.0\r\n");

printf("SECTION: FIRMWARE, Start\r\n");
	// FW and HW revision
	printf("HW: %s, FW: %s, TARGET:CC%u\r\n",
	HARDWARE_VERSION,FIRMWARE_VERSION,(unsigned short)(BT_CHIPSET_TARGET));
printf("SECTION: FIRMWARE, End\r\n");


	// Errors first
printf("SECTION: ERRORS, Start\r\n");
	if(reason == STOP_LOW_BATT) 		
	{
		// Has not assessed other HW
		printf("ERROR: BATT, Too low to continue\r\n");
		NvmSelfTestWriteEnd();
		return;
	} 
	if(reason == STOP_BLUETOOTH)
	{ 	
		printf("ERRORS: BLUETOOTH, During startup\r\n");
	}
	#ifdef BT_CHIPSET_TARGET
	if(chipsetId != BT_CHIPSET_TARGET)
		printf("ERRORS: BLUETOOTH, Chipset mismatch\r\n");
	#endif

	// Sensors
	if(!multiPresent.accel)	printf("ERRORS: ACCEL, Device not detected\r\n");
	if(!multiPresent.gyro)	printf("ERRORS: GYRO, Device not detected\r\n");
	if(!multiPresent.mag)	printf("ERRORS: MAG, Device not detected\r\n");
	if(!altPresent)			printf("ERRORS: ALT, Device not detected\r\n");
printf("SECTION: ERRORS, End\r\n");

printf("SECTION: PERIPHERALS, Start\r\n");
	printf("BLUETOOTH CHIPSET: CC%u\r\n",chipsetId);
	const unsigned char* mac = (const unsigned char*)GetBTMAC(); // Ram copy if module inited
	printf("BLUETOOTH MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]);

	// Get initial metadata point
	battmv = AdcToMillivolt(adcResult.batt)<<1;
	DelayMs(30);
	BMP085_Initiate_up_conversion();
	DelayMs(30);
	if (BMP085_read_up())status.pressure = BMP085_calc_pressure();	
	BMP085_Initiate_ut_conversion();	
	DelayMs(30);
	if(BMP085_read_ut())status.temperature = BMP085_calc_temperature();
	printf("BATTTERY: %u mV\r\n",battmv);
	printf("PRESSURE: %lu Pa\r\n",status.pressure);
	printf("TEMPERATURE: %d.%u %cC\r\n",(status.temperature/10),((unsigned short)status.temperature%10),0xF8);

	// Get initial multi sensor point
	int i,j;
	__attribute__((aligned(2)))sensor_t stats[6]; // Current, last, max, min, mean, deviation
	signed long accumulators[9]; // mean
	memset(stats,0,sizeof(sensor_t)*6);
	memset(accumulators,0,sizeof(signed long)*9);
	// Initialise our mins and maxs
	for(j=0;j<9;j++)
	{
		// Max [2] 
		stats[2].values[j] = 0x8000;
		// Min [3]
		stats[3].values[j] = 0x7FFF;
	}

	settings.accelOn				= 1;
	settings.accelRate				= 200;
	settings.accelRange				= 8;
	settings.gyroOn					= 1;
	settings.gyroRate				= 200;
	settings.gyroRange				= 2000;
	settings.magOn					= 1;
	settings.magRate				= 10;
	settings.accelInts				= 0;
	MultiStartup(&settings);		// Startup

	for(i=0;i<64;i++) // Dummy 64 samples
	{
		DelayMs(10);
		MultiSingleSample(&stats[0]);
	}

	for(i=0;i<64;i++) // Dummy 64 samples
	{
		DelayMs(10);
		MultiSingleSample(&stats[0]);

		for(j=0;j<9;j++)
		{
			// Max [2] 
			if(stats[0].values[j] > stats[2].values[j])
				stats[2].values[j] = stats[0].values[j];
			// Min [3] 
			if(stats[0].values[j] < stats[3].values[j])
				stats[3].values[j] = stats[0].values[j];

			// Sum noise sum to [2]
			if(stats[0].values[j] > stats[1].values[j])
				stats[5].values[j] += (stats[0].values[j] - stats[1].values[j]);
			else 
				stats[5].values[j] += (stats[1].values[j] - stats[0].values[j]);

			// Accumulators separate
			accumulators[j] += stats[0].values[j]; 
		}
		// Store this sample ofr deltas
		memcpy(&stats[1],&stats[0],sizeof(sensor_t));
	}// for 64

	// Done
	MultiStandby();
	// Find mean, (2^6) samples in accumulators, use shift to get mean in [4]
	for(j=0;j<9;j++)
	{
		// Mean values
		stats[4].values[j] = accumulators[j] >> 6;
	}


	// Results
	printf("ACCEL LAST: %d, %d, %d\r\n",	stats[1].accel.x, stats[1].accel.y, stats[1].accel.z);
	printf("ACCEL MAX: %d, %d, %d\r\n",		stats[2].accel.x, stats[2].accel.y, stats[2].accel.z);
	printf("ACCEL MIN: %d, %d, %d\r\n",		stats[3].accel.x, stats[3].accel.y, stats[3].accel.z);
	printf("ACCEL MEAN: %d, %d, %d\r\n",	stats[4].accel.x, stats[4].accel.y, stats[4].accel.z);
	printf("ACCEL DEV: %u, %u, %u\r\n",		stats[5].accel.x, stats[5].accel.y, stats[5].accel.z);

	printf("GYRO LAST: %d, %d, %d\r\n",		stats[1].gyro.x, stats[1].gyro.y, stats[1].gyro.z);
	printf("GYRO MAX: %d, %d, %d\r\n",		stats[2].gyro.x, stats[2].gyro.y, stats[2].gyro.z);
	printf("GYRO MIN: %d, %d, %d\r\n",		stats[3].gyro.x, stats[3].gyro.y, stats[3].gyro.z);
	printf("GYRO MEAN: %d, %d, %d\r\n",		stats[4].gyro.x, stats[4].gyro.y, stats[4].gyro.z);
	printf("GYRO DEV: %u, %u, %u\r\n",		stats[5].gyro.x, stats[5].gyro.y, stats[5].gyro.z);

	printf("MAG LAST: %d, %d, %d\r\n",		stats[1].mag.x, stats[1].mag.y, stats[1].mag.z);
	printf("MAG MAX: %d, %d, %d\r\n",		stats[2].mag.x, stats[2].mag.y, stats[2].mag.z);
	printf("MAG MIN: %d, %d, %d\r\n",		stats[3].mag.x, stats[3].mag.y, stats[3].mag.z);
	printf("MAG MEAN: %d, %d, %d\r\n",		stats[4].mag.x, stats[4].mag.y, stats[4].mag.z);
	printf("MAG DEV: %u, %u, %u\r\n",		stats[5].mag.x, stats[5].mag.y, stats[5].mag.z);

printf("SECTION: PERIPHERALS, End\r\n");

	// Done
	NvmSelfTestWriteEnd();

	return;
}
