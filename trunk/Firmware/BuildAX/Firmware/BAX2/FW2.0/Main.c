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

// BAX 1.0 sensor firmware
// Karim Ladha, 2013-2014

// Include
#include <stdlib.h>
#include <string.h>
#include <p18f26k20.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Settings.h"
#include "Analog.h"
#include "Peripherals/Si443x.h"
#include "Utils.h"
#include "aes.h"

// Debug setting
#ifndef DEBUG_ON
	#define DEBUG_ON
#endif
#include "Debug.h"

// Config
#pragma config IESO = OFF, FOSC = INTIO67, FCMEN = OFF, BOREN = OFF, BORV =	18, PWRT = ON
#pragma config WDTPS = WDT_SETTING, WDTEN = OFF /*32*4=128ms timeout*/
#pragma config CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON, HFOFST = ON, DEBUG = OFF
#pragma config STVREN = ON, XINST = OFF, LVP = OFF

// Bootloader interrupt remap 
//add code offset = 0x400 for boot
#define PROG_START 0x400
// First method...
//#define BOOT_START 0x000
//static void  bootmirror(void) @ BOOT_START
// Second method...
//add -L-Pstartup=0000h to linker options
static void __section("startup") bootmirror(void)
{
	asm("GOTO " ___mkstr(PROG_START)); 		// 0+2
	asm("NOP");								// 4
	asm("NOP");								// 6	
	asm("GOTO " ___mkstr(PROG_START+0x8));	// 8+A
	asm("NOP");								// C
	asm("NOP");								// E
	asm("NOP");								// 10
	asm("NOP");								// 12
	asm("NOP");								// 14
	asm("NOP");								// 16
	asm("GOTO " ___mkstr(PROG_START+0x18));	// 18+1A
}

// Globals
volatile unsigned char dummy;
settings_t settings;
static unsigned short pkt_counter = 0;
#ifdef ENCRYPT
// Encryption key created from main key
static unsigned char enckey[N_BLOCK];
#endif
#ifdef __DEBUG
volatile unsigned short CYCLES;
#endif

// Prototypes
unsigned char TxSensorData(void);
unsigned char TxLinkPacket(void);
void RunSensor(void);
void RunBatteryEmptySequence(void);
// Private
void EncryptPacket(packet_t* packet);
void WriteRandom(void* dest, unsigned char num);
void DebugSensors(void);
void SleepDebug(void);
void RadioRxDebug(void);
void RadioTxDebug(void);

void interrupt HighPriorityVector(void)
{
	if(Si44_IRQ_FLAG && INTCONbits.INT0IE)
	{
		Si44RadioIrqHandler();
	}
	return;
}

void interrupt LowPriorityVector(void)
{
	return;
}



