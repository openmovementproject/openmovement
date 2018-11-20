// Audio logging
// KL 21-06-2012

#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Logger.h"

#include "GraphicsConfig.h"
#include "Graphics/GOL.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/Primitive.h"
#include "Graphics/DisplayBasicFont.h"
#include "Graphics/PIRULEN32PIX.h"

#include "Analog.h"
#include "Utils/Wav.h"
#include "MDD File System/FSIO.h"
#include "Utils/Fifo.h"
#include "Utils/FSUtils.h"
#include "AudioStream.h"

//#include "Peripherals/myI2C.h"
#include "Peripherals/Accel.h"
#if (MODE_GYRO == MODE_LOG)
#include "Peripherals/Gyro.h"
#endif
#include "Peripherals/Prox.h"
#include "Codec AIC111 DCI.h"


#define WAIT_ACCEL_SECTORS 1
#define WAIT_GYRO_SECTORS 1
#define WAIT_ADC_SECTORS 1


#include "Utils/DataStream.h"
#include "Utils/FileStream.h"
#if (MODE_ACCEL == MODE_LOG)
	#define ACCEL_SECTOR_SAMPLE_COUNT	(480/sizeof(accel_t)) 		/*Number of samples per sector*/
	accel_t accelBuffer[WAIT_ACCEL_SECTORS*ACCEL_SECTOR_SAMPLE_COUNT + ACCEL_SECTOR_SAMPLE_COUNT]; 	 // 1.0 spare sectors
#endif
#if (MODE_ADC == MODE_LOG)
	#define ADC_SECTOR_SAMPLE_COUNT (480/sizeof(adc_results_t))
	adc_results_t adcBuffer[WAIT_ADC_SECTORS*ADC_SECTOR_SAMPLE_COUNT + (ADC_SECTOR_SAMPLE_COUNT/2)]; // 0.5 spare sectors
#endif
#if (MODE_GYRO == MODE_LOG)
	#define GYRO_SECTOR_SAMPLE_COUNT	(480/sizeof(gyro_t))		/*Number of samples per sector*/
	gyro_t gyroBuffer[WAIT_GYRO_SECTORS*GYRO_SECTOR_SAMPLE_COUNT + GYRO_SECTOR_SAMPLE_COUNT];        // 1.0 spare sectors
#endif
static FSFILE * logFile = NULL;
datastream_t 	accelDataStream, gyroDataStream, adcDataStream;
filestream_t 	accelFileStream, gyroFileStream, adcFileStream;
volatile dataStreamFlags_t gDataReady = {{0}}; 
volatile dataStreamFlags_t gStreamEnable = {{0}};
//#define INACTIVITY_THRESHOLD 30
volatile unsigned short inactivity_timer = 0xffff;	// Start as "inactive"
extern unsigned char scratchBuffer[512];	// Sector buffer for making the data sectors in
void LoggerRTCTasks(void) // Called at 1Hz from rtc int
{
	if(gStreamEnable.all>0)
	{
		#if (MODE_ADC >= MODE_USE)
			if(!gDataReady.adc)								// ADC sample 1Hz
			{
				DataStreamUpdateTimestamp(&adcDataStream); 	// Update the time stamp
				gDataReady.adc = TRUE; 						// Indicates fifo should sample the values
				DATA_STREAM_SW_INT_IF = 1; 					// SW trigger fifo int tasks
			}
		#endif

		if (gStreamEnable.accel_int2)
		{
			if (inactivity_timer < 0xffff) inactivity_timer++;	// Inactivity timer increment and ceiling
		}
		else
		{
			// Fall back to assuming active if just recording audio, or assume inactive otherwise
			if (settings.recordData == RECORD_SOUND || settings.recordData == RECORD_NONE) { inactivity_timer = 0; }
			else { inactivity_timer = 0xffff; }	
		}

	}


}
void __attribute__((interrupt,auto_psv)) ACCEL_INT_WATERMARK_HANDLER(void) // Accelerometer fifo interrupt
{
	// This is only for the fifo int
	ACCEL_INT_WATERMARK_IF = 0; 
	#if (MODE_ACCEL >= MODE_USE)
		if (!gDataReady.accel) 			// Data not already ready
		{
			gDataReady.accel = TRUE;	// Indicate data ready
			#if (MODE_ACCEL == MODE_LOG)
			DataStreamUpdateTimestamp(&accelDataStream); // Not actually valid until read in sw triggered int
			#endif
		}
		DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks 
	#endif
}
void __attribute__((interrupt,auto_psv)) _INT3Interrupt(void) // Gyro fifo interrupt
{
	// This is only for the fifo int
	IFS3bits.INT3IF = 0; 
	#if (MODE_GYRO == MODE_LOG)
		if (!gDataReady.gyro) 	// Data not already ready
		{
			gDataReady.gyro = TRUE;	// Indicate data ready
			DataStreamUpdateTimestamp(&gyroDataStream); // Not actually valid until read in sw triggered int
		}
		DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks
	#endif 
}
void __attribute__((interrupt,auto_psv)) ACCEL_INT_EVENT_HANDLER(void) // Other accelerometer interrupt
{
	ACCEL_INT_EVENT_IE = 0;			// Disable interrupts (will re-enable at 1Hz)
	ACCEL_INT_EVENT_IF = 0;
	// This is for other accel ints
	gDataReady.accel_int2 = TRUE;	// Indicate data ready
//	if (gStreamEnable.accel_int2)		// always read the accelerometer to clear this condition...
	{
		//DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks 
	}
}
/* 	The main data reading ISR, all I2C comms outside this need to disable it first!
	This is the interrupts for all device->fifo comms (i2c) at GLOBAL_I2C_PRIORITY 
	1) Polls each dataReadyFlag
	2) Grabs data and writes it to the fifo					*/
