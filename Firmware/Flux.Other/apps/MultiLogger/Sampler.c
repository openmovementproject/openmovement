/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// Unified Logger
// Dan Jackson, Karim Ladha

// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "TimeDelay.h"
#include "Peripherals/Rtc.h"

#include "Analog.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Mag.h"
#include "Peripherals/bmp085.h"
#include "Peripherals/Prox.h"

#include "Utils/Fifo.h"
#include "Utils/DataStream.h"

#include "Apps/MultiLogger/Sampler.h"
#include "Apps/MultiLogger/Settings.h"
#include "Utils/Util.h"
#include "Utils/FSUtils.h"
#include "Apps/MultiLogger/Recorder.h"
//#include "Utils/FileStream.h"
//#include "MDD File System/FSIO.h"

// Sampler State
sampler_t sampler;
volatile char gSecondElapsed = 1;


void SamplerInit()
{
    // Blank whole state
    memset(&sampler, 0, sizeof(sampler));

    // Setup peripherals to defaults
    {
        dataStreamFlags_t newFlags = {{0}};
        newFlags.all = 0;
        newFlags.accel_int2 = TRUE; // Allows orientation, taps, activity etc
#ifdef USE_PROXIMITY_SENSOR_1HZ
        newFlags.prox = TRUE; 		// Updates prox sensor at 1Hz
#endif
        SetupSampling(&newFlags);
    }
}


// Data stream software interrupt
#define DATA_STREAM_SW_INT_IF IFS0bits.IC1IF
#define DATA_STREAM_SW_INT_IE IEC0bits.IC1IE
#define DATA_STREAM_SW_INT_IP IPC0bits.IC1IP
#define DATA_STREAM_SW_INTERRUPT_HANDLER _IC1Interrupt


// Sampler buffer
typedef struct
{
	alt_t         altBuffer[SAMPLE_BUFFER_SIZE_ALT];
	adc_results_t adcBuffer[SAMPLE_BUFFER_SIZE_ADC];
	
	// Standard or 9-axis sampling buffer
	union
	{
		// Standard sampling
		struct
		{
			accel_t       accelBuffer[SAMPLE_BUFFER_SIZE_ACCEL];
			gyro_t        gyroBuffer [SAMPLE_BUFFER_SIZE_GYRO];
			mag_t         magBuffer  [SAMPLE_BUFFER_SIZE_MAG];
		} standardSampling;
	
#ifdef SYNCHRONOUS_SAMPLING
		// 9-axis sampling
		all_axis_t    allAxisBuffer[SAMPLE_BUFFER_SIZE_ALLAXIS];				// Manually defined size
		#define ALL_AXIS_SAMPLE_COUNT SAMPLE_BUFFER_SIZE_ALLAXIS				// Manually defined size		
		//all_axis_t    allAxisBuffer[0];										// "Flexible array member" (buffer size is the remainder of the space in this union)
		//#define ALL_AXIS_SAMPLE_COUNT ((sizeof(sampleBuffer_t) - ((size_t)&(((sampleBuffer_t*)0)->allAxisBuffer))) / sizeof(all_axis_t))		// Calculate the number of samples that fit in the buffer (determined by the other union members)
#endif
	};
	
} sampleBuffer_t;
static sampleBuffer_t sampleBuffer;

datastream_t altDataStream = {{0}}, adcDataStream = {{0}};
datastream_t accelDataStream = {{0}}, gyroDataStream = {{0}}, magDataStream = {{0}};
#ifdef SYNCHRONOUS_SAMPLING
datastream_t allAxisDataStream = {{0}};
#endif

//alt_t gPressure;					// Updated in logging loop

volatile dataStreamFlags_t gDataReady = {{0}}; 
volatile dataStreamFlags_t gStreamEnable = {{0}};


// Interrupts for getting fifo time stamps at RTC_INT_PRIORITY or below
#ifdef SYNCHRONOUS_SAMPLING
#define TIMER_RATE 32768UL
static unsigned short gTimerAccumulator = 0;
static unsigned short gTimerRemainder = 0;
#endif

