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
// Dan Jackson, Karim Ladha, Cas Ladha, 2010-2012.

// Needed, or else spi.h fails
#define USE_AND_OR

// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <spi.h>
#include <TimeDelay.h>
#include "Peripherals/Accel.h"

// Note: The ACC/GYRO share SPI bus 1 on the remappable pins. After calling InitIO() the pins are configured
// The interrupts are remapped as follows
//  AccInt1 RP21 -> Int1
//  AccInt2 RP26 -> Int2
//  GyrInt1 RP19 -> Int3
//  GyrInt2 RP27 -> Int4
// You can use read-modify-write sequences on the SPI buffer regs on a pic24

// Device id
#define ACCEL_DEVICE_ID 0xE5

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
#define	ACCEL_ADDR_ACT_INACT_CTL    0x27	// Axis Enable CTRL for activity inactivity detection
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
#define ACCEL_DELAY() {Delay10us(1);}

// SPI routines
inline void WriteSPIx(unsigned int data)	{
											unsigned int dummy;
											if(SPI1_Rx_Buf_Full)
												dummy = SPI1BUF; 				// Clear flag
											while(SPI1_Tx_Buf_Full);			// Dont write till last byte is txed	
											SPI1BUF=(data);
											while(!SPI1_Rx_Buf_Full);			// Wait till byte clocked in
											dummy = SPI1BUF;					// Read it
											}												
inline unsigned int ReadSPIx(void)			{
											unsigned int data;
											if(SPI1_Rx_Buf_Full)
												data = SPI1BUF; 				// Clear flag
											while(SPI1_Tx_Buf_Full);			// Dont write till last byte is txed	
											SPI1BUF=(0xff);
											while(!SPI1_Rx_Buf_Full);			// Wait till byte clocked in
											data = SPI1BUF;						// Read it
											return (data&0xff);
											}	

#define CloseSPIx()		CloseSPI1()
#define OpenSPIADXL(){  \
                    if (OSCCONbits.NOSC == 1) \
                        OpenSPI1((ENABLE_SCK_PIN | ENABLE_SDO_PIN | SPI_MODE8_ON | SPI_SMP_OFF | SPI_CKE_OFF | CLK_POL_ACTIVE_LOW | MASTER_ENABLE_ON | SEC_PRESCAL_1_1 | PRI_PRESCAL_4_1), /*config 1*/ \
								 (FRAME_ENABLE_OFF | FRAME_SYNC_OUTPUT | FRAME_SYNC_ACTIVE_LOW | SPI_FRM_PULSE_PREV_CLK | SPI_ENH_BUFF_DISABLE ), /*config 2*/ \
								 (SPI_ENABLE | SPI_IDLE_STOP | SPI_RX_OVFLOW_CLR) /*config 3*/ ); \
                    else    \
                        OpenSPI1((ENABLE_SCK_PIN | ENABLE_SDO_PIN | SPI_MODE8_ON | SPI_SMP_OFF | SPI_CKE_OFF | CLK_POL_ACTIVE_LOW | MASTER_ENABLE_ON | SEC_PRESCAL_1_1 | PRI_PRESCAL_1_1), /*config 1*/ \
								 (FRAME_ENABLE_OFF | FRAME_SYNC_OUTPUT | FRAME_SYNC_ACTIVE_LOW | SPI_FRM_PULSE_PREV_CLK | SPI_ENH_BUFF_DISABLE ), /*config 2*/ \
								 (SPI_ENABLE | SPI_IDLE_STOP | SPI_RX_OVFLOW_CLR) /*config 3*/ ); \
                     }

//old:OpenSPI2(SPI_FOSC_16,MODE_11,SMPMID)    

static unsigned char accelRate = ACCEL_DEFAULT_RATE;
static unsigned char accelRange = 3;
static unsigned short accelFrequency = ACCEL_FREQUENCY_FOR_RATE(ACCEL_DEFAULT_RATE);

unsigned char AccelRate(void) { return accelRate; }
unsigned short AccelFrequency(void) { return accelFrequency; }

// SPI: 3-wire configuration if DATA_FORMAT:SPI is cleared, or 4-wire configuration if it is set.
//      Maximum SPI clock speed is 5 MHz (with a 100 pF max loading).
//      Timing scheme follows clock polarity (CPOL) = 1 and clock phase (CPHA) = 1.
//      Data should be sampled at the rising edge of SCLK.

char accelPresent = 0;


