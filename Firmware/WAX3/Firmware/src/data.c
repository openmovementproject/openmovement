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

// data.c - Sample Data I/O
// Dan Jackson, 2010-2011.


#include "SystemProfile.h"
#include "HardwareProfile.h"
#include <stdio.h>
#include <string.h>
#include <flash.h>
#include <math.h>
#include "data.h"
#include "accel.h"
#include "util.h"
#include "usb_cdc.h"
#include "WirelessProtocols\MCHP_API.h"
//#include "gyro.h"


#pragma idata

// Circular buffer head/tail pointers
unsigned char accelBufferHead = 0, accelBufferTail = 0;
#if (DEVICE_TYPE==2)
unsigned char gyroBufferHead = 0, gyroBufferTail = 0;
#endif


#pragma udata

// Circular buffer sizes (*MUST* be 2^n for the mask to be correct)
#if (DEVICE_TYPE == 0)				     // 0 = FFD Attached receiver
	#if (DEVICE_TYPE==2)
	    #define ACCEL_BUFFER_CAPACITY 64
	    #define GYRO_BUFFER_CAPACITY 64
	#else
	    #define ACCEL_BUFFER_CAPACITY 64
	#endif
#else                                    //     RFD Transmitter
	#if (DEVICE_TYPE==2)
	    #define ACCEL_BUFFER_CAPACITY 128
	    #define GYRO_BUFFER_CAPACITY 128
	#else
	    #define ACCEL_BUFFER_CAPACITY 256
	#endif
#endif

// Circular buffer mask
#define ACCEL_BUFFER_MASK(_i) ((_i) & (unsigned char)(ACCEL_BUFFER_CAPACITY - 1))
#if (DEVICE_TYPE==2)
#define GYRO_BUFFER_MASK(_i) ((_i) & (unsigned char)(GYRO_BUFFER_CAPACITY - 1))
#endif


#pragma udata writebuff
static Accel accelBuffer[ACCEL_BUFFER_CAPACITY];

#if (DEVICE_TYPE==2)
static Gyro gyroBuffer[GYRO_BUFFER_CAPACITY];
#endif

#pragma udata



#pragma idata
// RAM config
DataConfig dataConfig = {0};

// Serial number in PIC memory f800 to fbff (1024 bytes), check matches with linker and HardwareProfile.h #define IDLOC
#if MY_ADDRESS_LENGTH != 8
    #error "Unexpected address length (require MY_ADDRESS_LENGTH == 8)"
#endif
#pragma romdata idlocs = 0xf800L

// ROM config
rom DataConfig romDataConfig =
{
	{EUI_0,EUI_1,EUI_2,EUI_3, EUI_4, EUI_5,EUI_6,EUI_7},	// myLongAddress
	{0,EUI_1,EUI_2,EUI_3, EUI_4, EUI_5,EUI_6,EUI_7},		// targetLongAddress
    ACCEL_DEFAULT_RATE,										// accelConfig
    ACCEL_DEFAULT_WATERMARK,								// accelWatermark
	ACCEL_DEFAULT_ACTIVITY_THRESHOLD,						// activityThreshold
	ACCEL_DEFAULT_INACTIVITY_THRESHOLD,						// inactivityThreshold
	ACCEL_DEFAULT_INACTIVITY_TIME,							// inactivityTime
	TRANSMIT_INTERVAL,										// jitterMask
	JITTER_MASK,											// transmitInterval
	DEBUG_FIXED_CHANNEL,									// channel
	DEFAULT_PAN,											// pan
#if defined(DEMO_DEVICE)
	RX_MODE_OSC,												// default start mode
#elif defined(WIFCO_RECEIVER)
	RX_MODE_TEXT,												// default start mode
#elif defined(TEDDI_DEVICE)
	RX_MODE_TEXT,												// default start mode
#elif defined(IMU_DEVICE)
	RX_MODE_TEXT,												// default start mode
#else
	RX_MODE_SLIP,												// default start mode
#endif
};


// USB identifier
#if (DEVICE_TYPE == 0)			// Receiver
	#define USB_ID_CHAR 'r'