void LoggerTimerTasks(void) 
{
	// Unused
	// The timer1 interrupt is at RTC_INT_PRIORITY, it may interrupt another DataStreamUpdateTimestamp() - check its safe!
	;
#ifdef SYNCHRONOUS_SAMPLING
	if (gStreamEnable.allAxis)
	{
		// Post-scale to required frequency by ignoring some samples (could improve on this to reduce jitter if needed)
		if (gTimerAccumulator > TIMER_RATE)
		{
			gTimerAccumulator -= TIMER_RATE;
		}
		else
		{
			gDataReady.allAxis = TRUE;		// set the data ready flag
		    DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks
		}
		
		// Add timer accumulator for exact rate
		gTimerAccumulator += gTimerRemainder;
	}
#endif
}

// Called at 1Hz from rtc int
void LoggerRTCTasks(void)
{
    gSecondElapsed = 1;	// Set flag for task function below
}

// Accelerometer fifo interrupt
void __attribute__((interrupt,auto_psv)) _INT1Interrupt(void)
{
	IFS1bits.INT1IF = 0; 
	// This is only for the fifo int
#ifdef SYNCHRONOUS_SAMPLING
if (gStreamEnable.allAxis) return;
#endif
    if (!gDataReady.accel) 			// Data not already ready
    {
        gDataReady.accel = TRUE;	// Indicate data ready
        DataStreamUpdateTimestamp(&accelDataStream); // Not actually valid until read in sw triggered int
    }
    DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks
}

// Gyro fifo interrupt
void __attribute__((interrupt,auto_psv)) _INT3Interrupt(void)
{
	// This is only for the fifo int
IFS3bits.INT3IF = 0; 
#ifdef SYNCHRONOUS_SAMPLING
if (gStreamEnable.allAxis) return;
#endif
    if (!gDataReady.gyro) 	// Data not already ready
    {
        gDataReady.gyro = TRUE;	// Indicate data ready
        DataStreamUpdateTimestamp(&gyroDataStream); // Not actually valid until read in sw triggered int
    }
    DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks
}

// Other accelerometer interrupt
void __attribute__((interrupt,auto_psv)) _INT2Interrupt(void)
{
	// This is for other accel int2 ints 
	// KL: new behaviour, int disabled itself but triggers SW int, re-armed on rtc tick
	ACCEL_INT2_IE = 0;				// Disable interrupt, won't refire
	ACCEL_INT2_IF = 0;				// Prevent the vector re-firing if it has not been read
	gDataReady.accel_int2 = TRUE;	// Indicate it fired
	DATA_STREAM_SW_INT_IF = 1; 		// Trigger SW int
}

// Magnetometer interrupt
void __attribute__((interrupt,auto_psv))  MAG_INT_VECTOR(void)
{
	 MAG_INT_IF = 0;
#ifdef SYNCHRONOUS_SAMPLING
if (gStreamEnable.allAxis) return;
#endif
	if(MAG_INT) 
	{
		if (gStreamEnable.mag > 0)
		{
			// Magnetometer
			if (!gDataReady.mag)
			{
				gDataReady.mag = TRUE;	// set the data ready flag
				DataStreamUpdateTimestamp(&magDataStream);
			}
			DATA_STREAM_SW_INT_IF = 1; 	// SW trigger fifo int tasks 
		}
	}	
}

/* 	The main data reading ISR, all I2C comms outside this need to disable it first!
	This is the interrupts for all device->fifo comms (i2c) at GLOBAL_I2C_PRIORITY 
	1) Polls each dataReadyFlag
	2) Grabs data and writes it to the fifo					*/
