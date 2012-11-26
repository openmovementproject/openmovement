/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// Dan Jackson, 2012

// Main 
#include "HardwareProfile.h"
#include "WirelessProtocols/Console.h"
#include "Transceivers/Transceivers.h"
#include "WirelessProtocols/MCHP_API.h"
#include "WirelessProtocols/SymbolTime.h"
#include "Settings.h"
#include "Network.h"
#include "Analogue.h"
#include <USB\usb.h>
#include <USB\usb_device.h>
#include <USB\usb_function_cdc.h>
#include "USB/USB_CDC_MSD.h"
#include "Utils/Fifo.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/Humidicon.h"
#include "Utils/BitPack.h"
#include "Peripherals/myI2C.h"
#include "Utils/Util.h"


#ifdef ROUTER
#error "ROUTER is defined"
#endif
#ifndef SENSOR
#error "SENSOR not defined"
#endif



// Sampling
unsigned int sampleIndex = 0;
reading_t reading;
static dataPacket_t readingDataPacket = {0};

// Packet FIFO
fifo_t packet_fifo;
dataPacket_t packet_fifo_buffer[MAX_PACKETS];
volatile char resultsOverflow = 0;

// Packet transmission
static dataPacket_t sendingDataPacket = {0};
unsigned char packetReady = 0;
unsigned short packetSequence = 0;

// Network
BOOL bConnected = FALSE;
unsigned short lastSuccessful = 0;

// Debugging
#define DEBUG_TIMEOUT 60
unsigned short debugTime = 0;
char debugLed = 1;

// Power
#define LOW_POWER_COUNT 12

void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks(); // Keeps time up to date
}

void __attribute__((interrupt,auto_psv)) _T1Interrupt(void)
{
#ifdef LED_DEBUG	
LED_B = !LED_B;
#endif
//LED_B = 1;
	RtcTimerTasks();// Needed to keep RTC upto date
	
	// If first sample, also capturing light and (old, if installed) temperature
	if (sampleIndex == 0)
	{
		// We'll be capturing light and (old, if installed) temperature in the next ADC...
		LDR_ENABLE();
		TEMP_ENABLE();
	 	
        // Request next humidity and temperature measurement
        Humidicon_measurement_request();
        
        // Light/temp settle time
	 	DelayMs(1UL);					// TODO: Energy-saving version using a timer
	}
	
	// Start ADC reading
	AdcInit(ADC_CLOCK_RC);
	AdcStartConversion(); // Fires ADC interrupt on completion
}


