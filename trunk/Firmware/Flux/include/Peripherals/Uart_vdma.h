// KL 2011
#ifndef UART_VDMA_H
#define UART_VDMA_H

/*
	This driver allow the user to use the PIC's uart as a virtual DMA with call backs. 
	The Rx side has two modes; external buffer with fixed receive length (useful for	
	binary stacks like the bluetooth HCI interface) OR internal FIFO (useful for unknown
	receive lengths like GSM module interfaces. Both modes have a call back function, the
	former calls back after N chars are received. The FIFO mode calls back on a 'special'
	character set by the user (such as '\r' or SLIP_END). 
	The TX side is similar; set the pointers and length to transmit data. Either poll 
	the remaining length or use the call back to drive a state machine.
*/

#include "HardwareProfile.h"
// Defines - to add to hardwareprofile.h
/*
	#define VDMA_DEFAULT_BAUD 	115200UL
 	#define VDMA_RTS			LATGbits.LATG9
	#define VDMA_CALLBACK_ON_COMPLETE
	#define VDMA_USES_UART_1
	#define VDMA_RX_INT_PRIORITY	7
	#define VDMA_TX_INT_PRIORITY	4
*/
// Startup baud (must be a supported rate) - see below for rates
#ifndef VDMA_DEFAULT_BAUD
	#warning "You should need to set a startup baud rate"
	#define VDMA_DEFAULT_BAUD 	115200UL
#endif

// Flow control pin. Set to the CTS of the other device (LAT REG)
#ifndef VDMA_RTS
	#warning "There is no hardware flow control pin defined, using the default setting will result in receive errors at high bauds"
	unsigned char vdma_dummy;
	#define VDMA_RTS		vdma_dummy	
#endif	

// Mode of receive opperation - set EITHER	
#if !defined(VDMA_CALLBACK_ON_COMPLETE) && !defined( VDMA_CALLBACK_ON_CHAR)
	#warning "You have not set the vdma receive behavior, the default is fifo mode (callback on char)"
	//#define VDMA_CALLBACK_ON_COMPLETE 	/*Best for known receive lengths, call back on N recieved chars*/
	#define VDMA_CALLBACK_ON_CHAR 		/*Best for relaying data of unknown receive lengths, callback on character, uses FIFO*/
#endif

// Which UART
#if !defined (VDMA_USES_UART_1) && !defined(VDMA_USES_UART_2)
	#warning "Using default vdma uart, uart 1"
	#define VDMA_USES_UART_1
#endif

#ifdef VDMA_USES_UART_1
	#define VDMA_UART_ENABLE	U1MODEbits.UARTEN
	#define VDMA_RX_IP			IPC2bits.U1RXIP
	#define VDMA_RX_IF			IFS0bits.U1RXIF  
	#define VDMA_RX_IE			IEC0bits.U1RXIE
	#define VDMA_TX_IP			IPC3bits.U1TXIP
	#define VDMA_TX_IF			IFS0bits.U1TXIF
	#define VDMA_TX_IE			IEC0bits.U1TXIE
	#define BAUD_REG			U1BRG
	#define VDMA_TX_REG			U1TXREG
	#define VDMA_RX_REG 		U1RXREG
	#define OPEN_VDMA 			OpenUART1
	#define VDMA_DATA_AVAILABLE	U1STAbits.URXDA
	#define VDMA_RX_OVERFLOW	U1STAbits.OERR
	#define VDMA_TX_INTERRUPT	_U1TXInterrupt
	#define VDMA_RX_INTERRUPT	_U1RXInterrupt
	#define VDMA_USTA			U1STA
#elif defined (VDMA_USES_UART_2)
	#define VDMA_UART_ENABLE	U2MODEbits.UARTEN
	#define VDMA_RX_IP			IPC7bits.U2RXIP
	#define VDMA_RX_IF			IFS1bits.U2RXIF  
	#define VDMA_RX_IE			IEC1bits.U2RXIE
	#define VDMA_TX_IP			IPC7bits.U2TXIP
	#define VDMA_TX_IF			IFS1bits.U2TXIF
	#define VDMA_TX_IE			IEC1bits.U2TXIE
	#define BAUD_REG			U2BRG
	#define VDMA_TX_REG			U2TXREG
	#define VDMA_RX_REG 		U2RXREG
	#define OPEN_VDMA 			OpenUART2
	#define VDMA_DATA_AVAILABLE	U2STAbits.URXDA
	#define VDMA_RX_OVERFLOW	U2STAbits.OERR
	#define VDMA_TX_INTERRUPT	_U2TXInterrupt
	#define VDMA_RX_INTERRUPT	_U2RXInterrupt
	#define VDMA_USTA			U2STA
#endif

// Available baud rate setings - extend as needed
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