void __attribute__((interrupt,auto_psv)) DATA_STREAM_SW_INTERRUPT_HANDLER(void)
{
	dataStreamFlags_t dataReady;    // Local copy

	// Find out which devices have data - need to protect this read from interrupts
	asm("#DISI 0x3FFF");
	DATA_STREAM_SW_INT_IF = 0;      // Clear the flag in the DISI
	dataReady = gDataReady;         // Grab a copy of the global var
	gDataReady.all = 0;             // Clear the global var

	// KL - suspect the DISI may be blocking the interrupt edge latch
	// Adding following code to protect for this. If there is no data ready for the device but the pin is high then an interrupt has been missed.
	if (!dataReady.accel && ACCEL_INT1)	{ ACCEL_INT1_IF = 1; }
	if (!dataReady.gyro  && GYRO_INT2)	{ GYRO_INT2_IF = 1;  }
	if (!dataReady.mag   && MAG_INT)	{ MAG_INT_IF = 1; 	 }
    if (!dataReady.accel_int2 && ACCEL_INT2) { ACCEL_INT2_IF = 1; }
	asm("#DISI 0x0000");            // Any missed interrupts will vector after this

	// Next, process all the device to software FIFOs
	// If the data source may be turned on/off you must always clear the condition so it can re-interrupt.
	// If a source may be ignored due to the gStreamEnable or initial mismatch then check the pin too.
	// e.g. For Accel interrupt 2 the device may have already interrupted and the pin high before the loop starts.
    #ifdef LOGGING_DEBUG_LOG_LED
        LOGGING_DEBUG_LOG_LED = 1;
    #endif

#ifdef SYNCHRONOUS_SAMPLING
	if (dataReady.allAxis)
	{
		all_axis_t sample;
		
		// Poll accelerometer
		if (gStreamEnable.accel) { AccelSingleSample(&sample.accel); }
		else { sample.accel.x = 0; sample.accel.y = 0; sample.accel.z = 0; }
		
		// Poll gyro
		if (gStreamEnable.gyro) { GyroSingleSample(&sample.gyro); }
		else { sample.gyro.x = 0; sample.gyro.y = 0; sample.gyro.z = 0; }
		
		// Poll magnetometer
		if (gStreamEnable.mag) { MagSingleSample(&sample.mag); }
		else { sample.mag.x = 0; sample.mag.y = 0; sample.mag.z = 0; }

        DataStreamUpdateTimestamp(&allAxisDataStream); 	// TODO: Pretty inefficient to do this all the time, change this code
		FifoPush(&allAxisDataStream.fifo, &sample, 1); 	// Push sample into the fifo
		
		// TODO: Move these updates of the globals to be lower frequency
		sampler.lastAccel = sample.accel;
		sampler.lastGyro = sample.gyro;
		sampler.lastMag = sample.mag;
	}
else
#endif
	{
		// Accelerometer data
		if (dataReady.accel)
		{
			if (!gStreamEnable.accel)
	        {
	            // We're not streaming data, dump h/w FIFO contents
	            AccelReadFIFO(NULL, ACCEL_MAX_FIFO_SAMPLES);
	        }
	        else
			{
	            // Stream is enabled - required upto 2 copy operations INCLUDING for pedometer
	            unsigned char passes;
	            // Empty hardware FIFO up to ACCEL_FIFO_WATERMARK bytes to keep time stamp valid from last interrupt
	            short remain = ACCEL_FIFO_WATERMARK;                    // Use the watermark level rather than the actual h/w FIFO length, so that the recorded timestamp is as accurate as possible
	
#ifndef DO_NOT_PREVENT_DOUBLE_FIRING
ACCEL_INT1_IE = 0;  // [prevent double-fire] Disable interrupts while reading to prevent double-fire fix
#endif

	            #ifdef LOGGING_DEBUG_ACCEL_LED
	            LOGGING_DEBUG_ACCEL_LED = 1;
	            #endif
	
	            // May need two passes if we're near the end of the software FIFO
		        for (passes = 2; passes != 0; --passes)
		        {
		            unsigned short contiguous, num;
		            void *buffer;
	
		            // See how much contiguous free space we have in the buffer
	                contiguous = FifoContiguousSpaces(&accelDataStream.fifo, &buffer);
	
	                // If we aren't able to fit *any* in, we've over-run our software buffer
		            if (contiguous == 0)
	                {
	                    // We've not kept up with the h/w FIFO
	                    AccelReadFIFO(NULL, ACCEL_MAX_FIFO_SAMPLES);    // Empty h/w FIFO to prevent another interrupt too soon
	                    break;                                          // Exit loop
	                }
		            if (contiguous > remain) contiguous = remain;       // We have more than enough space
	                num = AccelReadFIFO((accel_t *)buffer, contiguous); // Copy into s/w FIFO buffer
	                sampler.lastAccel = ((accel_t *)buffer)[0];         // Update global 'last value'
	                FifoExternallyAdded(&accelDataStream.fifo, num);	// Tell the s/w FIFO they were added
		            if (num == 0) 		{ break; }                      // Hardware reported no data
					remain -= num;                                      // Subtract number read
					if (remain <= 0) { break; }                         // See if any remain
				}

#ifndef DO_NOT_PREVENT_DOUBLE_FIRING
ACCEL_INT1_IF = ACCEL_INT1;     // [prevent double-fire] Set the flag to the pin level
ACCEL_INT1_IE = 1;              // [prevent double-fire] Re-enable interrupts while reading
#endif

	        }
		}
	
		// Gyro
	    if (dataReady.gyro)
	    {
			if (!gStreamEnable.gyro)
	        {
	            // We're not streaming data, dump h/w FIFO contents
	            GyroReadFIFO(NULL, GYRO_MAX_FIFO_SAMPLES);
	        }
	        else
			{
	            // Stream is enabled - required upto 2 copy operations INCLUDING for pedometer
	            unsigned char passes;
	            // Empty hardware FIFO up to GYRO_FIFO_WATERMARK bytes to keep time stamp valid from last interrupt
	            short remain = GYRO_FIFO_WATERMARK;                    // Use the watermark level rather than the actual h/w FIFO length, so that the recorded timestamp is as accurate as possible

#ifndef DO_NOT_PREVENT_DOUBLE_FIRING
GYRO_INT2_IE = 0;  // [prevent double-fire] Disable interrupts while reading to prevent double-fire fix
#endif
	            
	            #ifdef LOGGING_DEBUG_GYRO_LED
	            LOGGING_DEBUG_GYRO_LED = 1;
	            #endif
	
	            // May need two passes if we're near the end of the software FIFO
		        for (passes = 2; passes != 0; --passes)
		        {
		            unsigned short contiguous, num;
		            void *buffer;
	
		            // See how much contiguous free space we have in the buffer
	                contiguous = FifoContiguousSpaces(&gyroDataStream.fifo, &buffer);
	
	                // If we aren't able to fit *any* in, we've over-run our software buffer
		            if (contiguous == 0)
	                {
	                    // We've not kept up with the h/w FIFO
	                    GyroReadFIFO(NULL, GYRO_MAX_FIFO_SAMPLES);      // Empty h/w FIFO to prevent another interrupt too soon
	                    break;                                          // Exit loop
	                }
		            if (contiguous > remain) contiguous = remain;       // We have more than enough space
	                num = GyroReadFIFO((gyro_t *)buffer, contiguous);   // Copy into s/w FIFO buffer
	                sampler.lastGyro = ((gyro_t *)buffer)[0];           // Update global 'last value'
	                FifoExternallyAdded(&gyroDataStream.fifo, num);     // Tell the s/w FIFO they were added
		            if (num == 0) 		{ break; }                      // Hardware reported no data
					remain -= num;                                      // Subtract number read
					if (remain <= 0) { break; }                         // See if any remain
				}

#ifndef DO_NOT_PREVENT_DOUBLE_FIRING
GYRO_INT2_IF = GYRO_INT2;      // [prevent double-fire] Set the flag to the pin level
GYRO_INT2_IE = 1;              // [prevent double-fire] Re-enable interrupts while reading
#endif
	        }
	    }
	
		// Magnetometer
	    if (dataReady.mag)
	    {
#ifndef DO_NOT_PREVENT_DOUBLE_FIRING
MAG_INT_IE = 0;            // [prevent double-fire] Re-enable interrupts while reading
#endif
	        // Mag tasks
	        #ifdef LOGGING_DEBUG_MAG_LED
	            LOGGING_DEBUG_MAG_LED = 1;
	        #endif
	        MagSingleSample(&sampler.lastMag);  						// Sample always to re-arm the interrupt
	        if (gStreamEnable.mag)                                      // If stream is enabled
	        {
	            FifoPush(&magDataStream.fifo, &sampler.lastMag, 1); 	// Copy sample to the fifo
	        }
#ifndef DO_NOT_PREVENT_DOUBLE_FIRING
MAG_INT_IF = MAG_INT;      // [prevent double-fire] Set the flag to the pin level
MAG_INT_IE = 1;            // [prevent double-fire] Re-enable interrupts while reading
#endif
	    }
	}

	// Altimeter 
    if (dataReady.alt)
    {
        // Alt tasks
        if (gStreamEnable.alt)
        {
            #ifdef LOGGING_DEBUG_ALT_LED
                LOGGING_DEBUG_ALT_LED = 1;
            #endif
            // Updates temperature instead once every N samples -- as a consequence, each (N-1)th pressure reading is the previous value
            #ifdef USE_BAROMETER_TEMP
            	#ifndef BAROMETER_TEMP_CYCLE
            		#error "BAROMETER_TEMP_CYCLE not defined (e.g. 2 or 60)"
            	#endif
				#ifndef USE_CTRL_REG_BUSY_BIT
					#error "Define this to expose more advanced driver functionality"
				#endif
                static unsigned short sample_timer = (BAROMETER_TEMP_CYCLE - 1);

                if (sample_timer == 0)                          // (0)
                {
					// New behaviour only updates sample timer etc if the read was a success
                    if(BMP085_read_ut())						// Reads TEMPERATURE over i2c to local regs
                    {
						sampler.lastTemperature = BMP085_calc_temperature();// Converts last read vals to temperature, 0.1dC steps calibrated temperature from barometer if desired
                    	BMP085_Initiate_up_conversion();			// Starts a new PRESSURE conversion
                    	sample_timer = 1;       					// Next read will be PRESSURE
					}
                }
				#if (BAROMETER_TEMP_CYCLE > 2)
                else if (sample_timer < (BAROMETER_TEMP_CYCLE - 1)) // (1 to N-2)
                {
					if (BMP085_read_up())						// Reads PRESSURE over i2c to local regs
 					{
	                	BMP085_Initiate_up_conversion();		// Starts a new PRESSURE conversion
                    	sample_timer++;                         // Next read will be PRESSURE
					}
                }
				#endif
                else                                        	// (N-1)
                {
                    if (BMP085_read_up())					// Reads PRESSURE over i2c to local regs
					{											
						BMP085_Initiate_ut_conversion();	// Starts a new TEMPERATURE conversion
                    	sample_timer = 0;					// Next read will be TEMPERATURE
					}
                }
            #else // ifndef USE_BAROMETER_TEMP
                if (BMP085_read_up())						// Reads vals over i2c to local regs
                {
					BMP085_Initiate_up_conversion();		// Starts a new conversion
				}
            #endif
            sampler.lastPressure = BMP085_calc_pressure();			// Converts last read vals to pressure
            FifoPush(&altDataStream.fifo, &sampler.lastPressure, 1);	// Copy to fifo
        }
    }

#ifdef HAVE_PROXIMITY_SENSOR	
	// Prox sensor update
    if (dataReady.prox)
    {
		static char holdup = 0;
		if (ProxSampleReady() || holdup >= 2) // Basic unlocking if more than one second elapsed
		{
			holdup = 0;
			ProxReadSample();
			ProxStartSample();
		}
		else
		{
			holdup++;	
		}
	}
#endif

	// ADC conversion
    if (dataReady.adc)
    {

        if (gStreamEnable.adc)
        {
            adc_results_t values = adcResult;

            #ifdef LOGGING_DEBUG_ADC_LED
                LOGGING_DEBUG_ADC_LED = 1;
            #endif

            #ifdef USE_BAROMETER_TEMP
            // Update these values with non-ADC sources (e.g. barometer temperature)
            if (gStreamEnable.alt)
            {
                values.temp = sampler.lastTemperature;                  // 0.1dC steps calibrated temperature from barometer if desired
            }
            else
            #endif
            {
                values.temp = AdcTempToTenthDegreeC(adcResult.temp);    // 0.1dC steps too, to be compatible with values when barometer enabled
            }

			#ifdef  USE_PROXIMITY_SENSOR_1HZ
				values.light = prox.light;
			#endif
			
            // Push into fifo
            FifoPush(&adcDataStream.fifo, &values, 1);
        }
    }

	// Accel activity interrupts 
	if ((dataReady.accel_int2) || (ACCEL_INT2))
	{
        // Reset activity detection timer
        status.inactivity = 0;

        // (Always read source to clear interrupt pin)
		//if (gStreamEnable.accel_int2)
		{ 
			// Accel int2 indicates other accel event triggers
			unsigned char reason = AccelReadIntSource(); // Clears pin

            // Transient
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

			// Clear the flag, but not enabled, re-armed on rtc tick
			ACCEL_INT2_IF = ACCEL_INT2;
			//ACCEL_INT2_IE = 1;
		}
	}

	// All data sources are processed, this interrupt will re-enter if new sources have become available
}



