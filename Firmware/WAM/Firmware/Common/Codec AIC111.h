// Codec interface for the AIC111
// Written for PIC32 architecture
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
// The following define sets the codec read register - this is read once per sample (8LSBs of data word)
#ifdef __C32__
#define CODEC_READ_REG 	PGACREG
#elif defined __C30
#define CODEC_READ_REG 	0 /*Must be this otherwise bit-phase inversion results in corruption*/
#endif	
// Uncomment this to have the sample function return the global regs and isr to be left running
//#define CODEC_ISR_UPDATE_DRIVEN 

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

// Defines
#ifdef __C32__
	//#warning "Hard coded to use SPI2 here"
	#define NUM_TRANSFERS_PER_WRITE 2
	#define CODEC_INTS_INIT()		{IPC7SET = 0x10000000; /* Set interrupt priority 4*/\
									IFS1CLR = 0x000000e0; /* Clear interrupt flags TXIF,RXIF,ERIF*/}
	#define CODEC_INTS_CLR()		{IFS1CLR = 0x000000E0;}
	#define CODEC_INTS_UNMASK()		{IEC1SET = 0x000000e0;}
	#define CODEC_INTS_MASK()		{IEC1CLR = 0x000000e0;}
	#define CODEC_CLEAR_OVERFLOW()	{SPI2STATCLR=0x40;}
	#define CODEC_SPI_ON() 			{SPI2CONbits.ON = 1;}
	#define CODEC_INIT_SPI()		{\
									IEC1CLR = 0x000000e0;/* Clear interrupt enables */\
									SPI2CONbits.FRMEN = 1; /* Framed SPI */\
									SPI2CONbits.FRMSYNC = 1; /* Frame sync slave */\
									SPI2CONbits.FRMPOL = 1; /* Frame sync active high */\
									SPI2CONbits.SPIFE = 1; /* Frame sync coincided with first clock */\
									/*SPI2CONbits.ENHBUF = 1;  Use enhanced fifos */\
									SPI2CONbits.SIDL = 0; /* Continue module opperation in idle */\
									SPI2CONbits.MODE32 = 1; /* 32bit data, 32bit fifo */\
									SPI2CONbits.MODE16 = 0; /* 32bit data, 32bit fifo  */\
									SPI2CONbits.SMP = 0; /* This bit is ignored in slave mode */\
									SPI2CONbits.CKP = 1; /* Clock setting - 1 = latches data on falling edge */\
									SPI2CONbits.CKE = 0; /* This bit is not used but should be cleared */\
									/*SPI2CONbits.SSEN = 1;  SS pin used by module */\
									SPI2CONbits.MSTEN = 0; /* Slave mode */\
									SPI2CONbits.STXISEL = 0b00; /* Interrupt when the out fifo is empty */\
									SPI2CONbits.SRXISEL = 0b11; /* Interrupt when the in fifo is full */}
	#define CODEC_FRAME_POL(_x)		/*Not neccessary, 32 mode supported*/
	#define CODEC_SPI_OFF()			{SPI2CONCLR = _SPI2CON_ON_MASK;}
	#define CODEC_RESULT_REG		SPI2BUF
