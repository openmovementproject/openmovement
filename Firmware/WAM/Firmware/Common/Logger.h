/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// Data Logger
// Dan Jackson, K Ladha, 2010-2012


#ifndef LOGGER_H
#define LOGGER_H
// Includes

// Defines
// [logger]
#define DATA_STREAM_SW_INT_IF IFS5bits.IC9IF
#define DATA_STREAM_SW_INT_IE IEC5bits.IC9IE
#define DATA_STREAM_SW_INT_IP IPC23bits.IC9IP
#define DATA_STREAM_SW_INTERRUPT_HANDLER _IC9Interrupt


// Accelerometer
#ifndef ACCEL_SWAP_INTERRUPTS
	// Normal interrupt mapping (events -> INT2, watermark -> INT1)
	#define ACCEL_INT_WATERMARK_HANDLER _INT1Interrupt
	#define ACCEL_INT_WATERMARK_IE      ACCEL_INT1_IE
	#define ACCEL_INT_WATERMARK_IF      ACCEL_INT1_IF
	#define ACCEL_INT_WATERMARK         ACCEL_INT1
	#define ACCEL_INT_EVENT_HANDLER     _INT2Interrupt
	#define ACCEL_INT_EVENT_IE          ACCEL_INT2_IE
	#define ACCEL_INT_EVENT_IF          ACCEL_INT2_IF
	#define ACCEL_INT_EVENT             ACCEL_INT2
#else
	// Alternative interrupt mapping (events -> INT1, watermark -> INT2)
	#define ACCEL_INT_WATERMARK_HANDLER _INT2Interrupt
	#define ACCEL_INT_WATERMARK_IE      ACCEL_INT2_IE
	#define ACCEL_INT_WATERMARK_IF      ACCEL_INT2_IF
	#define ACCEL_INT_WATERMARK         ACCEL_INT2
	#define ACCEL_INT_EVENT_HANDLER     _INT1Interrupt
	#define ACCEL_INT_EVENT_IE          ACCEL_INT1_IE
	#define ACCEL_INT_EVENT_IF          ACCEL_INT1_IF
	#define ACCEL_INT_EVENT             ACCEL_INT1
#endif

// Globals

// Data events

// Privates
// Start the data logger
void RunLogging(void);

// Called from the RTC interrupt at 1Hz
void LoggerTimerTasks(void);

// Called from the timer1 interrupt if enabled
void LoggerRTCTasks(void);

// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename);

// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename);

// Sampling routine that fills the audio buffer sectors
extern void AudioSamplingInit(void);
extern void AudioSamplingStop(void);
extern void LowPowerSamplingRoutine(void);


/*

// Clear the data buffers
void LoggerClear(void);

// Initialise logging
char LoggerInit(const char *filename);

// Start logging
void LoggerStart(void);

// Check each stream for data and write it out to the log file
// 0 = no write, 1 = data written, -1 = write error
char LoggerPollAndWrite(dataStreamFlags_t forceWrite)

// Stop logging
void LoggerStop(void);

 */

// Sociometer audio sampling specific stuff:
#if defined __dsPIC33E__
	// Sociometer audio sampling specific stuff:
	// PIC24 specific clock switching and low power code
	#define OSCSWITCH_FAST()	CLOCK_INTOSC_PLL()
//	#define OSCSWITCH_FAST()	CLOCK_XTAL_PLL()
	#define OSCSWITCH_NORMAL()	{CLOCK_INTOSC();} //7.37MHz FRC
	#define OSCSWITCH_IDLE()	{CLOCK_INTOSC();CLKDIVbits.FRCDIV = 0b101;} //230kHz FRC
//	#define OSCSWITCH_IDLE()	CLOCK_EC_NOWAIT()
	#define NUM_AUDIO_SECTORS_PER_DMA_INT (AUDIO_MAX_SECTORS/2)	// *MUST* be (AUDIO_MAX_SECTORS/2)
#else
	#error "Other devices are no longer supported - revert SVN to pre 08-01-2013 to regain functionality"
#endif



// Data ready flags
typedef union {
	struct {
		unsigned char accel  : 1;
		unsigned char gyro 	 : 1;
		unsigned char temp   : 1;
		unsigned char adc	 : 1;
		unsigned char alt 	 : 1;
		unsigned char mag 	 : 1;
		unsigned char accel_int2 : 1;
		unsigned char epoc 	 : 1;
	};
	unsigned char all;
} dataStreamFlags_t;

// Globals
extern volatile dataStreamFlags_t gStreamEnable;
extern volatile dataStreamFlags_t gDataReady; 


#endif

//EOF
