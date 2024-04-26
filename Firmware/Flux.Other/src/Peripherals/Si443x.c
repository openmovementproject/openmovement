// Driver for the Si4432 Radio with Antenna switch controls like the RFM22
// Requires hardware definitions for SPI control interface
// KL 2013

// Include
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/Si443x.h"

// Debug setting
#undef DEBUG_LEVEL
#define DEBUG_LEVEL	0
#define DBG_FILE dbg_file
#if (DEBUG_LEVEL > 0)||(GLOBAL_DEBUG_LEVEL > 0)
static const char* dbg_file = "si443x";
#endif
#include "Debug.h"

#ifndef NULL
#define NULL 0
#endif

/* Hardware/firmware API requirements:
	Si44_IRQ_EN()
	Si44_IRQ_OFF()
	Si44_IRQ_FLAG
	Si44_SPI_INIT()
	Si44_SPI_OFF()
	Si44_SPI_OPEN()	
	Si44_SPI_PUTC(_c)
	Si44_SPI_GETC()						
	Si44_SPI_CLOSE()
	Si44RadioIrqHandler() 				// Call from interrupt
*/

volatile Si44Status_t Si44Status = {{0}};
volatile Si44RadioState_t Si44RadioState = SI44_OFF;

void Si44WriteReg(unsigned char reg, unsigned char val)
{
	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x80 | reg);
	Si44_SPI_PUTC(val);
	Si44_SPI_CLOSE();
}

unsigned char Si44ReadReg(unsigned char reg)
{
	unsigned char val;
	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x7F & reg);
	Si44_SPI_PUTC(0xff); // Dummy write
	val = Si44_SPI_GETC();
	Si44_SPI_CLOSE();
	return val;
}

void Si44RadioInit(void)
{
	unsigned char val, i;
	Si44_IRQ_OFF();
	Si44_SPI_INIT();
	// Check device type
	val = Si44ReadReg(Si44_Device_Type);
	if(val != 0x08) 
	{
		// Reset device incase it is a device lockup
		Si44WriteReg(Si44_OFC1,0x80);
		Si44Status.ofc1 = 0x80; 
		Si44RadioState = SI44_HW_ERROR; 
		return;
	}
	val = Si44ReadReg(Si44_Device_Version);
	if(val != 0x06) 
	{
		// Reset device incase it is a device lockup
		Si44WriteReg(Si44_OFC1,0x80); 
		Si44Status.ofc1 = 0x80;
		Si44RadioState = SI44_HW_ERROR; 
		return;
	}	
	// Reset device
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
		return;
	} 
	// Dummy read interrupt sources to clear flags
	Si44ReadReg(Si44_Interrupt_Status1);
	Si44ReadReg(Si44_Interrupt_Status2);
	Si44_IRQ_FLAG = 0; // Now we can clear the flag

	// Setup interrupts, adc, LDC config (off) and gpio
	Si44WriteReg(Si44_Interrupt_Enable1,0x06); 	// Pkt Txed, Pkt Rxed
	Si44WriteReg(Si44_Interrupt_Enable2 ,0x81); // POR, sync word detect