void SetupSampling(dataStreamFlags_t *newFlags)
{
    // Configure I2C reading interrupt
    DATA_STREAM_SW_INT_IP = DATA_STREAM_INTERRUPT_PRIORITY;
    DATA_STREAM_SW_INT_IF = 0;
    DATA_STREAM_SW_INT_IE = 0; // MUST block i2c reads whilst setting up sensors!!!

    // Disable peripheral interrupts
	ACCEL_INT1_IE = 0;
	ACCEL_INT2_IE = 0;
	GYRO_INT2_IE = 0;
    MAG_INT_IE = 0;
	RtcInterruptOn(0);

    // Clear data ready
    gDataReady.all = 0;
    gStreamEnable = *newFlags;

    // Initialize data streams
	DataStreamInit(&altDataStream,   sizeof(alt_t),         (sizeof(sampleBuffer.altBuffer) / sizeof(alt_t)),         sampleBuffer.altBuffer);
    DataStreamInit(&adcDataStream,   sizeof(adc_results_t), (sizeof(sampleBuffer.adcBuffer) / sizeof(adc_results_t)), sampleBuffer.adcBuffer);
    
    // Initialize standard data streams
	DataStreamInit(&accelDataStream, sizeof(accel_t),       (sizeof(sampleBuffer.standardSampling.accelBuffer) / sizeof(accel_t)),       sampleBuffer.standardSampling.accelBuffer);
    DataStreamInit(&gyroDataStream,  sizeof(gyro_t),        (sizeof(sampleBuffer.standardSampling.gyroBuffer)  / sizeof(gyro_t)),        sampleBuffer.standardSampling.gyroBuffer);
    DataStreamInit(&magDataStream,   sizeof(mag_t),         (sizeof(sampleBuffer.standardSampling.magBuffer)   / sizeof(mag_t)),         sampleBuffer.standardSampling.magBuffer);
    
#ifdef SYNCHRONOUS_SAMPLING    
	if (gStreamEnable.allAxis)
	{
		int rate = settings.sensorConfig[SENSOR_INDEX_ALLAXIS].frequency;
		if (rate <= 0) { rate = 1; }
		
		// Don't use standard streams (set to zero capacity)
		DataStreamInit(&accelDataStream, sizeof(accel_t), 0, NULL);
	    DataStreamInit(&gyroDataStream,  sizeof(gyro_t),  0, NULL);
	    DataStreamInit(&magDataStream,   sizeof(mag_t),   0, NULL);
	    
	    // Instead, use "all axis" synchronously sampled stream
	    DataStreamInit(&allAxisDataStream, sizeof(all_axis_t),  ALL_AXIS_SAMPLE_COUNT, sampleBuffer.allAxisBuffer);
	    
	    // TODO: IMPORTANT! Set interrupt rate from configuration
		gTimerAccumulator = 0;
		gTimerRemainder = TIMER_RATE - ((TIMER_RATE / rate) * rate);
		RtcInterruptOn(rate);
	}
	else
	{
		// Move earlier standard data stream initialziation here when the #ifdef's get removed
		DataStreamInit(&allAxisDataStream, sizeof(all_axis_t), 0, NULL);
	}
#endif    

    
    // Accel
    if (gStreamEnable.accel)
    {
#ifdef HAVE_DISPLAY // For display implementation we just use orientation change
	#ifdef SYNCHRONOUS_SAMPLING
			if (gStreamEnable.allAxis) AccelStartupSettings(settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency, settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity,  ACCEL_FLAG_ORIENTATION_INTERRUPTS);
			else
	#endif
        AccelStartupSettings(settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency, settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity, ACCEL_FLAG_FIFO_INTERRUPTS | ACCEL_FLAG_ORIENTATION_INTERRUPTS);
    }
    else if (gStreamEnable.accel_int2)
    {
        // Setup for transient/orientation change only
        AccelStartupSettings(12, 8, ACCEL_FLAG_ORIENTATION_INTERRUPTS);
    }
#else
	#ifdef SYNCHRONOUS_SAMPLING
			if (gStreamEnable.allAxis) AccelStartupSettings(settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency, settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity, ACCEL_FLAG_TRANSIENT_INTERRUPTS);
			else
	#endif
        AccelStartupSettings(settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency, settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity, ACCEL_FLAG_FIFO_INTERRUPTS | ACCEL_FLAG_TRANSIENT_INTERRUPTS);
    }
    else if (gStreamEnable.accel_int2)
    {
        // Setup for transient change only
        AccelStartupSettings(12, 8, 0);
        AccelEnableInterrupts(ACCEL_INT_SOURCE_TRANSIENT, 0);
    }
