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

// main-transmitter.c - Main module for transmitter.
// Dan Jackson, Cas Ladha, Karim Ladha, 2010-2011.

#include "HardwareProfile.h"

#if (DEVICE_TYPE == 1) || (DEVICE_TYPE == 2)				// 1 = RFD WAX or WAX+GYRO

// Sampling mode
//   Setup accelerometer to sample to FIFO with watermark interrupt, unlinked activity/inactivity detection. 
//   Sleep until timer/FIFO. At time-jittered intervals, flush the accelerometer FIFO to buffers, transmit the current buffer (include offset from end of buffer)
//   Calculate new jitter. If packet not ACK (or >= 1 packet still to transmit), change interval.
//
// TODO: If RAM buffer overflow (or n buffers dropped), assume out of range and give up for a while (forced sleep delay?)
// TODO: Dual-mode transmitter/receiver (fix MRF timing delays to be clock-sensitive)
// TODO: Update to latest version of stack
//

// TODO: Fixed sample rate for gyro/accel (sample on single accel receive?)


// Includes
#include "ConfigApp.h"
#include <stdio.h>
#include <delays.h>
#include <reset.h>
#include "usb_cdc.h"
#include "util.h"
#include "accel.h"
#include "data.h"
#include "WirelessProtocols\MCHP_API.h"
//#include "WirelessProtocols\SymbolTime.h"
#if (DEVICE_TYPE==2)
#include "exp.h"
#include "gyro.h"
#endif



// Variables
#pragma idata

// MiWi
#if ADDITIONAL_NODE_ID_SIZE > 0
    BYTE AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE] = {0};
#endif

// Variables
volatile unsigned char active = 0;
volatile unsigned char accelDoubleTaps = 0;
volatile unsigned char accelSingleTaps = 0;

// Timer status
volatile unsigned short timerElapse = 0;
volatile unsigned long timerCountdown = 0;
volatile unsigned char timerComplete = 1;


#pragma udata

// Private prototypes
extern void CriticalError(void);
extern void DisableIO(void);
extern void ShippingMode(void);
extern void WaitForPrecharge(void);
extern void DeviceId(void);
void RunStartup(void);
char RunAttached(void);
void RunDetatched(void);
void ProcessConsoleCommand(void);


#pragma code





// Update 'timerCountdown' and Call (from timer interrupt or main), updates timerComplete flag.
void TimerUpdate(void)
{
	// Setup the timer for the next iteration
	if (timerCountdown <= TIMER_CUTOFF)
	{
		// We have finished
		timerElapse = 0;
		timerCountdown = 0;
		timerComplete = 1;
		TIMER_IE = 0;		// Stop timer interrupts
	}
	else if (((unsigned short)(timerCountdown >> 16)) != 0)
	{
		// We want to wait for a complete cycle
		timerElapse = 0xffff;
		timerComplete = 0;
		TIMER_SET(0);
		TIMER_IE = 1;		// Start timer interrupts
	}
	else
	{
		// Preload required, calculate next preload value (first 'if' ensures TIMER_CUTOFF minimum ticks)
		unsigned short preload;
		timerElapse = (unsigned short)(timerCountdown & 0xffff);
		timerComplete = 0;
		preload = (0xffff - timerElapse);
		TIMER_SET(preload);
		TIMER_IE = 1;		// Start timer interrupts
	}
}


// Start timer countdown
void TimerStartCountdown(unsigned long v)
{
	TIMER_START();		// sets up and starts timer (doesn't enable interrupts)
    timerElapse = 0;
	timerCountdown = v;
	TimerUpdate();		// enables interrupts (if not already complete)
}




// HighISR - defined in MRF24J40 tranceiver code and chains to UserInterruptHandler()
// UserInterruptHandler - Non-RF high-priority interrupt handler
void UserInterruptHandler()
{
	// USB bus sense - our "reset switch"
	if (USB_BUS_SENSE_IF)
	{
		USB_BUS_SENSE_IF = 0;		// never called
		Reset();
	}

	#if defined(USB_INTERRUPT) && !defined(USB_INTERRUPT_LOW)
		// Check bus status and service USB interrupts (call once every ~100us, in most cases executes in ~50 instruction cycles)
		USBDeviceTasks(); 
	#endif

    // return to HighISR()
}


