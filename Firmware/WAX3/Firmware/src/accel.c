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

// accel.c - ADXL345 accelerometer interface
// Dan Jackson, Cas Ladha and Karim Ladha, 2010


// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <spi.h>
#include <delays.h>
#include "accel.h"


#pragma code


// ADXL SPI address registers
#define ACCEL_ADDR_DEVID            0x00	// Device ID
//                               0x01-0x1C  // (Reserved - do not access)
#define ACCEL_ADDR_THRESH_TAP       0x1D	// Tap Threshold
#define ACCEL_ADDR_OFSX		        0x1E	// Xaxis Offset
#define ACCEL_ADDR_OFSY		        0x1F	// Yaxis Offset
#define ACCEL_ADDR_OFSZ		        0x20	// Zaxis Offset
#define	ACCEL_ADDR_DUR			    0x21	// Tap Duration
#define ACCEL_ADDR_LATENT		    0x22	// Tap Latency
#define ACCEL_ADDR_WINDOW		    0x23	// Tap Window
#define ACCEL_ADDR_THRESH_ACT       0x24	// Activity Threshld
#define ACCEL_ADDR_THRESH_INACT     0x25	// Inactivity Threshold
#define ACCEL_ADDR_TIME_INACT       0x26	// Inactivity Time
#define	ACCEL_ADDR_ACT_INACT_CTL    0x27	// Axis Enable CTRL for activity/inactivity detection
#define ACCEL_ADDR_THRESH_FF        0x28	// FreeFall Threshold
#define ACCEL_ADDR_TIME_FF          0x29	// FreeFall Time
#define ACCEL_ADDR_TAP_AXES         0x2A	// Axis control tap/dbl- tap
#define ACCEL_ADDR_ACT_TAP_STATUS   0x2B	// Source Tap/Dbl-Tap
#define ACCEL_ADDR_BW_RATE          0x2C	// Data rate and PWR mde ctrl
#define ACCEL_ADDR_POWER_CTL        0x2D	// Power Feature Control
#define	ACCEL_ADDR_INT_ENABLE       0x2E	// Interrupt Enable ctrl
#define ACCEL_ADDR_INT_MAP          0x2F	// Interrupt Mapping Control
#define	ACCEL_ADDR_INT_SOURCE       0x30	// Interrupt Source
#define ACCEL_ADDR_DATA_FORMAT      0x31	// Data Format Control
#define	ACCEL_ADDR_DATAX0           0x32	// Xaxis Data 0
#define	ACCEL_ADDR_DATAX1           0x33	// Xaxis Data 1
#define	ACCEL_ADDR_DATAY0           0x34	// Yaxis Data 0
#define	ACCEL_ADDR_DATAY1           0x35	// Yaxis Data 1
#define	ACCEL_ADDR_DATAZ0           0x36	// Zaxis Data 0
#define	ACCEL_ADDR_DATAZ1           0x37	// Zaxis Data 1
#define	ACCEL_ADDR_FIFO_CTL         0x38	// FIFO Control 
#define ACCEL_ADDR_FIFO_STATUS      0x39	// FIFO Status

// ADXL SPI masks
#define ACCEL_MASK_READ             0x80
#define ACCEL_MASK_BURST            0x40
#define ACCEL_MASK_WRITE            0x00

// 3.4us delay is 6.8 Tcy @ 8 MHz; or 40.8 Tcy @ 48 MHz
#define ACCEL_DELAY_8MHz() { Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); }
#define ACCEL_DELAY_48MHz() { Delay10TCYx(4); Delay1TCY(); }
#if defined(FAST_USB_CLOCK) && defined(RADIO_WHEN_ATTACHED)
//#define ACCEL_DELAY() { if (USB_IS_FAST_CLOCK()) { ACCEL_DELAY_48MHz() } else { ACCEL_DELAY_8MHz() } }
#define ACCEL_DELAY() { ACCEL_DELAY_48MHz(); }
#else
#define ACCEL_DELAY() ACCEL_DELAY_8MHz()
#endif

