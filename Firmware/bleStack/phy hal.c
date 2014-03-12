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

// Bluetooth physical layer hardware abstraction layer
// Karim Ladha, 2013-2014

// Headers
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include <uart.h>
#include "Initialisation CC256x.h"
#include "phy hal.h"
#include "bt config.h"
#include "debug.h"

// DEBUG
// This can not be debugged using the normal output since it is timing sensitive
#if (DEBUG_PHY_EHCILL )
	static const char* file = "phyhal.c";
	#define DBG_INFO(X)			DBG_fast(X,4)
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__)
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
#elif(DBG_ALL_ERRORS)
	static const char* file = "phyhal.c";
	#define DBG_INFO(X)
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__)
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
#else
	#define DBG_INFO(X)
	#define DBG_ERROR(X, ...) 		
	#define ASSERT(X)			
#endif

// DEFINITIONS
#if (defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB004__))
	#ifndef GetPeripheralClock
		#define GetPeripheralClock() (GetInstructionClock()) /*No scaler for this processor*/
	#endif
	// These are valid for all pic24fj256gb106 hardware using intosc (+/- PLL) at 8 or 16 MIPs
	#define SETBAUD_57600()  		{if(OSCCONbits.COSC==0b001){BAUD_REG=68	/*68.4*/;}else if((OSCCONbits.COSC==0b000)||(OSCCONbits.COSC==0b111)){BAUD_REG=16;}}
	#define SETBAUD_115200() 		{if(OSCCONbits.COSC==0b001){BAUD_REG=34	/*33.7*/;}else if((OSCCONbits.COSC==0b000)||(OSCCONbits.COSC==0b111)){BAUD_REG=8;}}
	#define SETBAUD_921600() 		{if(OSCCONbits.COSC==0b001){BAUD_REG=3	/*3.34*/;}else if((OSCCONbits.COSC==0b000)||(OSCCONbits.COSC==0b111)){BAUD_REG=0;}}
	#define SETBAUD_1000000() 		{if(OSCCONbits.COSC==0b001){BAUD_REG=3	/*3.00*/;}else if((OSCCONbits.COSC==0b000)||(OSCCONbits.COSC==0b111)){BAUD_REG=0;}}
#else
	#error "Add baud rate BRG settings"
#endif

#define CURRENT_SCOPE_PRIORITY	(SRbits.IPL)	// Current priority of the scope, interrupts equal to or below will not be serviced

#ifdef BT_UART_USES_UART_1
	#define BT_UART_UART_ENABLE	U1MODEbits.UARTEN
	#define BT_UART_RX_IP			IPC2bits.U1RXIP
	#define BT_UART_RX_IF			IFS0bits.U1RXIF  
	#define BT_UART_RX_IE			IEC0bits.U1RXIE
	#define BT_UART_TX_IP			IPC3bits.U1TXIP
	#define BT_UART_TX_IF			IFS0bits.U1TXIF
	#define BT_UART_TX_IE			IEC0bits.U1TXIE
	#define BT_UART_ER_IF			IFS4bits.U1ERIF
	#define BT_UART_ER_IE			IEC4bits.U1ERIE
	#define BAUD_REG				U1BRG
	#define BT_UART_TX_REG			U1TXREG
	#define BT_UART_RX_REG 			U1RXREG
	#define OPEN_BT_UART 			OpenUART1
	#define BT_UART_DATA_AVAILABLE	U1STAbits.URXDA
	#define BT_UART_RX_OVERFLOW	U1STAbits.OERR
	#define BT_UART_TX_INTERRUPT	_U1TXInterrupt
	#define BT_UART_RX_INTERRUPT	_U1RXInterrupt
	#define BT_UART_ER_INTERRUPT	_U1ErrInterrupt
	#define BT_UART_USTA			U1STA