#elif (DEVICE_TYPE == 1)		// WAX transmitter
	#define USB_ID_CHAR 't'
#elif (DEVICE_TYPE == 2)		// WAX+GYRO transmiter
	#define USB_ID_CHAR 'g'
#elif (DEVICE_TYPE == 3)		// WAX+MOTOR transmiter/receiver
	#define USB_ID_CHAR 'm'
#elif (DEVICE_TYPE == 4)		// TEDDI transmitter
	#define USB_ID_CHAR 'T'
#else							// Unknown
	#define USB_ID_CHAR '?'
#endif	    


// Initial USB Product serial number (can be re-written at run-time)
ROM struct { BYTE bLength; BYTE bDscType; WORD string[13]; } sd003 = 
{
    sizeof(sd003), 0x03,		// USB_DESCRIPTOR_STRING = 0x03
    {
        'W',
        'A',
        'X',
        '-',
        USB_ID_CHAR,
        '0' + ((WAX >> 4) & 0xf),
        '0' + (WAX & 0xf),
        '_',
        '0' + ((DEVICE_ID / 10000) % 10),
        '0' + ((DEVICE_ID /  1000) % 10),
        '0' + ((DEVICE_ID /   100) % 10),
        '0' + ((DEVICE_ID /    10) % 10),
        '0' + ((DEVICE_ID /     1) % 10)
    }
};

#pragma romdata
#pragma code

// DeviceId -- patch device ID
extern BYTE myLongAddress[MY_ADDRESS_LENGTH];	// in P2P.c
extern BYTE targetLongAddress[MY_ADDRESS_LENGTH];	// in data.c
void DataConfigLoad()
{
	unsigned short id;
	int i;

	// Load config from ROM to RAM structure
	memcpypgm2ram(&dataConfig, (const far rom void *)&romDataConfig, sizeof(DataConfig));

	// Set device id
	dataStatus.deviceId = dataConfig.myLongAddress[0] | ((unsigned short)dataConfig.myLongAddress[1] << 8);

	// Patch over myLongAddress from P2P.c
	for (i = 0; i < 8; i++)
	{
	    myLongAddress[i] = dataConfig.myLongAddress[i];
	}

	// Patch over targetLongAddress
	for (i = 0; i < 8; i++)
	{
	    targetLongAddress[i] = dataConfig.targetLongAddress[i];
	}

	// Set current mode
	dataStatus.rxMode = dataConfig.startRxMode;
	
#ifdef ACCEL_FIXED_RATE
	// (For gyro mode) fixed accelerometer rate (flexible g-range and low-power bit)
	dataConfig.accelRate = (dataConfig.accelRate & 0xf0) | (ACCEL_FIXED_RATE & 0x0f);
#endif
	
}

void DataConfigSetId(unsigned short id)
{
	dataStatus.deviceId = id;

	dataConfig.myLongAddress[0] = (unsigned char)id;
	dataConfig.myLongAddress[1] = (unsigned char)(id >> 8);
    dataConfig.myLongAddress[2] = EUI_2;
    dataConfig.myLongAddress[3] = EUI_3;
    dataConfig.myLongAddress[4] = EUI_4;
    dataConfig.myLongAddress[5] = EUI_5;
    dataConfig.myLongAddress[6] = EUI_6;
    dataConfig.myLongAddress[7] = EUI_7;
}

void DataConfigSetTargetId(unsigned short id)
{
	dataStatus.deviceId = id;

	dataConfig.targetLongAddress[0] = (unsigned char)id;
	dataConfig.targetLongAddress[1] = (unsigned char)(id >> 8);
	dataConfig.targetLongAddress[2] = EUI_2;
	dataConfig.targetLongAddress[3] = EUI_3;
	dataConfig.targetLongAddress[4] = EUI_4;
	dataConfig.targetLongAddress[5] = EUI_5;
	dataConfig.targetLongAddress[6] = EUI_6;
	dataConfig.targetLongAddress[7] = EUI_7;
}