// Fifo buffer length - should be set by user
// Set circular buffer size - must be power of 2
#if !defined( VDMA_RX_FIFO_SIZE) && defined (VDMA_CALLBACK_ON_CHAR) 
	#warning "Default vdma rx buffer length used."
	#define VDMA_RX_FIFO_SIZE 128
#endif

// Priority control
/* interrupts above the RX priority must take less than one uart charcter to complete and be less than 50% duty*/
#ifndef VDMA_RX_INT_PRIORITY
	#warning "Using default (high) vdma ISR priorities."
	#define VDMA_RX_INT_PRIORITY	7
#endif
#ifndef VDMA_TX_INT_PRIORITY
	#warning "Using default (high) vdma ISR priorities."
	#define VDMA_TX_INT_PRIORITY	6
#endif

#if (VDMA_RX_INT_PRIORITY != 7)
	#warning "Any interrupts avove the RX priority could block the receiver,"
	#warning "ensure these interrupts never take more than half a receive 'byte time' to complete"
#endif

// Non-user defines
#define VDMA_BUFFER_INDEX_MASK 	(VDMA_RX_FIFO_SIZE - 1)
#define RX_INTS_DISABLE()		{IPLshadow = SRbits.IPL; if(SRbits.IPL<VDMA_RX_INT_PRIORITY)SRbits.IPL=VDMA_RX_INT_PRIORITY;}
#define RX_INTS_ENABLE()		{SRbits.IPL = IPLshadow;}



// User Functions

// Common Functions - all interfaces, use in this order

	// Initialisation
	void UartVdmaInit(void);
	
	// Reset receiver - (same as init but leaves state) must be initialised, not same as fifo init
	void UartVdmaReset(void);

	// Reset reciever - this one only clears incoming bytes
	void UartRxIdle(void);

	// Set baud rates, if not using default, CALL WITH 0 TO ADAPT TO CLOCK SWITCH
	int UartVdmaSetBaud(unsigned long baud);
	
	// Set call back for receive complete, if used
	void UartVdmaSetRxCallback( void (*the_rx_handler)(void));
	
	// Set call back for transmit complete, if used
	void UartVdmaSetTxCallback( void (*the_tx_handler)(void));
	
	// Shutdown the VDMA
	void UartVdmaOff(void);

	// Send data
	void UartVdmaSendBlock(const unsigned char* data, unsigned short len);
	
	// Number of bytes left to send
	unsigned short UartVdmaGetPendingTxLength(void);
	#define UartVdmaTxBusy()	(VDMA_TX_IE!=0)
// For fixed length receive callback

	// Receive data
	void UartVdmaReceiveBlock(unsigned char* buffer, unsigned short len);
	
	// Number of bytes left to receive
	unsigned short UartVdmaGetPendingRxLength(void);

// For FIFO operation only

	// Set the character that results in the rx callback
	void UartVdmaSetRxCallBackChar(unsigned char callOn);
	
	// Restart reciever - call at any time, discards buffer contents
	void UartVdmaRxFifoInit(void);
	
	// Get current received length
	unsigned short UartVdmaSafeFifoLength(void);
	
	// Remove bytes from the fifo
	unsigned short UartVdmaSafeFifoPop(void* destination, unsigned int count);
	
// Reciever/Transmitter hardware errors?
// Consider putting this in code somewhere with a handler - will call on error
//void __attribute__((interrupt,auto_psv)) _U1ERRInterrupt(void) 
//{
//	// UART has encountered an error!
//	LED_R = 1;
//	while(1);
//}

// You may have to 'tune' for some frequencies - see old code below
#if 0
/*Hacked to tune oscillator to suit high baud rates at low clocks*/
#if (((GetInstructionClock()) == 4000000)||((GetInstructionClock()) == 16000000))
	#define SETBAUD_57600()  		{if(OSCCONbits.COSC==1){BT_BAUD_REG=68	/*68.4*/;}else{BT_BAUD_REG=16;	OSCTUN=-6;	/*16.36*/}}
	#define SETBAUD_115200() 		{if(OSCCONbits.COSC==1){BT_BAUD_REG=34	/*33.7*/;}else{BT_BAUD_REG=8;	OSCTUN=9;	/*7.681*/}}
	#define SETBAUD_921600() 		{if(OSCCONbits.COSC==1){BT_BAUD_REG=3	/*3.34*/;}else{BT_BAUD_REG=0;	OSCTUN=-23;	/*0.085*/}}
	#define SETBAUD_1000000() 		{if(OSCCONbits.COSC==1){BT_BAUD_REG=3	/*3.00*/;}else{BT_BAUD_REG=0;	OSCTUN=0;	/*0.000*/}}
#else
	#error "Add baud rate BRG settings"
#endif
#endif

#endif
//EOF