// SPI routines
#if (ACCEL_USE_SPI==1)
	#define OpenSPIx	OpenSPI1
	#define CloseSPIx	CloseSPI1
	#define ReadSPIx	ReadSPI1
	#define WriteSPIx	WriteSPI1
	#define OpenSPIADXL()	OpenSPI1(SPI_FOSC_16,MODE_11,SMPMID)    // TODO: Check the SSP setup -- OpenI2C1(MASTER, SLEW_OFF) -- was using: OpenSPI2(SPI_FOSC_16,MODE_11,SMPEND);
#elif (ACCEL_USE_SPI==2)
	#define OpenSPIx	OpenSPI2
	#define CloseSPIx	CloseSPI2
	#define ReadSPIx	ReadSPI2
	#define WriteSPIx	WriteSPI2
	#define OpenSPIADXL()	OpenSPI2(SPI_FOSC_16,MODE_11,SMPMID)    // TODO: Check the SSP setup -- OpenI2C1(MASTER, SLEW_OFF) -- was using: OpenSPI2(SPI_FOSC_16,MODE_11,SMPEND);
#else
	#error ACCEL_SPI not valid define (must be 1 or 2)
#endif

static unsigned char accelRate = ACCEL_DEFAULT_RATE;
static unsigned char accelRange = 3;

// SPI: 3-wire configuration if DATA_FORMAT:SPI is cleared, or 4-wire configuration if it is set.
//      Maximum SPI clock speed is 5 MHz (with a 100 pF max loading).
//      Timing scheme follows clock polarity (CPOL) = 1 and clock phase (CPHA) = 1.
//      Data should be sampled at the rising edge of SCLK.


// Read device ID
unsigned char AccelReadDeviceId(void)
{
	unsigned char id;
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	WriteSPIx(ACCEL_ADDR_DEVID | ACCEL_MASK_READ);
	id = ReadSPIx();
	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // 3.4us
	return id;
}


// AccelCurrentRate
int AccelCurrentRate(void) { return accelRate; }