void __attribute__((interrupt, shadow, auto_psv)) _ADC1Interrupt(void)
{
	ADC1tasks(); 	// Must be first in ISR

	// Check we're reading a valid sample
	if (sampleIndex < DATA_INTERVAL)
	{
	    const adc_results_t *results;
	    
	    // ADC result pointer
	    results = (const adc_results_t *)&ADC1BUF0;

		// Add PIR and Microphone data
		reading.pir[sampleIndex] = results->pir;
		reading.audio[sampleIndex] = results->mic_ave;

#ifdef REMOVE_PIR_NOISE
// Use interpolation to remove noise in PIR on the samples with the light sensor enabled
if (sampleIndex == 1)
{
	reading.pir[0] = (reading.pir[1] + reading.pir[DATA_INTERVAL - 1]) / 2;
}
#endif

		// Reset audio integrators
		AUDIO_RESET = 1;// Reset integrators
		Delay10us(1UL);	// Wait for them to reset
		AUDIO_RESET = 0;// Start integrators
		
		// If this is the first sample, we're also capturing light and (old, if installed) temperature
		if (sampleIndex == 0)
		{
			// Read additional analogue values
			reading.temp = results->temp;		// (old, if installed) Temperature
			reading.light = results->light;		// Light
			reading.battery = results->batt;	// Battery
			
			// Disable light and (old, if installed) temperature sensors
			LDR_DISABLE();	// Turn off light
			TEMP_DISABLE();	// Turn off (old, if installed) temp
			
	        // Record accurate temperature and humidity measurement
	        if (humPresent)
	        {
		        // Read accurate temperature and humidity measurement
		        Humidicon_read();
		
				reading.temp = (0x8000 | gHumResults.temperature);	// Set top bit on Humidicon readings
			    reading.humidity = gHumResults.humidity;
	        }
	        else
	        {
			    reading.humidity = 0xffff;
	        }
        }
    
    	// We have finished sampling this sample    
		sampleIndex++;
	}	

	// Disable ADC here (not earlier, otherwise result registers are invalid)
	AdcOff();
	
	// If we've taken all of the samples, we need to add a new packet
	if (sampleIndex >= DATA_INTERVAL)
	{
		int i;
		
#ifdef LED_DEBUG	
LED_G = !LED_G;
#endif

	    // Start data packet
	    readingDataPacket.reportType = 0x12;                       // 0x12 (USER_REPORT_TYPE)
	    readingDataPacket.reportId = DATA_TEDDI_REPORT_ID;		    // [1] = 0x54 (ASCII 'T')
	    readingDataPacket.deviceId = settings.deviceId;		    // [2] = Short device identifier (16-bit) [doesn't have to be part of the payload, but this format is the same as the WAX]
	    readingDataPacket.version = ((unsigned char)settings.config << 4) | DATA_TEDDI_PAYLOAD_VERSION;	// [1] = (0x04)
	    readingDataPacket.sampleCount = DATA_INTERVAL;	            // [1] = (24 = 6 seconds)
		readingDataPacket.sequence = packetSequence++;			    // [2] = Sequence number
	    readingDataPacket.unsent = 0xffff;	                        // invalid for now, will be updated by TransmitPacket()

		// Measurements
		readingDataPacket.temp = reading.temp;        			// [2] = Temperature
		readingDataPacket.light = reading.light;      		    // [2] = Light
		readingDataPacket.battery = reading.battery;       	    // [2] = Batt
		readingDataPacket.humidity = reading.humidity;			// [2] = Humidity
	    
		// Pack PIR data
		for (i = 0; i < DATA_INTERVAL; i++)
		{
	        BitPack_uint10(&readingDataPacket.data, i, reading.pir[i]);
		}       
        
		// Pack Audio data
		for (i = 0; i < DATA_INTERVAL; i++)
		{
	        BitPack_uint10(&readingDataPacket.data, i + DATA_INTERVAL, reading.audio[i]);
		}       
		
		// Additional data
		{
			unsigned short *additional = (unsigned short *)((unsigned char *)&readingDataPacket + ADDITIONAL_OFFSET(DATA_INTERVAL));
	        /* parentAddress */    additional[0] = ((unsigned short)ConnectionTable[myParent].Address[0] | ((unsigned short)ConnectionTable[myParent].Address[1] << 8));	// Parent address
			/* parentAltAddress */ additional[1] = *(unsigned short *)ConnectionTable[myParent].AltAddress.v;	// Parent alt. address
	 	}
		
		// If the FIFO is full
		if (FifoFree(&packet_fifo) == 0)
		{
	    	FifoPop(&packet_fifo, NULL, 1);    // Dump a packet of data out
	    	resultsOverflow = 1;                // Set the overflow flag
	    }
		
		// Push new packet to FIFO, main loop will deal with transmission
		FifoPush(&packet_fifo, &readingDataPacket, 1); 

        // Reset sample index
        sampleIndex = 0;
	}
	
//LED_B = 0;
}


void __attribute__((interrupt,auto_psv)) _CNInterrupt(void)  // Change notification interrupts
{
	IFS1bits.CNIF = 0; // Clear flag
	// Process other CN sources here
}

