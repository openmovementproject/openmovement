// Header file for the Si443x radio
#ifndef _SI44_H_
#define _SI44_H_

// Types
// Current radio state
typedef enum {
	SI44_OFF = 0,		// 0, 	Unknown state
	SI44_HW_ERROR = 1,	// 1, 	Error occured	
	SI44_INITIALISED = 2,// 2 Duplicate, 1uA current draw, 500us to tx/rx
	SI44_STANDBY = 2,	// 2 Duplicate, 1uA current draw, 500us to tx/rx
	SI44_IDLE = 3,		// 3	1mA current draw, 1us to tx/rx
	SI44_RXING = 4,		// 4	19mA receiver on
	SI44_RXED = 5,		// 5	packet received (could be idle/standby)
	SI44_TXING = 6		// 6	20-85mA transmitting
}Si44RadioState_t;

// Radio command type, len, data* depends on cmdType 
typedef struct { /*Command type for radio control*/
	unsigned char type;
	unsigned char len;
	void* 	data;
}Si44Cmd_t;

// Format of read/write lists
// LSB is the register, MSB is the value
typedef short Si44Reg_t;

// Register descriptor defines
#define SI44_REG_TYPE_EOL	0x0000	// End of list special symbol (read zero bytes from or write to device version reg)
#define Si44_MAKE_LIST_VAL(_r,_v)	((((unsigned short)_v)<<8)|((unsigned char)_r)) 
#define Si44_MAKE_LIST_VAL_NB(_r,_v)	((((unsigned short)_v)<<8)|((unsigned char)_r)) 

// Useful macros
#define RssiTodBm(_c) 		((signed char)-128 + ((unsigned char)_c>>1))	// Returns signed dBm rssi from raw reg val
#define TxdBmToTxReg(_p)	((unsigned char)0x08|(((_p+1)/3)&0x07))			// Returns raw reg val for Si44_TX_Power to set closest dBm

// Format event meassages
typedef struct  { /*Event messages to host*/
	unsigned char type;	// Event type
	unsigned char err;	// Error code or zero
	unsigned char len;	// Event length
	unsigned char* data;// Data, [len] bytes
} Si44Event_t;

// Radio command types AND event types
typedef enum {
	SI44_RESET				= 0,	/*cmd data* is a NULL | evt is 0,0,0,NULL*/
	SI44_CMD_STANDBY		= 1,	/*cmd data* is a NULL | evt is 0,0,0,NULL*/
	SI44_CMD_IDLE			= 2,	/*cmd data* is a NULL | evt is 0,0,0,NULL*/
	SI44_TX					= 3,	/*cmd data* is a NULL | evt is 0,0,0,NULL*/
	SI44_RX					= 4,	/*cmd data* is a NULL or *uint8 NUM_TO_RX (FF = indefinite) | evt is 0,0,0,NULL*/
	SI44_WRITE_REG			= 5,	/*cmd data* is a reg, data array[len-1] | evt is 5,0,0,NULL*/
	SI44_WRITE_REG_LIST		= 6,	/*cmd data* is a write list of reg, value entrys | evt is 6,0,0,NULL*/
	SI44_READ_REG_LIST		= 7,	/*cmd data* is a list of reg, len entrys | evt is 7,0,N,*data[N]*/
	SI44_READ_PKT			= 8,	/*cmd data* is ignored, last/current packet is re-read | evt is 8,0,(pktLen + 5),*{headers[4],rssi[1],pkt[pktLen]}*/
	SI44_TX_DONE			= 9,	/*cmd not valid | packet sent evt is 9,0,0,NULL*/
	SI44_EVT_ERR			= 10,	/* evt is 10, err number, 0, NULL*/
	SI44_CMD_EOL			= 0xff	/* used internally, may be used to gate external cmds*/
}Si44ctrl_t;

// Enumerated errors
typedef enum {
	SI44_OK = 0,
	SI44_HW_ERR = 1,
	SI44_BAD_CMD = 2,
	SI44_NO_MEM = 3
} Si44errs_t;

// Globals
extern volatile Si44RadioState_t	Si44RadioState;	// User state of radio