// LowISR - Low-priority interrupt handler
#pragma interruptlow LowISR
void LowISR()
{
    // Timer interrupt
	if (TIMER_IE && TIMER_IF)
	{
		// Update countdown
		if (timerElapse < timerCountdown)
		{
			timerCountdown -= timerElapse;
		}
		else
		{
			timerCountdown = 0;
		}

		TimerUpdate();

		TIMER_IF = 0;
	}


    // Service the accelerometer interrupt: check which interrupt flag caused the interrupt, clear the interrupt flag.
	if (ACCEL_INT1_IF && ACCEL_INT1_IE)              // ADXL interrupt
	{
		unsigned char source;
		source = AccelReadIntSource();
		if ((source & ACCEL_INT_SOURCE_OVERRUN) || (source & ACCEL_INT_SOURCE_WATERMARK))
        {
            if (DataPerformAccelSample() == 0) 		// Sample measurements
            {
                AccelReadFIFO(NULL, 0xff);      // The only reason we should've read only 0 samples is if our write buffer is full -- dump ADXL FIFO if this occurs to prevent re-interrupt
#if (DEVICE_TYPE==2)
	            GyroReadFIFO(NULL, 0xff);      	// The only reason we should've read only 0 samples is if our write buffer is full -- dump gyro FIFO if this occurs to prevent re-interrupt
#endif
            }
active = 1;
        }
		if ((source & ACCEL_INT_SOURCE_DOUBLE_TAP) || (source & ACCEL_INT_SOURCE_SINGLE_TAP))	// tap
		{
            unsigned char axis = AccelReadTapStatus(); // b0 = Tap-Z
			if (source & ACCEL_INT_SOURCE_DOUBLE_TAP) { accelDoubleTaps++; }	   // double-tap
            else if (source & ACCEL_INT_SOURCE_SINGLE_TAP)	{ accelSingleTaps++; } // single-tap
active = 1;
		}
		if (source & ACCEL_INT_SOURCE_ACTIVITY) { active = 1; }	// activity
		if (source & ACCEL_INT_SOURCE_INACTIVITY) { active = 0; }				// inactivity

		// Clear the interrupt flag
		ACCEL_INT1_IF = 0;
	}
	
	
#if (DEVICE_TYPE==2)
    // Service the gyro interrupt: clear the interrupt flag
	if (GYRO_INT2_IF && GYRO_INT2_IE)              	// GYRO interrupt
	{
		if (DataPerformGyroSample() == 0)			// Empty gyro FIFO to get latest data, and to ensure it won't overflow while we're transmitting
		{
            AccelReadFIFO(NULL, 0xff);      		// The only reason we should've read only 0 samples is if our write buffer is full -- dump ADXL FIFO if this occurs to prevent re-interrupt
            GyroReadFIFO(NULL, 0xff);      			// The only reason we should've read only 0 samples is if our write buffer is full -- dump gyro FIFO if this occurs to prevent re-interrupt
		}
		// Clear the interrupt flag
		GYRO_INT2_IF = 0;
	}
#endif
	

	#if defined(USB_INTERRUPT) && defined(USB_INTERRUPT_LOW)
		// Check bus status and service USB interrupts (call once every ~100us, in most cases executes in ~50 instruction cycles)
		USBDeviceTasks(); 
	#endif

    // return from #pragma interruptlow section
}


// Declarations
#pragma code


// main - main function
void main(void)
{
    // Start-up code: remappable pins, battery pre-charge, initialize 8MHz INTOSC, read configuration from flash
    RunStartup();

    // If USB detected then run USB mode until disconnected (then reset); otherwise...
    if (RunAttached())            
    {
		// USB detatched...

		// Switch CPU to 8MHz and disable USB module
	    UCONbits.USBEN = 0;		// USBEN can only be change when module clocked
		CLOCK_INTOSC();
	    UCONbits.SUSPND = 1;    // Suspend USB

		// Reset back to non-attatched mode
		Reset();
	}

	// Enter standby until activity detected, then enter sampling mode
    RunDetatched();

    // Reset when charging / USB detected
    Reset();
}


