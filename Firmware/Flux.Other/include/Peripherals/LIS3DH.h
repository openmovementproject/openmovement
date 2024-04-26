/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *	 this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *	 this list of conditions and the following disclaimer in the documentation 
 *	 and/or other materials provided with the distribution.
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

// Based on original accel.h but specific to LIS3DH device and using stdint.h
// Functions changed slightly to improve api based on experiences using devices

#ifndef ACCEL_H
#define ACCEL_H

// Includes
#include <stdint.h>
#include "HardwareProfile.h"

// Defines
#define ACCEL_MAX_FIFO_SAMPLES 32

// Types
typedef union
{
	struct { int16_t x, y, z; };
	struct { uint8_t xl, xh, yl, yh,  zl, zh; };
	int16_t values[3];
} accel_t;

typedef struct
{
	uint8_t	ctrl_reg0;	// 0x1E
	uint8_t temp_cfg;	// 0x1F
	uint8_t ctrl_reg1;	// 0x20
	uint8_t ctrl_reg2;	// 0x21
	uint8_t ctrl_reg3;	// 0x22
	uint8_t ctrl_reg4;	// 0x23
	uint8_t ctrl_reg5;	// 0x24
	uint8_t ctrl_reg6;	// 0x25
	uint8_t reference;	// 0x26
	uint8_t status_reg;	// 0x27
	uint8_t xl;			// 0x28 - Note: 16bit Aligned within struct
	uint8_t xh;			// 0x29
	uint8_t yl;			// 0x2A
	uint8_t yh;			// 0x2B
	uint8_t zl;			// 0x2C
	uint8_t zh;			// 0x2D
	uint8_t fifo_ctrl;	// 0x2E
	uint8_t fifo_src;	// 0x2F
	uint8_t int1_cfg;	// 0x30
	uint8_t int1_src;	// 0x31
	uint8_t int1_ths;	// 0x32
	uint8_t int1_dur;	// 0x33
	uint8_t int2_cfg;	// 0x34
	uint8_t int2_src;	// 0x35
	uint8_t int2_ths;	// 0x36
	uint8_t int2_dur;	// 0x37
	uint8_t click_cfg;	// 0x38
	uint8_t click_src;	// 0x39
	uint8_t click_ths;	// 0x3A
	uint8_t time_limit;	// 0x3B
	uint8_t time_latency;// 0x3C
	uint8_t time_window;// 0x3D
} accel_settings_t;

// Orientation - accel_regs.int1_src & 0x3F
typedef enum {					// Positions referenced to sensor-band on left wrist watch with charge pins closest to elbow
	ACCEL_POS_Z_UP		= 0x20,	// Flat 
	ACCEL_POS_Z_DOWN	= 0x10,	// upside-down
	ACCEL_POS_Y_UP		= 0x08,	// Pointing up (viewing top)
	ACCEL_POS_Y_DOWN	= 0x04,	// Pointing down (viewing top)
	ACCEL_POS_X_UP		= 0x02,	// Point left (viewing bottom)
	ACCEL_POS_X_DOWN	= 0x01,	// Point right (viewing top)
	ACCEL_POS_UNSTABLE	= 0x00,	// Orientation settling
} AccelOrientation_t;

// Globals..
extern uint8_t accelPresent;
extern accel_settings_t accel_regs;
extern const accel_settings_t accel_regs_startup;

// Read device ID
uint8_t AccelPresent(void);

// Device settings translation. Values to register values
uint8_t AccelSetting(accel_settings_t* settings, uint8_t range, uint16_t rate);

// Initialize the accelerometer
uint8_t AccelStartup(accel_settings_t* settings);					

// Shut down the accelerometer (standby mode)
uint8_t AccelShutdown(void);

// Clear interrupts, read interrupt registers
uint8_t AccelReadEvents(void);

// Reads a 3-axis value from the accelerometer 
uint8_t AccelReadSample(accel_t *value);

// Read FIFO queue length
uint8_t AccelReadCount(void);

// Reads the accelerometer FIFO (bytes = 6 * entries)
uint8_t AccelReadFifo(accel_t *buffer, uint8_t maxEntries);

// Read all 'readable' registers to the global structure
void AccelReadRegs(void);

// Write all 'writeable' registers to the global structure
void AccelWriteRegs(void);

#endif