void DataConfigSave()
{
	// Check large code model
	#if !defined(__LARGE__)
		// Large codel model:  Project / Build Options / Project / MPLAB C18 / Memory Model / Large code model (> 64K bytes)
		#warning "Must compile with 'Large code model' so that TBLPTRU is preserved."
	#endif

	EraseFlash((unsigned long)IDLOC, (unsigned long)((unsigned char *)IDLOC + ERASE_BLOCK_SIZE - 1));
	
	WriteBytesFlash((unsigned long)IDLOC, (unsigned int)sizeof(DataConfig), (unsigned char *)&dataConfig);

	// sd003 USB serial number
	{
	    #define ID_SCRATCH_SIZE sizeof(sd003)
	    unsigned char scratch[ID_SCRATCH_SIZE];		// caution: on stack
	    unsigned char *p = scratch;

	    //ROM struct { BYTE bLength; BYTE bDscType; WORD string[5]; } sd003 = 
	    *p++ = sizeof(sd003);
	    *p++ = 0x03;	// USB_DESCRIPTOR_STRING;
	    *p++ = 'W';
	    *p++ = 0;
	    *p++ = 'A';
	    *p++ = 0;
	    *p++ = 'X';
	    *p++ = 0;
	    *p++ = '-';
	    *p++ = 0;
	    *p++ = USB_ID_CHAR,
	    *p++ = 0;
	    *p++ = '0' + ((WAX >> 4) & 0xf);
	    *p++ = 0;
	    *p++ = '0' + (WAX & 0xf);
	    *p++ = 0;
	    *p++ = '_';
	    *p++ = 0;
	    *p++ = '0' + ((dataStatus.deviceId / 10000) % 10);
	    *p++ = 0;
	    *p++ = '0' + ((dataStatus.deviceId /  1000) % 10);
	    *p++ = 0;
	    *p++ = '0' + ((dataStatus.deviceId /   100) % 10);
	    *p++ = 0;
	    *p++ = '0' + ((dataStatus.deviceId /    10) % 10);
	    *p++ = 0;
	    *p++ = '0' + ((dataStatus.deviceId /     1) % 10);
	    *p++ = 0;

		WriteBytesFlash((unsigned long)IDLOC + sizeof(DataConfig), ID_SCRATCH_SIZE, scratch);
	}

    return;
}





#pragma idata
// public
/*volatile*/ DataStatus dataStatus = {0};

// private
BYTE targetLongAddress[MY_ADDRESS_LENGTH] = {0};


#pragma code


void DataResetSequence(void)
{
	accelBufferHead = 0;
	accelBufferTail = 0;

    dataStatus.totalSamples = 0;
	dataStatus.totalOverflow = 0;
	dataStatus.retries = 0;
}


void DataInit(void)
{
	DataResetSequence();
}


// Read accel FIFO into our sample buffer queue
unsigned char DataPerformAccelSample(void)
{
    #define BUFFER_MINIMUM_FREE 32
    #if (BUFFER_MINIMUM_FREE >= ACCEL_BUFFER_CAPACITY)
        #error "BUFFER_MINIMUM_FREE must be less than ACCEL_BUFFER_CAPACITY."
    #endif
    unsigned int totalFree;
    unsigned char totalRead = 0;

    // Check total spaces free in buffer
    totalFree = ((accelBufferTail >= accelBufferHead) ? (ACCEL_BUFFER_CAPACITY - accelBufferTail + accelBufferHead - 1) : (accelBufferHead - accelBufferTail - 1));
    // If too few, overflow buffer
    if (totalFree < BUFFER_MINIMUM_FREE)
    {
        // Buffer overflow, remove some entries
        int drop = (BUFFER_MINIMUM_FREE - totalFree);
        accelBufferHead = ACCEL_BUFFER_MASK(accelBufferHead + drop);
        dataStatus.totalOverflow += drop;
        totalFree = drop;
    }

    // Read accelerometer entries
    for (;;)
    {
        Accel *freePointer;
        unsigned int contiguousFree;
        unsigned char numRead;

        // Calculate next position in buffer and number of contiguous entries available
        freePointer = accelBuffer + accelBufferTail;
        contiguousFree = ((accelBufferTail >= accelBufferHead) ? (ACCEL_BUFFER_CAPACITY - accelBufferTail - (accelBufferHead == 0 ? 1 : 0)) : (accelBufferHead - accelBufferTail - 1));
    
        if (contiguousFree == 0) { break; }
        if (contiguousFree > 255) { contiguousFree = 255; }

        // Read accelerometer entries
        numRead = AccelReadFIFO((Accel *)freePointer, (unsigned char)contiguousFree);
    
        // If no data read, exit
        if (numRead <= 0) { break; }
    
        // Adjust tail pointer for new entries
        accelBufferTail = ACCEL_BUFFER_MASK(accelBufferTail + numRead);

        // Update counts
        totalRead += numRead;               // local
        dataStatus.totalSamples += numRead;       // global
    }

	return totalRead;
}


