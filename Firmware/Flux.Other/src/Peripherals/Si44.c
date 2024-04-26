// Driver for the Si4432 Radio with Antenna switch controls like the RFM22
// Requires hardware definitions for SPI control interface
// KL 2013

// Include
#include <stdlib.h>
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/Si44.h"

// Debug setting
#undef DEBUG_LEVEL
#define DEBUG_LEVEL	0
#define DBG_FILE dbg_file
#if (DEBUG_LEVEL > 0)||(GLOBAL_DEBUG_LEVEL > 0)
static const char* dbg_file = "si44";
#endif
#include "Debug.h"

#ifndef NULL
#define NULL 0
#endif

/* Hardware/firmware driver dependencies:
	Si44_IRQ_EN()
	Si44_IRQ_OFF()
	Si44_IRQ_FLAG
	Si44_SPI_INIT()
	Si44_SPI_OFF()
	Si44_SPI_OPEN()	
	Si44_SPI_PUTC(_c)
	Si44_SPI_GETC()						
	Si44_SPI_CLOSE()
	Si44RadioIrqHandler() // Call from interrupt
*/

// Globals
volatile Si44RadioState_t 	Si44RadioState = SI44_OFF;
volatile Si44Status_t 		Si44Status = {{0}};
volatile Si44IntStat_t		Si44RadioInts = {{0}};
volatile Si44EventCB_t 		Si44EventCB = NULL;
volatile unsigned char		numPktsToRx = 0; // Set by rx command

/*	This is used for making pkt event types. It is not advised to poll
	for packets IF you have the call back set as duplicate pkt events 
	could result.*/
unsigned char localPktBuffer[64+4+1];

// Internal Prototypes
static void Si44RadioReadStatus(void);
static void Si44RadioClearInts(void);
static unsigned char Si44ReadReg(unsigned char reg);
static void Si44WriteReg(unsigned char reg, unsigned char val);
static void Si44WriteRegMultiple(unsigned char reg, unsigned char* val, unsigned char len);
static void Si44ReadRegMultiple(unsigned char reg, unsigned char* val, unsigned char len);
static void Si44RadioRxPktEvent(Si44Event_t* event, unsigned char* pktBuffer);
static void Si44RadioReset(void);
static void Si44RadioStandby(void);
static void Si44RadioIdle(void);
static void Si44RadioTx(void);
static void Si44RadioRx(void);

// Source

// Set event callback
void Si44SetEventCB(Si44EventCB_t CB)
{
	Si44EventCB = CB;
}

// List of commands, SI44_CMD_EOL terminated
void Si44CommandList(const Si44Cmd_t* cmdList)
{
	while((cmdList != NULL) && (cmdList->type != SI44_CMD_EOL))
	{
		Si44Command(cmdList, NULL);
		cmdList++;
	} 
}