void __attribute__((interrupt,auto_psv)) DATA_STREAM_SW_INTERRUPT_HANDLER(void) 
{
	dataStreamFlags_t dataReady;

	// Find out which devices have data - need to protect this read from interrupts
	asm("#DISI 0x3FFF");
	DATA_STREAM_SW_INT_IF = 0;	// Clear the flag in the DISI
	dataReady = gDataReady; 	// Grab a copy of the global var
	gDataReady.all = 0;			// Clear the global var
	//asm("#DISI 0x0000");

	// KL - suspect the DISI may be blocking the interrupt edge latch
	// Adding following code to protect for this. If there is no data ready for the 
	// device but the pin is high then an interrupt has been missed.
	if(!dataReady.accel && ACCEL_INT_WATERMARK)	{ACCEL_INT_WATERMARK_IF = 1;}	// This will vector immediately
#if (MODE_GYRO == MODE_LOG)
	if(!dataReady.gyro && GYRO_INT2)	{GYRO_INT2_IF = 1; 	}	// This will vector immediately
#endif
	//if(gStreamEnable.accel_int2 && !dataReady.accel_int2 && ACCEL_INT2)	{ACCEL_INT2_IF = 1;}	// This will vector immediately
	asm("#DISI 0x0000");

	// Next process all the device to fifo tasks
	// If the data source may be turned on/off you must always clear the condition so it can re-interrupt
	// If a source may be ignored due to the gStreamEnable or initial mismatch then check the pin too
	// e.g. For Accel interrupt 2 the device may have already interrupted and the pin high before the loop starts.
	#ifdef LOGGING_DEBUG_LOG_LED
		LOGGING_DEBUG_LOG_LED = 1;
	#endif

	// Accelerometer
	#if (MODE_ACCEL >= MODE_USE)

	// Accelerometer
	if (dataReady.accel)
	{
		// Empty hardware FIFO upto ACCEL_FIFO_WATERMARK bytes to keep time stamp valid from last interrupt
		unsigned short remain = ACCEL_FIFO_WATERMARK; 		// The number in the fifo is at least this number
		unsigned char written = 0;
		#ifdef LOGGING_DEBUG_ACCEL_LED
			LOGGING_DEBUG_ACCEL_LED = 1;
		#endif

		#ifdef USE_PEDOMETER
			void* dest = PedDataCaptureDest(ACCEL_FIFO_WATERMARK);	// Where to copy the pedometer data to, intended num samples
		#endif
		

		if (gStreamEnable.accel) // Stream is enabled - required upto 2 copy operations INCLUDING for pedometer
		{
			unsigned char passes;
	        for (passes = 2; passes != 0; --passes)
	        {
	            unsigned short contiguous, num;
	            void *buffer;
	            // See how much contiguous free space we have in the buffer
	            contiguous = FifoContiguousSpaces(&accelDataStream.fifo, &buffer);

	            // If we aren't able to fit *any* in, we've over-run our software buffer
	            if (contiguous == 0) {break;} // No space in fifo

	            if (contiguous > remain) contiguous = remain; // Only one copy operation needed

				num = AccelReadFIFO((accel_t *)buffer, contiguous); // Copy into fifo buffer
				FifoExternallyAdded(&accelDataStream.fifo, num);	// Tell fifo they were added
				#ifdef USE_PEDOMETER
					if(dest!=NULL)memcpy (dest,buffer, (num*sizeof(accel_t)));// Also copy to the ped buffer
					dest += num*sizeof(accel_t);						// Update pointer to the ped buffer
				#endif
	            if (num == 0) { break; }	// Hadware returned no samples error

				remain -= num;				// Subtract number read
				written += num;				// Update number read
				if (remain == 0) { break; }	// Exit if done
			}	// else do another pass
        } // If stream is enabled
		else // If steam is not enabled - just copy to ped buffer,AccelReadFIFO handles NULL ok					 
		{ 
			#ifdef USE_PEDOMETER
				AccelReadFIFO((accel_t *)dest, remain); // direct write to ped buffer
			#endif
		}

		// This function is very fast for small numbers of samples
		#ifdef USE_PEDOMETER
			PedTasks(); // Will process ACCEL_FIFO_WATERMARK number of samples from its last buffer
		#endif
	}
//	else
//	{
//		if(ACCEL_INT1) // Interrupt pin high but no data processed
//		{
//			Nop();
//			LED_SET(LED_WHITE);
//		}
//	}
	#endif

	// Gyro
	#if (MODE_GYRO == MODE_LOG)
		if (dataReady.gyro)
		{
			// Gyro tasks 
			// Empty hardware FIFO upto GYRO_FIFO_WATERMARK bytes to keep time stamp valid from last interrupt
			unsigned short remain = GYRO_FIFO_WATERMARK;
			unsigned char passes;
			#ifdef LOGGING_DEBUG_GYRO_LED
				LOGGING_DEBUG_GYRO_LED = 1;
			#endif
	        for (passes = 2; passes != 0; --passes)
	        {
	            unsigned short contiguous, num;
	            void *buffer;
	            // See how much contiguous free space we have in the buffer
	            contiguous = FifoContiguousSpaces(&gyroDataStream.fifo, &buffer);
	            // If we aren't able to fit *any* in, we've over-run our software buffer
	            if (contiguous == 0)
	            {
	                //status.events |= DATA_EVENT_BUFFER_OVERFLOW;    // Flag a software FIFO over-run error
	                GyroReadFIFO(NULL, GYRO_MAX_FIFO_SAMPLES);    // Dump hardware FIFO contents to prevent continuous watermark/over-run interrupts
	                break;
	            }
	            // Reads the gyro fifo to sw fifo
				if (contiguous > remain) contiguous = remain;
				if (gStreamEnable.gyro) 
				{ 
					num = GyroReadFIFO((gyro_t *)buffer, contiguous);
					FifoExternallyAdded(&gyroDataStream.fifo, num);
				}
				else					 
				{ 
					num = GyroReadFIFO(NULL, 32);break; // Just clear the fifo 
				}
	            // ??? There were no more entries to read - hardware fault, possibly wrong interrrupt triggered the fifo int
	            if (num == 0) { break; }
				remain -= num;
	            // Inform the buffer we've directly added some data
	            
				// If no more data to read out
				if (remain == 0) { break; }
	        }
		}
	#endif 


	// ADC conversion
	#if (MODE_ADC >= MODE_USE)
		if (dataReady.adc)
		{

			// Accel activity interrupts
			if ((dataReady.accel_int2)||(ACCEL_INT_EVENT))
			{
				// Accel int2 indicates other accel event triggers
				unsigned char reason = AccelReadIntSource(); // Clears pin(s)
				// Activity detection
				inactivity_timer = 0;
				if (reason & ACCEL_INT_SOURCE_TRANSIENT)  // Activity detected
				{	
					accelTransSource = AccelReadTransientSource();	// Read transient source
				}	
				// Pulse detection
				if (reason & ACCEL_INT_SOURCE_TAP)  		// Tap detected
				{
					accelTapStaus = AccelReadTapStatus();
				}
				// Orientation change
				if (reason & ACCEL_INT_SOURCE_ORIENTATION)  // Orientation change detected
				{
					accelOrientation = AccelReadOrientaion();
				}
				
// !! Re-enable interrupt				
ACCEL_INT_EVENT_IE = 1;
			}
			
			
			// Tagging the proximity reading to the ADC reads at 1Hz
			#if (MODE_PROX >= MODE_USE) 
				UpdateProximity(0);
			#endif

			// Always sample the ADC
			UpdateAdc();
			// Update ADC regs with other vars (light, temp)
			//adcResult.batt;
#ifdef HAS_PROX
			adcResult.light = prox.light; 				// Arbitrary units
			adcResult.prox = prox.proximity; 			// Arbitrary units
#else
			adcResult.light = -1;
			adcResult.prox = -1;
#endif
			adcResult.gain = GetGainInline();			// Audio gain from codec (high byte?)
			adcResult.inactivity = inactivity_timer;	// inactivity (sec)
			
			#ifdef OFFSET_LOGGING
				// Store cached file name
				adcResult.fileL = (unsigned short)(status.filenameBCD);
				adcResult.fileH = (unsigned short)(status.filenameBCD >> 16);
				
				// Store file offset (take pending buffer size into account)
				{
					unsigned long offset = status.offset;
					// Make this FIFO length safe against interrupts
					offset += (unsigned long)AudioStreamRecordingTaskLength() * AUDIO_SECTOR_SIZE;
					adcResult.offsetL = (unsigned short)(offset);
					adcResult.offsetH = (unsigned short)(offset >> 16);
				}
			#endif

Delay10us(40);	// 0.4 msec delay for prox. sensor

			if (gStreamEnable.adc)
			{ 
				#ifdef LOGGING_DEBUG_ADC_LED
					LOGGING_DEBUG_ADC_LED = 1;
				#endif
				// Push into fifo
				FifoPush(&adcDataStream.fifo, &adcResult, 1);
			}

			
			
		} 
	#endif

	// All data sources are processed, this interrupt will re-enter if new sources have become available
}