void main(void)
{
	// Initialisation
	unsigned char settingsChanged = FALSE;
	CLOCK_INTOSC16();
	InitIO();
	InitAnalog();
	SWITCH_ON();
	COMM_INIT();

	// Starting
	DBG_INFO("\fStartup.");

	// Setup radio to low power
	Si44RadioInit();

	// Clear sensor globals
	memset((void*)&sensorVals, 0, sizeof(sensorVals_t));
	pkt_counter = 0;

	// Get initial battery/sensor readings
	SampleSensors();
	sensorVals.switchState = SWITCH;
	DBG_INFO("\r\nInitial battery: ");
	DBG_PRINT_INT(sensorVals.vddmv);
	DBG_INFO("mV");

	// Try normal setting load
	if(!SetttingsAction(SETTINGS_LOAD))
	{
		DBG_INFO("\r\nSetting loaded ok.");
	}
	else
	{
		DBG_INFO("\r\nSetting checksum fail, set defaults.");
		settingsChanged = TRUE;
	}
	// Button on reset with no cable - clear all
	if((BUTTON)&&(!RX_LEVEL))
	{
		DBG_INFO("\r\nReset settings, encryption and id.");
		SetttingsAction(SETTINGS_RESET_ALL);
		settingsChanged = TRUE;
		while(BUTTON); // Debounce
	}

	// LED flashes
	if(settingsChanged)
	{
		// Indicate a setting changed reset green green green
		LED_SET(LED_GREEN);DelayMs(100);LED_SET(LED_OFF);DelayMs(100);
		LED_SET(LED_GREEN);DelayMs(100);LED_SET(LED_OFF);DelayMs(100);
		LED_SET(LED_GREEN);DelayMs(100);LED_SET(LED_OFF);DelayMs(100);
	}
	else
	{
		// Indicate normal startup/reset red green red green ...
		LED_SET(LED_GREEN);DelayMs(100);LED_SET(LED_RED);DelayMs(100);
		LED_SET(LED_GREEN);DelayMs(100);LED_SET(LED_RED);DelayMs(100);
		LED_SET(LED_GREEN);DelayMs(100);LED_SET(LED_RED);DelayMs(100);
		LED_SET(LED_OFF);
	}

	// Enter command mode if cable detected
	if(RX_LEVEL)
	{
		COMM_CLEAR(); // Reset uart
		SettingCommandMode();
	}

	#if !defined(__DEBUG) && !defined(DEBUG_ON)
	COMM_OFF();
	#endif

	// Run sensor mode
	if(sensorVals.vddmv < settings.batt_min)
	{
		// Low power 
		DBG_INFO("\r\nTurn off.");
		RunBatteryEmptySequence();
	}

	#ifdef ENCRYPT
	DBG_INFO("\r\nInit AES, key: ");
	// KL: Backwards decrypt one packet to convert local key to a 1 block key
	unsigned char nulldata[16] = {0};
	aes_decrypt_128(nulldata,nulldata,settings.key,enckey);
	DBG_DUMP(settings.key, AES_KEY_SIZE, '\0');
	#endif

	DBG_INFO("\r\nRun sensor.");
	RunSensor();

	// We must call the fuction somewhere for linker to generate it!
	bootmirror(); // Reset
	return;
}

