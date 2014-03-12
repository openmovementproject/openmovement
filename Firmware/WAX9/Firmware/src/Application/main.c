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


// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Peripherals/bmp085.h"
#include "Analog.h"
#include "Utils/Util.h"
#include "Settings.h"
#include "Peripherals/Timer1.h"
#include "Peripherals/Eeprom.h"
#include "Peripherals/MultiSensor.h"
#include "SamplerStreamer.h"
#include "bt serial.h"
#include "bt config.h"
#include "Profile.h"

#ifdef BT_USB_DEBUG
	#include "USB\USB_CDC_MSD.h"
#endif

// Types
typedef enum {
STOP_HIBERNATE,
STOP_LOW_BATT,
STOP_BLUETOOTH,
STOP_SENSORS,
STOP_EEPROM
} stopReson_t;

// Prototypes
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

void DebugReset(SwResetReason_t reason)
{
	LED_SET(LED_MAGENTA);
	DelayMs(3000);
	switch (reason){
		case (OTHER_INT):LED_SET(LED_OFF);break;
		case (STACK_ERR):LED_SET(LED_RED);break;
		case (ADDR_ERROR):LED_SET(LED_MAGENTA);break;
		case (MATH_ERR):LED_SET(LED_BLUE);break;
		case (BT_ERR):LED_SET(LED_CYAN);break;
		case (I2C_ERR):LED_SET(LED_GREEN);break;
		default:LED_SET(LED_YELLOW);break;
	}
	DelayMs(3000);
	Reset();
}
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
	#ifndef __DEBUG
    DebugReset(INTCON1val);
	#endif
}

// One Sec Handler - swwdt,bt eHCILL
#define OneSecTrigger()	IFS0bits.OC1IF=1;
void InitOneSecHandler(void)
{
	IPC0bits.OC1IP = ONE_SECOND_TRIGGER_INT_PRIORITY;
	IFS0bits.OC1IF = 0;
	IEC0bits.OC1IE = 1;
}
void __attribute__((interrupt, shadow, auto_psv)) _OC1Interrupt(void)
{
	// Flag
	IFS0bits.OC1IF = 0;
	// Inactivity
	if ((status.accelIntPending == TRUE)&&AccelReadIntSource()){
		status.accelIntPending=0;}// Same priority as other stats function
	else {	
		status.inactivity++;}
}

