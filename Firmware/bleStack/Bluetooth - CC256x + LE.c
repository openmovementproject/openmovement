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

// Standardised abstraction layer for Bluetooth devices
// Karim Ladha, 2013-2014

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "Compiler.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "TimeDelay.h"
#include "Bluetooth\Bluetooth.h" 	// The api for this C file
#include "BTApp.h"					// This is the only api it can use from bleStack
#include "bt config.h"
#include "debug.h"

#ifdef BT_USB_DEBUG
#include "Usb\USB_CDC_MSD.h"
#endif

// Timer specific variables
#ifdef USE_T1_FOR_TICK
	#include "Peripherals/Timer1.h"
#elif defined(USE_RTC_FOR_TICK)
	#include "Peripherals/Rtc.h" // For timeout
#else
	#warning "Default behaviour"
	#include "Peripherals/Rtc.h" // For timeout
#endif

// Stack specific variables
signed char btPresent = -1;       // maybe, not yet tried to init it
bt_state_t bluetoothState = BT_OFF;
bt_power_state_t bluetoothPowerState = BT_STANDBY;

// Buffer for get line - sets max command line length
unsigned char commEchoBt = FALSE;	
static char lineBuffer[BT_LINE_BUFFER_SIZE] = {0};
static int lineBufferLength = 0;