void RunSensor(void)
{
	unsigned char run = TRUE;
	unsigned char pirCredits = 1, swCredits = 1;
	unsigned char pirEvents = 0, swEvents = 0;
	unsigned short counter = settings.sensor_settle_time;
	unsigned short sampleCounter = settings.sample_interval;
	unsigned short creditCounter = settings.credit_interval;
	unsigned char header[4]; // Radio header

	enum{
		SENSOR_SETTLE,
		SENSOR_WAITING,
		SENSOR_TASKS,
		SENSOR_PIR_TX,
		SENSOR_PIR_LED,
		SENSOR_SW_TX,
		SENSOR_SW_LED,
		SENSOR_SAMPLE,
		SENSOR_SAMPLE_TX
	}sensorStatus = SENSOR_SETTLE;

	// Enable interrupts
	INTCONbits.GIE = 1; 	// Allow radio interrupt vector
	// Setup radio
	Si44RadioInit();
	// Set header bytes
	header[0] = settings.deviceId.val8[0];
	header[1] = settings.deviceId.val8[1];
	header[2] = settings.deviceId.val8[2];
	header[3] = settings.deviceId.val8[3]; // First byte in packet 'B'
	Si44RadioSetHeader(header,1);

	while(run)
	{
		// Sample counter - overide state
		if(!sampleCounter--)
		{
			sampleCounter = settings.sample_interval;
			// Check radio ok
			Si44RadioCheckOk();
			// Sample and send sensor values
			sensorStatus = SENSOR_SAMPLE;
		}
	
		// Sample counter - overide state
		if(!creditCounter--)
		{
			creditCounter = settings.credit_interval;
			if(pirCredits < settings.max_pir_credits) pirCredits++; 	// Grant pir credits
			if(swCredits < settings.max_switch_credits) swCredits++; 	// Grant switch credits
		}

		// PIR and switch tasks
		{
			pirEvents += PirTasks();
			swEvents += SwTasks();
		}

		switch (sensorStatus) {

			// Settling state - required as LEDs can trigger PIR
			case SENSOR_SETTLE: 
			{	
				LED_SET(LED_OFF);
				// Wait for count
				if(counter--) break; 
				// Enter prescale state
				counter = PIR_TASKS_PRESCALER;
				sensorStatus = SENSOR_WAITING;
			}
			/*Fall through*/
			// The wait state allows slower PIR tasks than loop tasks
			case SENSOR_WAITING: 
			{
				// Prescale of 0 or 1 does nothing
				if(counter-- < 2)break; 
				sensorStatus = SENSOR_TASKS;
			} 
			/*Fall through*/
			// Check for PIR or switch events
			case SENSOR_TASKS: 
			{
				// Check for sw events, set tx state
				#ifndef SW_OFF
				if(swEvents > 0)
				{
					sensorStatus = SENSOR_SW_TX; 
				}		
				#endif				
				// Check for new PIR events, set tx state
				#ifndef PIR_OFF
				if(pirEvents > 0)	
				{
					sensorStatus = SENSOR_PIR_TX; 						
				}
				#endif
				// Debugging of sw state
				#ifdef SW_LED_DEBUG
					if(!SWITCH) {LED_SET(LED_GREEN);}
					else 		{LED_SET(LED_RED);}
					DelayMs(1);
					LED_SET(LED_OFF);
				#endif
				// Enter prescale state
				counter = PIR_TASKS_PRESCALER;
				sensorStatus = SENSOR_WAITING;
				break;
			}

			// Transmit the PIR event
			case SENSOR_PIR_TX: 
			{
				if(pirCredits>0)
				{
					DBG_INFO("\r\nPIR EVENT TX.");
					TxSensorData(); // Transmit if there are credits
					pirCredits--;
				}
				else
				{
					DBG_INFO("\r\nPIR EVENT no credits.");
				}
				counter = settings.pir_led_time;
				sensorStatus = SENSOR_PIR_LED;
			}
			/*Fall through*/
			case SENSOR_PIR_LED: 
			{
				LED_SET(LED_RED);
				if(!counter--)
				{
					LED_SET(LED_OFF);
					pirEvents = 0;
					counter = settings.sensor_settle_time;
					sensorStatus = SENSOR_SETTLE;
				}
				break;
			}

			// Transmit the switch event 
			case SENSOR_SW_TX: 
			{
				if(swCredits>0)
				{
					DBG_INFO("\r\nSWITCH EVENT TX.");
					TxSensorData(); // Transmit if there are credits
					swCredits--;
				}
				else
				{
					DBG_INFO("\r\nSWITCH EVENT no credits.");
				}
				counter = settings.sw_led_time;
				sensorStatus = SENSOR_SW_LED;
			}
			/*Fall through*/
			case SENSOR_SW_LED: 
			{
				LED_SET(LED_GREEN);
				if(!counter--)
				{
					LED_SET(LED_OFF);
					swEvents = 0;
					counter = settings.sensor_settle_time;
					sensorStatus = SENSOR_SETTLE;
				}
				break;
			}
			
			// Transmit sensor data
			case SENSOR_SAMPLE: 
			{
				LED_SET(LED_OFF); /*Light sensor, must be off*/
				DBG_INFO("\r\nTIMED SAMPLE EVENT TX:");
				SampleSensors();
				counter = settings.num_tx_repeats;
				sensorStatus = SENSOR_SAMPLE_TX;
			}
			/*Fall through*/
			case SENSOR_SAMPLE_TX: 
			{
				// Repeat sends
				if(!counter--)
				{
					counter = settings.sensor_settle_time;
					sensorStatus = SENSOR_SETTLE;
					break;	
				}
				DBG_INFO("<TX>");
				TxSensorData();
				break;
			}

			default : break;
		};
		
		// Break on fail
		if(Si44RadioState == SI44_HW_ERROR)
		{
			DBG_INFO("\r\nRADIO ERROR EXIT");
			run = FALSE;
		}
		// Break on low batt
		if(sensorVals.vddmv < settings.batt_min)
		{
			DBG_INFO("\r\nLOW BATTERY EXIT");
			run = FALSE;
		}

		// Link pkt on button (plus add credits to help debug)
		if(BUTTON)
		{
			// 10 second count
			counter = 10; 
			
			// Tx packets at 1Hz, hold to clear
			while(BUTTON)
			{
				// Cable connected and button pressed, return (reset)
				if(RX_LEVEL) return;

				// After holding for max count (e.g. 10 seconds)
				if(!counter--) 		
				{
					// Reset settings only	
					DBG_INFO("\r\nBUTTON HOLD CLEAR");
					SetttingsAction(SETTINGS_CLEAR);
					LED_SET(LED_RED);
					while(BUTTON); 	// Wait
					SleepOnWdt(); 	// Debounce
					return;			// Return (reset)
				}
	
				// Flash green and tx link packets & data packets
				DBG_INFO("\r\nLINK PKT EVENT TX.");
				TxLinkPacket();		// Low power link pkt
				SampleSensors();	// Forced sample
				TxSensorData();		// Sensor packet

				// Flash LED
				LED_SET(LED_GREEN);
				DelayMs(500);
				LED_SET(LED_OFF);
				DelayMs(500);
			}
			pirCredits = settings.max_pir_credits;	// Create some credits
			swCredits = settings.max_switch_credits;// Useful for installation checks
			counter = settings.sensor_settle_time; 
			sensorStatus = SENSOR_SETTLE;
		}
		// Low power wait loop 
		SleepOnWdt();
	} // while(run)
	return;
}