// TMR1 - IPL7 for timer accuracy
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
	unsigned char oneSec = TimerTasks();

	// Trigger timer
	SamplerTrigger(); // Sample at IPL4

    if(oneSec) // If one second has elapsed?
	{	
		// One sec handler at IPL4
		OneSecTrigger(); 
		// Software WDT at IPL7
		SwwdtIncrement(); 
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
	Nop();
	RCON = 0;

    // Initialization
	InitIO();			// I/O pins
	myI2Cclear();		// Once only
	LED_SET(LED_YELLOW);// Startup colour

	// Set clock
	CLOCK_INTOSC();     // 8 MHz

	// Get valid ADC reading
	GetBatt();				// Ensure we have a valid battery level

	// Setup external eeprom (required for settings load)
	#ifndef HARDWAREPROFILE_CWA31_H
		I2C_eeprom_present();	// Required before retrieving data from eeprom
	#else
		unsigned char eepromPresent = TRUE;
	#endif
	if(	eepromPresent != TRUE) 	// Cant test bluetooth or continue without eeprom
		StopCode(STOP_EEPROM);
	// Read settings
	#ifdef __DEBUG
		SettingsInitialize(1);	// Reset settings to defaults in debug mode
	#else
		SettingsInitialize(0);	// Read from eeprom, must be after eeprom init
	#endif

	// I2C sensors and eeprom
	AltVerifyDeviceId();	// Altimeter
	MultiVerifyDeviceId();	// Accel, Gyro, Mag
	MultiStandby();			// Initial state is all off
	
	// Check the devices responded
	#ifndef __DEBUG
		if(	((multiPresent.all != 0x7) 	|| 	// Test all sensors 
			(altPresent != TRUE)) 		&&	// Test altimeter
			(settings.sleepMode != 5)) 		// Continue if in test mode 5 (self test)
			StopCode(STOP_SENSORS); 
	#endif

	// Sampler and ints fifo initialise 	
	InitOneSecHandler();	// Using the sw triggered OC0 int vector
	SamplerInitOff(); 		// Sensors turned off,timer set to min

	// Check battery - wait here for usb attach if discharged
	if (battRaw < BATT_CHARGE_MIN_SAFE)
		StopCode(STOP_LOW_BATT);			// Can't continue on empty battery

	// Connect if not hibernating
	do {
		if (settings.sleepMode != 4) 		// Sleep mode 4 = hibernate now, skip over
			RunStreamer();					// otherwise run main application
	// USB exit but sleep mode is 0, returns to run mode
	}while(USB_BUS_SENSE && settings.sleepMode == 0); 

	// Turn off all peripherals and radio
	BTDeinit();
	MultiStandby();
	TimerInterruptOff(); 		// This disables the swwdt too!

	// Hibernate mode set? Stop here
	if((settings.sleepMode == 2) || (settings.sleepMode == 3) || (settings.sleepMode == 4) )
		StopCode(STOP_HIBERNATE);// Hibernate until usb change or wake event e.g orientation change

	// Woke from hibernation
	if((USB_BUS_SENSE)&&(settings.sleepMode == 4)) 		
	{								// If in hibernate mode and usb woken
		settings.sleepMode =  1; 	// Set normally discoverable 
		SettingsSave();				// and save new settings
	}

	// Can occur if user sets this mode by accident
	if(settings.sleepMode == 5)	
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
	unsigned long ticks = 0;

	// Set clock speed here	
	if (settings.highPerformance)	{CLOCK_PLL();}
	else 							{CLOCK_INTOSC();}    

	// Redirect write handler to bluetooth
	stdout->_flag |= _IOBINARY;
	writeHandler = Bluetooth_write;

	// Turn on bluetooth
    TimerStartup();						// Setup timer to run
    TimerInterruptOn(MINIMUM_T1_RATE);  // For sw wdt and bt tick

	// Hardware regs - not needed really because these are defaults
	U1PWRCbits.USBPWR = 0; 	// Incase usb module was not disabled 
	RCONbits.PMSLP = 0;		// Ensure we use lowest power sleep

// Debug mode with USB CDC port
#ifdef BT_USB_DEBUG
{
	unsigned char wait = TRUE;

	CLOCK_PLL();
	settings.sleepMode = 0; // Run with usb attached
	settings.accelRate = 25;

	USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware
	#ifdef USB_INTERRUPT
	USBDeviceAttach();
	#endif

	LED_SET(LED_RED);
	while((USBGetDeviceState() < CONFIGURED_STATE) || (wait))
	{
		// USB tasks
		#ifndef USB_INTERRUPT
	       USBDeviceTasks(); 
	    #endif
		USBProcessIO();

		// Check for input
       	if (USBGetDeviceState() >= CONFIGURED_STATE)
        {
			const char *line = _user_gets(); // USB
			if (line != NULL)
			{
				// CR recieved
				writeHandler = usb_write;
				printf("\fStarting...\r\n");
				USBCDCWait();
				writeHandler = Bluetooth_write;
				wait = FALSE;
			}
		}
	}
}
#endif	

	// Final check before starting radio
	// Set initial usb condition depending on sleep mode (5 is special test mode)
	if (settings.sleepMode == 0 || settings.sleepMode == 5)
		status.usbAtStart = USB_BUS_SENSE;
	else if (USB_BUS_SENSE)		return; // Exit
	else 						status.usbAtStart = 0;

	// Init bluetooth
	if(!BluetoothInit(NULL, settings.deviceId, 0))
		{StopCode(STOP_BLUETOOTH);}
	
	// We have now tested the hardware - done
	if (settings.sleepMode == 5)
		{StopCode(STOP_HIBERNATE);}

	// Reset inactivity monitoring
	status.inactivity = 0;

	while(	(bluetoothState > BT_FAIL) && 
			(USB_BUS_SENSE==status.usbAtStart) &&
			(status.inactivity < settings.inactivityThreshold)) 
	{
		if((timerTicks.Val - ticks) > LOW_FREQ_TASKS_TICKS)
		{
			ticks = timerTicks.Val; 
			LowFreqTasks();

			// Block inactivity timeout for sleep modes other than 2 and 3
			if ((settings.sleepMode != 2)&&(settings.sleepMode != 3)) 
				status.inactivity = 0;
		}

		// Check input buffer for commands
        if (bluetoothState == BT_CONNECTED)
        {
            const char *line = Bluetooth_gets();
            if (line != NULL)
            {
				// If streaming, first turn off stream and sensors
				if(status.streaming)
				{
					status.streaming = FALSE;
					SamplerInitOff();
					BluetoothSerialWait(); // Wait for pointers to clear
				}
                SettingsCommand(line);
            }
			// Trys to send outgoing data from data buffer
			StreamerTasks();
        }
		else if (status.streaming)// Not connected, if streaming then stop
		{
			status.streaming = FALSE;
			status.ledOverride = -1;
			SamplerInitOff();
			if (OSCCONbits.COSC ==0b001)// Need to switch down clock too
			{
				// Wait until safe
				HciPhyPause();
				CLOCK_INTOSC();
				HciPhyAdaptClock(); 
				HciPhyPlay();
			}	
		}

		// Stack Tasks / data transfer
		BluetoothSerialIO();

		// Try and go to lower power if possible
		if(status.ledOverride == -1) LED_SET(LED_OFF);	
	
		if(settings.sleepMode > 0)	// 0 == NO SLEEP AT ALL (very power hungry), 1 is default (discoverable always)
		{
			HciPhySafeSleep();
		}
		
		#ifdef BT_USB_DEBUG
	        #ifndef USB_INTERRUPT
	        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	        #endif
	        USBProcessIO();	
		#endif
	} // Stopped

	// Wait for current activity to stop before returning
	status.streaming = FALSE;
	SamplerInitOff();
	BluetoothSerialWait(); // Wait for pointers to clear

	return;
}