// User event callback
typedef void(*Si44EventCB_t)(Si44Event_t* evt);
extern volatile Si44EventCB_t Si44EventCB;

// Prototypes
// Send command list to radio, null or SI44_CMD_EOL terminated
void Si44CommandList(const Si44Cmd_t* cmdList);
// Send single command to radio
Si44Event_t* Si44Command(const Si44Cmd_t* cmd, void* buffer);
// Callback for events
void Si44SetEventCB(Si44EventCB_t CB);
// Private prototypes
void Si44RadioIrqHandler(void); // Called from radios ISR

// Private
extern unsigned char localPktBuffer[64+4+1];
// Internal radio interrupt source regs
typedef union {
	unsigned char b[2];
	struct {
	// INTSTAT1
	unsigned int icrerror 	: 1; 
	unsigned int ipkvalid 	: 1;
	unsigned int pksent	 	: 1;
	unsigned int iext 		: 1;
	unsigned int irxffafull 	: 1;
	unsigned int itxffaem 	: 1;
	unsigned int itxffafull 	: 1;
	unsigned int ifferr 		: 1;
	// INTSTAT2
	unsigned int ipor 		: 1;
	unsigned int ichiprdy 	: 1;
	unsigned int ilbd 		: 1;
	unsigned int iwut  		: 1;
	unsigned int irssi 		: 1;
	unsigned int ipreainval  : 1;
	unsigned int ipreaval 	: 1;
	unsigned int iswdet  	: 1;
	};
} Si44IntStat_t;

// Internal status type for debug
typedef union {
	unsigned char val8[4];
	struct {
		unsigned char ezmacStat;
		unsigned char deviceStat;
		unsigned char ofc1;
		unsigned char lastRSSI;
	};
	struct {
		unsigned int packet_txed 	: 1;
		unsigned int packet_txing 	: 1;
		unsigned int crc_error 		: 1;
		unsigned int packet_rxed 	: 1;
		unsigned int packet_rxing 	: 1;
		unsigned int packet_searching : 1;
		unsigned int rxcrc1 		: 1;
		unsigned int reserved1 		: 1;

		unsigned int chip_power_state: 2;
		unsigned int reserved2 		: 1;
		unsigned int frequency_err 	: 1;
		unsigned int rxpkt_header_err : 1;
		unsigned int rxfifo_empty 	: 1;
		unsigned int fifo_underflow : 1;
		unsigned int fifo_overflow 	: 1;

		unsigned int ofc1_xton		: 1;
		unsigned int ofc1_pllon		: 1;
		unsigned int ofc1_rxon		: 1;
		unsigned int ofc1_txon		: 1;
		unsigned int ofc1_x32ksel	: 1;
		unsigned int ofc1_enwt		: 1;
		unsigned int ofc1_enlbd		: 1;
		unsigned int ofc1_swres		: 1;

		unsigned int rssi			: 8;
		};
} Si44Status_t;

// Private globals
extern volatile Si44Status_t 	Si44Status; 	// Debug only
extern volatile Si44IntStat_t	Si44RadioInts;	// Debug only

