// Codec interface for the AIC111
// Written for PIC32 architecture
// KL 26-04-2012
// Includes
#include <Compiler.h>
#include "GenericTypeDefs.h"
#include <TimeDelay.h>
#include "Codec AIC111 DCI.h"
#include "HardwareProfile.h"

// Globals
static volatile codec_din_t data_in = {{0}};
static volatile codec_dout_t data_out = {{0}};
static volatile unsigned short frame_count = 0;

const unsigned char codec_default_regs[5] = {CODEC_SETUP_FIXED};

#ifdef CODEC_DEBUG
unsigned char gCodecDebugWrite[6] = {0};
unsigned char gCodecDebugRead[6] = {0};
#endif

// Prototypes
void CodecInitDCI(void)
{
	/*Off*/
	Dma0Pause();
	IEC3bits.DCIIE = 0;
	DCICON1bits.DCIEN = 0;
	/*Setup*/
	DCICON1bits.DCISIDL = 0; /*Continue in idle*/
	DCICON1bits.DLOOP = 0; /*No loopback*/
	DCICON1bits.CSCKD = 1; /*Clock is input*/
	DCICON1bits.CSCKE = 1; /*Data sampled on rising edge*/
	DCICON1bits.COFSD = 1; /*Frame pin is input*/
	DCICON1bits.CSDOM = 0; /*Data out pin drive low when no data*/
	DCICON1bits.UNFM = 1;  /*Repeat last data on tx underflow*/
	DCICON1bits.DJST  = 1; /*Data coincides with frame pulse*/
	DCICON1bits.COFSM = 0b01; 	/*I2S frame mode*/
	DCICON2bits.BLEN = 0b01;  	/*2 data words buffered between ints, 1 I2S frame*/
	DCICON2bits.COFSG = 0b0000; /*Data frame has 1 word, 2 word I2S fram*/
	DCICON2bits.WS = 0b1111;	/*Word size is 16 bits*/
	DCICON3bits.BCG = 0;		/*Bit clock control bits, 0->off*/
	RSCON = 0; /*All receive data ignored for all slots*/
	TSCON = 0; /*All transmit data ignored for all slots*/
	RSCONbits.RSE0 = 1;			/*Enable first word of frame Rx*/
	TSCONbits.TSE0 = 1;			/*Enable first word of frame Tx*/
	/*Interrupts */
	IPC15bits.DCIIP=6; 
	IFS3bits.DCIIF = 0;
	IEC3bits.DCIIE = 0;
	/*Enable*/
	DCICON1bits.DCIEN = 1;		/*Codec on*/	
}

void CodecInit(const unsigned char* regs)
{
	if (regs == NULL) regs = codec_default_regs;

	CODEC_ENABLE();		// Power up
	DelayMs(10);		// Let power settle
	CodecInitDCI();  	// Setup DCI interface
	frame_count = 0;	// Stops ISR writing anything to codec
#ifdef USE_POLLED
	IEC3bits.DCIIE = 0; // Turn off the interrupt
#else
	IEC3bits.DCIIE = 1; // Turn on the interrupt
#endif
	CodecSetupRegs(regs[0],regs[1],regs[2],regs[3],regs[4]);	// Codec registers written
	IEC3bits.DCIIE = 0; // Turn off the interrupt
	DCICON2bits.BLEN = 0b11;//? This appears to work better
	return;
}

void CodecInitOff(void)
{
	// User should clear other interrupts accessing device first
	Dma0Pause();
	IEC3bits.DCIIE = 0;
	DCICON1bits.DCIEN = 0;
	CODEC_INIT_PINS(); 		// Powers off device
	return;
}

void __attribute__((__interrupt__, auto_psv)) _DCIInterrupt(void)
{
	IFS3bits.DCIIF = 0;
	if (frame_count)
	{
		// Frame counter decrement
		frame_count--;
		// Load tx buffers
		TXBUF0 = data_out.vals16[1];
		TXBUF1 = data_out.vals16[0];
		// Read rx buffers
		data_in.vals16[1] = RXBUF0;
		data_in.vals16[0] = RXBUF1; 
	}
	return;
}