// Led status while attached - IPL0 (main scope)
void LowFreqTasks(void)
{
	// LED tasks
	if(status.ledOverride>=0)
	{
		if (bluetoothState == BT_CONNECTED)	{LED_SET(status.ledOverride);}
		else status.ledOverride = -1;
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
			case (BT_INITIALISED):			{LED_SET(LED_YELLOW);break;}
			case (BT_CONNECTED):				
			case (BT_CONNECTION_REQUEST):	{LED_SET(LED_BLUE);break;}
		}
	}


#ifndef HARDWAREPROFILE_CWA31_H
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
				#ifdef ENABLE_LE_MODE
				if(status.streaming)	
					metaDataOut.dataFlag = TRUE; // Will send notification/indication as configured	
				#endif	
			}
		}
	}
#endif

	// Sample battery level
	GetBatt();
	status.battmv = AdcBattToMillivolt(battRaw);

	// Connection request timeout
	{
		static unsigned short requestTimeout = 0;
		if((bluetoothState == BT_CONNECTION_REQUEST)&&(requestTimeout >= 20))bluetoothState = BT_INITIALISED;
		else if (bluetoothState == BT_CONNECTION_REQUEST)requestTimeout++;
		else requestTimeout = 0;
	}

	// SW WDT
	SwwdtReset();
}

// Stop code returns only if set to hibernate (errors reset)
void StopCode(unsigned short reason)
{
	// Never stay stopped forever if usb attached (~40seconds)
	unsigned short usbExitTimer = 10;

	// Lowest power
	BTDeinit();		// Power off bluetooth
	SamplerInitOff(); 	// Will leave accel on if settings.accelInts is non zero
	TimerInterruptOff();// Don't wake on timer
	CLOCK_INTOSC();		// Incase at pll clock

	// Latch current usb sense
	status.usbAtStart = USB_BUS_SENSE;
	
	// First check if we are supposed to run test mode (sleep mode 5)
	if(settings.sleepMode == 5)
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
		if((settings.sleepMode == 2)||(settings.sleepMode == 3))
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

		GetBatt(); 	// Update battery level value and check
		if(battRaw < BATT_CHARGE_MIN_SAFE)
			reason = STOP_LOW_BATT; 

		// Led flashing switch
		switch (reason) {
			case STOP_HIBERNATE : {// No error, just hibernate (could wake on activity)
				LED_SET(LED_GREEN);
				DelayMs(10);
				LED_SET(LED_OFF);
				if(!status.inactivity)	// Accel movement in hibernate
					return;
				break;
			}
			case STOP_LOW_BATT : {// Battery low
				LED_SET(LED_RED);
				DelayMs(10);
				LED_SET(LED_OFF);
				break;
			}
			case STOP_BLUETOOTH : {// Bluetooth error
				LED_SET(LED_MAGENTA);
				DelayMs(10);
				LED_SET(LED_OFF);
				break;
			}
			case STOP_EEPROM :
			case STOP_SENSORS : {// Peripheral error
				LED_SET(LED_CYAN);
				DelayMs(10);
				LED_SET(LED_OFF);
				break;
			}
			default : break;
		}	
		SwwdtReset();
		SystemPwrSave(WAKE_ON_WDT|LOWER_PWR_SLOWER_WAKE); // 4s cycle depending on config words
	}
	// Dont return for errors
	if(reason == STOP_HIBERNATE)
	{	
		if((USB_BUS_SENSE) && (settings.sleepMode == 4))// Stop hibernating, USB attached
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
	// Check we can write eeprom
 	if(reason == STOP_EEPROM) return;	

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
	battmv = AdcBattToMillivolt(battRaw);
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