unsigned char TxSensorData(void)
{
	unsigned char ret;
	unsigned char timeout = 0;
	packet_t packet;

	// Make data packet
	packet.pktType = PACKET_TYPE;
	packet.pktId = pkt_counter++;
	packet.xmitPwrdBm = settings.tx_power;
	packet.battmv = sensorVals.vddmv;
	packet.humidSat = sensorVals.humidSat;
	packet.tempCx10 = sensorVals.tempCx10;
	packet.lightLux = sensorVals.lightLux;
	packet.pirCounts = sensorVals.pirCounts+sensorVals.pirSuspCounts;
	packet.pirEnergy = sensorVals.pirEnergy;
	packet.swCountStat = (sensorVals.switchCounts<<1) + sensorVals.switchState;

	// Goto idle, begins powering up radio (~600us min)
	Si44RadioIdle();

	// Encrypt packet while waiting for radio power up
	EncryptPacket(&packet);

	// Xmit it
	ret = Si44RadioTx(packet.b,sizeof(packet_t),settings.tx_power,1); // Send 

	// Wait tx
	while(Si44RadioState == SI44_TXING && timeout < 100)	// Wait with timeout
		{DelayMs(1);timeout++;}

	// Check timeout (KL: Measured at 6ms from standby->xtalon->plltuned->packetsent )
	if(timeout>=100) 
		{ret = FALSE; Si44RadioState = SI44_HW_ERROR;}		// Timed out on tx
	
	return ret;	
}

unsigned char TxLinkPacket(void)
{
	unsigned char ret;
	unsigned char timeout = 0;
	packet_t packet;
	// Make encryption pkt 
	packet.pktType = AES_KEY_PKT_TYPE;
	#ifdef ENCRYPT
	memcpy(&packet.b[1],settings.key,AES_KEY_SIZE);			// Assume host wants otf key
	#else
	memset(&packet.b[1],0,AES_KEY_SIZE);
	#endif
	// Send encryption packet at 0dbm
	ret = Si44RadioTx(packet.b,sizeof(packet_t),0,1); 		// Send, 0dBm
	// Wait tx
	while(Si44RadioState == SI44_TXING && timeout < 100)	// Wait with timeout
		{DelayMs(1);timeout++;}
	// Check timeout
	if(timeout>=100) 
		{ret = FALSE; Si44RadioState = SI44_HW_ERROR;}		// Timed out on tx
	return ret;	
}