void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
	Nop();
	Nop();
	Nop();
	Nop();
	Reset();
} 

        
char TransmitPacket(unsigned short unsentCount)
{
    int i;
    BYTE destinationAddress[8] = {0,0,0,0,0,0,0,0};
    size_t len;
    BYTE result;

    // Set the number of unsent samples
    sendingDataPacket.unsent = unsentCount;
    
    destinationAddress[0] = (unsigned char)settings.destination; //myLongAddress[0];
    destinationAddress[1] = (unsigned char)(settings.destination >> 8); //myLongAddress[1];
    destinationAddress[2] = myLongAddress[2];
    destinationAddress[3] = myLongAddress[3];
    destinationAddress[4] = myLongAddress[4];
    destinationAddress[5] = myLongAddress[5];
    destinationAddress[6] = myLongAddress[6];
    destinationAddress[7] = myLongAddress[7];
    
    //len = sizeof(sendingDataPacket);
    len = DATA_OFFSET + BITPACK10_SIZEOF(DATA_INTERVAL * 2) + ADDITIONAL_LENGTH;
    
    // Write packet
    MiApp_FlushTx();
    for (i = 0; i < len; i++)
    {
	    MiApp_WriteData(*((unsigned char *)&sendingDataPacket + i));
    }    

	//DumpPacket(&sendingDataPacket);
   
    // Transmit packet
    //ConsolePutROMString("Unicast... ");
    #if 0
        result = MiApp_UnicastAddress(destinationAddress, TRUE, FALSE);     // long
    #else
        result = MiApp_UnicastAddress(destinationAddress, FALSE, FALSE);    // short
    #endif
    //ConsolePutROMString(result ? "OK\r\n" : "FAIL\r\n");
    
    return result;
}    


void RunSensor(void)
{
    int failCount = 0;

    // Sense & transmit loop
    lastSuccessful = RtcSeconds();
    while (USB_BUS_SENSE == status.usb && status.lowpower < LOW_POWER_COUNT)
    {
        unsigned short now;
        int pendingPackets;
    		
   		if (!USB_BUS_SENSE)
   		{
			// Sleep - sampling done in background
			SystemPwrSave(	WAKE_ON_USB| WAKE_ON_BUTTON| WAKE_ON_TIMER1| WAKE_ON_RTC| WAKE_ON_ADC|
							LOWER_PWR_SLOWER_WAKE|
							SAVE_INT_STATUS| ALLOW_VECTOR_ON_WAKE);
							
			if (!USB_BUS_SENSE && reading.battery > 10 && reading.battery < BATT_CHARGE_MIN_SAFE) { status.lowpower++; } else { status.lowpower = 0; }
		}		
		else
		{
	    	const char *line = _user_gets();
	    	if (line != NULL) { SettingsCommand(line); }	
			USBProcessIO();
		}
		
		// Get time
		now = RtcSeconds();
		
		// Check button
	    if (PUSH_BUTTON_1 == 0)
	    {
		    debugTime = now;
		    debugLed = 1;
		    LED_SET(LED_BLUE);
		 	DelayMs(1UL);
	    }
	    else if (debugLed && now - debugTime > DEBUG_TIMEOUT) { debugLed = 0; }
	    
        // Check the number of samples in the FIFO  		
        pendingPackets = FifoLength(&packet_fifo);
        
        // If we have a results overflow, invalidate any existing packet
        if (resultsOverflow)
        {
            if (packetReady) { packetReady = 0; }   // Discard any old, unsent packet
            packetSequence = 0;                     // Reset the sequence
            resultsOverflow = 0;                    // Clear the overflow flag
        }    
        
		// If we already have a packet to transmit, or we have enough samples to start transmission, enter the transmit loop
		if (packetReady || pendingPackets >= TRANSMIT_THRESHOLD)
		{
    		int loops;
    		
            // Turn the radio on
            RadioOn();

			// Maximum of three times around this loop
            for (loops = 3; loops > 0; loops--)
    		{
	    		int unsentCount;
	    		
// Check for messages
if (MiApp_MessageAvailable()) { LED_SET(LED_WHITE); NetworkDumpMessage(); MiApp_DiscardMessage(); LED_SET(LED_OFF); }    // Needs to be in a loop, but we'll be turning the radio off anyway

        		// If we don't already have a packet to send, prepare one
        		if (!packetReady && pendingPackets > 0)
        		{
					FifoPop(&packet_fifo, &sendingDataPacket, 1);    // Dump a packet of data out
            		packetReady = 1;
                }
                
        		// If we don't have a packet to send, exit the transmit loop
        		if (!packetReady)
        		{
            		break;
                }      
        		
                // Update the number of packets in the FIFO  		
                pendingPackets = FifoLength(&packet_fifo);
                // TODO: Mask interrupts off for this line:
                unsentCount = pendingPackets * DATA_INTERVAL + sampleIndex;
                                
        		// Transmit the packet
                if (debugLed) { LED_SET(failCount ? LED_RED : LED_GREEN); }  // Indicate the success state of the last packet
        		if (TransmitPacket(unsentCount))
        		{
            		packetReady = 0;    // Clear the packet ready flag
            		failCount = 0;      // Reset the failure counter
            		lastSuccessful = RtcSeconds();
                }  		
                else
                {
                    failCount++;        // Increment the failure counter
                }
                LED_SET(LED_OFF); 
                
				// If we have finished, exit the loop
				if ((!packetReady && pendingPackets <= 0) || loops <= 1)
				{
					break;
				}
					
				// If we'd still like to transmit a packet, wait a short while first...
	            if (loops > 0)
				{
				    static char scramble[16] = { 0b0000, 0b1000, 0b0100, 0b1100, 0b0010, 0b1010, 0b0110, 0b1110, 0b0001, 0b1001, 0b0101, 0b1101, 0b0011, 0b1011, 0b0111, 0b1111 };
				    unsigned short delay = 2 + scramble[(TMR1 ^ settings.deviceId) & 0x0f];
				    DelayMs(delay / 2);		// TODO: Reduce power consumption
				}
            
            }

            // Turn the radio off
            RadioOff();

        }

        // Break if not successfully transmitted for a while
        if ((now - lastSuccessful) > 30)	// || failCount > (80 / DATA_INTERVAL))
        {
            break;
        }    

    }// while failcount    
    
	SystemPowerOff(LEAVE_ON_RF | LEAVE_ON_ANALOGUE);
    return;
}    