// Code
char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice)
{
	unsigned short i;
	unsigned short lastTime = 0xffff;
	char *name = GetBTName();
	char* pin = GetBTPin();
	char *mac = GetBTMAC();

	// If the btstack is not already initialised
	if (bluetoothState < BT_INITIALISED)
	{
		unsigned short OSCCON_save = OSCCON; 
		// PLL mode significantly speeds up startup times
		if(OSCCONbits.COSC != 0b001)
		{
			CLOCK_PLL();
			DelayMs(1);
		}

		// Init states
		bluetoothState = BT_STARTING;
		bluetoothPowerState = BT_ACTIVE;

		// In this C file the local name, pin and COD are all initialised to the defaults

		// Try to recover the nvm name if enabled
		#ifdef DEVICE_NAME_NVM
		ReadProgram((DEVICE_NAME_NVM), name, MAX_BT_NAME_LEN);
		for(i=0;i< MAX_BT_NAME_LEN;i++)
		{
			if (name[i] >= ' ' && name[i] < 0x7f) ; 	// Valid char
			else if (name[i] == '\0') break; 			// End of string
			else{i=0;break;	}							//Invalid char}	
		}
		if (i==0 || i== MAX_BT_NAME_LEN) 
		{
			memcpy(name,BT_DEFAULT_NAME,MAX_BT_NAME_LEN);								// Invalid entry
			WriteProgramPage_actual((DEVICE_NAME_NVM), name, MAX_BT_NAME_LEN);	// Write default name
		}
		else ; // Valid name read out of memory
		#endif
		
		// Has the user set the name - takes precedence
		if(deviceName != NULL)
		{
			unsigned short len = strlen(deviceName);
			if(len > MAX_BT_NAME_LEN)len = MAX_BT_NAME_LEN;
			memcpy(name,deviceName,len);
			name[len] = '\0';
		}

		// Try to recover the nvm pin code if nvm enabled
		#ifdef DEVICE_PAIRING_CODE
		ReadProgram((DEVICE_PAIRING_CODE), pin, 5);
		for(i=0;i<5;i++)
		{
			if (pin[i] >= '0' && pin[i] <= '9');// Valid char
			else if (pin[i] == '\0') break; 	// End of string
			else{i=0;break;	}					//Invalid char}	
		}
		if (i!=4) 
		{
			memcpy(pin,BT_DEFAULT_PIN,5);							// Invalid entry
			WriteProgramPage_actual((DEVICE_PAIRING_CODE), pin, 5);// Write default
		}
		else ; // Valid name read out of memory
		#endif

		#ifdef DEVICE_MAC_ADDRESS 	// Read MAC address
		ReadProgram(DEVICE_MAC_ADDRESS,mac, 7);
		#endif

		// Device name extensions
		{
			// Get remaining bytes in name
			unsigned short len = strlen(name); 
			if(deviceId == 0)
			{
				;// An id of zero prevents appending of if to name
			}
			#ifdef DEVICE_MAC_ADDRESS
			else if (deviceId == 0xffff)
			{
				// For an id of 0xffff we use the mac address bytes
				if ((MAX_BT_NAME_LEN-len) > 4) // If theres space to append 4 chars to the name
				{
					char *ptr = &mac[1];
					*(name+len++) = (((*ptr>>4)&0x000f)<0x0A)?('0'+((*ptr>>4)&0x000f)):('A'-0xA+((*ptr>>4)&0x000f));
					*(name+len++) = (((*ptr>>0)&0x000f)<0x0A)?('0'+((*ptr>>0)&0x000f)):('A'-0xA+((*ptr>>0)&0x000f));
					ptr = &mac[0];
					*(name+len++) = (((*ptr>>4)&0x000f)<0x0A)?('0'+((*ptr>>4)&0x000f)):('A'-0xA+((*ptr>>4)&0x000f));
					*(name+len++) = (((*ptr>>0)&0x000f)<0x0A)?('0'+((*ptr>>0)&0x000f)):('A'-0xA+((*ptr>>0)&0x000f));
					*(name+len++) = '\0';
				}	
			}
			#endif
			else 
			{
				// For any other device id, write the decimal out (if space)
				if ((MAX_BT_NAME_LEN-len) > 5) // If theres space to append a number to the name
				{
					*(name+len++) = '0' + (deviceId/10000)%10;
					*(name+len++) = '0' + (deviceId/1000)%10;
					*(name+len++) = '0' + (deviceId/100)%10;
					*(name+len++) = '0' + (deviceId/10)%10;
					*(name+len++) = '0' +  deviceId%10;
					*(name+len++) = '\0';
				}	
			}
		}
	
		// Set COD if non zero - set external variable
		if (classOfDevice) device_cod_local = classOfDevice;

		// Initialise module 
		BTInit(); // Sets up everything

		#ifdef BT_USB_DEBUG
			USBCDCWait();
		#endif

		while	((bluetoothState < BT_INITIALISED) 	&&
				(bluetoothState != BT_FAIL)	) // Wait for initalisation of hci layer
		{
			BTTasks();
			#ifdef BT_USB_DEBUG
				USBCDCWait();
			#endif
			// Wait upto 5s for inititalisation then fail
			if (lastTime == 0xffff) lastTime = rtcTicksSeconds;
		    if ((rtcTicksSeconds - lastTime) > 6)
		    	{bluetoothState = BT_FAIL;break;} // Timeout on init
		}

		// Check it worked
		if (bluetoothState >= BT_INITIALISED) btPresent = TRUE;
		else {btPresent = FALSE; BTDeinit();} // Power off hardware too

		// If the function was called with a different clock - try and switch it back
		if(bluetoothState >= BT_INITIALISED)
		{
			if(ReturnWhenSafeToClockSwitch())
			{
				switch ((OSCCON_save&0xf000)){
					case (0x1000): break; 					// Was already in pll mode
					case (0x0000): 							// Was in 8mhz int osc mode
					case (0x7000): CLOCK_INTOSC(); break; 	// Was in 8mhz int osc mode + clkdiv post scale (assume div by 1, 8MHz)
					default		 :{	asm("DISI #0x3FFF");// Unknown clock mode - just switch back
									OSCCON_save |= 0x01; 			// OSWEN enabled
									__builtin_write_OSCCONH( ((unsigned char*)  &OSCCON_save)[1] );
									__builtin_write_OSCCONL( ((unsigned char*)  &OSCCON_save)[0] );
									asm("DISI #0");
									while (OSCCONbits.OSWEN == 1);}	// Wait for switch						
				}
				BluetoothAdaptToClockSwitch();
			}
			else
			{
				bluetoothState = BT_FAIL;
			}
		}
	} // If bluetooth is already initialised then this just returns
	return btPresent;
}

void BluetoothOff(void)
{
	BTDeinit();
	bluetoothState = BT_OFF;	// Set state
	return;
}

void BluetoothSuspend(void)
{
	//TODO: Figure out to make the device low power but initialised
	// KL: Note: The device is low power when initialised
	return;
}

unsigned char ReturnWhenSafeToClockSwitch(void)
{
	// First check we have bluetooth
	if ((btPresent != TRUE) || (bluetoothState == BT_OFF)) return TRUE;
	// Pause uart comms
	HciPhyPause();
	return TRUE;
}

void BluetoothAdaptToClockSwitch(void)
{
	if ((btPresent != TRUE) || (bluetoothState == BT_OFF)) return;
	// Adapt to the new clock 
	HciPhyAdaptClock(); 
	HciPhyPlay();
}

bt_power_state_t BluetoothGetPowerState(void)
{
	// Early out if not connected or initialised or failed
	if ((btPresent == FALSE)||(bluetoothState == BT_OFF)||(bluetoothState == BT_FAIL)) bluetoothPowerState = BT_STANDBY;
	else if(bluetoothState == BT_CONNECTION_REQUEST)bluetoothPowerState = BT_ACTIVE;
	else if(phyClockNeededDuringSleep) 				bluetoothPowerState = BT_IDLE;
	else 											bluetoothPowerState = BT_SLEEP;
	return bluetoothPowerState;
}