// Startup code
void RunStartup(void)
{
    // Set mode
    dataStatus.sampling = 0;

#ifdef USE_WDT
    // Enable WDT (WDTEN hardware config must be OFF for software control)
	ClrWdt();
    WDTCONbits.SWDTEN = 1;
#endif

    // WaitForPrecharge - includes remappable pin setup
	WaitForPrecharge();

    // Switch to INTOSC
    CLOCK_INTOSC();
	ClrWdt();

    // Load configuration from ROM (patch device ID)
    DataConfigLoad();

	// Set standard output to user output function
	stdout = _H_USER;

	// NOTE: Remapping and configuration of pins already done in WaitForPrecharge()

	// Enable interrupts
    INTCONbits.GIE = 1;     // Enable interrupts

	// Start debug USART if enabled
#ifdef DEBUG_USART2_TX
	#warning "Not expected."
    USARTStartup();
#endif

    // We use bus sense interrupt as our reset switch
#ifdef USE_USB_BUS_SENSE_IO
    USB_BUS_SENSE_IE = 1;   // USB bus sense interrupt
#endif

    // Startup finished
    return;
}


// RunAttached - Run system in USB mode (recharge, communication) until a disconnect occurs
char RunAttached(void)
{
	unsigned int loopCounter = 0;
	unsigned int fullCount;

	// Return 0 if the USB is not connected
	if (!USB_BUS_SENSE) return 0;

	// Start USB (initializes PLL)
    USBInitializeSystem();

    // Now USB running at 48MHz -- CPU running at 8 MHz
    #if defined(USB_INTERRUPT)
		#if !defined(USB_INTERRUPT_LOW)
			USBDeviceAttach();
		#else
			USBDeviceAttachLow();
		#endif
    #endif

	// Enable interrupts
    INTCONbits.GIE = 1;     

    // Main loop -- as this isn't a receiver, low power mode while battery charging then return to normal mode
	fullCount = 0x6000;		// Assume charging
	while (USB_BUS_SENSE) 
	{
		if (BATT_NCHARGING && fullCount > 0) { fullCount--; }

		if (fullCount == 0) { LED = 1; }
		else { LED = (loopCounter & 0x0fff) < 0x10; }		// Charging = brief blink 

		loopCounter++;

        #if defined(USB_POLLING)
            // Check bus status and service USB interrupts (call once every ~100us, in most cases executes in ~50 instruction cycles)
            USBDeviceTasks(); 
        #endif

        // USB process I/O
        USBProcessIO();

// TODO: Very low power mode...

        // Handle serial console commands
        ProcessConsoleCommand();

        // Check we're still attached
		#ifndef USE_USB_BUS_SENSE_IO
		#error Must be bus-sense otherwise must clear watchdog timer
		#endif
        if (USB_BUS_SENSE) { ClrWdt(); }   // Clear WDT while the USB detect line is high
    }
	LED = 0;

    #if defined(USB_INTERRUPT)
        USBDeviceDetach();    // Wait >100ms before re-attach
    #endif

    return 1;                 // Attach mode was entered, now completed (detached)
}