// AccelStartup
void AccelStartup(unsigned char samplingRate, unsigned char watermark, unsigned char activityThreshold, unsigned char inactivityThreshold, unsigned char activityTime)
{
	int i;

	accelRate = samplingRate;

    // Hijack the top two bits of the sampling rate value to determine the range
    // For backwards compatibility we treat the top two bits as the reverse of the ADXL's range values, i.e. 0=±16g, 1=±8g, 2=±4g, 3=±2g
    accelRange = 0x3 - ((samplingRate >> 6) & 0x3);
    // Resulting samplingRange for ADXL: 0 = ±2g; 1 = ±4g; 2 = ±8g; 3 = ±16g; 

    ACCEL_INIT_PINS();     // Ensure pins are correct

	// Open SPI
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	Nop();
    ACCEL_DELAY();  // Delay 3.4us

    // Full-resolution sampling is 3.9 mg/LSB	

	// Sequentialy write registers
	WriteSPIx(ACCEL_ADDR_THRESH_TAP | ACCEL_MASK_WRITE | ACCEL_MASK_BURST);    // First Register
	WriteSPIx(0x40); // 0x30	// Tap Threshold: 62.5 mg/LSB (0xff = +16 g) [0x30 = 48 = 3 g]
	WriteSPIx(0x00);	// Xaxis Offset: signed 8-bit offset of 15.6 mg/LSB (0x7f = +2 g)
	WriteSPIx(0x00);	// Yaxis Offset: signed 8-bit offset of 15.6 mg/LSB (0x7f = +2 g)
	WriteSPIx(0x00);	// Zaxis Offset: signed 8-bit offset of 15.6 mg/LSB (0x7f = +2 g)
	WriteSPIx(0x06); // 0x02	// Tap Duration: 625 us/LSB <Maximum duration of a tap above threshold>
	WriteSPIx(0x40); // 0x10	// Tap Latency: 1.25 ms/LSB <Minimum time between taps>
	WriteSPIx(0xF0); // 0xF0	// Tap Window: 1.25 ms/LSB [0xF0 = 300 ms] <Time after latency to detect second tap>
	WriteSPIx(activityThreshold);	// Activity Threshld: 62.5 mg/LSB  [0x03 = 0.188g]
	WriteSPIx(inactivityThreshold);	// Inactivity Threshold: 62.5 mg/LSB [0x02 = 0.125g]
	WriteSPIx(activityTime);	// Inactivity Time: 1 sec/LSB [0x0a = 10s]
	WriteSPIx(0x00);	// Axis Enable CTRL for activity inactivity detection. [0 = Activity/inactivity detection OFF]
	WriteSPIx(0x00);	// FreeFall Threshold
	WriteSPIx(0x00);	// FreeFall Time
	WriteSPIx(0x00);	// Axis control tap/dbl- tap	(b2=X, b1=Y, b0=Z)	-- [0 = tap detection off]
	WriteSPIx(0x00);	// [Read-only] Source Tap/Dbl-Tap

	// BWRateAddr = 0x0A 
	// d7-5 = 0
	// d4 = 1 - LOW Power
	// d3-0 = 1010 (0x0A) - Rate = 100Hz, BW = 50Hz, I = 55uA
	WriteSPIx(samplingRate & 0x1f);  // To enter lower power mode, set the LOW_POWER bit (Bit 4) in the BW_RATE register (Address 0x2C).

	// PWRCtrlAddr = 0x08
	// d7-6 = 0
	// d5 = LINK - (1 = on, link activity and inactivity to be serially detected)
	// d4 = Auto Sleep between thresholds - (0 = off)
	// d3 = Measure - (1, Measurementmaking on)
	// d2 = Sleep - (0, No suppression of data into FIFO. No Data ready override)
	// d1-0 = Wakeup8Hz Sample rate if Sleep = 1
	WriteSPIx(0x28);
	
	// IntEnableAddr = 0x00
	// d7 = DataReady
	// d6 = Single Tap
	// d5 = Double Tap
	// d4 = Activity
	// d3 = Inactivity
	// d2 = FreeFall
	// d1 = Watermark
	// d0 = Overrun
	// 1 = on, 0 = off
	WriteSPIx(0x00);        // Disable interrupts


	// IntMapAddr = 0x00
	// d7 = DataReady
	// d6 = Single Tap
	// d5 = Double Tap
	// d4 = Activity
	// d3 = Inactivity
	// d2 = FreeFall
	// d1 = Watermark
	// d0 = Overrun
	// (1 = map to INT2 pin, 0 = map to INT1 pin)
	// MODDED FOR HWv2.1 WriteSPIx(0b00000011);                  // [IntMapAddr]    Watermark/Overflow: INT2 -> AccINT1 -> PIC RB0/INT0;  Single-tap,Dbl-tap,Activity,Inactivity: ADXL-INT1 -> "AccINT2" -> PIC-RB1/INT1
	WriteSPIx(0b00000000);   

	WriteSPIx(0x00);	    // [Read-only] Interrupt Source (indicates cause of interrupt)

	
	// DataFormatAddr = 0x08 
	// d7 = self test - (0, Disable self test)
	// d6 = spi (0, set to 4-wire SPI)
	// d5 = IntInvert (0, active high ints)
	// d4 = 0
	// d3 = FullRes (1, Dynamic scaling enabled)
	// d2 = Justify (0, Right Justify with sign extension)
	// d1,d0 = Range (1,1. 16g Range. Table 18 datasheet)
#ifdef ACCEL_DWORD_FORMAT
	WriteSPIx(0x00 | accelRange);
#else
	WriteSPIx(0x08 | accelRange);
#endif

	WriteSPIx(0x00);	// [Read-only] Xaxis Data 0
	WriteSPIx(0x00);	// [Read-only] Xaxis Data 1
	WriteSPIx(0x00);	// [Read-only] Yaxis Data 0
	WriteSPIx(0x00);	// [Read-only] Yaxis Data 1
	WriteSPIx(0x00);	// [Read-only] Zaxis Data 0
	WriteSPIx(0x00);	// [Read-only] Zaxis Data 1

	// FIFOCtrlAddr	=	0x00
	// d7,d6 = FIFO MODE (00 = Bypass FIFO Mode. 01 = FIFO, 10 = Stream, 11 = Trigger)
	// d5 = Triger (0 = Trigger event to INT1 pin. 1 = event to INT2 pin)
	// d4-0 = Samples (0. Dont care as not using the FIFO. Table 20 datasheet)
    //                                                                          Enables FIFO in 'stream' mode interrupting on ADXL-INT2 -> "AccINT1" -> PIC-RB0/INT0
	WriteSPIx(0xa0 + (watermark & 0x1f));                                  // ... sets water mark to 25 (of 31)

	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // Delay 3.4us
}