#elif defined __C30
#ifdef __dsPIC33E__
	//#warning "Hard coded to use SPI2 here"
	#define NUM_TRANSFERS_PER_WRITE 4
	#define CODEC_INTS_INIT()		{IPC8bits.SPI2IP = 4; IPC8bits.SPI2EIP = 4;/* Set interrupt priority 4*/\
									IFS2bits.SPI2IF = 0; IFS2bits.SPI2EIF = 0; /* Clear interrupt flags TXIF,RXIF,ERIF*/}
	#define CODEC_INTS_CLR()		{IFS2bits.SPI2IF = 0; IFS2bits.SPI2EIF = 0;}
	#define CODEC_ERROR_HANDLER()  	{if(CODEC_RESULT_REG);IFS2bits.SPI2EIF = 0;SPI2STATbits.SPIROV = 0;}
	#define CODEC_INTS_UNMASK()		{IEC2bits.SPI2IE = 1; IEC2bits.SPI2EIE = 1;}
	#define CODEC_INTS_MASK()		{IEC2bits.SPI2IE = 0; IEC2bits.SPI2EIE = 0;}
	#define CODEC_CLEAR_OVERFLOW()	{SPI2STATbits.SPIROV = 0;}
	#define CODEC_SPI_ON() 			{SPI2STATbits.SPIEN = 1;}
	#define CODEC_INIT_SPI()		{\
									SPI2STATbits.SPIEN = 0;		/* Off*/\
									SPI2STATbits.SPISIDL = 0;	/* Continue in idle*/\
									SPI2STATbits.SISEL = 0b011; /* Interrupt when RX buffer full (8*16bit transfers, 4 samples)*/\
									SPI2CON1bits.DISSCK = 1;	/* Disable clk*/\
									SPI2CON1bits.MODE16 = 1;	/* 16bit transfers*/\
									SPI2CON1bits.SMP = 0;		/* This bit must be cleared in slave mode */\
									SPI2CON1bits.CKP = 1;		/* Clock setting - 1 = latches data on falling edge */\
									SPI2CON1bits.CKE = 0;		/* Should be cleared in framed mode */\
									SPI2CON1bits.SSEN = 1;		/* Ignored in framed mode */\
									SPI2CON1bits.MSTEN = 0;		/* Slave mode*/\
									SPI2CON2bits.FRMEN = 1;		/* Framed mode enabled*/\
									SPI2CON2bits.SPIFSD = 1;	/* Frame sync pin is input*/\
									/*SPI2CON2bits.SPIFPOL = 1;	 Frame sync polarity*/\
									/*SPI2CON2bits.SPIFE = 0;	 Frame sync precedes with first clock*/\
									SPI2CON2bits.FRMPOL = 1;	/* Frame sync polarity*/\
									SPI2CON2bits.FRMDLY = 0;		/* Frame sync conicides with first clock*/\
									frame_pol = 0;\
									SPI2CON1bits.PPRE0 = 1;	/* Primary prescaler*/\
									SPI2CON1bits.PPRE1 = 1;	/* Primary prescaler*/\
									SPI2CON2bits.SPIBEN = 0;	/* Enhanced buffer mode disabled*/}
	#define CODEC_FRAME_REGS		0
	#define CODEC_FRAME_ADC			1
	extern volatile unsigned char frame_pol;
	#define CODEC_FRAME_POL(_x)		{unsigned char spien_save = SPI2STATbits.SPIEN;CODEC_SPI_OFF();SPI2CON2bits.FRMPOL = (_x)?0:1;SPI2CON2bits.FRMEN = (1);SPI2STATbits.SPIEN = spien_save;frame_pol = _x;}			
	#define CODEC_SPI_OFF()			{SPI2STATbits.SPIEN = 0;}
	#define CODEC_RESULT_REG		SPI2BUF