// Execute command
Si44Event_t* Si44Command(const Si44Cmd_t* cmd, void* buffer)
{
	unsigned short pair;
	unsigned char *cmdData;
	static Si44Event_t event = {0};

	// Interrupt off
	Si44_IRQ_OFF();

	// Checks
	if(cmd == NULL) return NULL;

	// Begin event
	event.type = cmd->type;
	event.data = NULL;

	// Convert pointer
	cmdData = (unsigned char*)cmd->data;

	// Process command
	DBG_INFO("\r\nSi44 cmd %u",cmd->type);
	switch (cmd->type) {
		case (SI44_RESET) 	: {
			if(cmd->len != 0) 
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;
			}
			else
			{
				event.err = SI44_OK;
				event.len = 0;
				Si44RadioReset();
				// Check for errors
				if(Si44RadioState == SI44_HW_ERROR)
					event.err = SI44_HW_ERR;
			}
			break;
		}
		case (SI44_CMD_STANDBY) : {
			if(cmd->len != 0) 
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;
			}
			else
			{
				event.err = SI44_OK;
				event.len = 0;
				Si44RadioStandby();
			}
			break;
		}
		case (SI44_CMD_IDLE) 	: {
			if(cmd->len != 0) 
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;
			}
			else
			{
				event.err = SI44_OK;
				event.len = 0;
				Si44RadioIdle();
			}
			break;
		}
		case (SI44_TX) 		: {
			if(cmd->len != 0) 
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;
			}
			else
			{
				event.err = SI44_OK;
				event.len = 0;
				Si44RadioTx();
			}
			break;
		}
		case (SI44_RX) 		: {
			if(cmd->len == 0) 
			{
				numPktsToRx = 0;
				event.err = SI44_OK;
				event.len = 0;
				Si44RadioRx();
			}
			else if (cmd->len == 1)
			{
				numPktsToRx = ((unsigned char*)cmd->data)[0];
				event.err = SI44_OK;
				event.len = 0;
				Si44RadioRx();
			}
			else
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;			
			}
			break;
		}
		case (SI44_WRITE_REG): {
			if(cmd->len < 2) 
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;	
			}
			else
			{
				event.err = SI44_OK;
				event.len = 0;
				Si44WriteRegMultiple(cmdData[0],&cmdData[1],(cmd->len - 1));
			}
			break;	
		}
		case (SI44_WRITE_REG_LIST): {
			unsigned char *lastEntry = ((unsigned char*)cmd->data) + (cmd->len-2);
			if(	(cmd->len < 2) || 			// No data
				((cmd->len & 0x01) != 0) ||	// Even num bytes
				(LE_READ_16(lastEntry) != SI44_REG_TYPE_EOL)) // Check for EOL token
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;	
			}
			else
			{
				event.err = SI44_OK;
				event.len = 0;
				for(;;) 
				{
					pair = ((unsigned short)cmdData[1]<<8) | cmdData[0];
					if(pair == SI44_REG_TYPE_EOL) break;
					Si44WriteReg(cmdData[0],cmdData[1]);
					cmdData += 2;
				}
			}
			break;
		}
		case (SI44_READ_REG_LIST): {
			if((cmd->len < 4) || ((cmd->len & 0x01) != 0)) // Even num bytes
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;	
			}
			else
			{
				unsigned short totalLen = 0;
				unsigned char i;
				// Begin event
				event.type = SI44_READ_REG_LIST;
				// Check buffer pointer
				if(buffer == NULL)
				{
					event.err = SI44_NO_MEM;
					event.len = 0;
					event.data = NULL;
					break;
				}
				// Get total length
				i = 0;
				while(cmdData[i+1] != 0)
				{
					totalLen += cmdData[i+1]; 
					i+=2;
					if(i >= cmd->len)
					{
						// Whole command read without EOL token
						event.err = SI44_BAD_CMD;
						event.len = 0;	
						break;						
					}
				}
				// Read in data
				unsigned char *ptr = buffer;
				i = 0;
				while(cmdData[i+1] != 0)
				{
					Si44ReadRegMultiple(cmdData[i], ptr, cmdData[i+1]);
					ptr += cmdData[i+1];
					i+=2;
				}
				event.err = SI44_OK;
				event.len = totalLen;
				event.data = buffer;
			}
			break;
		}
		case (SI44_READ_PKT) : {
			if(cmd->len != 0) 
			{
				event.err = SI44_BAD_CMD;
				event.len = 0;
			}
			else
			{
				// Make packet rx event 
				Si44RadioRxPktEvent(&event, buffer);
			}
			break;
		}
		default :  {
			// Bad command
			event.type = SI44_EVT_ERR;
			event.err = SI44_BAD_CMD;
			event.len = 0;
			break;	
		}
	}// End cmd switch

	// Errors
	if(Si44RadioState == SI44_HW_ERROR) 
	{
		event.err = SI44_HW_ERR;
	}
	// If callback is valid	
	if(Si44EventCB!=NULL)
	{
		Si44EventCB(&event);
	}

	// Interrupts back on
	Si44_IRQ_EN();

	return &event;
}

static void Si44RadioReset(void)
{
	unsigned char val, i;
	Si44_IRQ_OFF();
	Si44_SPI_INIT();
	for(;;)//break
	{
		// Check device type
		val = Si44ReadReg(Si44_Device_Type);
		if(val != 0x08) 
		{
			Si44RadioState = SI44_HW_ERROR; 
			break;
		}
		val = Si44ReadReg(Si44_Device_Version);
		if(val != 0x06) 
		{
			Si44RadioState = SI44_HW_ERROR; 
			break;
		}	
		// Reset device and wait for POR flag
		Si44WriteReg(Si44_OFC1,0x80);
		Si44Status.ofc1 = 0x80; 
		// Wait for reset done flag
		for(i=0;i<100;i++)
		{
			DelayMs(1);
			val = Si44ReadReg(Si44_Interrupt_Status2);
			if(val&0x02) break;
		}
		if(i >= 100) // Timeout fail
		{
			Si44RadioState = SI44_HW_ERROR; 
			break;
		} 
		// Clear interrupts
		Si44RadioClearInts();
		// Done, radio ready for Tx or Rx or Standby
		Si44RadioReadStatus(); // Globals set in call
		// Radio is initialised
		Si44RadioState = SI44_INITIALISED;
		break;
	}// for(;;)
	if(Si44RadioState != SI44_INITIALISED)
	{
		// Reset device incase it is a device lockup
		Si44WriteReg(Si44_OFC1,0x80);
		Si44Status.ofc1 = 0x80; 
	}
	numPktsToRx = 0;
	return;
}