//	Si44WriteReg(Si44_Interrupt_Enable2 ,0x01); // POR

	
	Si44WriteReg(Si44_OFC1,0x00); 				// POR val, standby mode
	Si44Status.ofc1 = 0x00;
	Si44WriteReg(Si44_OFC2,0); 					// POR val
	Si44WriteReg(Si44_Crystal_Load ,0x7f); 		// POR val
	Si44WriteReg(Si44_MCU_Clock_Out ,0x06); 	// POR val
	
	Si44WriteReg(Si44_GPIO0_Configuration,0x12);// RX state 
   	Si44WriteReg(Si44_GPIO1_Configuration,0x15);// TX state
	Si44WriteReg(Si44_GPIO2_Configuration ,0); 	// POR val
	Si44WriteReg(Si44_IO_Port,0); 				// POR val
	
	Si44WriteReg(Si44_ADC_Configuration,0); 	// POR val
	Si44WriteReg(Si44_ADC_Offset ,0); 			// POR val
	Si44WriteReg(Si44_Temperature_Control,0x20);// POR val
	Si44WriteReg(Si44_Temperature_Value ,0x00); // POR val
	
	Si44WriteReg(Si44_Wake_Up_Timer1 ,0x03); 	// POR val
	Si44WriteReg(Si44_Wake_Up_Timer2,0x00); 	// POR val
	Si44WriteReg(Si44_Wake_Up_Timer3 ,0x01); 	// POR val
	Si44WriteReg(Si44_Low_Duty_Cycle_Duration,0x01); // POR val
	Si44WriteReg(Si44_Low_Battery_Thresh,0x14); // POR val	
	
	// Setup baseband modem settings
	// Aiming for 150kbps, Fd=62.5KHz, Manchester coded, 434MHz band

	// RX/TX carrier freq settings
	Si44WriteReg(Si44_Frequency_Band   			,0x53); 	// 75 430MHz
	Si44WriteReg(Si44_Nominal_Carrier1 			,0x64);  	// 76
	Si44WriteReg(Si44_Nominal_Carrier0 			,0x00);  	// 77
	
	// TX data rate setting
	Si44WriteReg(Si44_TX_Data_Rate1 			,0x20); 	// 6E
	Si44WriteReg(Si44_TX_Data_Rate0  			,0x00); 	// 6F
	Si44WriteReg(Si44_Modulation_Mode1			,0x0E); 	// 70
	Si44WriteReg(0x58							,0xED); 	// 58

	// Tx frequency deviation settings
	Si44WriteReg(Si44_Frequency_Deviation		,0x64); 	// 72	
	Si44WriteReg(Si44_Modulation_Mode2 			,0x23); 	// 71 - dtmode = 10
	
	// GFSK/FSK Rx modem settings
	Si44WriteReg(Si44_IF_Filter_BW 				,0x8A); 	// 1C	
	Si44WriteReg(Si44_Clock_Recovery_Oversamp	,0x30);		// 20	
	Si44WriteReg(Si44_Clock_Recovery_Offset2 	,0x01); 	// 21	 
	Si44WriteReg(Si44_Clock_Recovery_Offset1 	,0x55); 	// 22	 
	Si44WriteReg(Si44_Clock_Recovery_Offset0	,0x55); 	// 23	
	Si44WriteReg(Si44_Clock_Recovery_Gain1		,0x07); 	// 24	
	Si44WriteReg(Si44_Clock_Recovery_Gain0 		,0xff);		// 25 
	Si44WriteReg(Si44_AFC_Loop_Override			,0x44); 	// 1D	
	Si44WriteReg(Si44_AFC_Timing_Control 		,0x02);		// 1E
	Si44WriteReg(Si44_AFC_Limiter 				,0x50); 	// 2A
	Si44WriteReg(Si44_Clock_Recovery_Override	,0x03); 	// 1F	
	Si44WriteReg(Si44_AGC_Override1 			,0x60);		// 69

	Si44WriteReg(Si44_TX_Power,	0x08); 						// 0dBm Tx power , direct tie antenna config

	Si44WriteReg(Si44_ADC8_Control  ,			0x10); 		// POR val
	Si44WriteReg(Si44_Channel_Filter_Coef ,		0x00);		// Continuous preamble search in Rx 
	Si44WriteReg(Si44_Crystal_Control ,			0x04);		//  POR val
	
	Si44WriteReg(Si44_RSSI_For_CCA,0x1E); 			// POR val
	Si44WriteReg(Si44_OOK_Counter1 ,0x18); 			// POR val 
	Si44WriteReg(Si44_OOK_Counter2 ,0xBC); 			// POR val
	Si44WriteReg(Si44_Slicer_Peak_Hold,0x2C); 		// POR val
	Si44WriteReg(Si44_Frequency_Offset1 ,0x00); 	// Offset from central freq <7:0>, POR val
	Si44WriteReg(Si44_Frequency_Offset2 ,0x00); 	// Offset from central freq <9:8>, POR val
	Si44WriteReg(Si44_Frequency_Hopping_Ch,0x00); 	// POR val
	Si44WriteReg(Si44_Frequency_Hopping_Size,0x00); // POR val
	
	// Packet specific settings
	Si44WriteReg(Si44_Data_Access_Control,0x8c);// Radio to perform data packet handling of Tx and Rx with CRC enabled
	Si44WriteReg(Si44_Header_Control1,0x00); 	// Dont check for header bytes
	Si44WriteReg(Si44_Header_Control2 ,0x42); 	// Use 4 header bytes, variable pkt len, sync words 3 and 2
	Si44WriteReg(Si44_Preamble_Length ,0x08); 	// Use 4 byte preamble (POR val)
	Si44WriteReg(Si44_Preamble_Detection ,0x10);// Preamble threshold 8bits of 010101...
	Si44WriteReg(Si44_Sync_Word3 ,0x2d); 		// POR val
	Si44WriteReg(Si44_Sync_Word2 ,0xd4); 		// POR val
	Si44WriteReg(Si44_Sync_Word1 ,0x00); 		// POR val
	Si44WriteReg(Si44_Sync_Word0 ,0x00); 		// POR val
	Si44WriteReg(Si44_Transmit_Header3 	,0); 	// Tx header
	Si44WriteReg(Si44_Transmit_Header2	,0); 	// Tx header
	Si44WriteReg(Si44_Transmit_Header1	,0); 	// Tx header
	Si44WriteReg(Si44_Transmit_Header0 	,0); 	// Tx header
	Si44WriteReg(Si44_Check_Header3	,0); 		// Rx header to look for
	Si44WriteReg(Si44_Check_Header2 ,0); 		// Rx header to look for
	Si44WriteReg(Si44_Check_Header1 ,0); 		// Rx header to look for
	Si44WriteReg(Si44_Check_Header0 ,0); 		// Rx header to look for
	Si44WriteReg(Si44_Header_Enable3 ,0xff); 	// Bit mask of Rx header bytes
	Si44WriteReg(Si44_Header_Enable2 ,0xff); 	// Bit mask of Rx header bytes
	Si44WriteReg(Si44_Header_Enable1 ,0xff); 	// Bit mask of Rx header bytes
	Si44WriteReg(Si44_Header_Enable0 ,0xff); 	// Bit mask of Rx header bytes
	Si44WriteReg(Si44_TX_FIFO1 ,0x37); 			// POR val
	Si44WriteReg(Si44_TX_FIFO2,0x40); 			// POR val
	Si44WriteReg(Si44_RX_FIFO_Ctrl ,0x37); 		// POR val

	// Done, radio ready for Tx or Rx or Standby
	Si44RadioReadStatus(); // Globals set in call
	Si44RadioState = SI44_INITIALISED;

	// Check 
	Si44RadioCheckOk();

	Si44_IRQ_EN();
	return;
}

