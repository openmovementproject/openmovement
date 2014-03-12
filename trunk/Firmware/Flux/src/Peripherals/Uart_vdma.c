// KL 2011
#include "Compiler.h"
#include "HardwareProfile.h"
#include "TimeDelay.h"
#include <stdint.h>
#include <uart.h>
#include "Peripherals/Uart_vdma.h"

//Globals
// rx state - depends on settings
void (*RxDoneHandler)(void) = NULL;
#ifdef VDMA_CALLBACK_ON_CHAR 
	unsigned char callBackChar = '\r';
	volatile unsigned char rxBuffer[VDMA_RX_FIFO_SIZE] = {0};
	volatile unsigned short rxBufferHead = 0;
	volatile unsigned short rxBufferTail = 0;	
#elif defined VDMA_CALLBACK_ON_COMPLETE
	volatile unsigned char* vdmaRxBuffer;
	volatile unsigned short	vdmaRxLen;
#else
	#error "What reciever mode?"
#endif

// tx state
void (*TxDoneHandler)(void) = NULL;
volatile unsigned short  vdmaTxLen = 0xffff;
volatile unsigned char* vdmaTxBuffer = NULL;

// Initialise uart
void UartVdmaInit(void)
{
	// Make sure its all off first
	UartVdmaOff();

	OPEN_VDMA (
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
	UartVdmaSetBaud(VDMA_DEFAULT_BAUD); 

	// Initialise TX state vars
	TxDoneHandler = NULL;
	vdmaTxLen = 0xffff; 
	vdmaTxBuffer = NULL;

	// Initialise RX state vars
	RxDoneHandler = NULL;
	#ifdef VDMA_CALLBACK_ON_CHAR 
		callBackChar = '\r';
		rxBufferHead = 0;
		rxBufferTail = 0;	
	#elif defined VDMA_CALLBACK_ON_COMPLETE
		vdmaRxLen = 0xFFFF;
		vdmaRxBuffer = NULL;
	#endif

	// Clear any pending ints and enable RX/TX
	VDMA_RX_IP = VDMA_RX_INT_PRIORITY;
	VDMA_RX_IF = 0;
	VDMA_RX_IE =0;
	VDMA_TX_IP = VDMA_TX_INT_PRIORITY;
	VDMA_TX_IF = 0;
	VDMA_TX_IE =0;

	// Clear uart fifo
	while(VDMA_DATA_AVAILABLE)
	{if (VDMA_RX_REG) VDMA_RX_OVERFLOW = 0;}
}

void UartVdmaReset(void)
{
	// Power off uart (clears fifo)
	VDMA_UART_ENABLE = 0;

	// Clear any pending ints and enable RX/TX
	VDMA_RX_IP = VDMA_RX_INT_PRIORITY;
	VDMA_RX_IF = 0;
	VDMA_RX_IE =0;
	VDMA_TX_IP = VDMA_TX_INT_PRIORITY;
	VDMA_TX_IF = 0;
	VDMA_TX_IE =0;

	// Set TX out to null/invalid
	vdmaTxLen = 0xffff; 
	vdmaTxBuffer = NULL;

	// Initialise RX state vars or reset FIFO
	#ifdef VDMA_CALLBACK_ON_CHAR 
		callBackChar = '\r';
		rxBufferHead = 0;
		rxBufferTail = 0;	
	#elif defined VDMA_CALLBACK_ON_COMPLETE
		vdmaRxLen = 0xFFFF;
		vdmaRxBuffer = NULL;
	#endif

	// Power on uart
	VDMA_UART_ENABLE = 1;

	// Clear uart fifo and overflow 
	while(VDMA_DATA_AVAILABLE)
	{if (VDMA_RX_REG) VDMA_RX_OVERFLOW = 0;}
}

void UartRxIdle(void)
{
	while(VDMA_RX_OVERFLOW || ((VDMA_DATA_AVAILABLE)&&(!(VDMA_USTA&0x10))))
	{if (VDMA_RX_REG) VDMA_RX_OVERFLOW = 0;DelayMs(1);}
}

int UartVdmaSetBaud(unsigned long baud)
{
	int result = 0;
	static uint32_t last_baud = VDMA_DEFAULT_BAUD;
	if (baud!=0) last_baud = baud;
	// Otherwise it will use the last one, use to reset baud for new clock

    switch (last_baud){
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

void UartVdmaSetRxCallback( void (*the_rx_handler)(void))
{
  	RxDoneHandler = the_rx_handler;
}

void UartVdmaSetTxCallback( void (*the_tx_handler)(void))
{
    TxDoneHandler = the_tx_handler;
}

void UartVdmaOff(void) 
{
	OPEN_VDMA(0,0,0xffff);
	VDMA_RX_IE=0;	
	VDMA_TX_IE=0;
}

void UartVdmaSendBlock(const unsigned char* data, unsigned short len)
{ 
	VDMA_TX_IE = 0;			// Stop current transmission if active
    vdmaTxBuffer = (unsigned char*) data; 
    vdmaTxLen = len;
	VDMA_TX_IF=1;			// Set flag, interrupt deals with transmission and load first char	
	VDMA_TX_IE = 1;			// Interrupt will fire once byte is clocked out too (loading subsequent chars)
}

unsigned short UartVdmaGetPendingTxLength(void)
{
	unsigned short to_write;
	if (VDMA_TX_IE)
	{
		VDMA_TX_IE=0;
		to_write = vdmaTxLen;
		VDMA_TX_IE=1;
	}
	else
	{
		to_write = vdmaTxLen;
	}
	return to_write;	
}

// External buffer operation
#if defined (VDMA_CALLBACK_ON_COMPLETE)
	void UartVdmaReceiveBlock(unsigned char* buffer, unsigned short len)
	{
		VDMA_RX_IE=0;			// halt rx opperations
		vdmaRxBuffer = buffer;
		vdmaRxLen = len;
		VDMA_RX_IF=1; 			// SW fire int, RTS cleared in isr
		VDMA_RX_IE=1;			// re-enable rx opperations
	}

	unsigned short UartVdmaGetPendingRxLength(void)
	{
		unsigned short to_read;
		if (VDMA_RX_IE)
		{
			VDMA_RX_IE=0;
			to_read = vdmaRxLen;
			VDMA_RX_IE=1;
		}
		else
		{
			to_read = vdmaRxLen;
		}
		return to_read;		
	}

// FIFO operation
#elif defined( VDMA_CALLBACK_ON_CHAR )
	void UartVdmaSetRxCallBackChar(unsigned char callOn)
	{
		callBackChar = callOn;
	}

	void UartVdmaRxFifoInit(void)
	{
		unsigned short IPLshadow;
		RX_INTS_DISABLE();
		rxBufferTail = 0;
		rxBufferHead = 0;
		VDMA_RX_IE = 1;
		RX_INTS_ENABLE();
	}
	
	unsigned short UartVdmaSafeFifoLength(void)
	{
		unsigned short IPLshadow;
		unsigned short length, localHead, localTail; 
	
		RX_INTS_DISABLE();
		localHead = rxBufferHead;
		localTail = rxBufferTail;
		RX_INTS_ENABLE();
	
	    if (localTail >= localHead) { length = localTail - localHead; }
	    else { length = (VDMA_RX_FIFO_SIZE - localHead) + localTail; }
	
		return length;
	}
	
	unsigned short UartVdmaSafeFifoPop(void* destination, unsigned int count)
	{
		unsigned short IPLshadow;
	    unsigned int remaining;
	    char pass;
	    const void *bufferPointer = NULL;
	    unsigned short contiguous;
		unsigned short n = 0;
	
	    // Up to two passes (FIFO wraps inside buffer)
	    remaining = count;
	
	    for (pass = 0; pass < 2; pass++)
	    {
			// See how many entries to process in this pass
			RX_INTS_DISABLE();
			if (rxBufferTail >= rxBufferHead) { contiguous = rxBufferTail - rxBufferHead; }
		    else { contiguous = VDMA_RX_FIFO_SIZE - rxBufferHead; }
			RX_INTS_ENABLE();
	
			// Grab a pointer to the first value
	        bufferPointer = (const void *)&rxBuffer[rxBufferHead];
	
			// See how many we can copy this pass through
	        if (remaining <= contiguous) { n = remaining; } else { n = contiguous; }
	        if (n <= 0) { break; }              // No more to process on this pass
	
			// Copy n bytes
			if (destination != NULL)
			{
				memcpy(destination, bufferPointer, n);
				destination += n;
			}
	
			// Update head pointer
			rxBufferHead += n;
			rxBufferHead &= VDMA_BUFFER_INDEX_MASK;     
	
	        // Decrease number remaining
	        remaining -= n;
	        if (remaining <= 0) { break; }      // Processed all 
	    }
	    
	    // Return number of entries processed
	    return (count - remaining);
	}

#endif //fifo functions

// Interrupt handlers

// TX BYTE ISR
void __attribute__((interrupt,auto_psv)) VDMA_TX_INTERRUPT(void) 
{
	VDMA_TX_IF=0;						// Clear flag
	if ((vdmaTxLen)&&(vdmaTxBuffer!=NULL))// If one or more bytes is left to send
	{
		VDMA_TX_REG = *vdmaTxBuffer++;	// Send a byte
        vdmaTxLen--;					// Decrease count
		return;
	}
    if (vdmaTxLen == 0)
	{
        VDMA_TX_IE=0;  					// Disable TX interrupts
		if(TxDoneHandler!=NULL)
			(*TxDoneHandler)();			// Call the done handler (if not null)
		return;
    }   
}

#ifdef VDMA_CALLBACK_ON_CHAR
// RX BYTE ISR - callback on char
void __attribute__((interrupt,auto_psv)) VDMA_RX_INTERRUPT(void) 
{
	VDMA_RX_IF = 0;						// Clear flag
	VDMA_RTS = 1;						// Halt receiver
	while (VDMA_DATA_AVAILABLE) 		// Read any required bytes into the rx fifo buffer
	{
		unsigned char inChar = VDMA_RX_REG;
		rxBuffer[rxBufferTail] = inChar;
		rxBufferTail++;
		rxBufferTail &= VDMA_BUFFER_INDEX_MASK; 
		if(inChar == callBackChar) 
		{
			if(RxDoneHandler!=NULL)		// Call the complete handler - if not null
				(*RxDoneHandler)();
		}
	}
	VDMA_RTS = 0;						// Resume reception
}
#endif

#ifdef VDMA_CALLBACK_ON_COMPLETE
// RX BYTE ISR - callback on complete
void __attribute__((interrupt,auto_psv)) VDMA_RX_INTERRUPT(void) 
{
	VDMA_RX_IF = 0;						// Clear flag
	VDMA_RTS = 1;						// Halt receiver
	while ((VDMA_DATA_AVAILABLE)&&(vdmaRxLen>0)&&(vdmaRxBuffer != NULL)) // Get every byte until complete
	{
		unsigned char inChar = VDMA_RX_REG;
		*vdmaRxBuffer++ = inChar;			// Pop it to the buffer location
        if(--vdmaRxLen == 0)				// Was it the last one requested?
		{
			if(RxDoneHandler!=NULL)			// Call the complete handler - if not null
				(*RxDoneHandler)();			// RTS line is held high - suitable for eHCILL
			return;
		}
	}
	if (vdmaRxLen>0)VDMA_RTS = 0;			// Resume reception
	return;
}
#endif


//EOF