int main(void)
{   
    BYTE i, j;
    unsigned short disconnectTime;
    unsigned short reconnectInterval;
	static unsigned int reset_reason; 
	
	reset_reason = RCON;
	RCON = 0;
    
    // Initialize the system + clock
    InitIO();
    CLOCK_SOSCEN();
    CLOCK_INTOSC();
	RtcStartup();
    RtcInterruptOn(0);  // RTC interrupt only -- needed for low power flash
    
	CheckLowPowerCondition();
	reading.battery = adcResult.batt;		// Initialize the periodically-updated reading
	
    SettingsInitialize();

	// If we're using the PLL when the radio is on, turn it on now
	#if defined(RADIO_PLL)
	    CLOCK_PLL();
	    DelayMs(5);
	#endif
	
	// Init analogue circuitry (80sec to stabilise PIR)
	nAUDIO_POWER = 0; 	// On
	nPIR_POWER = 0;		// On
	SettingsUpdateGain();

	// Humidity sensor
	InitI2C();
	Humidicon_init();
	
	// Initialise the data buffer
  	FifoInit(&packet_fifo, sizeof(dataPacket_t), MAX_PACKETS, packet_fifo_buffer);
	FifoClear(&packet_fifo);

	// This is the sampling interrupt, begins filling buffer at 4Hz
	RtcInterruptOn(4);

	// Attempt to connect to the USB
    ConsoleInit();

/*
#if 0
while(USB_BUS_SENSE)
{
	#ifndef USB_INTERRUPT
	USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	#endif
   	{ 
	   	const char *line = _user_gets();
	   	if (line != NULL) { SettingsCommand(line); }	
	}	
	USBProcessIO();
	if((USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl != 1))
	{
		if (FifoLength(&packet_fifo)>0) 
		{
#ifdef LED_DEBUG	
LED_R = !LED_R;
#endif
			FifoPop(&packet_fifo, &sendingDataPacket, 1);    // Dump a packet of data out
			printf("\r\nLight=%umV, ", AdcToMillivolt(sendingDataPacket.light));
			printf(", Temp=%u (%s^C), ", sendingDataPacket.temp, fixed_itoa(Humidicon_Convert_centiC(sendingDataPacket.temp), 2));
			printf(", Hum=%u (%s%%)", sendingDataPacket.humidity, fixed_itoa(Humidicon_Convert_percentage(sendingDataPacket.humidity), 2));
			//USBCDCWait();
		}
	}
}
USBDeviceDetach();
return;
#endif
*/



	// This turns on the RF
    NetworkInit();
    Printf("\r\nDevice id = "); PrintChar(settings.deviceId >> 8); PrintChar(settings.deviceId & 0xff);
    
    // Attempt to init with freezer unless button is pressed
    bConnected = TRUE;
    if ((PUSH_BUTTON_1 == 0) || (MiApp_ProtocolInit(TRUE) == FALSE))
    {
        MiApp_ProtocolInit(FALSE);  // Init without freezer
        bConnected = FALSE;
    }

    // Turn the radio off
    RadioOff();

    // Main sensor loop
    disconnectTime = RtcSeconds();
    reconnectInterval = 0;
    while (USB_BUS_SENSE == status.usb && status.lowpower < LOW_POWER_COUNT)
    {
        unsigned short now;
        
        if (bConnected)
        {
            RunSensor();				// Returns when failed to send -- disconnect
            MiApp_ProtocolInit(FALSE);  // Init without freezer
            bConnected = FALSE;
            disconnectTime = RtcSeconds();
            reconnectInterval = 0;
        }
        else
        {
    		if (!USB_BUS_SENSE)
    		{
	    		// Sleep - sampling done in background
	    		SystemPwrSave(	WAKE_ON_USB| WAKE_ON_BUTTON| WAKE_ON_TIMER1| WAKE_ON_RTC| WAKE_ON_ADC|
	    						LOWER_PWR_SLOWER_WAKE|
	    						SAVE_INT_STATUS| ALLOW_VECTOR_ON_WAKE);
	    						
				if (!USB_BUS_SENSE && reading.battery > 10 && reading.battery < BATT_CHARGE_MIN_SAFE) { status.lowpower++; } else { status.lowpower = 0; }
	    						
	    	}					
	    	else
	    	{
		    	const char *line = _user_gets();
		    	if (line != NULL) { SettingsCommand(line); }	
				USBProcessIO();
	    	}
        }    

		// Update time
        now = RtcSeconds();
        
   		// Check button
	    if (PUSH_BUTTON_1 == 0)
	    {
		    debugTime = now;
		    debugLed = 1;
		    LED_SET(LED_YELLOW);
		 	DelayMs(1UL);
	    }
	    else if (debugLed && now - debugTime > DEBUG_TIMEOUT) { debugLed = 0; }
        
        if ((now - disconnectTime) >= reconnectInterval)
        {
            disconnectTime = now;
            if (!USB_BUS_SENSE || !status.noconnect)
            {
	            RadioOn();
	            
				if (debugLed) { LED_SET(LED_YELLOW); }
	            bConnected = Reconnect();
			    LED_SET(LED_OFF); 
			    
	            if (!bConnected)
	            {
	                #define MAX_RECONNECT_INTERVAL (5*60)
	                reconnectInterval += (reconnectInterval / 2) + 10;
	                if (reconnectInterval > MAX_RECONNECT_INTERVAL) { reconnectInterval = MAX_RECONNECT_INTERVAL; }
	                
		            RadioOff();
	            }    
			} 
        }
        
    }

	#ifdef USE_USB_CDC
	    USBDeviceDetach();
    #endif
    
    // Shut down all, then reset
    RadioOff();
    SystemPowerOff(0);
    
    return(0);    
}