#endif
    else
    {
        AccelStandby();
    }

    // Gyro
    if (gStreamEnable.gyro)
    {
#ifdef SYNCHRONOUS_SAMPLING
		if (gStreamEnable.allAxis) GyroStartupSettings(settings.sensorConfig[SENSOR_INDEX_GYRO].frequency, settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity, 0);
		else
#endif
        GyroStartupSettings(settings.sensorConfig[SENSOR_INDEX_GYRO].frequency, settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity, GYRO_FLAG_FIFO_INTERRUPTS);
    }
    else
    {
        GyroStandby();
    }

    // Magnetometer
    if (gStreamEnable.mag)
    {
#ifdef SYNCHRONOUS_SAMPLING
		if (gStreamEnable.allAxis) MagStartupSettings(settings.sensorConfig[SENSOR_INDEX_MAG].frequency, settings.sensorConfig[SENSOR_INDEX_MAG].sensitivity, 0);
		else
#endif
        MagStartupSettings(settings.sensorConfig[SENSOR_INDEX_MAG].frequency, settings.sensorConfig[SENSOR_INDEX_MAG].sensitivity, MAG_FLAG_FIFO_INTERRUPTS);
    }
    else
    {
        MagStandby();
    }
#ifdef HAVE_PROXIMITY_SENSOR
    // Proximity
    if (gStreamEnable.prox)
    {
 		ProxVerifyDeviceId();
		ProxStartup();
	}