unsigned char Si44RadioCheckOk(void)
{
	unsigned char gpio0, gpio1;
	// Interrupt off
	Si44_IRQ_OFF();
	gpio0 = Si44ReadReg(Si44_GPIO0_Configuration);
   	gpio1 = Si44ReadReg(Si44_GPIO1_Configuration);
	if(gpio0 != 0x12 || gpio1 != 0x15) 	
	{	
		// Indicative of hardware error / reset (not values set)
		DBG_ERROR("Si443x.c: Config mismatch error!");
		Si44RadioState = SI44_HW_ERROR; 
		return 0;
	}
	// Interrupt on (if no error)
	Si44_IRQ_EN();
	return 1;
}

// Sets header bytes for Tx and Rx, assumed num to check refers to MSBs
void Si44RadioSetHeader(unsigned char* vals, unsigned char numtocheck)
{
	unsigned char temp = 0xf0;
	// Interrupt off
	Si44_IRQ_OFF();

	Si44WriteReg(Si44_Transmit_Header3 	,vals[3]); 	// Tx header
	Si44WriteReg(Si44_Check_Header3		,vals[3]); 	// Rx header to look for
	Si44WriteReg(Si44_Transmit_Header2 	,vals[2]); 	// Tx header
	Si44WriteReg(Si44_Check_Header2		,vals[2]); 	// Rx header to look for
	Si44WriteReg(Si44_Transmit_Header1 	,vals[1]); 	// Tx header
	Si44WriteReg(Si44_Check_Header1		,vals[1]); 	// Rx header to look for
	Si44WriteReg(Si44_Transmit_Header0 	,vals[0]); 	// Tx header
	Si44WriteReg(Si44_Check_Header0		,vals[0]); 	// Rx header to look for

	temp = (temp>>numtocheck)&0xf;
	Si44WriteReg(Si44_Header_Control1,temp); 	// Check for upto 4 header bytes
	// Interrupt on
	Si44_IRQ_EN();

	return;
}

void Si44RadioIdle(void)
{
	// Interrupt off
	Si44_IRQ_OFF();
	// Dummy read interrupt sources to clear flags
	Si44ReadReg(Si44_Interrupt_Status1);
	Si44ReadReg(Si44_Interrupt_Status2);
	Si44_IRQ_FLAG = 0;
	// Set ready mode - 200us to Tx/Rx
	Si44WriteReg(Si44_OFC1,0x01); // Goto idle/ready (~500us)
	Si44Status.ofc1 = 0x01;
	Si44RadioState = SI44_IDLE; 
	// Interrupt on
	Si44_IRQ_EN();
	return;
}