void WriteLog(const char *line)
{
	FSFILE *fp;
	fp = FSfopen(LOG_FILE, "a");
	if (fp != NULL)
	{
	    unsigned short fractional;
	    DateTime time;
	    time = RtcNowFractional(&fractional);
	    FSfprintf(fp, "%s.%03d,%s\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional), line);
	    FSfclose(fp);
	}
}


char LoggerInit(const char *filename)
{
	// Open the file for appendfail
	char retval = 0;

    // Initialize variables
    status.sampleCount = 0;
    status.lastWrittenTicks = 0;
    status.accelSequenceId = 0;
    status.gyroSequenceId = 0;
    //status.debugFlashCount = 3;     // Initially flash logging status

	// Initialise flags
	gDataReady.all = 0;
	gStreamEnable.all = 0;

	if (filename == NULL)
	{ 
		logFile = NULL; 
		return 0; 
	}

    // Open the log file for append
    logFile = FSfopen(filename, "a");
    if (logFile != NULL)
    {
		// Calculate number of sectors in file
		FSfseek(logFile, 0, SEEK_END);                              // Ensure seeked to end (should be in append mode)
		
		// Ensure archive attribute set to indicate file updated
		logFile->attributes |= ATTR_ARCHIVE;

#ifdef WRITE_LOG
		WriteLog("Start logging. ");
#endif
		
		// Initially flash to show started
		//status.debugFlashCount = 5;
		
		retval = 1;
    }
	else    
	{
		// Failed
    	return 0;
	}

	// Populate packing etc.
	// Setup the streams
	// Clear the data buffers

	// Accellerometer
	#if (MODE_ACCEL == MODE_LOG)
    accelFileStream.fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
	accelFileStream.streamId = 'a';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)

	// ! Sample rate from settings
	accelFileStream.sampleRate = ACCEL_FREQUENCY_FOR_RATE(settings.sampleRate);
	accelFileStream.sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)

	// ! Sample rate from settings
	if (ACCEL_RANGE_FOR_RATE(accelFileStream.sampleRate) == 8)
	{
		accelFileStream.dataType = FILESTREAM_DATATYPE_ACCEL_8G; // [1] Data type - see FileStream.h
	}
	else if (ACCEL_RANGE_FOR_RATE(accelFileStream.sampleRate) == 4)
	{
		accelFileStream.dataType = FILESTREAM_DATATYPE_ACCEL_4G; // [1] Data type - see FileStream.h
	}
	else if (ACCEL_RANGE_FOR_RATE(accelFileStream.sampleRate) == 2)
	{
		accelFileStream.dataType = FILESTREAM_DATATYPE_ACCEL_2G; // [1] Data type - see FileStream.h
	}
	else
	{
		accelFileStream.dataType = FILESTREAM_DATATYPE_ACCEL; // [1] Data type - see FileStream.h
	}

	#ifndef ACCEL_8BIT_MODE
		#if 	(ACCEL_RANGE == ACCEL_RANGE_2G)
			accelFileStream.dataConversion = -14;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#elif 	(ACCEL_RANGE == ACCEL_RANGE_4G)
			accelFileStream.dataConversion = -13;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#elif 	(ACCEL_RANGE == ACCEL_RANGE_8G)
			accelFileStream.dataConversion = -12;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#else
			accelFileStream.dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#endif
	#else
		#if 	(ACCEL_RANGE == ACCEL_RANGE_2G)
			accelFileStream.dataConversion = -6;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#elif 	(ACCEL_RANGE == ACCEL_RANGE_4G)
			accelFileStream.dataConversion = -5;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#elif 	(ACCEL_RANGE == ACCEL_RANGE_8G)
			accelFileStream.dataConversion = -4;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#else
			accelFileStream.dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
		#endif
	#endif
		#ifdef ACCEL_8BIT_MODE
			accelFileStream.channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT8);
		#else
			accelFileStream.channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
		#endif
	FileStreamInit(&accelFileStream, scratchBuffer);
	DataStreamInit(&accelDataStream, sizeof(accel_t), (sizeof(accelBuffer)/sizeof(accel_t)), accelBuffer);
	DataStreamClear(&accelDataStream);
	#endif

	// Gyro
	#if (MODE_GYRO == MODE_LOG)
    gyroFileStream.fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
	gyroFileStream.streamId = 'g';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	gyroFileStream.sampleRate = 100;          			// [2] Sample rate (Hz)
	gyroFileStream.sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	gyroFileStream.dataType = FILESTREAM_DATATYPE_GYRO; // [1] Data type - see FileStream.h
	gyroFileStream.dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	gyroFileStream.channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
	FileStreamInit(&gyroFileStream, scratchBuffer);
	DataStreamInit(&gyroDataStream, sizeof(gyro_t), (sizeof(gyroBuffer)/sizeof(gyro_t)), gyroBuffer);
	DataStreamClear(&gyroDataStream);
	#endif

	// ADC
	#if (MODE_ADC == MODE_LOG)
    adcFileStream.fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
	adcFileStream.streamId = 'l';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	adcFileStream.sampleRate = 1;          			// [2] Sample rate (Hz)
	adcFileStream.sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	adcFileStream.dataType = FILESTREAM_DATATYPE_BATTERY_LIGHT_TEMP; // [1] Data type - see FileStream.h
	adcFileStream.dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	#ifdef OFFSET_LOGGING
		adcFileStream.channelPacking = (FILESTREAM_PACKING_9_CHANNEL|FILESTREAM_PACKING_UINT16);      			// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
	#else
		adcFileStream.channelPacking = (FILESTREAM_PACKING_5_CHANNEL|FILESTREAM_PACKING_UINT16);      			// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
	#endif
	FileStreamInit(&adcFileStream, scratchBuffer);
	DataStreamInit(&adcDataStream, sizeof(adc_results_t), (sizeof(adcBuffer)/sizeof(adc_results_t)), adcBuffer);
	DataStreamClear(&adcDataStream);
	#endif

	return 1; // Done
}