void CodecSetupRegs(unsigned char reg1,unsigned char reg2,unsigned char reg3,unsigned char reg4,unsigned char reg5)
{
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

#ifdef CODEC_DEBUG
	{
		int i;
		gCodecDebugWrite[0] = 0x00;
		gCodecDebugWrite[1] = reg1;
		gCodecDebugWrite[2] = reg2;
		gCodecDebugWrite[3] = reg3;
		gCodecDebugWrite[4] = reg4;
		gCodecDebugWrite[5] = reg5;
		gCodecDebugRead[0] = 0x00;
		for (i = 1; i <= 5; i++)
		{
			gCodecDebugRead[i] = ReadReg(i);
		}	
	}
#endif

	ReadReg(CODEC_READ_REG); // This value is latched to data_out.all 

	return;
}

#ifndef USE_POLLED
/*Note will take two codec cycles to become valid, -1 indicates unsuccessful*/
short WriteReg(unsigned char reg, unsigned char val)
{
	if (frame_count) return -1;
	data_out.all = 0;
	data_out.wr = 1;
	data_out.reg = reg;
	data_out.val = val;
	frame_count += 2;
	return 0;	
}
/*Note will take two codec cycles to become valid, -1 indicates unsuccessful*/
short ReadReg(unsigned char reg)
{
	if (frame_count) return -1;
	data_out.all = 0;
	data_out.wr = 0;
	data_out.reg = reg;
	data_out.val = 0;
	frame_count += 8;		// was 2, 8 will work with any FIFO length
	while(frame_count){}
	return data_in.val;
}
unsigned char CodecBusy(void)
{
	return (frame_count==0)?FALSE:TRUE;
}
#else
/*Note will take two codec cycles to become valid, -1 indicates unsuccessful*/
short WriteReg(unsigned char reg, unsigned char val)
{
	int i;

	data_out.all = 0;
	data_out.wr = 1;
	data_out.reg = reg;
	data_out.val = val;

	IFS3bits.DCIIF = 0;

	// Load tx buffers
	TXBUF0 = data_out.vals16[1];
	TXBUF1 = data_out.vals16[0];
	// Read rx buffers
	data_in.vals16[1] = RXBUF0;
	data_in.vals16[0] = RXBUF1; 

	for (i=0;i<4;i++)
	{
		unsigned long timeout = 0xffffffUL;
		while (!IFS3bits.DCIIF && --timeout);
		IFS3bits.DCIIF = 0;
	}

	frame_count = 0;
	return 0;	
}
/*Note will take two codec cycles to become valid, -1 indicates unsuccessful*/
short ReadReg(unsigned char reg)
{
	int i;
	IFS3bits.DCIIF = 0;

	data_out.all = 0;
	data_out.wr = 0;
	data_out.reg = reg;
	data_out.val = 0;

	for (i=0;i<4;i++)
	{
		unsigned long timeout = 0xffffffUL;
		while (!IFS3bits.DCIIF && --timeout);

		// Load tx buffers
		TXBUF0 = data_out.vals16[1];
		TXBUF1 = data_out.vals16[0];
		// Read rx buffers
		data_in.vals16[1] = RXBUF0;
		data_in.vals16[0] = RXBUF1; 

		IFS3bits.DCIIF = 0;
	}

	frame_count = 0;
	return data_in.val;
	return 0;
}

unsigned char CodecBusy(void)
{
	return FALSE;
}
#endif

void Dma0Pause(void)
{
	IEC0bits.DMA0IE = 0;
	DMA0CONbits.CHEN = 0; /*Channel off*/
}
void Dma0Resume(void)
{
	DMA0CONbits.CHEN = 1; /*Channel on*/
	IEC0bits.DMA0IE = 1;
}