// Si443x Control registers
#define        Si44_Device_Type        		0x00
#define        Si44_Device_Version        	0x01
#define        Si44_Device_Status        	0x02
#define        Si44_Interrupt_Status1       0x03
#define        Si44_Interrupt_Status2       0x04
#define        Si44_Interrupt_Enable1       0x05
#define        Si44_Interrupt_Enable2       0x06
#define        Si44_OFC1     				0x07
#define        Si44_OFC2  					0x08
#define        Si44_Crystal_Load      		0x09
#define        Si44_MCU_Clock_Out  			0x0A
#define        Si44_GPIO0_Configuration     0x0B
#define        Si44_GPIO1_Configuration     0x0C
#define        Si44_GPIO2_Configuration     0x0D
#define        Si44_IO_Port	        		0x0E
#define        Si44_ADC_Configuration       0x0F
#define        Si44_ADC_Offset        		0x10
#define        Si44_ADC_Value        		0x11
#define        Si44_Temperature_Control     0x12
#define        Si44_Temperature_Value       0x13
#define        Si44_Wake_Up_Timer1        	0x14
#define        Si44_Wake_Up_Timer2        	0x15
#define        Si44_Wake_Up_Timer3        	0x16
#define        Si44_Wake_Up_Val1        	0x17
#define        Si44_Wake_Up_Val2        	0x18
#define        Si44_Low_Duty_Cycle_Duration	0x19
#define        Si44_Low_Battery_Thresh 		0x1A
#define        Si44_Battery_Voltage        	0x1B
#define        Si44_IF_Filter_BW       		0x1C
#define        Si44_AFC_Loop_Override  		0x1D
#define        Si44_AFC_Timing_Control 		0x1E
#define        Si44_Clock_Recovery_Override	0x1F
#define        Si44_Clock_Recovery_Oversamp	0x20
#define        Si44_Clock_Recovery_Offset2 	0x21
#define        Si44_Clock_Recovery_Offset1 	0x22
#define        Si44_Clock_Recovery_Offset0 	0x23
#define        Si44_Clock_Recovery_Gain1  	0x24
#define        Si44_Clock_Recovery_Gain0  	0x25
#define        Si44_RSSI		        	0x26
#define        Si44_RSSI_For_CCA        	0x27
#define        Si44_Antenna_DivRSSI1      	0x28
#define        Si44_Antenna_DivRSSI2      	0x29
#define        Si44_AFC_Limiter        		0x2A
#define        Si44_AFC_Correction        	0x2B
#define        Si44_OOK_Counter1       		0x2C
#define        Si44_OOK_Counter2       		0x2D
#define        Si44_Slicer_Peak_Hold  		0x2E
#define        Si44_Data_Access_Control		0x30
#define        Si44_EzMAC_status        	0x31
#define        Si44_Header_Control1        	0x32
#define        Si44_Header_Control2        	0x33
#define        Si44_Preamble_Length        	0x34
#define        Si44_Preamble_Detection      0x35
#define        Si44_Sync_Word3        		0x36
#define        Si44_Sync_Word2        		0x37
#define        Si44_Sync_Word1        		0x38
#define        Si44_Sync_Word0        		0x39
#define        Si44_Transmit_Header3    	0x3A
#define        Si44_Transmit_Header2    	0x3B
#define        Si44_Transmit_Header1    	0x3C
#define        Si44_Transmit_Header0    	0x3D
#define        Si44_Transmit_Packet_Len    	0x3E
#define        Si44_Check_Header3        	0x3F
#define        Si44_Check_Header2        	0x40
#define        Si44_Check_Header1        	0x41
#define        Si44_Check_Header0        	0x42
#define        Si44_Header_Enable3        	0x43
#define        Si44_Header_Enable2        	0x44
#define        Si44_Header_Enable1        	0x45
#define        Si44_Header_Enable0        	0x46
#define        Si44_Received_Header3       	0x47
#define        Si44_Received_Header2       	0x48
#define        Si44_Received_Header1       	0x49
#define        Si44_Received_Header0       	0x4A
#define        Si44_Received_Packet_Len    	0x4B
#define        Si44_ADC8_Control        	0x4F
#define        Si44_CPCUU					0x58
#define        Si44_Channel_Filter_Coef    	0x60
#define        Si44_Crystal_Control         0x62
#define        Si44_AGC_Override1        	0x69
#define        Si44_TX_Power        		0x6D
#define        Si44_TX_Data_Rate1  			0x6E
#define        Si44_TX_Data_Rate0  			0x6F
#define        Si44_Modulation_Mode1      	0x70
#define        Si44_Modulation_Mode2      	0x71
#define        Si44_Frequency_Deviation     0x72
#define        Si44_Frequency_Offset1       0x73
#define        Si44_Frequency_Offset2       0x74
#define        Si44_Frequency_Band        	0x75
#define        Si44_Nominal_Carrier1       	0x76
#define        Si44_Nominal_Carrier0       	0x77
#define        Si44_Frequency_Hopping_Ch    0x79
#define        Si44_Frequency_Hopping_Size  0x7A
#define        Si44_TX_FIFO1     			0x7C
#define        Si44_TX_FIFO2     			0x7D
#define        Si44_RX_FIFO_Ctrl   			0x7E
#define        Si44_FIFO_Access        		0x7F

#endif