// Shutdown the ADXL to standby mode (standby mode, interrupts disabled)
extern void AccelStandby(void)
{
    ACCEL_INIT_PINS();     // Ensure pins are correct

    // Disable processor interrupts 
    ACCEL_INT1_IE = 0; 
//    ACCEL_INT2_IE = 0; 

	// Open SPI
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	Nop();
    ACCEL_DELAY();  // Delay 3.4us

	// Tap detection off
	WriteSPIx(ACCEL_ADDR_TAP_AXES | ACCEL_MASK_WRITE);		// Axis control tap/dbl- tap
	WriteSPIx(0x00);										// (b2=X, b1=Y, b0=Z)	-- 0x07 = X/Y/Z

	// Act/Inact detection off
	WriteSPIx(ACCEL_ADDR_ACT_INACT_CTL | ACCEL_MASK_WRITE);	// Axis Enable CTRL for activity/inactivity detection
	WriteSPIx(0x00);										// (b7=Act-AC,b6-b5=Act-X/Y/Z,b4=Inact-AC,b3-b0=Inact-X/Y/Z)

	// Disable accelerometer interrupts
	WriteSPIx(ACCEL_ADDR_INT_ENABLE | ACCEL_MASK_WRITE);	// b1 = watermark, b5=double-tap, b6=single-tap
	WriteSPIx(0x00);                                        // [IntEnableAddr] Disable all interrupts

	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // Delay 3.4us

    // Clear acclerometer status flags and empty buffer
    AccelReadIntSource(); 
    AccelReadTapStatus(); 
    AccelReadFIFO(NULL, 0xff);
    
    // Clear processor interrupt flags
    ACCEL_INT1_IF = 0;
//    ACCEL_INT2_IF = 0;


	// Open SPI
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	Nop();
    ACCEL_DELAY();  // Delay 3.4us

	// --- Place accelerometer into standby ---
	// d7-6 = 0
	// d5 = LINK - (1 = on, link activity and inactivity to be serially detected)
	// d4 = Auto Sleep between thresholds - (0 = off)
	// d3 = Measure - (0, Measurementmaking off = standby 0.1ua)
	// d2 = Sleep - (0, No suppression of data into FIFO. No Data ready override)
	// d1-0 = Wakeup8Hz Sample rate if Sleep = 1
	WriteSPIx(ACCEL_ADDR_POWER_CTL | ACCEL_MASK_WRITE);
	WriteSPIx(0x00);
	
	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // Delay 3.4us

    //ACCEL_SCK_PIN = 1; ACCEL_SDI_PIN = 1; // ??? Set SPI pins as inputs ???
}


// Enable auto-sleep
void AccelEnableAutoSleep(char sleepNow)
{
	// Reset value for relative activity motion and read status
	AccelSingleSample(NULL);
	AccelIsAsleep();

    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low

	// d7-6 = 00
	// d5   =  1 LINK - (1 = on, link activity and inactivity to be serially detected)
	// d4   =  1 Auto Sleep between thresholds - (1 = on)
	// d3   =  1 Measure - (1, Measurementmaking on, 0 = stand-by mode)
	// d2   =  # Sleep - (0, No suppression of data into  . No Data ready override)
	// d1-0 = 00 Wakeup sample rate when sleeping (00=8Hz, 01=4Hz, 10=2Hz, 11=1Hz)
	WriteSPIx(ACCEL_ADDR_POWER_CTL | ACCEL_MASK_WRITE);
	WriteSPIx(0x38 | (sleepNow ? 0x04 : 0x00));

	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // 3.4us
	return;
}


