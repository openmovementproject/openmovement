/* 
 * Copyright (c) 2012-2013, Newcastle University, UK.
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

// Notes: This is the interface code to wake up the MCU and receive the data from the scales

// Include
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include <uart.h>
#include "HardwareProfile.h"
#include "Scales.h"
#include "WaveCap.h"

// Types

// Globals

unsigned char scales_buffer[SCALES_BUFFER_SIZE] = {0};
unsigned short scales_buffer_tail = 0;
volatile unsigned char gScalesDataReady = FALSE;
volatile scaleData_t gScaleData;

// Code

// Call to arm the MCU to receive data
void ScalesCommInit(void)
{
	// Clear interrupts
	SCALES_DATA_IF = 0;
	SCALES_DATA_IE = 0;
	SCALES_DATA_IP = 7;
	SCALES_WAKE_IE = 0;
	SCALES_WAKE_IF = 0;
	SCALES_WAKE_IP = 7;

	// Clear buffers
	memset(scales_buffer,0,sizeof(scales_buffer));

	OpenUART2 (
	/*UMODEvalue*/
	(UART_EN&
	UART_IDLE_CON&
	UART_IrDA_DISABLE&
	UART_DIS_WAKE&
	UART_DIS_LOOPBACK&
	UART_DIS_ABAUD&
	UART_UXRX_IDLE_ONE&
	UART_BRGH_FOUR&
	UART_NO_PAR_8BIT&
	UART_1STOPBIT&
	UART_MODE_SIMPLEX&
	UART_UEN_00),
	/*U1STAvalue*/
	(UART_INT_TX_LAST_CH&
	UART_TX_DISABLE&
	UART_INT_RX_CHAR&
	UART_ADR_DETECT_DIS&
	UART_RX_OVERRUN_CLEAR&
	UART_IrDA_POL_INV_ZERO&
	UART_SYNC_BREAK_DISABLED),
	/*baud*/
	SCALES_BRG);

	// Clear uart HW fifo
	while(U2STAbits.URXDA)
	{if (U2RXREG) U2STAbits.OERR = 0;}

	// Reset receiver vars
	scales_buffer_tail = 0;
	gScalesDataReady = FALSE;

	// Enable reception
	if (SCALES_WAKE == 1) 	SCALES_WAKE_IF = 1; // Set flag if already woken
	SCALES_WAKE_IE = 1;
}

void ScalesCommOff(void)
{
	// Turn off UART
	OpenUART2 (0,0,0);
	// Clear interrupts
	SCALES_DATA_IF = 0;
	SCALES_DATA_IE = 0;
	SCALES_WAKE_IE = 0;
	SCALES_WAKE_IF = 0;
}

// Call to sleep the MCU until data is received or until a wake event is encountered
// This could be the RTC interrupt for example, after the function returns the receiver 
// is still armed and can receive data. The call back function will be called if data is
// received.
void ScalesCommWait(void)
{	
	// If the scales have produced a data packet to send
	if (gScalesDataReady == TRUE)
	{
		return; 			// New data available, just return
	} 
	// If we are currently receiveing data from the device and there is incomplete / no data
	if (SCALES_DATA_IE || SCALES_WAKE == 1) 	
	{	
		Idle();				// Awaiting signals, idle to allow uart to continue to operate
	}	
	// Otherwise, if wake pin is low, just sleep
	else if (SCALES_WAKE == 0) 	
	{	
		SCALES_WAKE_IF = SCALES_WAKE;
		SCALES_WAKE_IE = 1; // Vectors when rising edge on is detected on wake pin
		Sleep();
	}
	else
	{
		Reset();			// Unknown case
	}	
}

void ScalesCommClear(void)
{
	ScalesCommInit();
}

// Wake interrupt - resets rx variables
void __attribute__((interrupt, shadow, auto_psv)) SCALES_WAKE_INT(void)
{
	LED_SET(LED_RED);
	// The device is now woken and uart will receive remaining edges
	ScalesCommClear();					// Reset receiver
	SCALES_WAKE_IF = 0;					// Clear the flag
	SCALES_WAKE_IE = 0;					// Clear the enable to prevent re-entry
	SCALES_DATA_IE = 1;					// Start uart reciever
}

// Data interrupt
void __attribute__((interrupt, shadow, auto_psv)) SCALES_DATA_INT(void)
{
	SCALES_DATA_IF = 0;
	while (U2STAbits.URXDA) 		// Read any required bytes into the rx fifo buffer
	{
		unsigned char inChar = U2RXREG;
		scales_buffer[scales_buffer_tail++] = inChar;
		scales_buffer_tail &= (SCALES_BUFFER_SIZE_MASK); 
		if (scales_buffer_tail >= SCALES_EXPECTED_PAYLOAD_LENGTH) 
		{
			gScalesDataReady = TRUE;
		}
	}
}

// Check if all bytes are received
unsigned char ScalesDataReady(void)
{
	return gScalesDataReady;
}

// If this is still waiting for >100ms then the receiver is locked up
unsigned char ScalesDataWaiting(void)
{
    if((SCALES_DATA_IE &&(gScalesDataReady==FALSE)))
    {
        if(SCALES_WAKE == 0) // Pins returned low
        {
             ScalesCommClear();
             return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}


// Call once you know data is in the buffer ready
unsigned char ScalesParseData(void)
{
	unsigned char i,retval = TRUE;
	unsigned char *ptr, checksum;

	// Try to correct erroneous byte misalignment
	ptr = scales_buffer;
	if(*ptr != 0x55)
	{
		DelayMs(100); // Let current partial packet finnish reception
		for(i=0;i<5;i++) // Look through the first 5 bytes for the code
		{
			if (*(++ptr) == 0x55) break;
		}
		if (i == 5) retval = FALSE; // Failed	
	}

	// Copy data to struct
	memcpy((unsigned char*)(void*)&gScaleData,ptr,SCALES_EXPECTED_PAYLOAD_LENGTH);
	ScalesCommInit(); // Free in buffer for next packet

	// TODO - figure out checksum here
	/*
	if checksum not ok retval = FALSE
	*/
	ptr = (unsigned char*)&gScaleData;
	checksum = 0;
	for(i=0;i<SCALES_EXPECTED_PAYLOAD_LENGTH;i++)
	{
		checksum ^=	 ptr[i];	
	}
	if (checksum != 0) 
	{
		retval = FALSE; // Failed checksum
	}

	// Check for sync code
	if (gScaleData.syncCode != 0x55) retval = FALSE;
	
	// Decode (kgs, lbs)
	gScaleData.weight = ((unsigned short)gScaleData.weightMSB<<8) | gScaleData.weightLSB;

	// Get time
	//TODO

	// Return result
	return retval;	
}
