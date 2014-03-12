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

// BT serial interface (older API)
// Karim Ladha, 2013-2014

/* 
Written specifically for bleStack to add compatibility to older API
and to add c standard calls to make the bluetooth appear as a serial
output stream for printf, getc, putc etc.
*/ 

#include "TimeDelay.h"
#include "bt app.h"					
#include "bt serial.h"
#include "bt nvm.h"					
#include "bt config.h"
#include "debug.h"

// Buffer for get line - sets max command line length
unsigned char commEchoBt = FALSE;	
static char lineBuffer[BT_LINE_BUFFER_SIZE] = {0};
static int lineBufferLength = 0;

// Code
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

void BluetoothSerialIO(void)
{
	BTTasks();
}

void BluetoothSerialWait(void)
{
	// Timeout
	unsigned short timeout = 10000UL;

	if (bluetoothState != BT_CONNECTED) return;

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

void Bluetooth_write(const void *buffer, unsigned int len)
{
	unsigned short free, toSend, timeout = 0xffff;

	// Early out if not connected
	if ((btPresent == FALSE) || (bluetoothState != BT_CONNECTED)) return;

	// Look for outgoing fifo space
	free = FifoFree(&bt_out_fifo);

	// If not everything sent
	while ((free < len) && timeout--)
	{
		// Call event handler to send some if possible
		EventTrigger();
		// Check how much to send now 
		free = FifoFree(&bt_out_fifo);
		// See how much
		if(free == 0) 	continue;		// No space
		if(free < len) 	toSend = free;	// Not enough
		else {toSend = len;break;}		// Enough-break
		// Put it into fifo
		FifoPush(&bt_out_fifo,(void*)buffer,toSend);
		len -= toSend;	
		// Check disconnected
		if (bluetoothState != BT_CONNECTED) return;
	}

	// Enough space, characters added to fifo
	FifoPush(&bt_out_fifo,(void*)buffer,len);

	return;
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

// LEGACY OPPERATION from here

void BluetoothOff(void)
{
	BTDeinit();
	return;
}

void BluetoothSuspend(void)
{
	// KL: Note: The device is low power when initialised
	return;
}

unsigned char ReturnWhenSafeToClockSwitch(void)
{
	// First check we have bluetooth
	if (bluetoothState == BT_OFF) return TRUE;
	// Pause uart comms
	HciPhyPause();
	return TRUE;
}

void BluetoothAdaptToClockSwitch(void)
{
	if (bluetoothState == BT_OFF) return;
	// Adapt to the new clock 
	HciPhyAdaptClock(); 
	HciPhyPlay();
}

bt_power_state_t BluetoothGetPowerState(void)
{
	// Early out if not connected or initialised or failed
	if ((bluetoothState == BT_OFF)||(bluetoothState == BT_FAIL)) 
													bluetoothPowerState = BT_STANDBY;
	else if(bluetoothState == BT_CONNECTION_REQUEST)bluetoothPowerState = BT_ACTIVE;
	else if(phyClockNeededDuringSleep) 				bluetoothPowerState = BT_IDLE;
	else 											bluetoothPowerState = BT_SLEEP;
	return bluetoothPowerState;
}

unsigned short BluetoothGetState(void)
{
	return bluetoothState;
}

//EOF

