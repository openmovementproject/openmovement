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

// IMU-BMI160.c - Sensor interface
// K Ladha 2018
#ifndef _IMU_BMI160_H_
#define _IMU_BMI160_H_

// Includes
#include <stdint.h>
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"

// Definitions
#define gyroPresent accelPresent

// Types
typedef accel_t gyro_t;

// Variables
extern unsigned char imu_sample_size; // 6 or 12 bytes depending on gyro
extern unsigned short imu_sample_rate;
extern unsigned short imu_accel_range;
extern unsigned short imu_gyro_range;
extern accel_t last_imu_sample[2];

// Fifo constants
#define IMU_FIFO_BUFFER_LEN	1024
#define IMU_FIFO_WATERMARK	(480/4) /* Units of 4 bytes p.40*/

// Prototypes
int8_t IMU_Init(void);
void IMU_Off(void);
void IMU_Enable_Interrupts(uint8_t flags, uint8_t pinMask);
void GyroSingleSample(accel_t *gyroBuffer);
void GyroSetRange(uint16_t gyroRange);
#endif