void Si44RadioStandby(void)
{
	// Interrupt off
	Si44_IRQ_OFF();
	// Dummy read interrupt sources to clear flags
	Si44ReadReg(Si44_Interrupt_Status1);
	Si44ReadReg(Si44_Interrupt_Status2);
	Si44_IRQ_FLAG = 0;
	// Set low power standby mode
	Si44WriteReg(Si44_OFC1,0x00); // Goto standby 
	Si44Status.ofc1 = 0x00;
	Si44RadioState = SI44_STANDBY; 
	// Interrupt on
	Si44_IRQ_EN();
	return;
}

void Si44RadioOff(void)
{
	Si44_IRQ_OFF();
	Si44RadioStandby();
	Si44_SPI_OFF();
	Si44RadioState = SI44_OFF; 
}

unsigned char Si44RadioRx(unsigned char doSleep)
{
	// Check radio is ready to transmit
	if(Si44RadioState != SI44_IDLE && Si44RadioState != SI44_STANDBY) return 0;

	Si44_IRQ_OFF();
	if(!doSleep)
	{
		Si44WriteReg(Si44_OFC1,0x05); // Reciever on, next state ready 
		Si44Status.ofc1 = 0x05;
	}
	else
	{
		Si44WriteReg(Si44_OFC1,0x04); // Reciever on, next state standby 	
		Si44Status.ofc1 = 0x04;
	}
	Si44RadioState = SI44_RXING; 
	Si44_IRQ_EN();
	return 1;
}

unsigned char Si44RadioReadPkt(unsigned char* header, unsigned char* data, unsigned char maxLen, unsigned char* rssi)
{
	unsigned char count, i;
	// Check we have a packet first
	if(Si44RadioState != SI44_RXED) return 0;

	Si44_IRQ_OFF();

	// Get byte count	
	count = Si44ReadReg(Si44_Received_Packet_Len);

	// Get rssi
	if(rssi != NULL)
		*rssi = Si44Status.lastRSSI;

	// Get header bytes
	if (header!= NULL)
	{
		header[3] = Si44ReadReg(Si44_Received_Header3); 
		header[2] = Si44ReadReg(Si44_Received_Header2); 
		header[1] = Si44ReadReg(Si44_Received_Header1); 
		header[0] = Si44ReadReg(Si44_Received_Header0); 
	}

	// Get data bytes
	if(data != NULL)
	{
		Si44_SPI_OPEN();
		Si44_SPI_PUTC(0x7F & Si44_FIFO_Access);
		for(i=0;i<count;i++)
		{
			if(i >= maxLen)			// Observe user max buffer len
			{
				count = i;
				break;
			}
			Si44_SPI_PUTC(0xff); 	// Dummy write
			data[i] = Si44_SPI_GETC();
		}
		Si44_SPI_CLOSE();
	}

	// Indicate it was read
	if(Si44Status.ofc1 & 0b1)
		Si44RadioState = SI44_IDLE;
	else
		Si44RadioState = SI44_STANDBY;
	Si44_IRQ_EN();

	// Done
	return count;
}

// Returns when channel is clear or on timeout
void Si44RadioCCA(void)
{
	unsigned char timeout;
	Si44_IRQ_OFF();
	Si44WriteReg(Si44_Data_Access_Control,0x0c);	// Radio Rx packet handling off (no interrupts made)
	if(!(Si44Status.ofc1 & 0x04))					// Reciever is off, start it first
	{
		Si44WriteReg(Si44_OFC1,0x07); 				// Set receive state to read rssi
		Si44Status.ofc1 = 0x07;						// Turn on receiver
		DelayMs(1);									// Wait for receiver startup
	}
	for(timeout = 10; timeout > 0; timeout--)		// Try to avoid packet collisions using rssi
	{
		Si44Status.ezmacStat = 	Si44ReadReg(Si44_EzMAC_status); // Read radio state
		Si44Status.lastRSSI = Si44ReadReg(Si44_RSSI); 			// Read antenna rssi
		if ((Si44Status.lastRSSI < SI44_CCA_THRESHOLD) &&		// If rssi is low enough, continue
			(Si44Status.ezmacStat & 0x20))break;				// and receiver is on
		DelayMs(1);									// Wait. 150kbps 12 byte pkt is 1.28ms (+12 bytes preamble4,sync2,header4,crc2) 
	}
	Si44WriteReg(Si44_OFC1,0x03); 					// Set radio to tuned (ready to tx immediatley)
	Si44WriteReg(Si44_Data_Access_Control,0x8c);	// Radio packet handling back on

	Si44ReadReg(Si44_Interrupt_Status1);			// Clear any generated interrupts (packets etc)
	Si44ReadReg(Si44_Interrupt_Status2);
	Si44_IRQ_FLAG = 0;

	Si44_IRQ_EN();									// Now we are ready to tx
	return;
}

