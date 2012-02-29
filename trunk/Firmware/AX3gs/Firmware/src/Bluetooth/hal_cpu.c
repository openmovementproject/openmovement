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
 * KL 2011
 */
#include "HardwareProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include <stdint.h>
#include "btstack\hal_cpu.h"

//Globals
unsigned char uartNeededDuringSleep = FALSE;

static int ipl_shadow = -1;

void hal_cpu_disable_irqs(){
ipl_shadow = SRbits.IPL;
SRbits.IPL = (BT_UART_INT_PRIORITY-1); // User interrupts off - allows UART Interrupts to carry on
}

void hal_cpu_enable_irqs(){
if (ipl_shadow>0) SRbits.IPL = ipl_shadow; // Restore
}

void hal_cpu_set_uart_needed_during_sleep(uint8_t enabled){ // Never called?
	uartNeededDuringSleep = enabled;
}

void hal_cpu_enable_irqs_and_sleep(){
	LED_G = 1;
	while(1) // False sleep - normal operation is wake on interrupt
	{
		if(	BT_UART_RX_IF|
			BT_UART_TX_IF|
			IFS4bits.U1ERIF|
			IFS0bits.T1IF|
			!BT_RTS)
				{break;}
	}
	if (ipl_shadow>0) SRbits.IPL = ipl_shadow; // Restore priority
	LED_G = 0;
}


