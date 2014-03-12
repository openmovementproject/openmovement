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

// Sampler
// Dan Jackson, K Ladha, 2010-2012


#ifndef SAMPLER_H
#define SAMPLER_H



// Initialize the sampler
void SamplerInit(void);

// Sampler tasks
void SamplerTasks(void);


#ifdef SYNCHRONOUS_SAMPLING
	// "All axis" (3x triaxial) data type
	typedef struct { accel_t accel; gyro_t gyro; mag_t mag; } all_axis_t;
#endif


// Data ready flags
typedef union {
	struct {
		// Keep these in the order from SENSOR_INDEX_* in Settings.h
		unsigned char accel  : 1;       // SENSOR_INDEX_ACCEL == 0
		unsigned char gyro 	 : 1;       // SENSOR_INDEX_GYRO  == 1
		unsigned char mag 	 : 1;       // SENSOR_INDEX_MAG   == 2
		unsigned char alt 	 : 1;       // SENSOR_INDEX_ALT   == 3
		unsigned char adc	 : 1;       // SENSOR_INDEX_ADC   == 4
		
#ifdef SYNCHRONOUS_SAMPLING
		unsigned char allAxis : 1;		// SENSOR_INDEX_ALLAXIS == 5  --  "all axis" (3x triaxial sensors)
#endif

		unsigned char accel_int2 : 1;
		unsigned char accel_int2_preview : 1;	// Set by interrupt (which is then masked off), at 1Hz: accel_int2 is updated to reflect this value, and the interrupt is re-enabled 
		//unsigned char temp   : 1;
		//unsigned char epoc 	 : 1;				
		unsigned char prox : 1;			// Proximity sensor
	};
	unsigned short all;
} dataStreamFlags_t;


// Logger state
typedef struct
{
    // Samples for point-sample queries
    accel_t lastAccel;
    gyro_t lastGyro;
    mag_t lastMag;
    long lastPressure;
    long lastTemperature;
} sampler_t;

extern sampler_t sampler;


// Globals (move these to state structure above)
extern volatile dataStreamFlags_t gStreamEnable;


void SetupSampling(dataStreamFlags_t *newFlags);


// Data buffers, must be >1 sectors worth of samples (480/entrySize) + (sampleRate * writeLatencyTime)

#define ACCEL_SECTOR_SAMPLE_COUNT (480/sizeof(accel_t))         // Samples per sector
extern datastream_t accelDataStream;

#define GYRO_SECTOR_SAMPLE_COUNT (480/sizeof(gyro_t))           // Samples per sector
extern datastream_t gyroDataStream;

#define MAG_SECTOR_SAMPLE_COUNT	(480/sizeof(mag_t))             // Samples per sector
extern datastream_t magDataStream;

#define ALT_SECTOR_SAMPLE_COUNT (480/sizeof(alt_t))             // Samples per sector
extern datastream_t altDataStream;

#define ADC_SECTOR_SAMPLE_COUNT (480/sizeof(adc_results_t))     // Samples per sector
extern datastream_t adcDataStream;

#ifdef SYNCHRONOUS_SAMPLING
	#define ALLAXIS_SECTOR_SAMPLE_COUNT ((sizeof(all_axis_t) == 18) ? 25 : (480/sizeof(all_axis_t)))     // Samples per sector
	extern datastream_t allAxisDataStream;
#endif


// Buffer sizes
#define SAMPLE_BUFFER_SIZE_ACCEL (ACCEL_SECTOR_SAMPLE_COUNT * 20/10)		// 2.0 * sectors
#define SAMPLE_BUFFER_SIZE_GYRO  ( GYRO_SECTOR_SAMPLE_COUNT * 20/10)		// 2.0 * sectors
#define SAMPLE_BUFFER_SIZE_MAG   (  MAG_SECTOR_SAMPLE_COUNT * 15/10)		// 1.5 * sectors
#define SAMPLE_BUFFER_SIZE_ALT   (  ALT_SECTOR_SAMPLE_COUNT * 12/10)		// 1.2 * sectors
#define SAMPLE_BUFFER_SIZE_ADC   (  ADC_SECTOR_SAMPLE_COUNT * 12/10)		// 1.2 * sectors
#define SAMPLE_BUFFER_SIZE_ALLAXIS ((SAMPLE_BUFFER_SIZE_ACCEL + SAMPLE_BUFFER_SIZE_GYRO + SAMPLE_BUFFER_SIZE_MAG) * (3 * 2) / (9 * 2))	// Shares with accel/gyro/mag


// Reduced buffer size builds (will only work for multi-axis logging)
#ifdef CWA_REDUCE_BUFFERS_MULTI_ONLY
#warning "This build is ONLY going to work for multi-axis logging -- otherwise sectors will overflow!"
#undef SAMPLE_BUFFER_SIZE_ACCEL
#define SAMPLE_BUFFER_SIZE_ACCEL (ACCEL_SECTOR_SAMPLE_COUNT * 8/10)		// 0.8 * sectors (not enough for accel logging alone)
#undef SAMPLE_BUFFER_SIZE_GYRO
#define SAMPLE_BUFFER_SIZE_GYRO  ( GYRO_SECTOR_SAMPLE_COUNT * 8/10)		// 0.8 * sectors (not enough for gyro logging alone)
#undef SAMPLE_BUFFER_SIZE_MAG
#define SAMPLE_BUFFER_SIZE_MAG   (  MAG_SECTOR_SAMPLE_COUNT * 8/10)		// 0.8 * sectors (not enough for mag. logging alone)
#undef SAMPLE_BUFFER_SIZE_ALLAXIS
#define SAMPLE_BUFFER_SIZE_ALLAXIS ((SAMPLE_BUFFER_SIZE_ACCEL + SAMPLE_BUFFER_SIZE_GYRO + SAMPLE_BUFFER_SIZE_MAG) * (3 * 2) / (9 * 2))	// Shares with accel/gyro/mag
#endif

#endif