static void Si44RadioIdle(void)
{
	if(Si44RadioState == SI44_HW_ERROR)return;
	// Clear interrupts
	Si44RadioClearInts();
	// Set ready mode - 200us to Tx/Rx
	Si44WriteReg(Si44_OFC1,0x01); // Goto idle/ready (~500us)
	Si44Status.ofc1 = 0x01;
	Si44RadioState = SI44_IDLE; 
	// Interrupt on
	Si44_IRQ_EN();
	return;
}

static void Si44RadioStandby(void)
{
	if(Si44RadioState == SI44_HW_ERROR)return;
	// Clear interrupts
	Si44RadioClearInts();
	// Set low power standby mode
	Si44WriteReg(Si44_OFC1,0x00); // Goto standby 
	Si44Status.ofc1 = 0x00;
	Si44RadioState = SI44_STANDBY; 
	return;
}

static void Si44RadioTx(void)
{
	// Check radio is ready to transmit
	if(Si44RadioState == SI44_OFF || Si44RadioState == SI44_HW_ERROR)return;

	// Clear interrupts
	Si44RadioClearInts();

	Si44Status.ofc1 = Si44Status.ofc1 | 0x08;	
	Si44Status.ofc1 &= (~0x04);// Rx operation cancelled if active

	Si44WriteReg(Si44_OFC1,Si44Status.ofc1); // Set Tx state, note: next state is last state	

	Si44RadioState = SI44_TXING; 
	return;
}

static void Si44RadioRx(void)
{
	// Check radio is ready to transmit
	if(Si44RadioState == SI44_OFF || Si44RadioState == SI44_HW_ERROR)return;

	Si44Status.ofc1 = Si44Status.ofc1 | 0x04;	
	Si44WriteReg(Si44_OFC1,Si44Status.ofc1);	// Set Rx state, note: next state is last state

	Si44RadioState = SI44_RXING; 
	return;
}

static void Si44RadioRxPktEvent(Si44Event_t* event, unsigned char* pktBuffer)
{
	unsigned short pktLen = 0;
	unsigned char* dest;
	// Begin event
	pktLen = Si44ReadReg(Si44_Received_Packet_Len);
	event->type = SI44_READ_PKT;
	// Make a buffer
	if(pktBuffer == NULL)
	{
		event->err = SI44_NO_MEM;
		event->len = 0;
		event->data = NULL;
	}
	else
	{
		// Set event
		event->err = SI44_OK;
		event->len = pktLen+5;
		event->data = pktBuffer;
		dest = pktBuffer;
		// Read in header
		*dest++ = Si44ReadReg(Si44_Received_Header0);
		*dest++ = Si44ReadReg(Si44_Received_Header1);
		*dest++ = Si44ReadReg(Si44_Received_Header2);
		*dest++ = Si44ReadReg(Si44_Received_Header3);
		// Read in rssi
		*dest++ = Si44Status.lastRSSI;
		// Read in data
		Si44ReadRegMultiple(Si44_FIFO_Access, dest, pktLen);
	}
	return;
}

static void Si44RadioReadStatus(void)
{
	if(Si44RadioState == SI44_HW_ERROR)return;
	// Get radio status
	Si44Status.ezmacStat = 	Si44ReadReg(Si44_EzMAC_status);
	Si44Status.deviceStat = Si44ReadReg(Si44_Device_Status);
	Si44Status.ofc1 = 		Si44ReadReg(Si44_OFC1);
}

static void Si44RadioClearInts(void)
{
	Si44_IRQ_FLAG = 0;
	if(Si44RadioState == SI44_HW_ERROR)return;
	// Read interrupt sources to clear flags
	Si44RadioInts.b[0] = Si44ReadReg(Si44_Interrupt_Status1);
	Si44RadioInts.b[1] = Si44ReadReg(Si44_Interrupt_Status2);
}

