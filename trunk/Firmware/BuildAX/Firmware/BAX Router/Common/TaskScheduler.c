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

#include "TaskScheduler.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/ExRtc.h"
#include "Settings.h"

//#define DEBUG_ON
#include "Common/debug.h"

// Definitions
#define INVALID_TASK_INDEX 0xffff

// Variables
static unsigned short lastTaskIndex = 0; // Newest task added
static TimedEvent_t* scheduledTasksList[MAX_SCHEDULED_TASKS] = {0};
#ifndef USE_MALLOC // Create static list
static TimedEvent_t scheduledTasksItems[MAX_SCHEDULED_TASKS];
#endif

// Prototypes
static unsigned short GetEmptyTaskSlot(void);
static void ScheduleTaskProcessed(unsigned short index);

void SchedulerInit(void)
{
	// Null entries
	memset(scheduledTasksList,NULL,sizeof(scheduledTasksList));
}

// Call once per second from main - returns TRUE on second tick
unsigned char SchedulerTasks(void)
{
	unsigned short index;
	DateTime now;

	// Process tasks once every second 
	static unsigned short lastTime = 0;
	if(lastTime == rtcTicksSeconds) return FALSE;
	lastTime = rtcTicksSeconds;

	// Get time
	now = RtcNow();

	for(index=0;index<MAX_SCHEDULED_TASKS;index++)
	{
		// Check if we need to handle each task in turn
		TimedEvent_t* task = scheduledTasksList[index];
		if((task == NULL) || (task->time > now))
			continue; // next

		DBG_INFO("Task #%u called, interval %lus\r\n",index,task->reschedule);
		// Handle this task
		if(task->fptr != NULL)
		{
			// Temporary ptrs to tell compiler what to do
			void (*fptrv)(void) = task->fptr;
			void (*fptra)(void* aptr) = task->fptr;

			if(task->mode.flags.use_cb_aptr) 
				fptra(task->aptr);
			else
				fptrv();
		}
		// Call processed on it (may reschedule)
		ScheduleTaskProcessed(index);
	}// For	

	return TRUE;
}

void SchedulerAddTask(unsigned long time, unsigned long interval, unsigned short eventMode ,void* fptr, void* aptr)
{
	// Initialise a tasks
	TimedEvent_t temp = {
		.mode.val = eventMode,
		.fptr = fptr,
		.aptr = aptr,
		.reschedule = 0
	};
	// Set the time based on flags
	if		(temp.mode.flags.use_time)		temp.time = (DateTime)time;
	else if (temp.mode.flags.use_offset)	temp.time = RtcCalcOffset(RtcNow(), time);
	else 
	{
		DBG_ERROR("Error, malformed task add.\r\n");
		return; // Malformed mode argument
	}
	// Record repeat interval if repeating
	if		(temp.mode.flags.rpt_flag)		temp.reschedule = interval;
	// Add it to the list
	ScheduleAddTaskRaw(&temp);
}

void ScheduleAddTaskRaw(TimedEvent_t* task)
{
	// Get a place in queue
	unsigned short index = GetEmptyTaskSlot();
	if(index == INVALID_TASK_INDEX) 
	{
		DBG_ERROR("Error, task add fail, full.\r\n");
		return;
	}
#ifdef USE_MALLOC
	// Alocate space on heap
	TimedEvent_t* newTask = malloc(sizeof(TimedEvent_t));
	if(newTask == NULL) 
	{
		DBG_ERROR("Error, task malloc fail.\r\n");
		return;
	}
#else
	// Static list option
	TimedEvent_t* newTask = &scheduledTasksItems[index];
#endif
	// Copy in
	memcpy(newTask,task,sizeof(TimedEvent_t));
	// Set ptr in queue
	scheduledTasksList[index] = newTask;
	return;
}

void ScheduleTaskRemove(void* fptr)
{
	unsigned short index;
	// Find and remove all matching for type && fptr && aptr
	for(index=0;index<MAX_SCHEDULED_TASKS;index++)
	{
		TimedEvent_t* task = scheduledTasksList[index];
		if(task->fptr == fptr)
		{
			#ifdef USE_MALLOC
			// Free if not null
			if(task)free(task);
			#endif
			// Remove from queue
			DBG_INFO("Task #%u removed",index);
			scheduledTasksList[index] = NULL;	
		}
	}
	return;
}


void ScheduleTaskRemoveRaw(TimedEvent_t* taskType)
{
	unsigned short index;
	// Find and remove all matching for type && fptr && aptr
	for(index=0;index<MAX_SCHEDULED_TASKS;index++)
	{
		TimedEvent_t* task = scheduledTasksList[index];
		if(	(task->mode.userId == taskType->mode.userId) && 
			(task->fptr == taskType->fptr) &&
			(task->aptr == taskType->aptr))
		{
			#ifdef USE_MALLOC
			// Free if not null
			if(task)free(task);
			#endif
			// Remove from queue
			DBG_INFO("Task #%u removed",index);
			scheduledTasksList[index] = NULL;	
		}
	}
	return;
}

static unsigned short GetEmptyTaskSlot(void)
{
	unsigned short index;
	for(index=0;index<MAX_SCHEDULED_TASKS;index++)
	{
		lastTaskIndex = index;
		if(scheduledTasksList[index] == NULL)return index;	// First empty slot
	}
	// No empty task slots
	#ifdef REPLACE_NONE
		return INVALID_TASK_INDEX;
	#elif defined (REPLACE_NEWEST)
		// Return last added location 
		return lastTaskIndex;
	#elif defined(REPLACE_OLDEST)
	{
		// Search for oldest
		DateTime currentOldest = 0;
		unsigned short currrentIndex = 0;
		for(index=0;index<MAX_SCHEDULED_TASKS;index++)
		{
			// For tasks with SAME time, uses highest index
			if(scheduledTasksList[index]->taskTime >= currentOldest) 
				currrentIndex = index;
		}
		return currrentIndex;
	}
	#else
		#error "Specify behaviour for no list space."
	#endif
}

static void ScheduleTaskProcessed(unsigned short index)
{
	TimedEvent_t* task = scheduledTasksList[index];
	// Null check
	if(task != NULL)
	{
		// Reschedule?
		if(task->mode.flags.rpt_flag)
		{
			if(task->mode.flags.rpt_one_hz)
			{
				// Scheduled to be called every second, time irrelevant
			}
			else if(task->reschedule == 1) 
			{
				task->mode.flags.rpt_one_hz = 1;
			}
			else if(task->mode.flags.rpt_from_time)
			{
				task->time = RtcCalcOffset(task->time, task->reschedule);
			}
			else if (task->mode.flags.rpt_from_now)
			{
				task->time = RtcCalcOffset(RtcNow(), task->reschedule);
			}
		}
		// Remove
		else 
		{
			#ifdef USE_MALLOC
			// Free if not null
			if(task)free(task);
			#endif
			// Remove from queue
			DBG_INFO("Task #%u removed",index);
			scheduledTasksList[index] = NULL;
		}
	}
	return;	
}