unsigned short BluetoothGetState(void)
{
	return bluetoothState;
}

unsigned char BluetoothSendData(const void* source, unsigned short num)
{
	return RfcommSendData(source,num);
}

unsigned char BluetoothTxBusy(void)
{
	return RfcommTxBusy();
}

unsigned char BluetoothRxDone(void)
{
	// Not useful, can only say if a char has been received
	return Bluetooth_haschar();
}

// Bluetooth-specific character I/O
void Bluetooth_putchar(unsigned char v)
{
	FifoPush(&bt_out_fifo,&v,1);
	return;
}

int Bluetooth_getchar(void)
{
	int retval = 0;
	if (FifoLength(&bt_in_fifo) > 0) 
	{
		FifoPop(&bt_in_fifo, &retval , 1);
	}
	else
	{
		retval = -1;	
	}
	return retval;
}

char Bluetooth_haschar(void)
{
	char retval = FALSE;
	if (FifoLength(&bt_in_fifo)) 
	{
		retval = TRUE;
	}
	return retval;
}

void Bluetooth_write(const void *buffer, unsigned int len)
{
	unsigned short nowFree, free;

	// Early out if not connected
	if ((btPresent != TRUE) || (bluetoothState != BT_CONNECTED)) return;

	// Look for outgoing fifo space
	free = FifoFree(&bt_out_fifo);

	// If not enough space to send everything
	while (free < len)
	{
		// Try and send something
		if (free !=0)
		{
			FifoPush(&bt_out_fifo, (void*)buffer, free);
			len -= free;
			buffer += len;
			free = 0;
		}

		// Wait for some data to send - if not locked up
		BluetoothSerialWait();

		// Check for tx success
		nowFree = FifoFree(&bt_out_fifo);
		if (nowFree == 0) 	// No progress
			return;
		else free = nowFree;// Some progress

		// Break if connection lost
		if (bluetoothState != BT_CONNECTED) 
			return;
	}

	// Enough space, characters added to fifo
	FifoPush(&bt_out_fifo,(void*)buffer,len);
	return;
}

void BluetoothSerialIO(void)
{
	if ((btPresent == TRUE) && (bluetoothState != BT_OFF))
		BTTasks();
}

void BluetoothSerialWait(void)
{
	// Timeout
	unsigned short timeout = 10000UL;

	if ((btPresent == FALSE) || (bluetoothState != BT_CONNECTED)) return;

	// Call tasks until it sends
	while(timeout--)
	{
		BluetoothSerialIO();
		// Check if there is fifo spaces
		unsigned short free = FifoFree(&bt_out_fifo);
		// Break when no tx queued and fifo space available detected
		if ( !BluetoothTxBusy() && free)
			{break;}
		// Break if connection lost
		if (bluetoothState != BT_CONNECTED) 
			break;
	}
}

const char *Bluetooth_gets(void)
{
    unsigned char i;
    int c;

    for (i = 0; i < BT_IN_BUFFER_SIZE; i++)       // Maximum number of iterations (bails out early if no input)
    {
		c = Bluetooth_getchar();

		if (c <= 0) { break; }
        if (c == 13)                        // CR
        {
            if (commEchoBt && c != 10) { Bluetooth_putchar(c); }
            lineBuffer[lineBufferLength] = '\0';
            lineBufferLength = 0;
            if (commEchoBt) { Bluetooth_putchar(10); }
            return lineBuffer;
        }
        else if (c == 8 || c == 127)        // BS and DEL
        {
            if (lineBufferLength > 0)
            {
                if (commEchoBt && c != 10) { Bluetooth_putchar(8); Bluetooth_putchar(32); Bluetooth_putchar(8); }
                lineBufferLength--;
                lineBuffer[lineBufferLength] = '\0';
            }
        }
        else if (c == 10)                   // LF
        {
            ;   // ignore
        }
        else if (lineBufferLength + 1 < BT_LINE_BUFFER_SIZE)
        {
            if (commEchoBt && c != 10) { Bluetooth_putchar(c); }
            lineBuffer[lineBufferLength] = (char)c;
            lineBufferLength++;
            lineBuffer[lineBufferLength] = '\0';
        }
        else
        {
            if (commEchoBt && c != 10) { Bluetooth_putchar('\a'); }
            ;   // Discard (out of buffer)
        }
    }
    return NULL;
}

#endif