#elif defined (BT_UART_USES_UART_2)
	#define BT_UART_UART_ENABLE	U2MODEbits.UARTEN
	#define BT_UART_RX_IP			IPC7bits.U2RXIP
	#define BT_UART_RX_IF			IFS1bits.U2RXIF  
	#define BT_UART_RX_IE			IEC1bits.U2RXIE
	#define BT_UART_TX_IP			IPC7bits.U2TXIP
	#define BT_UART_TX_IF			IFS1bits.U2TXIF
	#define BT_UART_TX_IE			IEC1bits.U2TXIE
	#define BT_UART_ER_IF			IFS4bits.U2ERIF
	#define BT_UART_ER_IE			IEC4bits.U2ERIE
	#define BAUD_REG				U2BRG
	#define BT_UART_TX_REG			U2TXREG
	#define BT_UART_RX_REG 			U2RXREG
	#define OPEN_BT_UART 			OpenUART2
	#define BT_UART_DATA_AVAILABLE	U2STAbits.URXDA
	#define BT_UART_RX_OVERFLOW	U2STAbits.OERR
	#define BT_UART_TX_INTERRUPT	_U2TXInterrupt
	#define BT_UART_RX_INTERRUPT	_U2RXInterrupt
	#define BT_UART_ER_INTERRUPT	_U2ErrInterrupt
	#define BT_UART_USTA			U2STA
#endif

// TYPES
// GLOBALS
volatile unsigned char 	blockReceived = FALSE_;
volatile unsigned char 	blockSent = FALSE_;
volatile unsigned char 	phyClockNeededDuringSleep = TRUE_;
// Buffers
unsigned char eventBuffer[MAX_EVENT_PACKET_LENGTH]; 	// Incoming
unsigned char aclInDataBuffer[MAX_IN_ACL_LENGTH];		// Incoming
// Private
echill_t ehcill_state = RADIO_AWAKE;
void (*TransferCompleteCB)(void) = NULL;
static volatile unsigned char 	firstReceivedByte = 0;
static volatile unsigned char 	firstSentByte = 0;
static volatile unsigned char* 	uartRxBuffer = NULL;
static volatile unsigned char* 	uartTxBuffer = NULL;
static volatile unsigned short 	uartTxLen = 0;
static volatile unsigned long 	gBaud = BT_UART_DEFAULT_BAUD; 		// Reset value should be 115200 for most bt modules

// Private Prototypes
static inline void UartInit(void);
static inline void UartOff(void); 
static inline void UartPutc(unsigned char c);
static inline void UartWaitTx(void);
static inline void UartWaitRx(void);
static inline void UartWaitRxStop(void);
static inline void UartWaitRxGo(void);
static inline void EhcillHandler(void);
static inline void UartRxHandler(void);
static inline void UartTxHandler(void);

// Code
unsigned char PhyPowerUp(void)			
{
	unsigned short result = 0;
	// Initialise and reset module
	BT_INIT_PINS(); // Held in reset
	// Turn on the modules slow clock
	SETUP_BT_SLW_CLK();
	// Power module
	BT_EN = 1; 		// Out of reset
	DelayMs(1);
	// Start comms at default baud
	UartInit();
	UartRxIdle();
	// If there is an init script, send it
	#if defined( BT_CHIPSET_CC2564) || defined (BT_CHIPSET_CC2560)
	result = CC256x_Startup();
	switch(result)				
	{
		case(0):{break;}
		default:DBG_ERROR("Init! %02X\n",result);break;
//		case(1):DBG_ERROR("Fail on reset cmd\n");break;
//		case(2):DBG_ERROR("Fail on baud cmd\n");break;
//		case(3):DBG_ERROR("Chipset mismatch\n");break;
//		case(4):DBG_ERROR("Fail on init script\n");break;
//		default:DBG_ERROR("Fail on startup\n");break;
	}
	#else
	{
		// Other chipsets just need a reset command
		const unsigned char cmd_reset[]	=	{0x01,0x03,0x0c,0x00};
		PhyReceiveBlock();
		PhySendBlock(&cmd_reset[1],cmd_reset[3]+3,cmd_reset[0]);	
	}
	#endif
	// Zero flags
	//blockReceived = FALSE_; // DO NOT CLEAR (we are expecting a reset complete resp)
	blockSent = FALSE_;
	phyClockNeededDuringSleep = TRUE_;
	// Enable handlers
	BT_eHCILL_HANDLER_FLAG = 0; 	// Reset will have set the flag
	BT_eHCILL_HANDLER_ENABLE = 1;
	return (result == 0);
}

unsigned char PhySupportsLe(void)
{
	return (chipsetId == 2564);
}

void PhyInstallEventHandlerCB(void (*CallBack)(void))
{
	TransferCompleteCB = CallBack;		
}