#endif
    // Altimeter (pressure)
    if (gStreamEnable.alt)
    {
       	AltInit();

        // Prime initial values
		// Following temp conversion already done in init function
        //BMP085_Initiate_ut_conversion();	// Begin a new sample
        //DelayMs(20);						// Otherwise the following temperature measure will fail
        //BMP085_read_ut();					// Read last sample
        sampler.lastTemperature = BMP085_calc_temperature(); // This is just the math bit
        BMP085_Initiate_up_conversion();	// Begin a new sample
        DelayMs(100);
        BMP085_read_up();					// Read last sample
        sampler.lastPressure = BMP085_calc_pressure(); // This is just the math bit
    }
    else
    {
    	AltStandby();
    }

	if (*(short*)newFlags != 0)
	{
    	IC1CON1 = 7;				// The IC module is used for sw interrupt generation and must be setup
    	DATA_STREAM_SW_INT_IE = 1;  // Now we can allow it to vector
	}

    return;
}



/*
// Check if we're ready to write any sectors
// This function makes things a lot cleaner if we can query whether we want to write before writing (e.g. for setting PLL, debug flashing, etc.)
char LoggerReadyToWrite(void)
{
    if (FifoLength(&accelDataStream.fifo) >= ACCEL_SECTOR_SAMPLE_COUNT) { return 1; }
    if (FifoLength(&gyroDataStream.fifo)  >= GYRO_SECTOR_SAMPLE_COUNT)  { return 1; }
	if (FifoLength(&magDataStream.fifo)   >= MAG_SECTOR_SAMPLE_COUNT)   { return 1; }
	if (FifoLength(&altDataStream.fifo)   >= ALT_SECTOR_SAMPLE_COUNT)   { return 1; }
	if (FifoLength(&adcDataStream.fifo)   >= ADC_SECTOR_SAMPLE_COUNT)   { return 1; }
	return 0;
}
*/