// RunDetatched - Run system in radio mode (standby waiting for activity, or sampling then transmitting FIFO waiting for inactivity)
void RunDetatched()
{
	#define asleep (!active)
#ifndef asleep
	char asleep = 0;
#endif

	ClrWdt();

    // Start MiWi
    RF_INIT_PINS();

	// Initialize the protocol stack defined in ConfigApp.h. (false = don't restore previous network configuration)
    MiApp_ProtocolInit(FALSE);

    // Set default channel
    MiApp_SetChannel(dataConfig.channel);	// == FALSE -- Channel not supported in current condition.

    // Select the connection mode (from: ENABLE_ALL_CONN, ENABLE_PREV_CONN, ENABLE_ACTIVE_SCAN_RSP, DISABLE_ALL_CONN)
    MiApp_ConnectionMode(ENABLE_ALL_CONN);	// DISABLE_ALL_CONN

	// Try to establish a new connection with peer device (index 0xff = any peer, mode = CONN_MODE_DIRECT)
    //#ifdef ENABLE_HAND_SHAKE
        //i = MiApp_EstablishConnection(0xFF, );
    //#endif
    //MiApp_StartConnection(START_CONN_DIRECT, 10, 0);
    //DumpConnection(0xFF);  // Debug dump connection entry (0xff = all)

	// Start with RF off until activity
	MiApp_TransceiverPowerState(POWER_STATE_SLEEP);	// RF to sleep (== SUCCESS)
    dataStatus.sampling = 0;

	// Clear data buffers
	DataInit();

    // Start accelerometer and interrupts
    AccelStartup(dataConfig.accelRate, dataConfig.accelWatermark, dataConfig.activityThreshold, dataConfig.inactivityThreshold, dataConfig.activityTime);
	AccelEnableAutoSleep(0);	// Enable auto-sleep, 1 = sleep right now (how does this interact with auto-sleep?)
	AccelEnableInterrupts(ACCEL_INT_SOURCE_WATERMARK | ACCEL_INT_SOURCE_SINGLE_TAP | ACCEL_INT_SOURCE_DOUBLE_TAP | ACCEL_INT_SOURCE_ACTIVITY | ACCEL_INT_SOURCE_INACTIVITY);

	LED = 0;        // initially off
	RCONbits.IPEN = 1;
    INTCONbits.GIE = 1;     // Enable high interrupts (RF/timer0, USB attach)
	INTCONbits.GIEL = 1;	// Enable low interrupts (accelerometer)

    // Seed random number generator
    srand(DEVICE_ID);

    // Initialize timer
    TimerStartCountdown(0);

    for (;;)
    {
		ClrWdt();

        // If powered, reset (will run in USB mode)
        if (USB_BUS_SENSE) { break; }
	
		// Check whether the accelerometer is asleep
#ifndef asleep
#error "must define asleep"
//ACCEL_INT1_IE = 0; asleep = AccelIsAsleep(); ACCEL_INT1_IE = 1;
#endif

		// Rest the processor -- if accelerometer is asleep then processor to SLEEP, if not then IDLE
		if (asleep)
		{
			// Accelerometer is asleep and we were in sampling mode  ==>  put device to sleep
			if (dataStatus.sampling)
			{
				dataStatus.sampling = 0;
				MiApp_TransceiverPowerState(POWER_STATE_SLEEP); // RF to sleep (== SUCCESS)
#if (DEVICE_TYPE==2)
				GYRO_INT2_IE = 0;
				GyroStandby();
#endif
			}
	
// TODO: Very low power mode (leave accel) - can't call DisableIO as it puts accelerometer to standby
//			DisableIO();
			LED = 0; 

			// Don't need to disable TMR0 as SLEEP won't clock peripherals // T0CONbits.TMR0ON = 0;
    		OSCCONbits.IDLEN = 0;           // Real SLEEP not IDLE (peripheral clock off too)
			WDTCONbits.SWDTEN = 0;			// Turn off WDT to stop it waking the device
			WDTCONbits.REGSLP = 1;			// Power off the VREG while asleep for ultra low power 

INTCONbits.GIE = 0;     // Disable interrupts (ensures asleep flag valid)
//LED = 1; 
			if (asleep)
				Sleep();                        // Enter SLEEP (clears WDT), return on accelerometer interrupt or USB attach (or WDT timeout if we didn't disable it)
            Nop();                          // NOP after SLEEP
			Nop();
//LED = 0; 
INTCONbits.GIE = 1;     // Re-enable interrupts (will vector now)
//Delay10kTCYx(2);
			Nop();
			Nop();
			Nop();
			Nop();

    	    // If watchdog fired (won't actually be possible right now as it was disabled before sleep)
            //if (isWDTWU()) { ... }  // If asleep (STANDBY), no activity has interrupted us -- manually check accelerometer values for change (and reset if detected or fail to read)

#ifdef USE_WDT
			// Re-enable WDT
			ClrWdt();  
			WDTCONbits.SWDTEN = 1; 
#endif

        } else {	// If awake...

			// Accelerometer is not asleep and if we weren't in sampling mode  ==>  device has woken up
			if (!dataStatus.sampling)
			{
				ACCEL_INT1_IE = 0;
#if (DEVICE_TYPE==2)
				GYRO_INT2_IE = 0;
#endif
				DataInit();
				MiApp_TransceiverPowerState(POWER_STATE_WAKEUP); // Wake up RF (== SUCCESS)
				dataStatus.sampling = 1;
#if (DEVICE_TYPE==2)
				RtcClear();
				GyroStartupFifoInterrupts();
				GYRO_INT2_IE = 1;
#endif
				ACCEL_INT1_IE = 1;
			}

	        // Check whether a packet has been received by the transceiver. 
	        while (MiApp_MessageAvailable())
	        {
	            // Examine message stored in the rxFrame structure of RECEIVED_MESSAGE.
	            #ifdef ENABLE_SECURITY
	                //if (rxMessage.flags.bits.secEn) ...		// Check whether secured
	            #endif
	            #ifndef TARGET_SMALL
	                //if (rxMessage.flags.bits.broadcast) ...	// Check whether broadcast or unicast
	                //rxMessage.PacketRSSI
	                //if (rxMessage.flags.bits.srcPrsnt)		// Check whether has a source
	                //{
	                //    if (rxMessage.flags.bits.altSrcAddr)
	                //    {
	                //        rxMessage.SourceAddress[1], rxMessage.SourceAddress[0]
	                //    }
	                //    else
	                //    {    
	                //        //for (i = 0; i < MY_ADDRESS_LENGTH; i++) { rxMessage.SourceAddress[MY_ADDRESS_LENGTH-1-i]; }
	                //    }
	                //}
	            #endif
	
				// TODO: Check message type for on-the-fly configuration
				
	            // rxMessage.Payload[0] // rxMessage.PayloadSize
	            
	            // Release the current received packet so that the stack can start to process the next received frame.
	            MiApp_DiscardMessage();
	        }
	
			// Flush the data buffers (transmit) if non-empty
            if (timerComplete)
            {
				ACCEL_INT1_IE = 0;
#if (DEVICE_TYPE==2)
				GYRO_INT2_IE = 0;
#endif
                DataPerformAccelSample();	// Empty accel FIFO to get latest data, and to ensure it won't overflow while we're transmitting
#if (DEVICE_TYPE==2)
				DataPerformGyroSample();  	// Empty gyro FIFO to get latest data, and to ensure it won't overflow while we're transmitting
#endif
	            DataTransmit();
				LED = 0;
#if (DEVICE_TYPE==2)
				GYRO_INT2_IE = 1;
#endif
				ACCEL_INT1_IE = 1;

				// Calculate new jitter...
                {
                	unsigned short jitter = rand();
                	jitter ^= ((unsigned short)TMR3L << 5);
                	jitter ^= ((unsigned short)TMR0L << 5);
                	jitter ^= (DEVICE_ID << 5);
                	jitter &= dataConfig.jitterMask;
            
                    TimerStartCountdown(dataConfig.transmitInterval + jitter);
                }
			}


    		OSCCONbits.IDLEN = 1;           // IDLE not real SLEEP (peripheral clock on for Timer0)
			WDTCONbits.REGSLP = 0;			// Don't power off the VREG while asleep
			INTCONbits.GIE = 0;     // Disable interrupts (ensures asleep flag valid)
			if (!asleep && !timerComplete)		// Last check accel not asleep right now
				Sleep();                        // Enter IDLE (clears WDT), return on RF interrupt, accelerometer interrupt or USB attach, or WDT timeout
            Nop();                          // NOP after SLEEP
			Nop();
			INTCONbits.GIE = 1;     // Re-enable interrupts (will vector now)

    	    // If watchdog fired: the sampling queue has failed to interrupted us -- reset.
            if (isWDTWU()) 
			{
				//if (!asleep) { CriticalError(); }	// A small chance the accelerometer fell asleep before we did (and so we weren't interrupted)
			}

		}


    }

	LED = 0;
	return;
}