// Checks whether accelerometer is in asleep
unsigned char AccelIsAsleep(void)
{
	unsigned char status;
unsigned char gie = INTCONbits.GIE; 
INTCONbits.GIE = 0;
	status = AccelReadTapStatus();
INTCONbits.GIE = gie;
	return ((status & 0x08) >> 3);
}


void AccelSingleSample(Accel *value)
{
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	Nop();
    
	// Initialise  sequential read on X axis
	WriteSPIx(ACCEL_ADDR_DATAX0 | ACCEL_MASK_READ | ACCEL_MASK_BURST);
    
	// Data 0 contains LSB. Data 1 contains MSB
	if (value != NULL)
	{
#ifdef ACCEL_DWORD_FORMAT
		unsigned char v;

		// NOTE: Careful with assignments and masks: remember, destination not initialized to zero.

		// X
		value->byte_x07 = ReadSPIx();            // (set 1st byte)	// XL (0-7) -- low 8 bits of X to X0-X7
		value->byte_x89_y05 = ReadSPIx() & 0x03; // (set 2nd byte)	// XH (8-9) -- upper 2 bits of X to X8-X9

		// Y
		v = ReadSPIx();												// YL (0-7)
		value->byte_x89_y05 |= (v << 2);		 // (mask 2nd byte)	//          -- low 6 bits of Y to Y0-Y5
		value->byte_y69_z03 = (v >> 6);          // (set 3rd byte)	//          -- middle 2 bits of Y to Y6-Y7
		v = ReadSPIx();												// YH (8-9)
		value->byte_y69_z03 |= ((v & 0x03) << 2);// (mask 3rd byte)	//          -- upper 2 bits of Y to Y8-Y9

		// Z
		v = ReadSPIx();												// ZL (0-7)
		value->byte_y69_z03 |= (v << 4);		 // (mask 3rd byte)	//          -- lower 4 bits of Z to Z0-Z3
		value->byte_z49_e01 = (v >> 4);          // (set 4th byte)	//          -- middle 4 bits of Z to Z4-Z7
		v = ReadSPIx();												// ZH (8-9)
		value->byte_z49_e01 |= ((v & 0x03) << 4);// (mask 4th byte)	//          -- upper 2 bits of Z to Z8-Z9

		// Exponent
		value->byte_z49_e01 |= (accelRange << 6);// (mask 4th byte)	//          -- full 2 bits of E to E0-E1

#else
		value->xl = ReadSPIx();
		value->xh = ReadSPIx();
		value->yl = ReadSPIx();
		value->yh = ReadSPIx();
		value->zl = ReadSPIx();
		value->zh = ReadSPIx();
#endif
	}
	else
	{
		ReadSPIx(); ReadSPIx(); ReadSPIx(); ReadSPIx(); ReadSPIx(); ReadSPIx();
	}
	
	ACCEL_CS = 1;		// active low
	CloseSPIx();

    ACCEL_DELAY();  // Remember not to call this fuction within 3.4us

	return;
}