// Read device ID
unsigned char AccelVerifyDeviceId(void)
{
	unsigned char id;
    ACCEL_INIT_PINS();     // Ensure pins are correct
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	WriteSPIx(ACCEL_ADDR_DEVID | ACCEL_MASK_READ);
	id = ReadSPIx();
	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // 3.4us
    accelPresent = (id == ACCEL_DEVICE_ID) ? 1 : 0;
	return accelPresent;
}


// AccelStartup
void AccelStartup(unsigned char samplingRate)
{
	accelRate = samplingRate;
    accelFrequency = ACCEL_FREQUENCY_FOR_RATE(accelRate);

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
WriteSPIx(16); // 0x02	// Tap Duration: 625 us/LSB 16=10ms <Maximum duration of a tap above threshold>
WriteSPIx(80); // 0x10	// Tap Latency: 1.25 ms/LSB 0x50 = 100ms <Minimum time between taps>
WriteSPIx(160); // 0xF0	// Tap Window: 1.25 ms/LSB [0xFF = 320 ms] <Time after latency to detect second tap>
WriteSPIx(5);	// Activity Threshld: 62.5 mg/LSB  [0x03 = 0.188g]
WriteSPIx(2);	// Inactivity Threshold: 62.5 mg/LSB [0x02 = 0.125g]
WriteSPIx(10);	// Inactivity Time: 1 sec/LSB [0x0a = 10s]
	WriteSPIx(0xFF);	// Axis Enable CTRL for activity inactivity detection. [0 = Activity/inactivity detection OFF]
	WriteSPIx(0x00);	// FreeFall Threshold
	WriteSPIx(0x00);	// FreeFall Time
	WriteSPIx(0x01);	// Axis control tap/dbl- tap	(b2=X, b1=Y, b0=Z)	-- [0 = tap detection off]
	WriteSPIx(0x00);	// [Read-only] Source Tap/Dbl-Tap

	// BWRateAddr = 0x0A 
	// d7-5 = 0
	// d4 = 1 - LOW Power
	// d3-0 = 1010 (0x0A) - Rate = 100Hz, BW = 50Hz, I = 55uA
	//WriteSPIx(0x10 | (rateCode & 0x1f));  // To enter lower power mode, set the LOW_POWER bit (Bit 4) in the BW_RATE register (Address 0x2C).
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
	WriteSPIx(0x00);

	WriteSPIx(0x00);	    // [Read-only] Interrupt Source (indicates cause of interrupt)

	
	// DataFormatAddr = 0x08 
	// d7 = self test - (0, Disable self test)
	// d6 = spi (0, set to 4-wire SPI)
	// d5 = IntInvert (0, active high ints)
	// d4 = 0
	// d3 = FullRes (1, Dynamic scaling enabled)
	// d2 = Justify (0, Right Justify with sign extension)
	// d1,d0 = Range (1,1. 16g Range. Table 18 datasheet)
	WriteSPIx(0x08 | accelRange);

	WriteSPIx(0x00);	// [Read-only] Xaxis Data 0
	WriteSPIx(0x00);	// [Read-only] Xaxis Data 1
	WriteSPIx(0x00);	// [Read-only] Yaxis Data 0
	WriteSPIx(0x00);	// [Read-only] Yaxis Data 1
	WriteSPIx(0x00);	// [Read-only] Zaxis Data 0
	WriteSPIx(0x00);	// [Read-only] Zaxis Data 1

	// FIFOCtrlAddr	=	0x00
	// d7,d6 = FIFO MODE (00 = Bypass FIFO Mode. 01 = FIFO, 10 = Stream, 11 = Trigger)
	// d5 = Trigger (0 = Trigger event to INT1 pin. 1 = event to INT2 pin)
	// d4-0 = Samples (0. Dont care as not using the FIFO. Table 20 datasheet)
    //                                                                          Enables FIFO in 'stream' mode interrupting on ADXL-INT1 
	WriteSPIx(0x80 + ACCEL_DEFAULT_WATERMARK);                                  // ... sets water mark to 25 (of 32)

	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // Delay 3.4us
}


// Shutdown the ADXL to standby mode (standby mode, interrupts disabled)
extern void AccelStandby(void)
{
    ACCEL_INIT_PINS();     // Ensure pins are correct

	// Open SPI
	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	Nop();
    ACCEL_DELAY();  // Delay 3.4us

	// Sequentialy write registers
	WriteSPIx(ACCEL_ADDR_POWER_CTL | ACCEL_MASK_WRITE | ACCEL_MASK_BURST);    // First Register

	// PWRCtrlAddr = 0x08
	// d3 = Measure - (0, Measurementmaking off -- standby 0.1ua)
	WriteSPIx(0x00);
	
	// IntEnableAddr = 0x00
	WriteSPIx(0x00);    // None

	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // Delay 3.4us

    //ACCEL_SCK_PIN = 1; ACCEL_SDI_PIN = 1; // ??? Set SPI pins as inputs ???
}