// ProcessConsoleCommand
void ProcessConsoleCommand(void)
{
    const char *line;
    line = _user_gets();
    if (line != NULL && strlen(line) > 0)
    {
        //printf((const rom far char *)"%s\r\n", line);
        if (stricmp_rom(line, (const rom far char *)"help") == 0)
        {
            printf((const rom far char *)"HELP: help|echo|status|id|reset|device|target|channel|rate|watermark|actthres|inactthres|acttime|tranint|jittermask\r\n");
        }
        else if (strnicmp_rom(line, (const rom far char *)"echo", 4) == 0)
        {
            int value = -1;
            if (line[5] != '\0') { value = (int)my_atoi(line + 5); }
            if (value == 0 || value == 1)
            {
                commEcho = value;
            }
            printf((const rom far char *)"ECHO=%d\r\n", commEcho);
        }
        else if (stricmp_rom(line, (const rom far char *)"status") == 0)
        {
      		printf((const rom far char *)"$STATUS1=INT0E:%d,INT0F:%d,INT1E:%d,INT1F:%d,INT2E:%d,INT2F:%d,GIE:%d\r\n", INTCONbits.INT0IE, INTCONbits.INT0IF, INTCON3bits.INT1IE, INTCON3bits.INT1IF, INTCON3bits.INT2IE, INTCON3bits.INT2IF, INTCONbits.GIE);
        }
        else if (stricmp_rom(line, "id") == 0)
        {
#if (DEVICE_TYPE==2)
           printf("ID=WAX-g,%x,%x,%u\r\n", HARDWARE_VERSION, SOFTWARE_VERSION, dataStatus.deviceId);
#else
           printf("ID=WAX-t,%x,%x,%u\r\n", HARDWARE_VERSION, SOFTWARE_VERSION, dataStatus.deviceId);
#endif
        }
        else if (stricmp_rom(line, (const rom far char *)"reset") == 0)
        {
      		printf((const rom far char *)"RESET\r\n");
      		printf((const rom far char *)"OK\r\n");
            Delay10KTCYx(200);        // Briefly wait (allows client to close port before device dies)
            #if defined(USB_INTERRUPT)
                USBDeviceDetach();    // Wait >100ms before re-attach
            #endif
            Reset();
        }
        else if (stricmp_rom(line, (const rom far char *)"shipping") == 0)
        {
      		printf((const rom far char *)"SHIPPING\r\n");
      		printf((const rom far char *)"OK\r\n");
            Delay10KTCYx(200);        // Briefly wait (allows client to close port before device dies)
            #if defined(USB_INTERRUPT)
                USBDeviceDetach();    // Wait >100ms before re-attach
            #endif
            ShippingMode();
        }
        else if (strnicmp_rom(line, "device", 6) == 0)
        {
            unsigned short id = 0xffff;
            if (line[6] != '\0') { id = (unsigned short)my_atoi(line + 7); }
            if (id != 0xffff)
            {
				DataConfigSetId(id);
				DataConfigSave();
            }
            printf("DEVICE=%u\r\n", dataStatus.deviceId);
        }
        else if (strnicmp_rom(line, "target", 6) == 0)
        {
            unsigned short id = 0xffff;
			unsigned short tid;
            if (line[6] != '\0') { id = (unsigned short)my_atoi(line + 7); }
            if (id != 0xffff)
            {
				DataConfigSetTargetId(id);
				DataConfigSave();
            }
			tid = dataConfig.targetLongAddress[0] | ((unsigned short)(dataConfig.targetLongAddress[1]) << 8);
            printf("TARGET=%u\r\n", tid);
        }
        else if (strnicmp_rom(line, "channel", 7) == 0)
        {
            unsigned char channel = 0xff;
            if (line[7] != '\0') { channel = (unsigned char)my_atoi(line + 8); }
            if (channel != 0xff)
            {
				dataConfig.channel = channel;
				DataConfigSave();
            }
            printf("CHANNEL=%u\r\n", dataConfig.channel);
        }
        else if (strnicmp_rom(line, "rate", 4) == 0)
        {
            unsigned char v = 0xff;
            if (line[4] != '\0') { v = (unsigned char)my_atoi(line + 5); }
            if (v != 0xff)
            {
                dataConfig.accelRate = v;
#ifdef ACCEL_FIXED_RATE
				// (For gyro mode) fixed accelerometer rate (flexible g-range and low-power bit)
				dataConfig.accelRate = (dataConfig.accelRate & 0xf0) | (ACCEL_FIXED_RATE & 0x0f);
#endif
				DataConfigSave();
            }
#ifdef ACCEL_FIXED_RATE
            printf("RATE=%u,%u,fixed\r\n", dataConfig.accelRate, ACCEL_FREQUENCY_FOR_RATE(dataConfig.accelRate));
#else
            printf("RATE=%u,%u\r\n", dataConfig.accelRate, ACCEL_FREQUENCY_FOR_RATE(dataConfig.accelRate));
#endif
		}
        else if (strnicmp_rom(line, "watermark", 9) == 0)
        {
            unsigned char watermark = 0xff;
            if (line[9] != '\0') { watermark = (unsigned char)my_atoi(line + 10); }
            if (watermark != 0xff)
            {
				dataConfig.accelWatermark = watermark;
				DataConfigSave();
            }
            printf("WATERMARK=%u\r\n", dataConfig.accelWatermark);
        }
        else if (strnicmp_rom(line, "actThres", 8) == 0)
        {
            unsigned char value = 0xff;
            if (line[8] != '\0') { value = (unsigned char)my_atoi(line + 9); }
            if (value != 0xff)
            {
				dataConfig.activityThreshold = value; DataConfigSave();
            }
            printf("ACTTHRES=%u\r\n", dataConfig.activityThreshold);
        }
        else if (strnicmp_rom(line, "inactThres", 10) == 0)
        {
            unsigned char value = 0xff;
            if (line[10] != '\0') { value = (unsigned char)my_atoi(line + 11); }
            if (value != 0xff)
            {
				dataConfig.inactivityThreshold = value; DataConfigSave();
            }
            printf("INACTTHRES=%u\r\n", dataConfig.inactivityThreshold);
        }
        else if (strnicmp_rom(line, "actTime", 7) == 0)
        {
            unsigned char value = 0xff;
            if (line[7] != '\0') { value = (unsigned char)my_atoi(line + 8); }
            if (value != 0xff)
            {
				dataConfig.activityTime = value; DataConfigSave();
            }
            printf("ACTTIME=%u\r\n", dataConfig.activityTime);
        }
        else if (strnicmp_rom(line, "tranInt", 7) == 0)
        {
            unsigned long value = 0xffffffff;
            if (line[7] != '\0') { value = (unsigned long)my_atoi(line + 8); }
            if (value != 0xffffffff)
            {
				dataConfig.transmitInterval = value; DataConfigSave();
            }
            printf("TRANINT=%lu\r\n", dataConfig.transmitInterval);
        }
        else if (strnicmp_rom(line, "jitterMask", 10) == 0)
        {
            unsigned short value = 0xffff;
            if (line[10] != '\0') { value = (unsigned short)my_atoi(line + 11); }
            if (value != 0xffff)
            {
				dataConfig.jitterMask = value; DataConfigSave();
            }
            printf("JITTERMASK=%u\r\n", dataConfig.jitterMask);
        }
        else if (stricmp_rom(line, (const rom far char *)"exit") == 0)
        {
      		printf((const rom far char *)"NOTE: You'll have to close the terminal window yourself!\r\n");
        }
        else if (stricmp_rom(line, (const rom far char *)"AT") == 0)
        {
            ;
        }
        else if (line[0] == '\0')
        {
            printf((const rom far char *)"\r\n");
        }
        else
        {
            printf((const rom far char *)"ERROR: Unknown command: %s\r\n", line);
        }
    }
}



#else
static char dummy;	// C18 doesn't like empty object files
#endif