void SamplerTasks(void)
{
    // Most sampler tasks are handled in interrupts
    
   	// 1Hz updates
	if (gSecondElapsed)
	{
		char swTrigger = 0;
		
	    gSecondElapsed = 0;
	    
	    // Timer increment and ceiling
	    if (status.inactivity < 0xffff) status.inactivity++;			// Inactivity timer
	    if (status.alertTimer < 0xffff) status.alertTimer++;			// Alert timer
	    if (status.connection != CONNECTION_NONE) { status.disconnectTimer = 0; logger.debugFlashCount = LED_STATE_FLASH_TIMEOUT;} // Set flash timout
	    else if (status.disconnectTimer < 0xffff) status.disconnectTimer++;	// Disconnect timer
    
	    // Always update ADC readings
		UpdateAdcWait();		// Includes 1 msec delay

	    // ADC sample 1Hz
	    if (gStreamEnable.adc && !gDataReady.adc)
	    {
	        DataStreamUpdateTimestamp(&adcDataStream); 	// Update the time stamp
	        gDataReady.adc = TRUE; 						// Indicates fifo should sample the values
	        swTrigger = 1;
	    }
	
	    // Altimeter sample 1Hz
	    if (gStreamEnable.alt && !gDataReady.alt)
	    {
	        DataStreamUpdateTimestamp(&altDataStream); 	// Update the time stamp
	        gDataReady.alt = TRUE; 						// Indicates fifo should sample the values
	        swTrigger = 1;
	    }
	    
	    // Accel interrupt 2 (1Hz servicing), KL: New behaviour, just re-enable the vector on rtc tick
		if (gStreamEnable.accel_int2 && ACCEL_INT2_IE == 0 )
		{
			ACCEL_INT2_IE = 1;	// If there has been activity in the last second it will vector immediately						
		}	

		// Prox?
		#ifdef USE_PROXIMITY_SENSOR_1HZ
		if (gStreamEnable.prox)
	    {
	        gDataReady.prox = TRUE; 					// Indicates fifo should sample the values
	        swTrigger = 1;
	    }
	    #endif

	    if (swTrigger) { DATA_STREAM_SW_INT_IF = 1; } 	// SW trigger fifo int tasks
	}

    // If the pin is high and the data-ready flag not sent, the interrupt flag must have been missed (or just set anyway since the DISI).
    {
        asm("#DISI 0x3FFF");
        if (!gDataReady.accel && ACCEL_INT1) { ACCEL_INT1_IF = 1; }
        if (!gDataReady.gyro  && GYRO_INT2)	 { GYRO_INT2_IF  = 1; }
        if (!gDataReady.mag   && MAG_INT)	 { MAG_INT_IF    = 1; }        
		// KL: Careful not to become re-enterrant here i.e. ENABLED && should have fired (pin) && hasn't fired (preview)
        if (!gDataReady.accel_int2 && ACCEL_INT2 && ACCEL_INT2_IE) { ACCEL_INT2_IE = 0; gDataReady.accel_int2 = 1; }
        asm("#DISI 0x0000");            // Any interrupt just set will vector after this
    }

    // If sampling...
    if (logger.sampling == SAMPLING_ON)
    {
        // ...but not recording and not streaming...
        if (logger.recording == RECORDING_OFF && logger.stream == STREAM_NONE)
        {
            // ... Cancel the sampling
            StateSetSampling(SAMPLING_OFF, 1);
        }
    }
}


//EOF

