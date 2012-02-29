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
 *
 * KL 2011
 */
 
#include "HardwareProfile.h"
#include "Compiler.h"
#include "TimeDelay.h"
#include <stdint.h>
#include <uart.h>
#include "btstack\hal_uart_dma.h"
#include "btstack\hal_cpu.h"

/*Hacked to tune oscillator to suit high baud rates at low clocks*/
#if (((GetInstructionClock()) == 4000000)||((GetInstructionClock()) == 16000000))
	#define SETBAUD_57600()  		{if(OSCCONbits.COSC==1){BT_BAUD_REG=68	/*68.4*/;}else{BT_BAUD_REG=16;	OSCTUN=-6;	/*16.36*/}}
	#define SETBAUD_115200() 		{if(OSCCONbits.COSC==1){BT_BAUD_REG=34	/*33.7*/;}else{BT_BAUD_REG=8;	OSCTUN=9;	/*7.681*/}}
	#define SETBAUD_921600() 		{if(OSCCONbits.COSC==1){BT_BAUD_REG=3	/*3.34*/;}else{BT_BAUD_REG=0;	OSCTUN=-23;	/*0.085*/}}
	#define SETBAUD_1000000() 		{if(OSCCONbits.COSC==1){BT_BAUD_REG=3	/*3.00*/;}else{BT_BAUD_REG=0;	OSCTUN=0;	/*0.000*/}}
#else
	#error "Add baud rate BRG settings"
#endif

extern void hal_cpu_set_uart_needed_during_sleep( unsigned char); 

void dummy_handler(void){};

//Globals
// rx state
static volatile uint16_t  uart_bytes_to_read = 0xffff;
static volatile uint8_t * uart_rx_buffer_ptr = NULL;

// tx state
static volatile uint16_t  uart_bytes_to_write = 0xffff;
static volatile uint8_t * uart_tx_buffer_ptr = NULL;

// handlers
static void (*rx_done_handler)(void) = dummy_handler;
static void (*tx_done_handler)(void) = dummy_handler;
static void (*cts_irq_handler)(void) = dummy_handler;

void hal_uart_dma_init(void)
{
// Turn on the UART to the BT module 
// Actual clock frequency is only 8MHz (4MIPS)
// UxBRG = ((Fcy/Baud)/4)-1, 4MIPS, 57600->16.36*/
	BT_INIT_PINS(); // Sets RTS high

	// Turn on the modules slow clock
	SETUP_BT_SLW_CLK();
	DelayMs(10);

	OpenUART1 (
		/*U1MODEvalue*/
		(UART_EN&
		UART_IDLE_STOP&
		UART_IrDA_DISABLE&
		UART_DIS_WAKE&
		UART_DIS_LOOPBACK&
		UART_DIS_ABAUD&
		UART_UXRX_IDLE_ONE&
		UART_BRGH_FOUR&
		UART_NO_PAR_8BIT&
		UART_1STOPBIT&
		UART_MODE_FLOW&
		UART_UEN_10),
		/*UART_UEN_00),*/
		/*U1STAvalue*/
		(UART_INT_TX_LAST_CH&
		UART_TX_ENABLE&
		UART_INT_RX_CHAR&
		UART_ADR_DETECT_DIS&
		UART_RX_OVERRUN_CLEAR&
		UART_IrDA_POL_INV_ZERO&
		UART_SYNC_BREAK_DISABLED),
		/*baud*/
		0);
	SETBAUD_115200(); 

	// Turn off/on to reset the BT module
	BT_EN = 0;
	DelayMs(10);
	BT_EN = 1;

	// Wait for module to power up
	DelayMs(10);

	// Clear any pending ints and enable RX/TX
	IPC2bits.U1RXIP = 7;
	BT_UART_RX_IF = 0;
	BT_UART_RX_IE =0;
	IPC3bits.U1TXIP = 7;
	BT_UART_TX_IF = 0;
	BT_UART_TX_IE =0;

	// Clear uart fifo
	while(U1STAbits.URXDA)
	{if (U1RXREG) U1STAbits.OERR = 0;}
}