// This interrupts when buffer is half full
void AssignDma0CircularSampleBuffer(short* addressStartA, short numElementsTotal)
{
	// Calculations
	DWORD_VAL startAddressA = {0};
	DWORD_VAL startAddressB = {0};

	// We know the pointers are 16 bit on this architecture, just accept the warning
	startAddressA.Val = (unsigned long)(unsigned short)(void*)addressStartA;
	startAddressB.Val = (unsigned long)(unsigned short)(void*)addressStartA + (sizeof(short) * numElementsTotal)/2;

	// Control registers
	DMA0CONbits.CHEN = 0; /*Channel off*/
	DMA0CONbits.SIZE = 0; /*Word size transfer*/
	DMA0CONbits.DIR = 0;  /*Transfer from periferal to ram*/
	//DMA0CONbits.HALF = 1; /*Interrupt when half way off, using ping pong mode*/
	DMA0CONbits.NULLW = 0;/*No null write needed*/
	DMA0CONbits.AMODE = 0b00;/*Register indirect with post inc*/
	DMA0CONbits.MODE = 0b10; /*Continuous ping pong xfer*/
	DMA0REQbits.FORCE = 0;/*Transfer on request*/
	DMA0REQbits.IRQSEL = 0b00000010;/*OC1 trigger source*/

	// DMA addresses
	DMA0CNT = (numElementsTotal/2)-1; /*Inform DMA of the buffer length of EACH half*/
	DMA0PAD = (unsigned short)(void*)(&RXBUF0);
	DMA0STAH = startAddressA.word.HW;
	DMA0STAL = startAddressA.word.LW;
	DMA0STBH = startAddressB.word.HW;
	DMA0STBL = startAddressB.word.LW;
	
	// DMA debugging - info only
	// DSADR most recent DMA address
	// DMAPWCbits.PWCOL0 write colission flage 0-14
	// DMARQCbits.RQCOL0 request collision register
	// DMALCA last active DMA channel
	// DMAPPSbits.PPST0 is buffer B active?

	// Turn channel on
	IPC1bits.DMA0IP = DMA0_INT_PRIORITY;	/* Set priority*/
	IFS0bits.DMA0IF = 0; 					/* Clear DMA interrupt */
	IEC0bits.DMA0IE = 1; 					/* Enable DMA interrupt */
	DMA0CONbits.CHEN = 1; 					/* Channel on*/
}

void SetupOC1toTriggerSamplingDMA(int period)
{
	// Turn on Timer1
	if (T1CONbits.TON == 0) // Only init timer if it is off
	{
		CLOCK_SOSCEN();
		T1CONbits.TCS = 1;	// Secondary osc timer input, 32khz xtal
		T1CONbits.TSYNC = 1;// Sync'ed, must be to trigger OC1
		T1CONbits.TCKPS = 0;// No prescale
		T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
		T1CONbits.TSIDL = 0;// Timer on in idle mode
		PR1 = 0xffff;		// Not important
		T1CONbits.TON = 1; 	// On
	}
	else
	{
		T1CONbits.TON = 0; 	// Off
		T1CONbits.TSYNC = 1;// Sync'ed, must be to trigger OC1
		T1CONbits.TON = 1; 	// On
	}
	// Timer 1 produces a 32.768 kHz clock to OC module
	OC1CON1bits.OCM = 0; 	// Off
	OC1CON1bits.OCSIDL = 0; // Continue in idle
	OC1CON1bits.OCTSEL = 0b100; // Timer1 is the clock source
	OC1CON1bits.ENFLTC = 0;	// No fault pins
	OC1CON1bits.ENFLTB = 0;	// No fault pins
	OC1CON1bits.ENFLTA = 0;	// No fault pins
	OC1CON1bits.TRIGMODE = 0; 	// Continuous mode 
	OC1CON2bits.FLTMD = 0;	// Fault mode auto	
	OC1CON2bits.FLTOUT = 0; // PWM pin low on fault
	OC1CON2bits.FLTTRIEN = 0;// PWM pin not tristated
	OC1CON2bits.OCINV = 0;	// OC pin not inverted
	OC1CON2bits.OC32 = 0;	// 16bit opperation
	OC1CON2bits.OCTRIG = 0; // Synchronise to sync source
	OC1CON2bits.TRIGSTAT = 1;	// Trigger source run
	OC1CON2bits.OCTRIS = 1; // Output is tristated, off
	OC1CON2bits.SYNCSEL = 0x1f; // Trig/sync source is itself
	
	// Set period (period=2 for 16.384kHz, period=4 for 8.192kHz) --> register value 1 or 3
	OC1R = period - 1;
	OC1RS = period - 1; 

	// Start module
	OC1CON1bits.OCM = 0b011;// Toggle mode
	OC1CON2bits.TRIGSTAT = 1;	// Trigger source run
}
//EOF