void PhyPowerOff(void)			
{
	// Stop comms - quickly kill interrupts
	BT_eHCILL_HANDLER_ENABLE = 0;
	UartOff();
	// Power off module
	BT_INIT_OFF();
	// Turn on the modules slow clock
	STOP_BT_SLW_CLK();
	return;
}

// Use to fire interrupts manually (to send data out or poll timeout)
void EventTrigger(void)
{
	BT_EVENT_HANDLER_FLAG = 1;		
}

void UartRxIdle(void)	// Re-synchronises receiver by dumping incoming data
{
	while(BT_UART_TX_IE);	// Allow sends to complete
	BT_UART_RTS = 0; 		// Unblock receiver
	while(BT_UART_RX_OVERFLOW || (BT_UART_DATA_AVAILABLE) || (!(BT_UART_USTA&0x10))) // While: overflowed || data available || recieve not idleing
		{if (BT_UART_RX_REG) BT_UART_RX_OVERFLOW = 0;DelayMs(1);} // Dump incoming data
	BT_UART_RTS = 1; 		// Re-block
	BT_UART_RX_IE = 0;		// Re-block 
	blockReceived = FALSE_;	// Clear flag
	firstReceivedByte = 0; 	// Resync on next byte
}

unsigned char UartSetBaud(unsigned long baud)
{
	int result = 0;
	if (baud!=0) gBaud = baud;
	// Otherwise it will use the last one, use to reset baud for new clock

    switch (gBaud){
        case 4000000:
            result = -1;
            break;
        case 1000000:
			SETBAUD_1000000(); 
            break;
        case 115200:
			SETBAUD_115200(); 
            break;
        default:
			result = -1; 
            break;
    }
	return result;
}

void UartSafeSleep(void)
{
	// Only sleep if radio is asleep
	asm("DISI #0x3FFF"); // Remove possible race conditions
	Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop(); // Wait latency
	if(phyClockNeededDuringSleep == FALSE_)	{Sleep();}
	else 									{Idle();}
	asm("DISI #0x0"); // Allow vectors	
}

static unsigned short IPLpause = 0; // IPL shadow during pause/play
void UartPause(void)
{
	// Block 
	IPLpause = CURRENT_SCOPE_PRIORITY;
	CURRENT_SCOPE_PRIORITY = BT_UART_RX_INT_PRIORITY;
	if(BT_UART_RX_IE)				// If receiver is active
	{
		BT_UART_RTS = 1;Delay10us(1); 	// Block and wait setup time
		while(!(BT_UART_USTA&0x10));	// If currently receiving a byte, wait on it
	}
	return;
}

void UartPlay(void)
{
	// Un-block
	if(BT_UART_RX_IE)					// If we were receiving
		BT_UART_RX_IF = 1;				// Unblocked in ISR
	CURRENT_SCOPE_PRIORITY = IPLpause;
}

