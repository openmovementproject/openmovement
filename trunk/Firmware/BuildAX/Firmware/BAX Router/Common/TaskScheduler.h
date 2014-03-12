/* 
 * Copyright (c) 2014, Newcastle University, UK.
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

// Simple timed scheduler for common server/router/logger tasks
// Karim Ladha, 2014

/*
KL 03-03-2014
Simple timed scheduler for common server/router/logger tasks
Called from main; Uses rtc to allow timely execution of tasks
and management of logs.
*/

#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

// Required headers
#include "Peripherals/Rtc.h"

// The task list is a fixed length
#define MAX_SCHEDULED_TASKS	10
// Select behaviour for full queue
#define REPLACE_NONE
//#define REPLACE_NEWEST
//#define REPLACE_OLDEST

// Memory usage
#ifndef USE_MALLOC
#define USE_MALLOC
#endif

// Type macros use OR
#define TASK_CB_USE_ARG				0x0100	// Use argptr on call back
#define TASK_RPT_TIME_TIME			0x4600	// Repeating task, set by time, rescheduled by time
#define TASK_RPT_OFFSET_OFFSET		0x2A00	// Repeating task, set by offset from now, reschedule from now
#define TASK_RPT_OFFSET_TIME		0x2600	// Repeating task, set by offset from now, rescheduled by time
#define TASK_RPT_OS_ONE_HZ			0x3200	// Repeating task at 1 hz, starts at offset
#define TASK_TIME_ONCE				0x4000	// Task called once at set time
#define TASK_OFFSET_ONCE			0x2000	// Task called once at set offset

typedef union {
		unsigned char userId;	
		unsigned short val;
		struct {
			unsigned char userIdByte;// Lower 8 bits
			int use_cb_aptr 	: 1; // 0x0100
			int rpt_flag		: 1; // 0x0200
			int rpt_from_time	: 1; // 0x0400
			int rpt_from_now	: 1; // 0x0800
			int rpt_one_hz		: 1; // 0x1000
			int use_offset		: 1; // 0x2000
			int use_time		: 1; // 0x4000
			int unused			: 1; // 0x8000	
		}flags;
} EventMode_t; // 2 bytes

typedef struct {
	DateTime time;						// Execute at....
	EventMode_t mode;					// Task mode (use EventMode_t)
	unsigned long reschedule;			// Reschedule by (sec)
	void* fptr;							// Function pointer
	void* aptr;							// Passed in callback
} TimedEvent_t;

// Call first, once
void SchedulerInit(void);
// Call at main scope, returns true once a second
unsigned char SchedulerTasks(void);
// Contruct a task and add it to the queue
void SchedulerAddTask(unsigned long time, unsigned long interval, unsigned short eventMode ,void* fptr, void* aptr);
// Add a raw task to queue, the task is copied internally
void ScheduleAddTaskRaw(TimedEvent_t* task);
// Simple task remove just using function pointer
void ScheduleTaskRemove(void* fptr);
// Find and remove all matching tasks (mode.userId && fptr && aptr)
void ScheduleTaskRemoveRaw(TimedEvent_t* taskType);

#endif
//



