// Codec interface for the AIC111
// Written for dsPIC33 architecture
// KL 26-04-2012

#ifndef _CODEC_AIC111_H
#define _CODEC_AIC111_H

#include "HardwareProfile.h"

// Register definitions
#define PGACREG 	0x01
#define HPFSFTREG 	0x02
#define PDCREG 		0x03
#define FASTARREG 	0x04
#define SLOWARREG 	0x05

// Setup simplified parameters,used by codec init, list register settings to write - see datasheet
#define CODEC_SETUP_REGS 0xC6,0b00010000, 0b00000010, 0b11110111, 0b01000010
#define CODEC_SETUP_FIXED 0xD2,0b00010000, 0b00001110, 0b11110111, 0b01000010
// The following define sets the codec read register - this is read once per sample (8LSBs of data word)
#define CODEC_READ_REG 	PGACREG
//#define CODEC_READ_REG	FASTARREG 

extern const unsigned char codec_default_regs[];

// Data types
typedef union {
	struct {
		unsigned char val;
		unsigned int reg : 3;
		unsigned int wr : 1; /*1=READ, and 0=WRITE.*/
		signed long dac_val : 20;
		};	 
	unsigned long all;
	unsigned short vals16[2];
}codec_dout_t;

typedef union {
	struct {
		unsigned char val;
		unsigned char zeros;
		signed short audio;
		}; 
	unsigned long all;
	unsigned short vals16[2];
}codec_din_t;

#ifdef CODEC_DEBUG
extern unsigned char gCodecDebugWrite[6];
extern unsigned char gCodecDebugRead[6];
#endif

// Defines

// Prototypes
extern void CodecInit(const unsigned char*);
extern void CodecInitOff(void);

extern void SetupOC1toTriggerSamplingDMA(int period);	// period=2 for 16.384kHz, period=4 for 8.192kHz
extern void AssignDma0CircularSampleBuffer(short* addressStartA, short numElementsTotal);
extern void Dma0Pause(void);
extern void Dma0Resume(void);

// Private - debug only 
extern void CodecISRHandler(void); /*Called from ISR*/
extern void CodecSetupRegs(unsigned char reg1,unsigned char reg2,unsigned char reg3,unsigned char reg4,unsigned char reg5);
extern short WriteReg(unsigned char reg, unsigned char val);
extern short ReadReg(unsigned char reg);
extern unsigned char CodecBusy(void);

// This inline functionis only valid if CODEC_ISR_UPDATE_DRIVEN is not defined
#define GetSample()	GetSampleInline()
#define GetSampleInline()	(RXBUF0)
#define GetGainInline()	    (RXBUF1)

#endif
//EOF