void EncryptPacket(packet_t* packet)
{
	#ifdef ENCRYPT
	unsigned char nullkey[16] = {0};
	aes_encrypt_128(&packet->b[1],&packet->b[1],enckey,nullkey);
	// If you wanted to be really careful you could check nullkey
	// was equal to settings.key here as well
	#endif
}

void RunBatteryEmptySequence(void)
{
	unsigned char counter;
	unsigned long timeout;
	do 
	{
		InitIO();
		InitAnalog();
		COMM_OFF();
		Si44RadioStandby(); // 1uA
		timeout = 600; // 10 minutes

		while(1)
		{
			counter = WDT_COUNT_1S;
			// Wait 1 second at low power
			for(;counter>0;counter--)
				{SleepOnWdt();}
			// Led flash
			LED_SET(LED_RED);
			// Reset on button
			if(BUTTON)
				{break;}
			// Break on radio fail
			if(!Si44RadioCheckOk())
				{break;}
			// Recheck batt
			if(!--timeout)
				{break;}
			// Led flash
			LED_SET(LED_OFF);
		}
		
		// Sample batt after timeout - return if now ok
		sensorVals.fvr = SampleFvrForVdd();
		sensorVals.vddmv = ConvertFvrToVddMv(sensorVals.fvr);
	}while(sensorVals.vddmv > settings.batt_min);
		
}

////////////////////////////////////////////////////////////////////////////////////////
// DEBUGGING FUNCTIONS FOLLOW //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void RadioTxDebug(void)
{
	unsigned char run = TRUE;
	unsigned char counter = 0;
	unsigned char buttonCounter = 0;
	// Set header bytes
	unsigned char header[4];
	// Enable interrupts
	INTCONbits.GIE = 1; 	// Allow radio interrupt vector
	// Setup radio
	Si44RadioInit();
	header[0] = settings.deviceId.val8[0];
	header[1] = settings.deviceId.val8[1];
	header[2] = settings.deviceId.val8[2];
	header[3] = settings.deviceId.val8[3];
	Si44RadioSetHeader(header,1);

	while(run)
	{
		static unsigned char prescaler = 0;
		static unsigned char prescaler2 = 0;

		// Transmit tasks
		if(++prescaler >= WDT_COUNT_1S)
		{
			prescaler = 0;
			LED_SET(LED_RED);
			// Get values
			SampleSensors();
			// Transmit
			TxSensorData();
			LED_SET(LED_OFF);
		}

		// Transmit tasks
		if(++prescaler2 >= WDT_COUNT_5S)
		{
			prescaler2 = 0;
			// Check radio ok
			Si44RadioCheckOk();
		}

		// Break on fail
		if(Si44RadioState == SI44_HW_ERROR)
		{
			run = FALSE;
		}

		// Button exit
		if(BUTTON)
		{ 
			if(buttonCounter++ > WDT_COUNT_5S)
			{
				LED_SET(LED_RED);
				while(BUTTON);
				run = FALSE;
				SleepOnWdt();
				break;
			}
		}
		else buttonCounter = 0;

		// Low power wait loop 
		LED_SET(LED_OFF);
		SleepOnWdt();
	}
	return;		
}