void LoggerStart(void)
{
	// Clear data ready flags
	gDataReady.all = FALSE;

	// Start the streams
	DATA_STREAM_SW_INT_IP = DATA_STREAM_INTERRUPT_PRIORITY;
	DATA_STREAM_SW_INT_IF = 0;
	DATA_STREAM_SW_INT_IE = 0; // MUST block i2c reads whilst setting up sensors!!!

	// Enable desired streams
	gStreamEnable.accel = (MODE_ACCEL == MODE_LOG)? TRUE:FALSE;
	gStreamEnable.gyro 	= (MODE_GYRO == MODE_LOG)? TRUE:FALSE;
	gStreamEnable.adc 	= (MODE_ADC == MODE_LOG)? TRUE:FALSE;
	gStreamEnable.accel_int2 = TRUE; // Allows orientation, taps, activity etc

    // Check if we have an accelerometer
	#if (MODE_ACCEL > MODE_OFF)
    	AccelVerifyDeviceId();
	#endif 
    // Check if we have a gyro
	#if (MODE_GYRO > MODE_OFF)
    	GyroVerifyDeviceId();
	#endif 
	// Check if we have an proximity sensor + initialise it
#ifdef HAS_PROX
	#if (MODE_PROX > MODE_OFF)
		ProxVerifyDeviceId();
	#endif
#endif

	// Turn on interrupts - NOTE: The HW is disbled in main by failing to call the verify device id functions
	RtcInterruptOn(0); // 1Hz streams, ADC, Altimeter 
	AccelStartup(settings.sampleRate);	// ACCEL_RANGE_8G|ACCEL_RATE_100);
#ifndef ACCEL_SWAP_INTERRUPTS
	// Standard interrupt mapping
	AccelEnableInterrupts(	ACCEL_INT_SOURCE_TRANSIENT | ACCEL_INT_SOURCE_WATERMARK, ACCEL_INT_SOURCE_WATERMARK ); // Last field diverts source to int1  // ACCEL_INT_SOURCE_TRANSIENT | ACCEL_INT_SOURCE_ORIENTATION | ACCEL_INT_SOURCE_TAP | 
#else
	// Swap interrupts over (transients are ignored in this mode for now)
	#warning "This is a swapped interrupt build."
	AccelEnableInterrupts(	ACCEL_INT_SOURCE_TRANSIENT | ACCEL_INT_SOURCE_WATERMARK, ACCEL_INT_SOURCE_TRANSIENT ); // Last field diverts source to int1  // ACCEL_INT_SOURCE_TRANSIENT | ACCEL_INT_SOURCE_ORIENTATION | ACCEL_INT_SOURCE_TAP | 
#endif	
	
#if MODE_GYRO != MODE_OFF
	GyroStartupFifoInterrupts();
#endif

	IC1CON1 = 7;
	DATA_STREAM_SW_INT_IF = 0; // Set flag, also doesn't matter if it vectors if no data is ready
	DATA_STREAM_SW_INT_IE = 1; // Now we can allow it to vector
	// The fifos will start filling up now... they will need emptying 
}


// Check if we're ready to write
// -- this makes things a lot cleaner if we can query whether we want to write before writing (e.g. for setting PLL, debug flashing, etc.)
char LoggerReadyToWrite(void)
{
	// This shouldn't happen - if it has, return non-zero so that the caller will call LoggerPollAndWrite() and notice the error value from that
	if (logFile == NULL) { return -1; }
	
	#if (MODE_ACCEL == MODE_LOG)
		if (FifoLength(&accelDataStream.fifo) >= WAIT_ACCEL_SECTORS * ACCEL_SECTOR_SAMPLE_COUNT) { return 1; }
	#endif
	#if (MODE_GYRO == MODE_LOG)
		if (FifoLength(&gyroDataStream.fifo) >= WAIT_GYRO_SECTORS * GYRO_SECTOR_SAMPLE_COUNT) { return 1; }
	#endif
	#if (MODE_ADC == MODE_LOG)
		if (FifoLength(&adcDataStream.fifo) >= WAIT_ADC_SECTORS * ADC_SECTOR_SAMPLE_COUNT) { return 1; }
	#endif	
	return 0;
}