void Si44RadioIrqHandler(void)
{
	// Debug
	DBG_INFO("\r\nSi44 isr>");

	// Local event to send out
	Si44Event_t event = {0};
	// Get radio status
	Si44RadioReadStatus();
	// For silicon bug, write ofc1 back to latch new state reliably
	Si44WriteReg(Si44_OFC1,Si44Status.ofc1);
	// Read interrupts, clears flag
	Si44RadioClearInts();
	
	// Packet sent flag 
	if(Si44RadioInts.pksent)
	{
		DBG_INFO("\r\nSi44 pkt sent ->");
		// Packet sent
		if(Si44Status.ofc1 & 0b1)
		{
			Si44RadioState = SI44_IDLE;
		}
		else 
		{
			Si44RadioState = SI44_STANDBY;
		}
		// If callback is valid	
		if(Si44EventCB!=NULL)
		{
			event.type = SI44_TX_DONE;
			// Check radio
			if(Si44RadioState == SI44_HW_ERROR) 
				event.err = SI44_HW_ERR;
			else
				event.err = SI44_OK;
			Si44EventCB(&event);
		}
	}
	// Packet received flag
	else if(Si44RadioInts.ipkvalid)
	{
		DBG_INFO("\r\nSi44 pkt rxed <-");
		// Handle packet received
		if(Si44Status.ofc1 & 0b1)
		{
			Si44RadioState = SI44_IDLE;
		}
		else 
		{
			Si44RadioState = SI44_STANDBY;
		}
		// Should we receive another automatically
		if(numPktsToRx > 0)
		{
			// 0xff is infinite
			if(numPktsToRx != 0xff) 
				numPktsToRx--;
			Si44Status.ofc1 = Si44Status.ofc1 | 0x04;	
			// Set Rx state, note: next state is previous state
			Si44WriteReg(Si44_OFC1,Si44Status.ofc1);	
			Si44RadioState = SI44_RXING; 
		}
		// If callback is valid	
		if(Si44EventCB!=NULL)
		{
			// Check radio
			if(Si44RadioState == SI44_HW_ERROR) 
			{
				event.type = SI44_READ_PKT;
				event.err = SI44_HW_ERR;
			}
			else
			{
				// Make a rx event
				Si44RadioRxPktEvent(&event, localPktBuffer);	
			}
			Si44EventCB(&event);
		}
	}
	// Sync word detected, start of packet, latch rssi if used
	else if(Si44RadioInts.iswdet)	
	{
		Si44Status.lastRSSI = Si44ReadReg(Si44_RSSI);
	}
	// Device power on reset flag
	else if(Si44RadioInts.ipor)	
	{
		// Indicative of hardware error
		Si44RadioState = SI44_HW_ERROR; 
		DBG_ERROR("Reset detected");
	}

	return;
}

static unsigned char Si44ReadReg(unsigned char reg)
{
	unsigned char val;
	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x7F & reg);
	Si44_SPI_PUTC(0xff); // Dummy write
	val = Si44_SPI_GETC();
	Si44_SPI_CLOSE();
	DBG_INFO("\r\nSi44 read 0x%02X, val 0x%02X",reg,val);
	return val;
}

static void Si44WriteReg(unsigned char reg, unsigned char val)
{
	DBG_INFO("\r\nSi44 write 0x%02X, val 0x%02X",reg,val);
	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x80 | reg);
	Si44_SPI_PUTC(val);
	Si44_SPI_CLOSE();
}

static void Si44ReadRegMultiple(unsigned char reg, unsigned char* val, unsigned char len)
{
	DBG_INFO("\r\nSi44 read 0x%02X, len %u",reg,len);
	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x7F & reg);
	while(len-- > 0)
	{
		Si44_SPI_PUTC(0xff); // Dummy write
		*val++ = Si44_SPI_GETC();
	}
	Si44_SPI_CLOSE();
	return;
}

static void Si44WriteRegMultiple(unsigned char reg, unsigned char* val, unsigned char len)
{
	DBG_INFO("\r\nSi44 write 0x%02X, len %u",reg,len);
	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x80 | reg);
	while(len-- > 0)
	{
		Si44_SPI_PUTC(*val++);
	}
	Si44_SPI_CLOSE();
}




//EOF