int hal_uart_dma_set_baud(uint32_t baud){
	int result = 0;

    switch (baud){
        case 4000000:
            result = -1;
            break;
        case 3000000:
            result = -1;
            break;
        case 2400000:
             result = -1;
            break;
        case 2000000:
            result = -1;
            break;
        case 1000000:
			SETBAUD_1000000(); 
            break;
        case 921600:
			SETBAUD_921600(); 
            break;
         case 115200:
			SETBAUD_115200(); 
            break;
        case 57600:
			SETBAUD_57600(); 
            break;
        default:
            result = -1;
            break;
    }
	return result;
}

void hal_uart_dma_set_block_received( void (*the_block_handler)(void)){
  	rx_done_handler = the_block_handler;
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void)){
    tx_done_handler = the_block_handler;
}

void hal_uart_dma_set_csr_irq_handler( void (*the_irq_handler)(void)){
        cts_irq_handler = the_irq_handler;
}

/**********************************************************************/
/**
 * @brief  Disables the serial communications peripheral and clears the GPIO
 *         settings used to communicate with the BT.
 * 
 * @param  none
 * 
 * @return none
 **************************************************************************/
void hal_uart_dma_shutdown(void) {
	BT_UART_RX_IE=0;	// Interrupts off
	BT_UART_TX_IE=0;
}


void hal_uart_dma_send_block(const uint8_t * data, uint16_t len){ 
	LED_G=1;
	BT_UART_TX_IE=0;			// Stop current transmission if active
    uart_tx_buffer_ptr = (uint8_t *) data; 
    uart_bytes_to_write = len;
	BT_UART_TX_IF=1;			// Set flag, interrupt deals with transmission	
	BT_UART_TX_IE=1;			// Interrupt will fire once byte is clocked out too
}

// int used to indicate a request for more new data
void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len){
	LED_R=1;
	BT_UART_RX_IE=0;			// Stop current rx opperations
    uart_rx_buffer_ptr = buffer;
    uart_bytes_to_read = len;
	BT_UART_RX_IF=1; 			// SW fire int, CTS cleared in isr
	BT_UART_RX_IE=1;			// re-enable rx opperations
}

// Indicate whether uart should be active in sleep
void hal_uart_dma_set_sleep(uint8_t sleep){
    hal_cpu_set_uart_needed_during_sleep(!sleep);    
}

// CTS ISR
void _ToDo_(void)
{
	BT_UART_TX_IF = 1;  			// Trigger any pending transmission
  	(*cts_irq_handler)();			// This is never set by the btstack?	
}

// TX BYTE ISR
void __attribute__((interrupt,auto_psv)) _U1TXInterrupt(void) 
{
	BT_UART_TX_IF=0;						// Clear flag
	if ((uart_bytes_to_write)&&(uart_tx_buffer_ptr!=NULL))// If one or more bytes is left to send
	{
		BT_UART_TX_REG = *uart_tx_buffer_ptr++;	// Send a byte
        uart_bytes_to_write--;					// decrease count
		return;
	}
    if (uart_bytes_to_write == 0)
	{
		LED_G=0;
        BT_UART_TX_IE=0;  					// disable TX interrupts
		(*tx_done_handler)();				// call the done handler
		return;
    }   
}

// RX BYTE ISR
void __attribute__((interrupt,auto_psv)) _U1RXInterrupt(void) 
{
	BT_UART_RX_IF = 0;						// Clear flag
	BT_CTS = 1;								// Halt receiver
	while ((U1STAbits.URXDA) && (uart_bytes_to_read>0)&&(uart_rx_buffer_ptr!=NULL)) // Read any required bytes into the rx buffer
	{
		*uart_rx_buffer_ptr++ = (uint8_t)BT_UART_RX_REG; 	// Read the byte off the fifo
        if(--uart_bytes_to_read == 0)						// Was it the last one requested?
		{
			LED_R=0;
			(*rx_done_handler)();			// call done handler
			return;
		}
	}
	if (uart_bytes_to_read>0)BT_CTS = 0;	// Resume reception
}