// Check each stream for data and write it out to the log file
// 0 = no write, 1 = data written, -1 = write error
char LoggerPollAndWrite(dataStreamFlags_t forceWrite)
{
	unsigned int retval = 0;
	static unsigned long timestamp;			// Time,factional and length must be set by the internal read functions
	static unsigned short timeFractional;	// Fractional part of time
	static unsigned int fifoLength; 		// Current time stamp is for the value at the end of the fifo
	unsigned int writeLength;
	unsigned int popped;
	static void* temp;
	// Now check if any of the data streams has enough data to make up a sector

	if (logFile == NULL)
	{
		return -1;
    }

	// Accelerometer
	#if (MODE_ACCEL == MODE_LOG)
		DataStreamTimestamp(&accelDataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
		if (fifoLength >= ACCEL_SECTOR_SAMPLE_COUNT || (forceWrite.accel))
		{
			while((fifoLength >= ACCEL_SECTOR_SAMPLE_COUNT) || (forceWrite.accel))// Copy out all available full sectors
			{
				retval = 1;
				writeLength = fifoLength;					// Change number to write for next sector
				if (writeLength > ACCEL_SECTOR_SAMPLE_COUNT) 
					{writeLength = ACCEL_SECTOR_SAMPLE_COUNT;}// set write length for full sector
				if (writeLength == 0) break;
		
				temp = FileStreamPrepareData(&accelFileStream, timestamp, timeFractional, fifoLength, writeLength);
				popped = FifoPop(&accelDataStream.fifo, temp, writeLength);
		
				if (popped != writeLength) 							{retval = -1; goto end;} /*Fifo pop FAIL*/
				if(!FileStreamOutputData(&accelFileStream, 1 , 1))	{retval = -1; goto end;} /*FS write FAIL*/
		
				fifoLength-=writeLength;
				// Subtract from fifo length and loops back
			}
		}
	#endif

	// Gyro
	#if (MODE_GYRO == MODE_LOG)
		DataStreamTimestamp(&gyroDataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
		if ((fifoLength >= GYRO_SECTOR_SAMPLE_COUNT) || (forceWrite.gyro))// Copy out all available full sectors
		{
			while((fifoLength >= GYRO_SECTOR_SAMPLE_COUNT) || (forceWrite.gyro))// Copy out all available full sectors
			{
				retval = 1;
				writeLength = fifoLength;					// Change number to write for next sector
				if (writeLength > GYRO_SECTOR_SAMPLE_COUNT) 
					{writeLength = GYRO_SECTOR_SAMPLE_COUNT;}// set write length for full sector
				if (writeLength == 0) break;
		
				temp = FileStreamPrepareData(&gyroFileStream, timestamp, timeFractional, fifoLength, writeLength);
				popped = FifoPop(&gyroDataStream.fifo, temp, writeLength);
		
				if (popped != writeLength) 							{retval = -1; goto end;} /*Fifo pop FAIL*/
				if(!FileStreamOutputData(&gyroFileStream, 1 , 1))	{retval = -1; goto end;} /*FS write FAIL*/
		
				fifoLength-=writeLength;
				// Subtract from fifo length and loops back
			}
		}
	#endif

	// ADC, Batt,Light,Temp (6 bytes)
	#if (MODE_ADC == MODE_LOG)
		DataStreamTimestamp(&adcDataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
		if((fifoLength >= ADC_SECTOR_SAMPLE_COUNT) || (forceWrite.adc))
		{
			while((fifoLength >= ADC_SECTOR_SAMPLE_COUNT) || (forceWrite.adc))// Copy out all available full sectors
			{
				retval = 1;
				writeLength = fifoLength;					// Change number to write for next sector
				if (writeLength > ADC_SECTOR_SAMPLE_COUNT) 
					{writeLength = ADC_SECTOR_SAMPLE_COUNT;}// set write length for full sector
				if (writeLength == 0) break;
		
				temp = FileStreamPrepareData(&adcFileStream, timestamp, timeFractional, fifoLength, writeLength);
				popped = FifoPop(&adcDataStream.fifo, temp, writeLength);
		
				if (popped != writeLength) 							{retval = -1; goto end;} /*Fifo pop FAIL*/
				if(!FileStreamOutputData(&adcFileStream, 1 , 1))	{retval = -1; goto end;} /*FS write FAIL*/
		
				fifoLength-=writeLength;
				// Subtract from fifo length and loops back
			}
		}
	#endif
	
end:
	return retval;
	// Thats it..
}

void LoggerStop(void)
{
	// Stop all the interrupts
	DATA_STREAM_SW_INT_IE = 0;
	ACCEL_INT1_IE = 0;
	ACCEL_INT2_IE = 0;
#if MODE_GYRO != MODE_OFF
	GYRO_INT2_IE = 0;
#endif
	gStreamEnable.all = 0;

	// Turn off the peripherals
	AccelStandby();
#if MODE_GYRO != MODE_OFF
	GyroStandby();
#endif

	// Finalise the file streams
	if (logFile != NULL)
	{   // Close the file
		// Force write remaining data in buffers
		LoggerPollAndWrite((dataStreamFlags_t)((unsigned char)0xFF)); // all streams forced		
        FSfclose(logFile);
        logFile = NULL;
    }
    
#ifdef WRITE_LOG
	WriteLog("Stop logging. ");
#endif

	//FtlShutdown();
	//RtcInterruptOn(0);

	return;
}


void LowPowerWaitOnButton(void);

#define DEBUG_LED_SET(_c) { if (settings.debuggingInfo >= 3 || (settings.debuggingInfo >= 1 && status.debugFlashCount)) { if (status.debugFlashCount) { status.debugFlashCount--; } LED_SET((_c)); } }

// Flush file interval
//#define FLUSH_INTERVAL (5*60)		// Could this be unsafe?


// Stop logging conditions (values seen in log entries)
typedef enum
{ 
	STOP_NONE                   = 0,    // (not seen)
	STOP_INTERVAL               = 1,    // Blue 0 ..
	STOP_SAMPLE_LIMIT           = 2,    // Blue 1 -.-.
	STOP_DISK_FULL              = 2,    // Blue 1 -.-. (same as sample limit)
	NOT_STARTED_NO_INTERVAL     = 3,    // Blue 2 --.--.
	NOT_STARTED_AFTER_INTERVAL  = 4,    // Blue 3 ---.---.
	NOT_STARTED_DISK_FULL       = 5,    // Blue 4 ----.----.
	NOT_STARTED_WAIT_USB        = 6,    // (not seen)
	STOP_USB                    = 7,    // (not seen)
	NOT_STARTED_INITIAL_BATTERY = 8,    // Red 0 ..
	NOT_STARTED_WAIT_BATTERY    = 9,    // Red 1 -.-.
	STOP_BATTERY                = 10,   // Red 2 --.--.
	NOT_STARTED_FILE_OPEN       = 11,   // Red 3 ---.---.
	STOP_LOGGING_WRITE_ERR      = 12,   // Red 4 ----.----. (not seen, restarted)
	STOP_LOGGING_SAMPLE_ERR     = 13,   // Red 5 -----.-----. (not seen, restarted)
} StopCondition;

// Log entry strings
const char *stopConditionString[] =
{
	"STOP_NONE",                   // 0
	"STOP_INTERVAL",               // 1
	"STOP_SAMPLE_LIMIT",           // 2 (or STOP_DISK_FULL)
	"NOT_STARTED_NO_INTERVAL",     // 3
	"NOT_STARTED_AFTER_INTERVAL",  // 4
	"NOT_STARTED_SAMPLE_LIMIT",    // 5 (or NOT_STARTED_DISK_FULL)
	"NOT_STARTED_WAIT_USB",        // 6
	"STOP_USB",                    // 7
	"NOT_STARTED_INITIAL_BATTERY", // 8
	"NOT_STARTED_WAIT_BATTERY",    // 9
	"STOP_BATTERY",                // 10
	"NOT_STARTED_FILE_OPEN",       // 11
	"STOP_LOGGING_WRITE_ERR",      // 12
	"STOP_LOGGING_SAMPLE_ERR",     // 13
};

// Flash codes
const char stopFlashCode[] =
{
//     0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13
//   off,  B:0,  B:1,  B:2,  B:3,  B:4,  B:5,  B:6,  R:0,  R:1,  R:2,  R:3,  R:4,  R:5
    0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45
};

static char stopCondition = STOP_NONE;


void RunLogging(void)
{
	#ifdef FLUSH_INTERVAL
	DateTime lastFlush = 0;
	#endif
	static char audioSampling, sensorSampling;
	static unsigned char batFailCount = 0;	// Counts battery glitches with stop condition
	char oscFast;								// Track the PLL state

	CLOCK_PLL();					// Oscillator

#ifndef NO_DISPLAY
	DisplayRestore();
	DisplayClear();
	Display_print_xy("<LOGGER>",0,2,2);
	Display_print_xy("Running...",0,6,1);
	DelayMs(500);					// Allow user to see message on screen
	DisplayOff();
#endif
	SD_ENABLE(); 					// Ensure SD card is turned on

	// Initialize audio recording state
	audioSampling = 0;
	CodecInitOff();					// Codec off
	AudioStreamInit();				// Audio recording state cleared
	
	// Turn off unused peripherals
	NVMCONbits.NVMSIDL = 1; 	// Flash voltage regulator goes into Stand-by mode during Idle mode +30us resume
	PMD1 = 0b1110011001110110; 	// Leave on T1,T2,DCI,I2C1,SPI1,AD1
	PMD2 = 0b1111111011111110; 	// Leave on IC1,OC1
	PMD3 = 0b1111010010101011; 	// Leave on RTCC,PMP
	PMD4 = 0b0000000000101001; 	// Leave on USB
	PMD5 = 0b1111111011111111;	// Leave on IC9 (DATA_STREAM_SW_INTERRUPT_HANDLER)
	PMD6 = 0b0111111100000011;
	PMD7 = 0b0000000011100000; 	// Leave on DMA0

	// Initialize sensor logging state
	sensorSampling = 0;
	gDataReady.all = 0;
	gStreamEnable.all = 0;
	LoggerInit(NULL);				// Sensor logging state cleared
//	gStreamEnable.accel_int2 = TRUE;// [TODO: Need to enable accelerometer & interrupts here if this is needed] Always track orientation, taps, activity etc.

	// Initialize other state
	status.debugFlashCount = 20;	// Show debugging flash at startup
	
	// If we're set to record any data, and the file system initialized OK
	stopCondition = STOP_NONE;
	if (settings.loggingStartTime >= settings.loggingEndTime) { stopCondition = NOT_STARTED_NO_INTERVAL; }
	if (settings.recordData == RECORD_NONE) { stopCondition = NOT_STARTED_NO_INTERVAL; }
	if (RtcNow() > settings.loggingEndTime) { stopCondition = NOT_STARTED_AFTER_INTERVAL; }			
	if (!FSInit()) { stopCondition = NOT_STARTED_FILE_OPEN; }
	else
	{
#ifdef WRITE_LOG
		WriteLog("Startup. ");
#endif
		;
	}

	OSCSWITCH_NORMAL();				// Start with a standard oscillator for main loop
	oscFast = 0;

	// === MAIN LOGGING LOOP ===
	while (stopCondition == STOP_NONE)					// While not stopped for any reason
	{			
		DateTime dateTime = RtcNow();
	
		// TODO : This should not be called unless we are successfully logging/waiting to log
		if(!USB_BUS_SENSE)RtcSwwdtReset();		

		// === SENSOR LOGGING START/STOP: Start or stop logging if appropriate ===
		{
			// Check if we are within the interval and we would like to record sensor data?
			char inSensorSamplingInterval = (dateTime >= settings.loggingStartTime && dateTime < settings.loggingEndTime && (settings.recordData == RECORD_ACCEL || settings.recordData == RECORD_BOTH));
			if (sensorSampling == 0 && inSensorSamplingInterval)
			{
				// We're not sampling sensors at the moment, but are now inside the interval -> start recording sensors
				LoggerInit(DEFAULT_FILE);				// Open default log file
				LoggerStart();							// Start logging
				sensorSampling = 1;						// Update status
			} 
			else if (sensorSampling == 1 && !inSensorSamplingInterval)
			{
				// We are recording sensors but are now outside the interval -> stop recording sensors
				LoggerStop();							// Cleanly stop logging and close file
//					gStreamEnable.accel_int2 = TRUE;		// [TODO: Need to enable accelerometer & interrupts here if this is needed] Always track orientation, taps, activity etc.
				sensorSampling = 0;						// Update status
			}
			if (dateTime > settings.loggingEndTime) { stopCondition = STOP_INTERVAL; }			
		}	

		// === NOT RECORDING AUDIO: Sleep, see if we want to start recording ===
		if (!audioSampling)
		{
			// Sleep until interrupt
			LED_SET(LED_OFF);							// Debug LED off
			Sleep();
			DEBUG_LED_SET(LED_YELLOW);					// DEBUG: Yellow = not recording audio, woken from sleep
			
			// If we would like to record audio...
			if (settings.recordData == RECORD_SOUND || settings.recordData == RECORD_BOTH)
			{
				// Audio stream recording tasks will checks the interval and hour mask and opens a new file if needed (unless it has recently failed to open a file)
				if (AudioStreamRecordingTasks(0) >= 0)
				{
					// We've opened a new file: enable the codec and start the sampling
					CodecInit(NULL);						// Enable audio codec
					AudioSamplingInit();				// Start sampling
					audioSampling = 1;					// Set status
					
#ifdef WRITE_LOG
					{
						static char line[80];
						char *p;
						p = line;
						p += sprintf(p, "Start audio. ");
#ifdef CODEC_DEBUG
						p += sprintf(p, "[CODEC: %02x:%02x:%02x:%02x:%02x / %02x:%02x:%02x:%02x:%02x.]", gCodecDebugWrite[1], gCodecDebugWrite[2], gCodecDebugWrite[3], gCodecDebugWrite[4], gCodecDebugWrite[5], gCodecDebugRead[1] , gCodecDebugRead[2] , gCodecDebugRead[3] , gCodecDebugRead[4] , gCodecDebugRead[5]);
#endif    
						WriteLog(line);
					}
#endif
				}
			}
		}
		
		
		// === RECORDING AUDIO: Low power wait and write to file ===
		if (audioSampling)
		{
			unsigned int length;
			
			// Low power routine to stall the processor for 32ms whilst it fills 1 or more sectors
			LowPowerSamplingRoutine();					// ? Skip this if debugging? (#ifndef __DEBUG)
		
			// Check whether we have enough audio data to write
			length = AudioStreamRecordingTaskLength();	// How many sectors are full?
			if (length >= MIN_SECTORS_TO_WRITE)			// If there are enough to write to file
			{
				int audioStreamRecordingTasksRetval;
				char resetCodec = 0;

				DEBUG_LED_SET(LED_GREEN);				// DEBUG: Green = writing audio sectors
				
				// Turn PLL on (SD card access should be very fast)
				oscFast = 1; OSCSWITCH_FAST();
				
#ifdef CHECK_AUDIO
				// If we're checking for non-zero audio after initialization...
				if (AudioStreamNumEmptySectors() > 160)		// 160 sectors is 10 seconds at 8kHz
				{
					resetCodec = 1;		// Flag for codec reset (reset calls AudioStreamCodecHasBeenReset() which starts the count again)
				}
#endif
				
				// Write to the data to file
				audioStreamRecordingTasksRetval = AudioStreamRecordingTasks(MIN_SECTORS_TO_WRITE);
				if (audioStreamRecordingTasksRetval < 0)			// File closed or re-opened
				{
					if (audioStreamRecordingTasksRetval == -2)	
					{
						// A new file was opened
						audioSampling = 1;					// Set status	
						resetCodec = 1;						// Flag to reset the codec
					}
					else										
					{
						// File closed (the write failed, or we're now in a non-recording period) -> stop sampling and stop the codec
						AudioSamplingStop(); 		// Stop sampling
						CodecInitOff();			// Turn off audio codec
						audioSampling = 0;		// Set sampling status
						resetCodec = 0;			// Cancel any pending reset
#ifdef WRITE_LOG
						WriteLog("Stop audio. ");
#endif
					}
				}
				
				// Reset the codec on each new file (and if empty data found at start of file)
				if (resetCodec)
				{
					Dma0Pause();						// Stop sampling
					CodecInit(NULL);					// Enable audio codec
					AudioStreamCodecHasBeenReset();
					Dma0Resume();
				}
				
				LED_SET(LED_OFF);						// Debug LED off
			}
		}
		

		// === LOGGING SENSORS: If we're logging sensor data and now is a good choice for writing it ===
		// ...do this if the PLL is on (we've just written audio sectors), or if we're not audio sampling
		if (sensorSampling == 1 && (oscFast || !audioSampling))
		{
			// Check whether we have enough data to write
			if (LoggerReadyToWrite())
			{
				DEBUG_LED_SET(LED_CYAN);				// DEBUG: Cyan = writing sensor value sectors
				
				// Turn PLL on if not already on (SD card access should be very fast)
				if (!oscFast) { oscFast = 1; OSCSWITCH_FAST(); }
				
				// Write to the file
				if (LoggerPollAndWrite((dataStreamFlags_t)((unsigned char)0)) < 0)
				{ 
					sensorSampling = -1;		 		// Failed (e.g. write error), file was closed, set an invalid sensorSampling flag
//						gStreamEnable.accel_int2 = TRUE;	// [TODO: Need to enable accelerometer & interrupts here if this is needed] Always track orientation, taps, activity etc.
				}
				
				LED_SET(LED_OFF);						// Debug LED off
			}
		}

// Missed interrupt handling 
#if 1
// If there is no data ready for the device but the pin is high then an interrupt has been missed.
asm("#DISI 0x3FFF");
if(!gDataReady.accel && ACCEL_INT_WATERMARK)	{ACCEL_INT_WATERMARK_IF = 1;}	// This will vector immediately
#if (MODE_GYRO == MODE_LOG)
if(!gDataReady.gyro && GYRO_INT2)	{GYRO_INT2_IF = 1; 	}	// This will vector immediately
#endif
//if(!gDataReady.accel_int2 && ACCEL_INT2)	{ACCEL_INT2_IF = 1;}	// This will vector immediately
asm("#DISI 0x0000");
#endif

		// If the PLL has been used, turn it off now
		if (oscFast) { OSCSWITCH_NORMAL(); oscFast = 0; }

		// If button clicked, flash for a while (proximity updated in RTC interrupt)
		if (gButtonClick) { status.debugFlashCount = 10; }
		
		// STOP: if battery too low (ADC updated in RTC interrupt)
		if (adcResult.batt < BATT_CHARGE_MIN_LOGGING) 
		{
			UpdateAdc();
			if(adcResult.batt < BATT_CHARGE_MIN_LOGGING) 
			{
				if (batFailCount++ > 5)	stopCondition = STOP_BATTERY;
			}
			else batFailCount = 0;
		}
		else batFailCount = 0;
	
		// STOP: if USB connected
		if (USB_BUS_SENSE) { stopCondition = STOP_USB; }
		
		// Flush files periodically
		#ifdef FLUSH_INTERVAL
		if (dateTime - lastFlush > FLUSH_INTERVAL)
		{
			lastFlush = dateTime;
			if (logFile != NULL) { FSfflush(logFile); }
			if (audioStreamRecorder.fp != NULL) { FSfflush(audioStreamRecorder.fp); }
		}	
		#endif
		

	} // Back to the top of the logging loop
	
	// Logging over
	OSCSWITCH_NORMAL();
	if (sensorSampling == 1) { LoggerStop(); sensorSampling = 0; }
	if (audioSampling) { AudioStreamStopRecording(); audioSampling = 0; }
	//Display_print_xy("Stopped.",0,7,1);

	LED_SET(LED_OFF);
	
	//LoggerStop();	// already stopped
#ifndef NO_DISPLAY
	DisplayRestore();
#endif
	CodecInitOff();
	SD_DISABLE();
	DelayMs(1000);
	DisplayOff();
	UpdateProximity(1); // Clears previous value
	LowPowerWaitOnButton();		
	return;
}

void LowPowerWaitOnButton(void)
{
    unsigned char led = (stopFlashCode[(int)stopCondition] >> 4);
    unsigned char countReset = stopFlashCode[(int)stopCondition] & 0x0f;
    unsigned char countdown = countReset;

	//gButton = 0;
	RtcInterruptOn(0); 	// RTC + Timer1
	gButtonClick = 0;
	LED_SET(LED_OFF);
	while ((!gButtonClick) && (!USB_BUS_SENSE)) // For now: stop here, logs on button press, rtc 1hz wake
	{
		Sleep();
        if (led)
        {
            LED_SET(led);
            if (countdown > 0)
            {
            	Delay10us(15);
            	countdown--;
            }
            else
            {
                Delay10us(3);
                countdown = countReset;
            }
            LED_SET(LED_OFF);
        }
		if (!USB_BUS_SENSE) { RtcSwwdtReset(); }
	}
	//gButton = 0;
}

// DMA interrupt 
void __attribute__((__interrupt__, auto_psv)) _DMA0Interrupt(void)
{
	// Clear flag
	IFS0bits.DMA0IF = 0;

	if (DMAPPSbits.PPST0)	// Just written buffer A (now writing buffer B)
	{
		FifoSetTail(&audioStreamRecorder.fifo, NUM_AUDIO_SECTORS_PER_DMA_INT); 
	}
	else					// Just written buffer B (now writing buffer A)
	{
		FifoSetTail(&audioStreamRecorder.fifo, 0); 
	}
	
}

void LowPowerSamplingRoutine(void)
{
//	#ifndef __DEBUG
	// The dsPIC uses the DMA for sampling and can idle
	// Interrupts for the other peripherals can wake the device from idle as well
	INTCON2bits.GIE = 0;		// [dgj] Mask off interrupts so they won't run while the clock is slow
	OSCSWITCH_IDLE();
	Idle();
	OSCSWITCH_NORMAL();
	INTCON2bits.GIE = 1;		// [dgj] Interrupt now the the clock is faster
//	#endif
}
void AudioSamplingInit(void)
{
	// Initialise the DMA to use the audio buffer
	AssignDma0CircularSampleBuffer(audioStreamRecorder.buffer,(AUDIO_MAX_SECTORS * AUDIO_SECTOR_SIZE/2) );
	// Begin sampling - Note: Timer 1 is altered to asynchronous so sleep will stop millisec count (can't sleep with DMA on anyway)
	
	// Set period (period=2 for 16.384kHz, period=4 for 8.192kHz) --> register value 1 or 3
    if (settings.audioRate == 1) // 1=16kHz
    {
		SetupOC1toTriggerSamplingDMA(2); // Set period to 2 -> 16.384kHz
	} 
	else
	{
		SetupOC1toTriggerSamplingDMA(4); // Set period to 4 -> 8.192kHz
	}	
	
	// The next thing to happen is the DMA interrupt after NUM_AUDIO_SECTORS_PER_DMA_INT number of sectors 
}
void AudioSamplingStop(void)
{
	// Turn channel on
	IEC0bits.DMA0IE = 0; 	/* Enable DMA interrupt */
	IFS0bits.DMA0IF = 0; 	/* Clear DMA interrupt */
	DMA0CONbits.CHEN = 0; 	/* Channel off*/
	OC1CON1 = 0;	
}


#if (defined(__PIC24F__) || defined (__C32__)) 
#error "These devices are no longer supported - revert SVN to pre 08-01-2013 to regain functionality"
#endif
//EOF