#if (DEVICE_TYPE==2)
// Read gyro FIFO into our sample buffer queue
unsigned char DataPerformGyroSample(void)
{
    #define BUFFER_MINIMUM_FREE 32
    #if (BUFFER_MINIMUM_FREE >= GYRO_BUFFER_CAPACITY)
        #error "BUFFER_MINIMUM_FREE must be less than GYRO_BUFFER_CAPACITY."
    #endif
    unsigned int totalFree;
    unsigned char totalRead = 0;

    // Check total spaces free in buffer
    totalFree = ((gyroBufferTail >= gyroBufferHead) ? (GYRO_BUFFER_CAPACITY - gyroBufferTail + gyroBufferHead - 1) : (gyroBufferHead - gyroBufferTail - 1));
    // If too few, overflow buffer
    if (totalFree < BUFFER_MINIMUM_FREE)
    {
        // Buffer overflow, remove some entries
        int drop = (BUFFER_MINIMUM_FREE - totalFree);
        gyroBufferHead = GYRO_BUFFER_MASK(gyroBufferHead + drop);
        dataStatus.totalOverflow += drop;
        totalFree = drop;
    }

    // Read gyro entries
    for (;;)
    {
        Gyro *freePointer;
        unsigned int contiguousFree;
        unsigned char numRead;

        // Calculate next position in buffer and number of contiguous entries available
        freePointer = gyroBuffer + gyroBufferTail;
        contiguousFree = ((gyroBufferTail >= gyroBufferHead) ? (GYRO_BUFFER_CAPACITY - gyroBufferTail - (gyroBufferHead == 0 ? 1 : 0)) : (gyroBufferHead - gyroBufferTail - 1));
    
        if (contiguousFree == 0) { break; }
        if (contiguousFree > 255) { contiguousFree = 255; }

        // Read gyro entries
        numRead = GyroReadFIFO((Gyro *)freePointer, (unsigned char)contiguousFree);
    
        // If no data read, exit
        if (numRead <= 0) { break; }
    
        // Adjust tail pointer for new entries
        gyroBufferTail = GYRO_BUFFER_MASK(gyroBufferTail + numRead);

        // Update counts
        totalRead += numRead;               // local
        dataStatus.totalSamples += numRead;       // global
    }

	return totalRead;
}
#endif