// Enable interrupts
void AccelEnableInterrupts(unsigned char flags)
{
    // Temporarily disable processor interrupts 
    ACCEL_INT1_IE = 0; 
//    ACCEL_INT2_IE = 0; 
    
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
    ACCEL_DELAY();  // 3.4us
	ACCEL_CS = 0;		// active low

	// Interrupt map (Watermark on INT2, tap/double-tap/activity/inactivity on INT1)
	WriteSPIx(ACCEL_ADDR_INT_MAP | ACCEL_MASK_WRITE);		// b0 = overrun, b1 = watermark, b5=double-tap, b6=single-tap
	WriteSPIx(0b00000000); // Moddeed for HW2.1                                  // [IntMapAddr]    Watermark/Overflow: INT2 -> AccINT1 -> PIC RB0/INT0;  Single-tap,Dbl-tap,Activity,Inactivity: ADXL-INT1 -> "AccINT2" -> PIC-RB1/INT1

	// Act/Inact -- axis Enable CTRL for activity/inactivity detection
	WriteSPIx(ACCEL_ADDR_ACT_INACT_CTL | ACCEL_MASK_WRITE);	// Axis Enable CTRL for activity/inactivity detection
	WriteSPIx(0xff);										// (b7=Act-AC,b6-b5=Act-X/Y/Z,b4=Inact-AC,b3-b0=Inact-X/Y/Z)

	// Tap detection axis
	WriteSPIx(ACCEL_ADDR_TAP_AXES | ACCEL_MASK_WRITE);		// Axis control tap/dbl- tap
	WriteSPIx(0x07);										// (b2=X, b1=Y, b0=Z)	-- 0x07 = X/Y/Z

	// Enable required interrupts
	WriteSPIx(ACCEL_ADDR_INT_ENABLE | ACCEL_MASK_WRITE);	// b1 = watermark, b3=activity, b4=inactivity, b5=double-tap, b6=single-tap
	WriteSPIx(flags);                                       // [IntEnableAddr] Enable interrupts: 0x76=watermark/act/inact/dt/st, 0x62=single-tap/dbl-tap/watermark

	ACCEL_CS = 1;		// active low
    ACCEL_DELAY();  // 3.4us
	CloseSPIx();

    // Clear acclerometer status flags and empty buffer
    AccelReadIntSource(); 
    AccelReadTapStatus(); 
    AccelReadFIFO(NULL, 0xff);
    
    // Clear processor interrupt flags before enabling masks
    ACCEL_INT1_IF = 0;
//    ACCEL_INT2_IF = 0;

    // Enable processor interrupt masks

// Added
ACCEL_INT1_IP = 0;
//


    ACCEL_INT1_IE = 1; 
//    ACCEL_INT2_IE = 0; // Modded for HW2.1

    return;
}