void AccelSingleSample(accel_t *accelBuffer)
{
    // Preserve interrupt status
    char int1 = ACCEL_INT1_IE;
    char int2 = ACCEL_INT2_IE;

    // Temporarily disable processor interrupts
    ACCEL_INT1_IE = 0;
    ACCEL_INT2_IE = 0;

	OpenSPIADXL();
	ACCEL_CS = 0;		// active low
	Nop();
    
	// Initialise  sequential read on X axis
	WriteSPIx(ACCEL_ADDR_DATAX0 | ACCEL_MASK_READ | ACCEL_MASK_BURST);
    
	// Data 0 contains LSB. Data 1 contains MSB
	if (accelBuffer != NULL)
	{
        unsigned char *p = (unsigned char *)accelBuffer;
		*p++ = ReadSPIx();  // XL
		*p++ = ReadSPIx();  // XH
		*p++ = ReadSPIx();  // YL
		*p++ = ReadSPIx();  // YH
		*p++ = ReadSPIx();  // ZL
		*p++ = ReadSPIx();  // ZH
	}
	
	ACCEL_CS = 1;		// active low
	CloseSPIx();

    ACCEL_DELAY();  // Remember not to call this fuction within 3.4us

    // Restore processor interrupts
    ACCEL_INT1_IE = int1;
    ACCEL_INT2_IE = int2;

	return;
}



// Enable interrupts
void AccelEnableInterrupts(unsigned char flags, unsigned char pinMask)
{
    // Temporarily disable processor interrupts 
    ACCEL_INT1_IE = 0;
    ACCEL_INT2_IE = 0;
    
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
    ACCEL_DELAY();  // 3.4us
	ACCEL_CS = 0;		// active low

	// Interrupt map (Watermark on INT2, /double-tap/activity/inactivity on INT1)
	WriteSPIx(ACCEL_ADDR_INT_MAP | ACCEL_MASK_WRITE);		// b0 = overrun, b1 = watermark, b5=double-tap, b6=single-tap
	WriteSPIx(pinMask); // Moddeed for PDQWAX hack     - 0x00 -> all int1                             // [IntMapAddr]    Watermark/Overflow: INT2 -> AccINT1 -> PIC RB0/INT0;  Single-tap,Dbl-tap,Activity,Inactivity: ADXL-INT1 -> "AccINT2" -> PIC-RB1/INT1
	

	// Act/Inact -- axis Enable CTRL for activity/inactivity detection
	WriteSPIx(ACCEL_ADDR_ACT_INACT_CTL | ACCEL_MASK_WRITE);	// Axis Enable CTRL for activity/inactivity detection
	WriteSPIx(0xff); //all AC coupled						// (b7=Act-AC,b6-b5=Act-X/Y/Z,b4=Inact-AC,b3-b0=Inact-X/Y/Z)

//	// Tap detection axis
//	WriteSPIx(ACCEL_ADDR_TAP_AXES | ACCEL_MASK_WRITE);		// Axis control tap/dbl- tap
//	WriteSPIx(0x01); // z only										// (b2=X, b1=Y, b0=Z)	-- 0x07 = X/Y/Z

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
    
    // Clear processor interrupt flags before re-enabling masks
    ACCEL_INT1_IF = 0;
	ACCEL_INT1_IE = 1;
    ACCEL_INT2_IF = 0;
	ACCEL_INT2_IE = 1;

    return;
}


// Read FIFO queue length
unsigned char AccelReadFifoLength(void)
{
	unsigned char availableFifo;
    ACCEL_DELAY();  // 3.4us
	OpenSPIADXL();
	// Read number of available samples
	ACCEL_CS = 0;		// active low
	WriteSPIx(ACCEL_ADDR_FIFO_STATUS | ACCEL_MASK_READ);
	availableFifo = ReadSPIx() & 0x3f;	// FIFO_STATUS
	ACCEL_CS = 1;		// active low
	CloseSPIx();
    ACCEL_DELAY();  // 3.4us
	return availableFifo;
}