// Flush buffers to radio
unsigned char DataTransmit(void)
{
    // Calculate number of samples in the buffer
    unsigned int accelSampleCountInt = ((accelBufferTail >= accelBufferHead) ? (accelBufferTail - accelBufferHead) : (ACCEL_BUFFER_CAPACITY - accelBufferHead + accelBufferTail));
    unsigned char sampleCount;
    unsigned int sequenceId;
    unsigned int outstanding;
    unsigned char tempAccelHead;
#if (DEVICE_TYPE==2)
    unsigned int gyroSampleCountInt = ((gyroBufferTail >= gyroBufferHead) ? (gyroBufferTail - gyroBufferHead) : (GYRO_BUFFER_CAPACITY - gyroBufferHead + gyroBufferTail));
    unsigned char tempGyroHead;
#endif
    unsigned char i;
    char success;

    #define BUFFER_MINIMUM_PACKET 1

	#define IMU_BUFFER_MAXIMUM_DISCREPANCY 1	// Balance queues if over N out (otherwise allow mis-matched queue lengths)
	
	#if defined(IMU_REMOVE_ACCEL) && defined(IMU_BUFFER_MAXIMUM_DISCREPANCY)
	#undef IMU_BUFFER_MAXIMUM_DISCREPANCY
	#endif
    
LED = 1;
    
    sequenceId = (unsigned int)(dataStatus.totalSamples - accelSampleCountInt);
    outstanding = accelSampleCountInt;

    // HACK: Trim sample queues to match the smaller queue
	#if (DEVICE_TYPE==2 && defined(IMU_BUFFER_MAXIMUM_DISCREPANCY))
	    // Assumption: DataPerformSample() has just been called, devices have the same sample rate set.
	    
		// While there is more than one gyro sample more than accelerometer samples, discard additional gyro samples
	    while (gyroSampleCountInt > accelSampleCountInt + IMU_BUFFER_MAXIMUM_DISCREPANCY)
	    {
		    gyroBufferHead = GYRO_BUFFER_MASK(gyroBufferHead + 1);
		    gyroSampleCountInt--;
	    }
	    
		// While there is more than one accelerometer sample more than gyro samples, discard additional accelerometer samples
	    while (accelSampleCountInt > gyroSampleCountInt + IMU_BUFFER_MAXIMUM_DISCREPANCY)
	    {
		    accelBufferHead = ACCEL_BUFFER_MASK(accelBufferHead + 1);
		    accelSampleCountInt--;
	    }
	
		// Choose the smaller of the two queues    
	    if (gyroSampleCountInt < accelSampleCountInt) { accelSampleCountInt = gyroSampleCountInt; }
	    else if (accelSampleCountInt < gyroSampleCountInt) { gyroSampleCountInt = accelSampleCountInt; }
	    
    #endif


	// Clamp if too many accel samples
    if (accelSampleCountInt > BUFFER_PACKET_MAX_SAMPLES) { accelSampleCountInt = BUFFER_PACKET_MAX_SAMPLES; }    
    
#if (DEVICE_TYPE==2 && defined(IMU_REMOVE_ACCEL))
	outstanding = 0;
	accelSampleCountInt = 0;
    accelBufferTail = accelBufferHead;
#endif

	#if (DEVICE_TYPE==2)
		// Clamp if too many gyro samples
	    if (gyroSampleCountInt > BUFFER_PACKET_MAX_SAMPLES) { gyroSampleCountInt = BUFFER_PACKET_MAX_SAMPLES; }
	    
	    // Only transmit if enough accel or gyro samples
	    if (accelSampleCountInt < BUFFER_MINIMUM_PACKET && gyroSampleCountInt < BUFFER_MINIMUM_PACKET) { return 0; }
    #else
	    // Only transmit if enough accel samples
    	if (accelSampleCountInt < BUFFER_MINIMUM_PACKET) { return 0; }
    #endif
    
    // Update number of samples outstanding to reflect samples added to buffer
    sampleCount = (unsigned char)accelSampleCountInt;
    outstanding -= sampleCount;

	// Reset the transmit buffer
	MiApp_FlushTx();

    // IMPORTANT: This code has to mirror the DataPacket struct
	MiApp_WriteData(USER_REPORT_TYPE);                      // reportType;		    // [1] = 0x12 (USER_REPORT_TYPE)
	MiApp_WriteData(DATA_ACCEL_REPORT_ID);                  // reportId;            // [1] = 0x78 (ASCII 'x')
    MiApp_WriteData((unsigned char)(dataStatus.deviceId));      // deviceId:LOW     // [2] = Device identifier (16-bit)
    MiApp_WriteData((unsigned char)(dataStatus.deviceId >> 8)); // deviceId:HIGH    // ''
    MiApp_WriteData(0);                                     // status	            // [1] = Device status (bit 0 is battery warning, top 7 bits reserved)
    MiApp_WriteData((unsigned char)(0));                    // sample:LOW           // [2] = Analogue sample (top 6 bits indicate measurement information, lower 10 bits are the value)
    MiApp_WriteData((unsigned char)((0) >> 8));             // sample:HIGH          // ''    <placeholder>
#ifdef ACCEL_DWORD_FORMAT
    MiApp_WriteData((AccelCurrentRate() & 0xc0) | 0x00 | (AccelCurrentRate() & 0x0f));          // format;	            // [1] = Accelerometer data format 0xE9 (2g, 3x10+2-bit, 50Hz); Top two bits is G-range, next two bits is format [0=10-bit raw -> 3x10-bit + 2, 2=signed 16-bit] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
#else
    MiApp_WriteData((AccelCurrentRate() & 0xc0) | 0x20 | (AccelCurrentRate() & 0x0f));          // format;	            // [1] = Accelerometer data format 0xE9 (2g, 3x16-bit, 50Hz); Top two bits is G-range, next two bits is format [0=10-bit raw -> 30-bit + 2, 2=signed 16-bit] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
#endif
    MiApp_WriteData((unsigned char)(sequenceId));           // sequenceId:LOW       // [2] = Sequence number of first accelerometer reading in this packet (16-bit sample index -- will wrap or be reset if device sleeps and resumes)
    MiApp_WriteData((unsigned char)(sequenceId >> 8));      // sequenceId:HIGH      // ''
    MiApp_WriteData((unsigned char)outstanding);            // outstanding          // [1] = Number of samples remaining on device after this packet (0xff: >= 255)
    MiApp_WriteData(sampleCount);                           // sampleCount          // [1] = Number of samples in this packet. 0x0c = 13 samples (number of accelerometer samples)

    // Output signed short sampleData[BUFFER_SAMPLE_COUNT * 3];	// [sampleCount * numAxes * bytesPerSample = 78 -- would be 102 with 17 samples] (sample data)
    tempAccelHead = accelBufferHead;
    for (i = 0; i < sampleCount; i++)
    {
        // Remove the head item and increment the head
        Accel *v = accelBuffer + tempAccelHead;
        tempAccelHead = ACCEL_BUFFER_MASK(tempAccelHead + 1);

        // Add sample to radio packet
		#ifdef ACCEL_DWORD_FORMAT
	   		MiApp_WriteData(v->data[0]);
	   		MiApp_WriteData(v->data[1]);
	   		MiApp_WriteData(v->data[2]);
	   		MiApp_WriteData(v->data[3]);
		#else
	   		MiApp_WriteData(v->xl); MiApp_WriteData(v->xh);
	   		MiApp_WriteData(v->yl); MiApp_WriteData(v->yh);
	   		MiApp_WriteData(v->zl); MiApp_WriteData(v->zh);
		#endif
    }
    
#if (DEVICE_TYPE==2)
    tempGyroHead = gyroBufferHead;
    for (i = 0; i < gyroSampleCountInt; i++)
    {
        // Remove the head item and increment the head
        Gyro *v = gyroBuffer + tempGyroHead;
        tempGyroHead = GYRO_BUFFER_MASK(tempGyroHead + 1);

   		MiApp_WriteData(v->xl); MiApp_WriteData(v->xh);
   		MiApp_WriteData(v->yl); MiApp_WriteData(v->yh);
   		MiApp_WriteData(v->zl); MiApp_WriteData(v->zh);
    }
#endif
    
    // Transmit packet
	success = MiApp_UnicastAddress(targetLongAddress, TRUE, FALSE);     // Send to specific address (BYTE *address, true=permanent, false=unsecured)
	//success = MiApp_UnicastConnection(id, FALSE) 						// Send to known connection ID (id, false=unsecure)
	//success = MiApp_BroadcastPacket(FALSE);								// Broadcast packet (false=unsecure)

    // If failed, leave entries in buffer and return zero
    if (!success)
    {
        return 0;
    }

    // Success, remove samples from buffer
    accelBufferHead = tempAccelHead;      // ACCEL_BUFFER_MASK(accelBufferHead + sampleCount);
#if (DEVICE_TYPE==2)
    gyroBufferHead = tempGyroHead;      // GYRO_BUFFER_MASK(gyroBufferHead + sampleCount);
#endif

    // Return number of samples successfully sent
	return sampleCount;
}