// Read at most 'maxEntries' 3-axis samples (3 words = 6 bytes, or 4 bytes if packed format) from the ADXL FIFO into the specified RAM buffer
unsigned char AccelReadFIFO(Accel *accelBuffer, unsigned char maxEntries)
{
	unsigned char availableFifo;
	unsigned char numRead;
	unsigned char *p;
	int i;

    // Check if we have any room to fit
    if (maxEntries == 0) { return 0; }

	OpenSPIADXL();
	numRead = 0;
	p = (unsigned char *)accelBuffer;

    ACCEL_DELAY();  // 3.4us

	// Read number of available samples
	ACCEL_CS = 0;		// active low
	WriteSPIx(ACCEL_ADDR_FIFO_STATUS | ACCEL_MASK_READ);
	availableFifo = ReadSPIx() & 0x3f;	// FIFO_STATUS
	ACCEL_CS = 1;		// active low

    ACCEL_DELAY();  // 3.4us

	// While data is in the FIFO and we have space in the buffer
	while (availableFifo > 0 && maxEntries > 0)
	{
		// cannot re-enter burst mode for (5us) wait 3.4us
		ACCEL_CS = 1;		// active low
        ACCEL_DELAY();  // 3.4us
		ACCEL_CS = 0;		// active low
		
		// FIFOStatusAddr // FIFO_STATUS 0x39 (DATA_READY bit) 
		// DataX0Addr // DATAX 0x32/0x33, DATAY 0x34/0x35, DATAZ 0x36/0x37
		// Wait 3.4us
		WriteSPIx(ACCEL_ADDR_DATAX0 | ACCEL_MASK_READ | ACCEL_MASK_BURST);

		// Data 0 contains LSB. Data 1 contains MSB
        if (accelBuffer != 0)
        {
#ifdef ACCEL_DWORD_FORMAT
			unsigned char t, v;
	
			// X
			*p++ = ReadSPIx();     // (set 1st byte)	// XL (0-7) -- low 8 bits of X to X0-X7
			t = ReadSPIx() & 0x03; // (set 2nd byte)	// XH (8-9) -- upper 2 bits of X to X8-X9
	
			// Y
			v = ReadSPIx();								// YL (0-7)
			t |= (v << 2);		   // (mask 2nd byte)	//          -- low 6 bits of Y to Y0-Y5
			*p++ = t;
			t = (v >> 6);          // (set 3rd byte)	//          -- middle 2 bits of Y to Y6-Y7
			v = ReadSPIx();								// YH (8-9)
			t |= ((v & 0x03) << 2);// (mask 3rd byte)	//          -- upper 2 bits of Y to Y8-Y9
	
			// Z
			v = ReadSPIx();								// ZL (0-7)
			t |= (v << 4);		   // (mask 3rd byte)	//          -- lower 4 bits of Z to Z0-Z3
			*p++ = t;
			t = (v >> 4);          // (set 4th byte)	//          -- middle 4 bits of Z to Z4-Z7
			v = ReadSPIx();								// ZH (8-9)
			t |= ((v & 0x03) << 4);// (mask 4th byte)	//          -- upper 2 bits of Z to Z8-Z9
	
			// Exponent
			t |= (accelRange << 6);// (mask 4th byte)	//          -- full 2 bits of E to E0-E1
			*p++ = t;
#else
    		*p++ = ReadSPIx();	// DATAX0
    		*p++ = ReadSPIx();	// DATAX1
    		*p++ = ReadSPIx();	// DATAY0
    		*p++ = ReadSPIx();	// DATAY1
    		*p++ = ReadSPIx();	// DATAZ0
    		*p++ = ReadSPIx();	// DATAZ1
#endif
        }
        else
        {
            // Empty one entry from FIFO if NULL pointer (discarding packet)
            ReadSPIx(); ReadSPIx(); ReadSPIx(); ReadSPIx(); ReadSPIx(); ReadSPIx();
        }

		// Update status
		maxEntries--;
		availableFifo--;
		numRead++;

		// Update number of bytes in FIFO
		#if 0
			ReadSPIx();	// FIFO_MODE
			availableFifo = ReadSPIx() & 0x3f;	// FIFO_STATUS
		#endif
	}

	ACCEL_CS = 1;		// active low
	CloseSPIx();

    ACCEL_DELAY();  // 3.4us

	return numRead;
}


// Read tap status
unsigned char AccelReadTapStatus(void)
{
	unsigned char status;
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	WriteSPIx(ACCEL_ADDR_ACT_TAP_STATUS | ACCEL_MASK_READ);		// b0 = Tap-Z
	status = ReadSPIx();	// & 0x07;	// tap status
	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // 3.4us
	return status;
}

// Read interrupt source
unsigned char AccelReadIntSource(void)
{
	unsigned char status;
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	WriteSPIx(ACCEL_ADDR_INT_SOURCE | ACCEL_MASK_READ);		// b1 = watermark, b5=double-tap, b6=single-tap
	status = ReadSPIx();
	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // 3.4us
	return status;
}


// Debug dump registers
void AccelDebugDumpRegisters(void)
{
    int i;
    unsigned char value;

	OpenSPIADXL();

    ACCEL_DELAY();  // 3.4us

	ACCEL_CS = 0;		// active low

	WriteSPIx(ACCEL_ADDR_THRESH_TAP | ACCEL_MASK_READ | ACCEL_MASK_BURST);

    printf((const rom far char *)"ADXL: ");
    for (i = 0; i < 29; i++)
    {
    	value = ReadSPIx();
        printf((const rom far char *)"0x%02x=0x%02x,", ACCEL_ADDR_THRESH_TAP + i, value);
    }
    printf((const rom far char *)"\r\n");

	ACCEL_CS = 1;		// active low

    ACCEL_DELAY();  // 3.4us

	CloseSPIx();

    return;
}