#define MAX_RX_LEN 64
void RadioRxDebug(void)
{
	unsigned char run = TRUE, led_overide = 0;
	unsigned char buttonCounter = 0;
	static unsigned char len, inPkt[MAX_RX_LEN+1]; // one byte for Null
	unsigned char header[4];

	// Indicate uart works
	DBG_INFO("\r\nDebug Rx mode...\r\n");

	// Enable interrupts
	INTCONbits.GIE = 1; 	// Allow radio interrupt vector
	// Setup radio
	Si44RadioInit();
	// Set header bytes
	header[0] = 0;
	header[1] = 0;
	header[2] = 0;
	header[3] = HEADER_BYTE_MSB; // First byte in packet
	Si44RadioSetHeader(header,0);// No check bytes
	// Start receiving
	INTCONbits.GIE = 1; 	// Allow radio interrupt vector
	Si44RadioRx(1);

	while(run)
	{
		// LED
		if(led_overide<=0)
		{
			LED_SET(LED_RED);
		}

		// Read packet if received
		if(Si44RadioState == SI44_RXED)
		{
			unsigned char rssi, len;
			LED_SET(LED_GREEN);
			led_overide = 5;
			len = Si44RadioReadPkt(header,inPkt,MAX_RX_LEN,&rssi); // Read packet
			Si44RadioRx(1); // Re-enable receiver
			#if 0 /*Strings*/
			// Dump the packet over the uart
			inPkt[len] = '\0';
			DBG_INFO(inPkt);
			#else /*Binary data*/
			DBG_INFO("\r\nRx:header = ");
			DBG_INFO(Byte2Hex(header[3]));
			DBG_INFO(Byte2Hex(header[2]));
			DBG_INFO(Byte2Hex(header[1]));
			DBG_INFO(Byte2Hex(header[0]));
			DBG_INFO(", len = ");
			DBG_PRINT_INT(len);
			DBG_INFO(", rssi = ");
			DBG_PRINT_INT(rssi);
			DBG_INFO("\r\ndata:");
			DBG_DUMP(inPkt,len,' ');
			#endif
		}
		else
		{
			// RSSI output
			static unsigned char prescaler = 0;
			if(++prescaler >= WDT_COUNT_5S)
			{
				signed char rssi = Si44ReadRssidBm();
				prescaler = 0;
				rssi = -rssi;
				DBG_INFO("\r\nrssi = -");
				DBG_PRINT_INT(rssi);
				DBG_INFO("dBm");
				// Check radio ok
				Si44RadioCheckOk();
			}
		}

		// Break on fail
		if(Si44RadioState == SI44_HW_ERROR)
		{
			run = FALSE;
		}

		// Button exit
		if(BUTTON)
		{ 
			if(buttonCounter++ > WDT_COUNT_5S)
			{
				LED_SET(LED_RED);
				while(BUTTON);
				run = FALSE;
				SleepOnWdt();
				break;
			}
		}
		else buttonCounter = 0;

		// LED off 
		if(led_overide>0)led_overide--;
		else LED_SET(LED_OFF);

		// Low power wait loop (radio receive current dominates)
		SleepOnWdt();

	}
	return;
}

void SleepDebug(void)
{
	unsigned char run = TRUE;
	unsigned char counter = 0;
	unsigned char buttonCounter = 0;

	// Enable interrupts
	INTCONbits.GIE = 1; 	// Allow radio interrupt vector
	// Setup radio off
	Si44RadioInit();

	while(run)
	{

		// Flash led 
		static unsigned char prescaler = 0;
		if(++prescaler >= WDT_COUNT_1S)
		{
			prescaler = 0;
			LED_SET(LED_RED);
			DelayMs(1);
			LED_SET(LED_GREEN);
			DelayMs(1);
		}


		// Button exit
		if(BUTTON)
		{ 
			if(buttonCounter++ > WDT_COUNT_5S)
			{
				LED_SET(LED_RED);
				while(BUTTON);
				run = FALSE;
				SleepOnWdt();
				break;
			}
		}
		else buttonCounter = 0;

		// Break on fail
		if(Si44RadioState == SI44_HW_ERROR)
		{
			run = FALSE;
		}
		
		// Low power wait loop 
		LED_SET(LED_OFF);
		SleepOnWdt();
	}
	return;		
}

// EOF