// Write a SLIP-encoded OSC string (length prefix, null-padded to multiple of four bytes)
void write_slip_osc_string(const rom far char *string)
{
	unsigned char len;
	
	len = strlenpgm(string);
	usb_slip_encoded_rom((const rom far void *)string, len);
	usb_putchar(0x00); 
	
	// Check overall length for required padding
	len = (len + 1) & 3;
	if (len != 0)
	{
		if (len <= 1) { usb_putchar(0x00); }
		if (len <= 2) { usb_putchar(0x00); }
		if (len <= 3) { usb_putchar(0x00); }
	}
}

// Write a SLIP-encoded OSC int (big-endian 32-bit integer)
void write_slip_osc_int(signed long value)
{
	unsigned char buffer[4];
	
#if 1
	buffer[0] = (unsigned char)(value >> 24);
	buffer[1] = (unsigned char)(value >> 16);
	buffer[2] = (unsigned char)(value >>  8);
	buffer[3] = (unsigned char)(value      );
#else	
	buffer[0] = *(((unsigned char *)&value) + 3);
	buffer[1] = *(((unsigned char *)&value) + 2);
	buffer[2] = *(((unsigned char *)&value) + 1);
	buffer[3] = *(((unsigned char *)&value) + 0);
#endif	

	usb_slip_encoded(buffer, 4);
}