void PhySendBlock(const unsigned char* data, unsigned short len, unsigned char header)
{
	UartWaitTx();						// Wait for current transmit
	UartWaitRxStop();					// Stop receiver - elevate ipl
	// Rx/Tx/Cts is blocked but module may be cts waking us as well - ok
	if(blockReceived == HCI_HCILL_BLOCK)// eHCILL signal received (goto sleep)
		EhcillHandler();				// Handle it (will send sleep ack)
	if (ehcill_state != RADIO_AWAKE)	// Could be	CTS woken
	{
		if (ehcill_state == RADIO_ASLEEP)	// Radio asleep
		{
			ASSERT(BT_UART_RTS);			// Should be blocking if asleep
			ASSERT(blockReceived == FALSE_);	// Can't have received in sleep
			phyClockNeededDuringSleep = TRUE_;// Can't sleep while sending
			DBG_INFO("T32\n");				// Debug out
			UartPutc(HCILL_WAKE_UP_IND);	// Send wake indicator
			ehcill_state = WAKE_IND_SENT; 	// Awaiting ack
			firstReceivedByte = 0;			// We don't know what the packet may be yet
			blockReceived = FALSE_;			// Clear flag
			uartRxBuffer = NULL;			// Pointer switched on byte 0
			BT_UART_RX_IE = 1;				// re-enable rx opperations
		}	
		else ASSERT(ehcill_state==CTS_WOKEN);// Check, only possible state
		UartWaitRx();						// Wait for wake ack/ind
		ASSERT(blockReceived == HCI_HCILL_BLOCK);
		EhcillHandler();					// We are now awake (may send ack)
	}
	ASSERT(ehcill_state == RADIO_AWAKE);// Should be awake
	blockSent = FALSE_;					// Now we can send the data out
    uartTxBuffer = (unsigned char*)data;// Set source 
    uartTxLen = len;					// Set length
	firstSentByte = header;				// Set the packet type flag
	BT_UART_TX_IF = 0;					// Interrupt flag must be clear 
	BT_UART_TX_REG = header;			// This will set the flag once the header byte sends
	BT_UART_TX_IE = 1;					// Interrupt will fire and send subsequent chars
	if(BT_UART_RX_IE)					// If we were receiving
		{BT_UART_RX_IF = 1;}			// Unblocked in ISR
	UartWaitRxGo();
	return;
}
void PhySendWait(void)					// In case you need to guarantee the send completes
{
	UartWaitTx();
}
void PhyReceiveBlock(void)
{
	ASSERT(blockReceived == FALSE_); // Last packet delt with
	ASSERT(!BT_UART_RX_IE);			// Not waiting on packet
	BT_UART_RX_IE = 0;			// halt rx opperations, should be off already
	firstReceivedByte = 0;		// We don't know what the packet may be yet
	blockReceived = FALSE_;		// Clear flag
	uartRxBuffer = NULL;		// Pointer switched on byte 0
	BT_UART_RX_IF = 1; 			// SW fire int, RTS cleared in isr
	BT_UART_RX_IE = 1;			// re-enable rx opperations
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE // PRIVATE // PRIVATE // PRIVATE // PRIVATE // PRIVATE // PRIVATE // PRIVATE//
/////////////////////////////////////////////////////////////////////////////////////////

// Initialise uart
static inline void UartInit(void)
{
	// Make sure its all off first
	UartOff();

	OPEN_BT_UART (
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

	blockReceived = FALSE_;
	blockSent = FALSE_;
	uartRxBuffer = NULL;
	uartTxBuffer = NULL;
	uartTxLen = 0;
	gBaud = BT_UART_DEFAULT_BAUD; // Reset value of most bt modules

	UartSetBaud(gBaud); // Should have been inited

	// Clear any pending ints and enable RX/TX
	BT_UART_RX_IP = BT_UART_RX_INT_PRIORITY;
	BT_UART_RX_IF = 0;
	BT_UART_RX_IE =0;
	BT_UART_TX_IP = BT_UART_TX_INT_PRIORITY;
	BT_UART_TX_IF = 0;
	BT_UART_TX_IE =0;
	BT_eHCILL_HANDLER_PRIORITY = BT_UART_CTS_INT_PRIORITY;	
	BT_EVENT_HANDLER_FLAG = 0;							
	BT_eHCILL_HANDLER_ENABLE = 0;	

	// To check for uart glitches
	#ifdef __DEBUG
	BT_UART_ER_IF = 0;
	BT_UART_ER_IE = 1;
	#endif

	// Clear uart fifo
	while(BT_UART_DATA_AVAILABLE)
	{if (BT_UART_RX_REG) BT_UART_RX_OVERFLOW = 0;}
	return;
}

static inline void UartOff(void) 
{
	OPEN_BT_UART(0,0,0xffff);
	BT_UART_RX_IE=0;	
	BT_UART_TX_IE=0;
	return;
}

static inline void UartWaitTx(void)
{
	while(BT_UART_TX_IE)			// Wait for current tx
	{
		if(CURRENT_SCOPE_PRIORITY >= BT_UART_TX_INT_PRIORITY)
			if(BT_UART_TX_IF)
				UartTxHandler();
	}
}

static unsigned short IPLstop = 0; 	// IPL shadow during uart stop
static inline void UartWaitRxStop(void)
{
	IPLstop = CURRENT_SCOPE_PRIORITY;
	CURRENT_SCOPE_PRIORITY = BT_UART_RX_INT_PRIORITY;
	BT_UART_RTS = 1;Delay10us(1); 	// Block and wait setup time
	if(	(!BT_UART_DATA_AVAILABLE)&&	// Rx not started
		((BT_UART_USTA&0x10)))		// Rx idle
			return;					// Successfully stopped
	else							// Mid byte/block
	{
		UartWaitRx();				// Wait for this block
		Nop();
	}
	return;
}
static inline void UartWaitRx(void)	// Manual receive one block after stopped
{
	ASSERT(CURRENT_SCOPE_PRIORITY >= BT_UART_RX_INT_PRIORITY);
	BT_UART_RX_IF = 1;			// Flag set
	while(BT_UART_RX_IE)		// Wait for current RX to end
	{
		if(BT_UART_RX_IF)
			UartRxHandler();	// First call lowers RTS
	}							// Reciever now has a block
}
static inline void UartWaitRxGo(void) // Re-start receive
{
	CURRENT_SCOPE_PRIORITY = IPLstop;
}

static inline void UartPutc(unsigned char c)
{
	UartWaitTx();		// Wait for current packet to send
	BT_UART_TX_IF = 0;	// Clear flag
	BT_UART_TX_REG = c;	// Send char
	while(!BT_UART_TX_IF); // Wait for send
	BT_UART_TX_IF = 0;	// Clear flag
}

// Interrupt handlers
// TX BYTE ISR
void __attribute__((interrupt,auto_psv)) BT_UART_TX_INTERRUPT(void)
	{UartTxHandler();}
static inline void UartTxHandler(void)
{
	BT_UART_TX_IF=0;						// Clear flag
	if ((uartTxLen)&&(uartTxBuffer!=NULL))	// If one or more bytes is left to send
	{
		BT_UART_TX_REG = *uartTxBuffer++;	// Send a byte
        uartTxLen--;						// Decrease count
		return;								// Return
	}
    if (uartTxLen == 0)						// If no bytes to send
	{
        BT_UART_TX_IE=0;  					// Disable TX interrupts
		blockSent = firstSentByte;			// Command/acl data block
		EventTrigger();						// Trigger done signal
		return;
    }   
}

// RX BYTE ISR
void __attribute__((interrupt,auto_psv)) BT_UART_RX_INTERRUPT(void) 
	{UartRxHandler();}
static inline void UartRxHandler(void)
{
	BT_UART_RX_IF = 0;						// Clear flag
	while (BT_UART_DATA_AVAILABLE) // Get every byte until complete
	{
		BT_UART_RTS = 1;						// Halt receiver
		static unsigned short byteNum = 0;
		static unsigned short lastByte = 0;
		static volatile unsigned char inChar = 0;

		inChar = BT_UART_RX_REG;
		if (firstReceivedByte == 0)				// First byte of a packet, set correct buffer pointer
		{	
			byteNum = 0;						// Next byte will be treated as byte 1
			lastByte = 0;						// Will be set below
			if	(inChar == HCI_EVENT_BLOCK)	
				{firstReceivedByte = HCI_EVENT_BLOCK; uartRxBuffer = eventBuffer;}
			else if(inChar == HCI_ACL_BLOCK)	
				{firstReceivedByte = HCI_ACL_BLOCK; uartRxBuffer = aclInDataBuffer;}
			else 			
			{	// Other types delt with in event handler
				ASSERT(blockReceived==FALSE_);
				uartRxBuffer = NULL;				// Clear pointer
				firstReceivedByte = inChar;			// either glitch or eHCILL signalling char received
				blockReceived = HCI_HCILL_BLOCK;	// Set done flag
				BT_UART_RX_IE = 0;					// Call recieve block to get the next one
				BT_eHCILL_HANDLER_FLAG = 1;			// Handle eHCILL sent events in dedicated isr
				return;								// Return and block
			}
		}
		else if (uartRxBuffer != NULL)
		{
			*uartRxBuffer++ = inChar;			// Pop it to the buffer location
			byteNum++;							// Increment count

			if (byteNum == 2 && firstReceivedByte == 0x04){	// Event packet total length
				lastByte = inChar + 2;
			}

			else if (byteNum == 4 && firstReceivedByte == 0x02){	// Data packet total length
				lastByte = inChar;				
				lastByte = (lastByte<<8)+(*(uartRxBuffer-2))+4;
			}

			else if ((byteNum==lastByte) && (lastByte!=0)){	// Full data/event block received		
				ASSERT(blockReceived==FALSE_);
				uartRxBuffer = NULL;
				blockReceived = firstReceivedByte;	// Set done flag
				BT_UART_RX_IE = 0;					// Call recieve block to get the next one
				EventTrigger();						// Trigger done signal, RTS left blocking
				return;								// Return and block
			}
		}
		else 
		{
			ASSERT(0);
		}
	}
	BT_UART_RTS = 0;								// Resume reception
	return;
}

// HANDLE EHCILL EVENTS
void __attribute__((interrupt,auto_psv)) BT_eHCILL_HANDLER_VECTOR(void)	
	{EhcillHandler();}
static inline void EhcillHandler(void)
{
	BT_eHCILL_HANDLER_FLAG = 0;		// Clear flag
	DBG_INFO( "\rEH:");				// Debug

	switch(ehcill_state){

		case(RADIO_AWAKE):{					// Awaiting sleep signal
			if	(firstReceivedByte==HCILL_GO_TO_SLEEP_IND)	{DBG_INFO( "R30\n");}
			else 	{DBG_INFO( "E00\n");break;} // Glitch or cts pin toggled 
			/*NOTE: This is due to errata on CC2564*/
			unsigned short i;for(i=0;i<500;i++){Nop();}
			/***************************************/
			ehcill_state = GOTO_SLEEP_RXED; // Now send ack
			DBG_INFO( "T31\n");				// Debug out
			UartPutc(HCILL_GO_TO_SLEEP_ACK);// Send ack
			ehcill_state = GOTO_SLEEP_ACK_SENT;// Asleep
			DBG_INFO( "zzz\n");				// Debug out
			phyClockNeededDuringSleep=FALSE_;// Inform higher layers we are asleep
			blockReceived = FALSE_;
			break;
		}	

		case(RADIO_ASLEEP):{ 				// Wake up signal 
			if(blockReceived == HCI_HCILL_BLOCK) // False interrupt
				{DBG_INFO( "E01\n");break;}
			// Await clock stabilisation (will have woken from sleep)
			if (OSCCONbits.COSC == 0b001)	// If the pll is on, give it extra time to wake
				{while (OSCCONbits.LOCK != 1){;}Delay10us(50);}
			Delay10us(50);	
			DBG_INFO( "CTS\n");				// Debug out
			phyClockNeededDuringSleep=TRUE_;// Set clock needed
			ehcill_state = CTS_WOKEN;		// Set new state
			blockReceived = FALSE_;			// Clear flag
			PhyReceiveBlock(); 				// Re-enable receiver (for wake ind)
			break;
		}

		case(CTS_WOKEN):{					// Awaiting wake ind
			if		(firstReceivedByte==HCILL_WAKE_UP_IND)	DBG_INFO( "R32\n");
			else 	{DBG_INFO( "E02\n");break;} // Glitch or cts pin toggled twice (observed on scope)
			DBG_INFO( "T33\n");				// Debug here to preserve ordering
			ehcill_state = WAKE_IND_RXED; 	// Wait for ack to send
			UartPutc(HCILL_WAKE_UP_ACK);	// Send ack
			ehcill_state = WAKE_ACK_SENT; 	// Awake
			blockReceived = FALSE_;			// Clear flag
			PhyReceiveBlock(); 				// See what radio wants
			break;
		}

		case(WAKE_IND_SENT):{				// Awaiting ack
			if		(firstReceivedByte==HCILL_WAKE_UP_ACK)	DBG_INFO( "R33\n");
			else if (firstReceivedByte==HCILL_WAKE_UP_IND)	DBG_INFO( "R32\n");
			else 	{DBG_INFO("cts\n");ehcill_state=CTS_WOKEN;break;} // We were interrupted by cts							

			ehcill_state = WAKE_ACK_RXED; 	// Awake state
			blockReceived = FALSE_;			// Clear flag
			PhyReceiveBlock(); 				// Re-enable receiver	
			break;
		}
		
		default : {	
			DBG_INFO( "E03\n");
			break;
		}
	}
	return;
}

void __attribute__((interrupt,auto_psv)) BT_EVENT_HANDLER_VECTOR(void)
{
	// Call transfer complete call back
	BT_EVENT_HANDLER_FLAG = 0;
	if(TransferCompleteCB!=NULL)
		TransferCompleteCB();
}


void __attribute__((interrupt,auto_psv)) BT_UART_ER_INTERRUPT(void)
{
	BT_UART_ER_IF = 0;
	#ifndef __DEBUG
	DebugReset(BT_ERR); // It is very hard to re-sync the module, suggest resetting
	#endif
	Nop();
	Nop();
	return;
}

// EOF
