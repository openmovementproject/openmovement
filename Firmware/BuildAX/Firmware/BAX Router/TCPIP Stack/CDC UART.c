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

// USB CDC debug out for TCPIP stack and C30
// Karim Ladha, 2013-2014

#include "TCPIPConfig.h"
#if defined(STACK_USE_UART)

#include "GenericTypeDefs.h"
#include "Usb/USB_CDC_MSD.h"
#include "TCPIP Stack/UART.h"


void WriteUART2(unsigned int data)
{
	#ifndef USB_INTERRUPT
	USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	#endif
	USBProcessIO();
    usb_putchar((unsigned char)data);
}

char DataRdyUART2(void)
{
	#ifndef USB_INTERRUPT
	USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	#endif
	USBProcessIO();
	return usb_haschar();
}

char BusyUART2(void)
{
	#ifndef USB_INTERRUPT
	USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	#endif
	USBProcessIO();
	return USBCDCBusy();
}

unsigned int ReadUART2(void)
{
	#ifndef USB_INTERRUPT
	USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	#endif
	USBProcessIO();
	return usb_getchar();
}

void putsUART2(unsigned int *buffer)
{
	#ifndef USB_INTERRUPT
	USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
	#endif
	USBProcessIO();
    usb_write(buffer, strlen((char*)buffer));
}

unsigned int getsUART2(unsigned int length,unsigned int *buffer,unsigned int uart_data_wait)
{
	unsigned short remaining = length;
	char* ptr = (char*)&buffer;
	signed int read;

	do{
		read = usb_getchar();
		if(read != -1)
		{
			remaining--;
			ptr ++;
		}
	}while(remaining && (uart_data_wait || read != -1));

	return length - remaining;	
}

BYTE ReadStringUART(BYTE *Dest, BYTE BufferLen)
{
	BYTE c;
	BYTE count = 0;

	if(BufferLen == 0) return 0;

	while(BufferLen--)
	{
		*Dest = '\0';

		while(!usb_haschar())
		{	
			#ifndef USB_INTERRUPT
			USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
			#endif
			USBProcessIO();
		}
		c = usb_getchar();

		if(c == '\r' || c == '\n')
			break;

		count++;
		*Dest++ = c;
	}

	return count;
}






#endif