unsigned char Si44RadioTx(const unsigned char* data, unsigned short len, unsigned char dBmPwr, unsigned char doSleep)
{
	unsigned char txPwrRegSetting;

	// Check radio is ready to transmit
	if(Si44RadioState != SI44_IDLE && Si44RadioState != SI44_STANDBY) return 0;

	Si44_IRQ_OFF();

	// Dummy read interrupt sources to clear flags
	Si44ReadReg(Si44_Interrupt_Status1);
	Si44ReadReg(Si44_Interrupt_Status2);
	Si44_IRQ_FLAG = 0;

	if(dBmPwr>20)dBmPwr = 20;						// Clamp max
	txPwrRegSetting = (0x08|(((dBmPwr+1)/3)&0x07)); // Truncated by HW to steps of 3dBm
	Si44WriteReg(Si44_TX_Power,	txPwrRegSetting);	// Direct tie bit set too

	Si44WriteReg(Si44_Transmit_Packet_Len, len); 	// Write num bytes to transmit
	
	Si44WriteReg(Si44_OFC2,0x01); 					// Reset Tx fifo
	Si44WriteReg(Si44_OFC2,0x00); 					// Reset Tx fifo

	Si44_SPI_OPEN();
	Si44_SPI_PUTC(0x80 | Si44_FIFO_Access);
	for(;len>0;len--)
	{
		Si44_SPI_PUTC(*data++); 					// Write bytes to fifo
	}	
	Si44_SPI_CLOSE();

	if(!doSleep)
	{
		Si44WriteReg(Si44_OFC1,0x09); 				// Set Tx state, note: next state is ready
		Si44Status.ofc1 = 0x09;	
	}
	else
	{
		Si44WriteReg(Si44_OFC1,0x08); 				// Set Tx state, note: next state is standby
		Si44Status.ofc1 = 0x08;
	}

	Si44RadioState = SI44_TXING; 
	Si44_IRQ_EN();

	return 1;
}

signed char Si44ReadRssidBm(void)
{
	signed char ret;
	Si44_IRQ_OFF();
	ret = Si44Status.lastRSSI = Si44ReadReg(Si44_RSSI); 	// Read antenna rssi
	Si44_IRQ_EN();
	ret = -128 + (ret>>1); 	
	return ret;
}

void Si44RadioReadStatus(void)
{
	// Get radio status
	Si44_IRQ_OFF();
	Si44Status.ezmacStat = 	Si44ReadReg(Si44_EzMAC_status);
	Si44Status.deviceStat = Si44ReadReg(Si44_Device_Status);
	Si44Status.ofc1 = 		Si44ReadReg(Si44_OFC1);
	Si44_IRQ_EN();
}

/*
Interrupts:
INTSTAT1
7: ifferr 
6: itxffafull 
5: itxffaem 
4: irxffafull 
3: iext 
2: pksent 
1: ipkvalid 
0: icrerror
INTSTAT2
7: iswdet 
6: ipreaval 
5: ipreainval 
4: irssi 
3: iwut 
2: ilbd 
1: ichiprdy 
0: ipor
*/

void Si44RadioIrqHandler(void)
{
	unsigned char intStat[2];
	Si44_IRQ_FLAG = 0;

	// Get radio status
	Si44RadioReadStatus();
	// Device won't consistently latch new ofc1 setting without write
	Si44WriteReg(Si44_OFC1,Si44Status.ofc1);
	
	// Get interrupt event flags
	intStat[0] = Si44ReadReg(Si44_Interrupt_Status1);
	intStat[1] = Si44ReadReg(Si44_Interrupt_Status2);
	
	// Packet sent flag 
	if(intStat[0] & 0x04)
	{
		// Packet sent
		if(Si44Status.ofc1 & 0b1)
			Si44RadioState = SI44_IDLE;
		else 
		{
			Si44RadioState = SI44_STANDBY;
		}
		DBG_INFO("\r\nSi44 pkt sent ->");
	}
	// Packet received flag
	else if(intStat[0] & 0x02)
	{
		// Handle packet received
		Si44RadioState = SI44_RXED;
		DBG_INFO("\r\nSi44 pkt rxed <-");
	}

	// // Sync word detected, start of packet, latch rssi if used
	if(intStat[1] & 0x80)	
	{
		Si44Status.lastRSSI = Si44ReadReg(Si44_RSSI);
	}

	// Device reset flag and eroneous interrupt check
	if((intStat[1] & 0x01)||(intStat[0] == 0 && intStat[1] == 0))	
	{
		// Indicative of hardware error
		Si44RadioState = SI44_HW_ERROR; 
		DBG_ERROR("Si443x.c: Reset detected");
	}

	return;
}








//EOF