// Write a SLIP-encoded OSC float (big-endian 32-bit float)
void write_slip_osc_float(float value)
{
	write_slip_osc_int(*((signed long *)(&value)));
}

// Write a SLIP-encoded OSC long -1 (big-endian 64-bit integer)
void write_slip_osc_long_minus_one(void)
{
	usb_writer((const rom char far *)"\377\377\377\377\377\377\377\377", 8);
}


void DataDumpPacket(DataPacket *dp, unsigned char size)
{
	unsigned int tries = 0;
	
	if (dataStatus.rxMode == RX_MODE_SLIP)
	{
	    // Send as SLIP (RFC 1055) encoded packet
	    usb_putchar(SLIP_END);
	    usb_slip_encoded(dp, size); //sizeof(DataPacket) + (dp->sampleCount * BYTES_PER_SAMPLE_FROM_FORMAT(dp->format)));
	    usb_putchar(SLIP_END);
	    
	    
	}
	else if (dataStatus.rxMode == RX_MODE_TEXT)
	{
		int i;
		unsigned char normalPacketSize = sizeof(DataPacket) + (dp->sampleCount * BYTES_PER_SAMPLE_FROM_FORMAT(dp->format));
		char additionalPayload = (char)size - (char)normalPacketSize;


		// WAX+GYRO packet
		if (additionalPayload > 0 && (additionalPayload % 6) == 0)
		{
			char gyroSamples = additionalPayload / 6;
			
			usb_putrs((const rom char far *)"$GYRO,");
			usb_putrs((const rom char far *)"1,");	// version,
			usb_puts(my_itoa(gyroSamples)); 
			
			for (i = 0; i < gyroSamples; i++)
			{
				// Get 3x16-bit gyro sample values
				signed short *data = (signed short *)((void *)dp + normalPacketSize + 6 * i);
			
				// Print gyro X/Y/Z
				usb_putchar(','); usb_puts(my_itoa(data[0])); 
				usb_putchar('/'); usb_puts(my_itoa(data[1])); 
				usb_putchar('/'); usb_puts(my_itoa(data[2])); 
			
// Flush intermittently
for (tries = 0x7FFF; tries != 0 && usbOutFree() < 64; --tries) { USBProcessIO(); }
			}
			
			usb_putrs((const rom char far *)"\r\n");
		} else if (additionalPayload > 0) {
			usb_putrs((const rom char far *)"$ADD,");
			usb_puts(my_itoa(additionalPayload)); 
			usb_putrs((const rom char far *)"\r\n");
		}
		
		// Print packet details
		//printf((const rom char far *)"$ADXL,%u,1,%u,%u,%u,%u,%u", dp->deviceId, dp->status, dp->sample, dp->sequenceId, dp->format, dp->sampleCount);
	
		usb_putrs((const rom char far *)"$ADXL,");
		usb_puts(my_itoa(dp->deviceId)); usb_putrs((const rom char far *)",1,");	// deviceId, version,
		usb_puts(my_itoa(dp->status)); usb_putchar(',');
		usb_puts(my_itoa(dp->sample)); usb_putchar(',');
		usb_puts(my_itoa(dp->sequenceId)); usb_putchar(',');
		usb_puts(my_itoa(dp->format)); usb_putchar(',');
		usb_puts(my_itoa(dp->sampleCount)); 

		for (i = 0; i < dp->sampleCount; i++)
		{
			signed short x, y, z;
			
			if (((dp->format >> 4) & 3) == 0)
			{
				// Get 3x10+2-bit sample values
				Accel a = *((Accel *)((void *)dp + sizeof(DataPacket) + 4 * i));
				x = ACCEL_XVALUE(a);
				y = ACCEL_YVALUE(a);
				z = ACCEL_ZVALUE(a);
			}
			else if (((dp->format >> 4) & 3) == 2)
			{
				// Get 3x16-bit sample values
				signed short *data = (signed short *)((void *)dp + sizeof(DataPacket) + 6 * i);
				x = data[0];
				y = data[1];
				z = data[2];
			}
			else { x = 0; y = 0; z = 0; }	
			
			// Print X/Y/Z
			usb_putchar(','); usb_puts(my_itoa(x)); 
			usb_putchar('/'); usb_puts(my_itoa(y)); 
			usb_putchar('/'); usb_puts(my_itoa(z)); 
			
// Flush intermittently
for (tries = 0x7FFF; tries != 0 && usbOutFree() < 64; --tries) { USBProcessIO(); }
		}
		
		usb_putrs((const rom char far *)"\r\n");
		
		
	}
	else if (dataStatus.rxMode == RX_MODE_OSC)
	{
		int i;
		
	    // Send as SLIP-encoded OSC bundle
	    usb_putchar(SLIP_END);
	    
	    // CAUTION: Ensure any binary output is SLIP-compatible...
    
		// [OSC string] bundle identifier: "#bundle" <pads to 8 bytes>
		write_slip_osc_string((const rom char far *)"#bundle");
		
		// [OSC long] timestamp: -1 (now)
	    write_slip_osc_long_minus_one();

		// OSC messages
		for (i = 0; i < dp->sampleCount; i++)
		{
			signed short x, y, z;
			
// Flush intermittently
for (tries = 0x7FFF; tries != 0 && usbOutFree() < 40; --tries) { USBProcessIO(); }

			if (((dp->format >> 4) & 3) == 0)
			{
				// Get 3x10+2-bit sample values
				Accel a = *((Accel *)((void *)dp + sizeof(DataPacket) + 4 * i));
				x = ACCEL_XVALUE(a);
				y = ACCEL_YVALUE(a);
				z = ACCEL_ZVALUE(a);
			}
			else if (((dp->format >> 4) & 3) == 2)
			{
				// Get 3x16-bit sample values
				signed short *data = (signed short *)((void *)dp + sizeof(DataPacket) + 6 * i);
				x = data[0];
				y = data[1];
				z = data[2];
			}
			else { x = 0; y = 0; z = 0; }	

		    // [OSC int] message length: 36 (8 + 8 + 4 + 4 + 4 + 4 + 4)
		    write_slip_osc_int(36);

			// [OSC string] address: "/wax" <pads to 8 bytes>
		    write_slip_osc_string((const rom char far *)"/wax");
	    	
			// [OSC string] type tag: ",iiiii" <pads to 8 bytes>
		    write_slip_osc_string((const rom char far *)",iifff");
		    
			// [OSC int] device id
			write_slip_osc_int(dp->deviceId);
			
			// [OSC int] sample index
			write_slip_osc_int(dp->sequenceId + i);
			
			// [OSC float] X
			write_slip_osc_float((float)x / 256.0f);
			
			// [OSC float] Y
			write_slip_osc_float((float)y / 256.0f);
			
			// [OSC float] Z
			write_slip_osc_float((float)z / 256.0f);
	
		}
	    
	    usb_putchar(SLIP_END);
	}
	else if (dataStatus.rxMode == RX_MODE_HEX)
	{
		hexdump(dp, size);
	}
	
}