#else

	//#warning "Hard coded to use SPI2 here"
	#define NUM_TRANSFERS_PER_WRITE 4
	#define CODEC_INTS_INIT()		{IPC8bits.SPI2IP = 4; IPC8bits.SPF2IP = 4;/* Set interrupt priority 4*/\
									IFS2bits.SPI2IF = 0; IFS2bits.SPF2IF = 0; /* Clear interrupt flags TXIF,RXIF,ERIF*/}
	#define CODEC_INTS_CLR()		{IFS2bits.SPI2IF = 0; IFS2bits.SPF2IF = 0;}
	#define CODEC_ERROR_HANDLER()  	{if(CODEC_RESULT_REG);IFS2bits.SPF2IF = 0;SPI2STATbits.SPIROV = 0;}
	#define CODEC_INTS_UNMASK()		{IEC2bits.SPI2IE = 1; IEC2bits.SPF2IE = 1;}
	#define CODEC_INTS_MASK()		{IEC2bits.SPI2IE = 0; IEC2bits.SPF2IE = 0;}
	#define CODEC_CLEAR_OVERFLOW()	{SPI2STATbits.SPIROV = 0;}
	#define CODEC_SPI_ON() 			{SPI2STATbits.SPIEN = 1;}
	#define CODEC_INIT_SPI()		{\
									SPI2STATbits.SPIEN = 0;		/* Off*/\
									SPI2STATbits.SPISIDL = 0;	/* Continue in idle*/\
									SPI2STATbits.SISEL = 0b011; /* Interrupt when RX buffer full (8*16bit transfers, 4 samples)*/\
									SPI2CON1bits.DISSCK = 1;	/* Disable clk*/\
									SPI2CON1bits.MODE16 = 1;	/* 16bit transfers*/\
									SPI2CON1bits.SMP = 1;		/* This bit must be cleared in slave mode */\
									SPI2CON1bits.CKP = 1;		/* Clock setting - 1 = latches data on falling edge */\
									SPI2CON1bits.CKE = 0;		/* Should be cleared in framed mode */\
									SPI2CON1bits.SSEN = 1;		/* Ignored in framed mode */\
									SPI2CON1bits.MSTEN = 0;		/* Slave mode*/\
									SPI2CON2bits.FRMEN = 1;		/* Framed mode enabled*/\
									SPI2CON2bits.SPIFSD = 1;	/* Frame sync pin is input*/\
									/*SPI2CON2bits.SPIFPOL = 1;	 Frame sync polarity*/\
									/*SPI2CON2bits.SPIFE = 0;	 Frame sync precedes with first clock*/\
									SPI2CON2bits.SPIFPOL = 1;	/* Frame sync polarity*/\
									SPI2CON2bits.SPIFE = 0;		/* Frame sync conicides with first clock*/\
									frame_pol = 0;\
									SPI2CON2bits.SPIFE = 0;		/* Frame sync conicides with first clock*/\
									SPI2CON1bits.PPRE0 = 1;	/* Primary prescaler*/\
									SPI2CON1bits.PPRE1 = 1;	/* Primary prescaler*/\
									SPI2CON2bits.SPIBEN = 0;	/* Enhanced buffer mode enabled*/}
	#define CODEC_FRAME_REGS		0
	#define CODEC_FRAME_ADC			1
	extern volatile unsigned char frame_pol;
	#define CODEC_FRAME_POL(_x)		{unsigned char spien_save = SPI2STATbits.SPIEN;CODEC_SPI_OFF();SPI2CON2bits.SPIFPOL = (_x)?0:1;SPI2CON2bits.SPIFE = (_x);SPI2STATbits.SPIEN = spien_save;frame_pol = _x;}			
	#define CODEC_SPI_OFF()			{SPI2STATbits.SPIEN = 0;}
	#define CODEC_RESULT_REG		SPI2BUF
#endif
#else
	#error "Compiler?"
#endif

// Prototypes
extern void CodecInit(void);
extern void CodecInitOff(void);
void CodecInitInterrupt(void);
extern void CodecSetupRegs(unsigned char reg1,unsigned char reg2,unsigned char reg3,unsigned char reg4,unsigned char reg5);
extern signed short GetSample(void);
signed short GetRegRead(void);  /*Valid after call to ReadReg(), -1 indicates invalid*/
extern short WriteReg(unsigned char reg, unsigned char val);
extern short ReadReg(unsigned char reg);
unsigned char CodecBusy(void);
// Private - debug only 
extern void CodecISRHandler(void); /*Called from ISR*/

// This inline functionis only valid if CODEC_ISR_UPDATE_DRIVEN is not defined
extern __inline__ signed short __attribute__((always_inline)) GetSampleInline(void)
{
	#ifdef __C32__
		CODEC_CLEAR_OVERFLOW();							// Clear overflow
		CODEC_RESULT_REG = 0x00000000; 					// This is the data sent to the device
		return (signed short)(CODEC_RESULT_REG >> 16);  // Return upper 16 bits of the result
	#elif defined __C30
		signed short samples[16];
		int i;
//LED_R = 1;
		for(i=0;i<16;i++)
		{
			samples[i] = CODEC_RESULT_REG;
		}
		CODEC_CLEAR_OVERFLOW();
//LED_R = 0;
		if(samples[0])			return samples[0];
		else if (samples[1])	return samples[1];
		else return samples[2];
	#endif
}

#endif

/*
Example verification code:
CodecInit();
CODEC_FRAME_POL(CODEC_FRAME_REGS);
ReadReg(PGACREG); // Can view PGA settling on scope if desired, take 2-4s
{
	unsigned char i;
	static unsigned char values[8];
	DelayMs(4000);
	for (i=0;i<8;i++)
	{
		ReadReg(i);
		while(CodecBusy());
		values[i] = GetRegRead();
	}
	Nop(); // Break here to inspect regs
	Nop();
	Nop();
}
*/
//EOF
