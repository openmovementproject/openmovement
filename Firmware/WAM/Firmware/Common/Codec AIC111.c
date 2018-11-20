// Codec interface for the AIC111
// Written for PIC32 architecture
// KL 26-04-2012
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "Codec AIC111.h"
#include "HardwareProfile.h"

// Globals
static codec_din_t data_in[2] = {{0}};
static codec_dout_t data_out[2] = {{0}};
static signed char use_index = -1; // ISR will write to other index if it interrupts user code
static volatile unsigned short frame_count = 8;
#ifdef __C30
volatile unsigned char frame_pol = 0; // 0 means registers are used in frame, 1 means adc/dac
#endif

//#define CODEC_DEBUG

// Prototypes
void CodecInit(void)
{
	CODEC_ENABLE();
	DelayMs(10);						// Let power settle
	CodecInitInterrupt();				// Interrupts turned on
	#ifdef CODEC_DEBUG
		#warning "Codec not inited properly!"
		// Debug only
		return;	#endif
	CodecSetupRegs(CODEC_SETUP_REGS);	// Codec registers written
	#ifndef CODEC_ISR_UPDATE_DRIVEN	
		CODEC_INTS_MASK();
		#ifdef __C30
			// For C30 we need to swap back to reading the adc/dac registers
			// and enable the buffers to allow asynchronous sampling - fixes p24 bug
			CODEC_FRAME_POL(CODEC_FRAME_ADC);
			CODEC_SPI_OFF();
			SPI2CON2bits.SPIBEN = 1;
			CODEC_SPI_ON();
		#endif
	#endif
	return;
}
void CodecInitOff(void)
{
	// User should clear other interrupts accessing device first
	CODEC_INTS_MASK();				// Diable any SPI interrupts if still on
	CODEC_SPI_OFF(); 				// Disable spi module
	CODEC_INIT_PINS(); 					// Powers off device
	return;
}
void CodecInitInterrupt(void)
{
	/* 
	Setup framed SPI interface for AIC111 on SPI2
	IF IMODE = 1
	After the frame sync pulse there in a delay, then data is clocked in/out on 24 rising edges
	The first 16 bits read are big endian adc conversion, then 8bits data
	The first 20 bits data out is the DAC value, then the W/R bit and a 3bit address. I.E. (Host ref'd)
	SDI <ADC RESULT 15:0><DATA 7:0>
	SDO <DAC VALUE 19:0><W/R><ADD 2:0>
	IF IMODE = 0
	After the frame sync pulse rising edge data is clocked in/out on 32 rising edges
	The first 16 bits read are big endian adc conversion, then 8bits data
	The first 20 bits data out is the DAC value, then the W/R bit and a 3bit address. I.E. (Host ref'd)
	SDI <ADC RESULT 15:0><UNUSED ZEROs 7:0><DATAIN 7:0>
	SDO <DAC VALUE 19:0><W/R ><REGADD 2:0><DATAOUT 7:0>
	*/

	data_out[0].all = 0;
	data_out[1].all = 0;
	use_index = -1; // i.e. unused

	CODEC_ENABLE();			// Codec will be powered and start generating data
	CODEC_INIT_SPI();

	// Empty rx buffer
	{unsigned char i; for(i=0;i<32;i++){volatile unsigned short dummy = CODEC_RESULT_REG;if(dummy);}}

	CODEC_INTS_INIT();
	CODEC_INTS_UNMASK();
	CODEC_CLEAR_OVERFLOW();

	#ifdef __C32__
	INTEnableSystemMultiVectoredInt();
	#endif 
	
	CODEC_SPI_ON();
	CODEC_ENABLE();		// Not sure why the power pin gets cleared when setting above bit
	return;
}