// Read at most 'maxEntries' 3-axis samples (3 words = 6 bytes) from the ADXL FIFO into the specified RAM buffer
unsigned char AccelReadFIFO(accel_t *accelBuffer, unsigned char maxEntries)
{
	unsigned char availableFifo;
	unsigned char numRead;
	unsigned char *p;

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
    		*p++ = ReadSPIx();	// DATAX0
    		*p++ = ReadSPIx();	// DATAX1
    		*p++ = ReadSPIx();	// DATAY0
    		*p++ = ReadSPIx();	// DATAY1
    		*p++ = ReadSPIx();	// DATAZ0
    		*p++ = ReadSPIx();	// DATAZ1
        }
        else
        {
            // Empty one entry from FIFO if NULL pointer
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
	status = ReadSPIx() & 0x07;	// tap status
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

    printf("ADXL: ");
    for (i = 0; i < 29; i++)
    {
    	value = ReadSPIx();
        printf("0x%02x=0x%02x,", ACCEL_ADDR_THRESH_TAP + i, value);
    }
    printf("\r\n");

	ACCEL_CS = 1;		// active low

    ACCEL_DELAY();  // 3.4us

	CloseSPIx();

    return;
}



/*
    // Packed accelerometer value
    // [byte-3] [byte-2] [byte-1] [byte-0]
	// eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
	// 10987654 32109876 54321098 76543210
    typedef union Accel_t
    {
        struct
        {
            signed short x : 10;
            signed short y : 10;
            signed short z : 10;
            unsigned char exp: 2;
        };
        unsigned long value;
    } Accel;
*/
// Packs a buffer of 16-bit (x,y,z) values into an output buffer (4 bytes per entry)
void AccelPackData(short *input, unsigned char *output)
{
#if 1
    unsigned short wordl, wordh;

    // Calculate low and high words of packet data
         if (accelRange == 3) { wordl = ((input[0] >> 3) & 0x03ff) | ((input[1] <<  7) & 0xfc00); wordh = 0xc000 | ((input[2] << 1) & 0x3ff0) | ((input[1] >> 9) & 0x000f); }
    else if (accelRange == 2) { wordl = ((input[0] >> 2) & 0x03ff) | ((input[1] <<  8) & 0xfc00); wordh = 0x8000 | ((input[2] << 2) & 0x3ff0) | ((input[1] >> 8) & 0x000f); }
    else if (accelRange == 1) { wordl = ((input[0] >> 1) & 0x03ff) | ((input[1] <<  9) & 0xfc00); wordh = 0x4000 | ((input[2] << 3) & 0x3ff0) | ((input[1] >> 7) & 0x000f); }
    else                      { wordl = ((input[0]     ) & 0x03ff) | ((input[1] << 10) & 0xfc00); wordh =          ((input[2] << 4) & 0x3ff0) | ((input[1] >> 6) & 0x000f); }

    // Output
    ((unsigned short*)output)[0] = wordl;
    ((unsigned short*)output)[1] = wordh;
    return;
#else
    // This old version based on 8-bit code
    unsigned char in[6];
    unsigned char v;

    // Undo dynamic scaling... (would be more efficient if received non-scaled input)
    in[0] = (unsigned char)(input[0] >> accelRange); in[1] = (unsigned char)(input[0] >> (accelRange + 8));
    in[2] = (unsigned char)(input[1] >> accelRange); in[3] = (unsigned char)(input[1] >> (accelRange + 8));
    in[4] = (unsigned char)(input[2] >> accelRange); in[5] = (unsigned char)(input[2] >> (accelRange + 8));

    // X
    output[0] = in[0];            // (set 1st byte)	// XL (0-7) -- low 8 bits of X to X0-X7
    output[1] = in[1] & 0x03; // (set 2nd byte)	// XH (8-9) -- upper 2 bits of X to X8-X9

    // Y
    v = in[2];												// YL (0-7)
    output[1] |= (v << 2);		 // (mask 2nd byte)	//          -- low 6 bits of Y to Y0-Y5
    output[2] = (v >> 6);          // (set 3rd byte)	//          -- middle 2 bits of Y to Y6-Y7
    v = in[3];												// YH (8-9)
    output[2] |= ((v & 0x03) << 2);// (mask 3rd byte)	//          -- upper 2 bits of Y to Y8-Y9

    // Z
    v = in[4];												// ZL (0-7)
    output[2] |= (v << 4);		 // (mask 3rd byte)	//          -- lower 4 bits of Z to Z0-Z3
    output[3] = (v >> 4);          // (set 4th byte)	//          -- middle 4 bits of Z to Z4-Z7
    v = in[5];												// ZH (8-9)
    output[3] |= ((v & 0x03) << 4);// (mask 4th byte)	//          -- upper 2 bits of Z to Z8-Z9

    // Exponent
    output[3] |= (accelRange << 6);// (mask 4th byte)	//          -- full 2 bits of E to E0-E1

    return;
#endif
}