// DO NOT SCHEDULE ANY OF THE ABOVE FUNCTIONS TO RUN AT HIGHER PRIORITY TO THIS ONE!!!!
extern void __attribute__((always_inline))CodecISRHandler(void) /*Called from ISR*/
{
	// Clear flags and overflow
	CODEC_INTS_CLR();
	CODEC_CLEAR_OVERFLOW();
	{
	// Taansfer data TX/RX data to buffers
	#ifdef __C32__
		static unsigned long dummy;
		// Index to use
		if(use_index < 0)
		{
			//LED_G = !LED_G;
			// Assume the interrupt will need to service both in and out buffers
			dummy = CODEC_RESULT_REG;  	// Read the SPI buffer
			CODEC_RESULT_REG = 0; 			// Write the SPI buffer
		}
		else
		{
			if(use_index)use_index = 0;
			else use_index = 1;
	
			// Assume the interrupt will need to service both in and out buffers
			data_in[use_index].all = CODEC_RESULT_REG;  // Read a conversion
			CODEC_RESULT_REG = data_out[use_index].all; // Write the SPI buffer
		}
		// Decrement frame count - used to qualify reads/writes of registers
		if (frame_count) frame_count--;
	#elif defined __C30
		volatile short dummy;
		if (CODEC_FRAME != frame_pol){ // The pic24 generates interrupts twice per frame, the int occurs after the frame - this is a bug fix
			// Index to use
			if(use_index < 0)
			{
				// Assume the interrupt will need to service both in and out buffers
				#ifdef CODEC_DEBUG // Debugging only - to check timing
					static unsigned char i;
					dummy = (0xA000|i++);		
					CODEC_RESULT_REG = dummy;	// Write the SPI buffer
					dummy = CODEC_RESULT_REG;  	// Read the SPI buffer
					return;
				#else
					dummy = CODEC_RESULT_REG;  	// Read the SPI buffer
					CODEC_RESULT_REG = 0; 		// Write the SPI buffer
				#endif
	
			}
			else
			{
				if(use_index)use_index = 0;
				else use_index = 1;
				if (frame_pol == 0) 	// Accessing config registers
				{
					// Assume the interrupt will need to service both in and out buffers
					data_in[use_index].vals16[0] = CODEC_RESULT_REG;  // Read a conversion
					CODEC_RESULT_REG = data_out[use_index].vals16[0]; // Write the SPI buffer
				}
				else					// Accessing adc/dac registers
				{
					data_in[use_index].vals16[1] = CODEC_RESULT_REG;  // Read a conversion
					CODEC_RESULT_REG = data_out[use_index].vals16[1]; // Write the SPI buffer
				}
			}
			// Decrement frame count - used to qualify reads/writes of registers
			if (frame_count) frame_count--;
		}
		else
		{
			// Then this is the interrupt caused by the extra 16bits of data;
			LED_B = 1;
			CODEC_ERROR_HANDLER(); // Discard the extra word
			LED_B = 0;
		}	
	#endif
	}
	return;
}
signed short GetSample(void)
{
	#ifdef CODEC_ISR_UPDATE_DRIVEN
		if (use_index <0) return 0; 
		return	data_in[use_index].audio;
	#else
		return GetSampleInline();
	#endif
}

void CodecSetupRegs(unsigned char reg1,unsigned char reg2,unsigned char reg3,unsigned char reg4,unsigned char reg5)
{
	// Switch to correct frame timing for writing regs (C30 only)
	CODEC_FRAME_POL(CODEC_FRAME_REGS);
	frame_count = 8;
	while(frame_count){}

	while(frame_count){}
	WriteReg(0x01, reg1);
	while(frame_count){}
	WriteReg(0x02, reg2);
	while(frame_count){}
	WriteReg(0x03, reg3);
	while(frame_count){}
	WriteReg(0x04, reg4);
	while(frame_count){}
	WriteReg(0x05, reg5);
	while(frame_count){}

	ReadReg(CODEC_READ_REG); // This value is latched to data_out[all].all (in C32 case only);
	while(frame_count){}

	return;
}

// For interrupt drive read/writes only
signed short GetRegRead(void) 
{
	if (frame_count) return -1;
#ifdef __C32__
	else return  data_in[use_index].val;
#elif defined __C30
	else return  (data_in[use_index].vals16[0]>>1); // Fix timing issue
#endif
}

/*Note will take two codec cycles to become valid, -1 indicates unsuccessful*/
short WriteReg(unsigned char reg, unsigned char val)
{
	if (frame_count) return -1;
	use_index = -1;
	data_out[0].all = 0;
	data_out[0].wr = 1;
	data_out[0].reg = reg;
	data_out[0].val = val;
	data_out[1].all = 0;
	data_out[1].wr = 1;
	data_out[1].reg = reg;
	data_out[1].val = val;
	frame_count += NUM_TRANSFERS_PER_WRITE;
	use_index = 0;
	return 0;	
}
/*Note will take two codec cycles to become valid, -1 indicates unsuccessful*/
short ReadReg(unsigned char reg)
{
	if (frame_count) return -1;
	use_index = -1;
	data_out[0].all = 0;
	data_out[0].wr = 0;
	data_out[0].reg = reg;
	data_out[0].val = 0;
	data_out[1].all = 0;
	data_out[1].wr = 0;
	data_out[1].reg = reg;
	data_out[1].val = 0;
	frame_count += NUM_TRANSFERS_PER_WRITE;
	use_index = 0;
	return 0;
}

unsigned char CodecBusy(void)
{
	return (frame_count==0)?FALSE:TRUE;
}

//EOF

