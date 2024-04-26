// Karim Ladha, 2015
// Light weight flexible task scheduler for various tasks
// For normal usage call the tasks function from main scope
#include <string.h>
#include "Utils/TaskScheduler.h"
#include "Peripherals/SysTime.h"
#include "config.h"

// Debug setting
#define DEBUG_LEVEL_LOCAL	DEBUG_SYS
#define DBG_FILE			"tasksched.c"	
#include "utils/debug.h"

// Definitions
#ifndef SCHEDULER_MAX_TASKS
	#ifndef SCHEDULER_USES_MALLOC
		#warning "Max tasks undefined, dynamic allocation used."
	#endif
	#define SCHEDULER_USES_MALLOC
#endif
#if	!defined(SCHEDULER_REPLACE_NONE)\
&& 	!defined(SCHEDULER_REPLACE_NEWEST)\
&& 	!defined(SCHEDULER_REPLACE_LAST)
	#warning "Default task overflow behaviour!"
	#define SCHEDULER_REPLACE_NONE
#endif
#define SCHEDULER_MIN_TASKS	2	/* Only used with dynamic allocation */
#define INVALID_TASK_INDEX 	0xffff
#define INVALID_TIME_EPOC	0xffffffff

// Variables
uint16_t lastAddedTaskIndex = 0; 
#ifdef SCHEDULER_USES_MALLOC 
	// Set the scheduler list to point to empty list
	uint16_t taskListLen = 0;
	TimedTask_t** scheduledTasksList = NULL;
#else
	// If not using dynamic memory, create static list
	TimedTask_t scheduledTasksItems[SCHEDULER_MAX_TASKS];
	// Make a constant pointer list to add tasks to
	const uint16_t taskListLen = SCHEDULER_MAX_TASKS;
	TimedTask_t* scheduledTasksList[SCHEDULER_MAX_TASKS] = {0};
#endif

// Private prototypes
static uint16_t GetEmptyTaskSlot(void);
static void ScheduleTaskProcessed(uint16_t index);

// Source
// Initialise task list
void SchedulerInit(void)
{
#ifdef SCHEDULER_USES_MALLOC 
	// Make a minimalist list to begin with, can extend later
	TimedTask_t** scheduledTasksBaseList;
	taskListLen = SCHEDULER_MIN_TASKS;
	// Allocate pointer list
	scheduledTasksBaseList = DBG_MALLOC((taskListLen * sizeof(TimedTask_t*)));
	// Check it worked
	if(scheduledTasksBaseList == NULL)
	{
		DBG_ERROR("unable to start scheduler");
		taskListLen = 0;
		scheduledTasksList = NULL;	
		return;	
	}
	// Assign list to global
	scheduledTasksList = scheduledTasksBaseList;
#endif
	// Clear all the entries to null
	memset(scheduledTasksList, 0, (taskListLen * sizeof(TimedTask_t*)));
}

// Called at least once per second from main - returns TRUE on second tick
uint16_t SchedulerTasks(void)
{
	static int16_t lastSec = 0;
	uint16_t index, handled = 0;
	EpochTime_t now;

	// Get time now
	now = SysTimeEpoch();
	
	// Only check tasks once per second, returns -1 for early out 
	if(lastSec == (int16_t)now) return INVALID_TASK_INDEX;
	lastSec = (int16_t)now;

	for(index=0;index<taskListLen;index++)
	{
		// Check if we need to handle each task in turn
		TimedTask_t* task = scheduledTasksList[index];
		if((task == NULL) || (task->time > now) || (task->fptr == NULL))
			continue; // next
		// Count the task
		handled++;
		// Debug
		DBG_INFO("\r\nTask #%u called, interval %lus",index,task->interval);
		// Call function with appropriate prototype
		if(task->mode.use_cb_aptr) 
		{
			void (*fptr)(void* aptr) = task->fptr;	
			fptr(task->aptr);
		}
		else if(task->mode.use_cb_uid)
		{
			void (*fptr)(uint8_t arg) = task->fptr;
			fptr(task->mode.uid);
		}
		else
		{
			void (*fptr)(void) = task->fptr;
			fptr();
		}
		// Call 'processed' method on it (may be rescheduled)
		ScheduleTaskProcessed(index);
	}// For(;;)	
	// Return the number of handled tasks
	return handled;
}

void SchedulerAdd(uint32_t time, uint32_t interval, uint16_t taskMode ,void* fptr, void* aptr)
{
	TimedTask_t task;
	uint8_t flag_count, errors = 0;

	// Copy the mode setting and check it
	task.mode.val = taskMode;
	
	// Early out on missing callback
	if(fptr == NULL) 
		errors++;

	// Callback setting, *either* uid *or* aptr argument
	if((task.mode.use_cb_aptr) && (task.mode.use_cb_uid))
	{
		errors++;
	}
	
	// Set the time based on flags, only one flag allowed
	flag_count = 0;
	if(task.mode.sched_at_offset)
	{
		flag_count++;
		time += (uint32_t)SysTimeEpoch();
	}
	if(task.mode.sched_at_time)
	{
		flag_count++;
	}
	if(flag_count != 1)
		errors++;
		
	// Repeat settings, only one flag allowed
	flag_count = 0;	
	if(task.mode.rpt_from_now)
	{
		flag_count++;
	}
	if(task.mode.rpt_from_time)
	{
		flag_count++;
	}
	if(task.mode.rpt_one_hz)
	{
		flag_count++;
	}		
	if(interval == 1) // Override 1s intervals
	{
		if(flag_count != 1) errors++;
		else flag_count = 1;
		task.mode.rpt_from_now = 0;
		task.mode.rpt_from_time = 0;
		task.mode.rpt_one_hz = 1;
	}
	// Check repeat flags are valid
	if(task.mode.repeat_task)
	{
		if(flag_count != 1)
			errors++;
	}
	else if (flag_count != 0)
		errors++;
	
	// Check no errors
	if(errors != 0)
	{
		DBG_ERROR("invalid sheduled task");
		return;
	}
	
	// Initialise a task variable
	task.time = time;
	task.interval = interval;
	task.fptr = fptr;
	task.aptr = aptr;
	
	// Add it to the list
	SchedulerAddRaw(&task);
}

void SchedulerAddRaw(TimedTask_t* task)
{
	TimedTask_t* newTask;
	uint16_t index;
	
	// Get a place in queue
	index = GetEmptyTaskSlot();
	
	if(index == INVALID_TASK_INDEX) 
	{
		DBG_ERROR("task add fail, full.");
		return;
	}
#ifdef SCHEDULER_USES_MALLOC
	// Allocate space on heap
	newTask = DBG_MALLOC(sizeof(TimedTask_t));
	if(newTask == NULL) 
	{
		DBG_ERROR("task add malloc fail");
		return;
	}
#else
	// Static list option
	newTask = &scheduledTasksItems[index];
#endif
	// Copy into memory
	memcpy(newTask, task, sizeof(TimedTask_t));
	// Set ptr in queue
	scheduledTasksList[index] = newTask;
	return;
}

void SchedulerRemove(void* fptr, void* aptr, uin8_t uid)
{
	uint16_t index;
	// Find and remove all matching for fptr && aptr
	for(index = 0; index < taskListLen; index++)
	{
		TimedTask_t* task = scheduledTasksList[index];
		if(	((fptr == NULL) || (task->fptr == fptr)) && 
			((aptr == NULL) || (task->aptr == aptr)) &&
			((uid == SCHEDULER_INVALID_UID) || (task->mode.uid == uid)) )
		{
			#ifdef SCHEDULER_USES_MALLOC
			// Free if not null
			if(task)DBG_FREE(task);
			#endif
			// Remove from queue
			DBG_INFO("\r\nTask #%u removed",index);
			scheduledTasksList[index] = NULL;	
		}
	}
	return;
}

static uint16_t GetEmptyTaskSlot(void)
{
	uint16_t index;
	// First check for empty queue entry
	for(index = 0; index < taskListLen; index++)
	{
		lastAddedTaskIndex = index;
		if(scheduledTasksList[index] == NULL)
			return index;	// First empty slot
	}
	
	// If dynamic length, try to extend list length first
	#ifdef SCHEDULER_USES_MALLOC
	{
		TimedTask_t** longerSchedulerList;
		uint16_t longerListLen = 1;
		
		// Set the new length larger than current list by doubling
		while(longerListLen < taskListLen)
			longerListLen <<= 1;

		// Allocate pointer list
		longerSchedulerList = DBG_MALLOC((longerListLen * sizeof(TimedTask_t*)));
		if(longerSchedulerList != NULL)
		{
			uint16_t firstUnusedIndex = taskListLen;
			// List extension worked, clear and copy in old list
			memset(longerSchedulerList, 0, longerListLen * sizeof(TimedTask_t*));
			memcpy(longerSchedulerList, scheduledTasksList, taskListLen * sizeof(TimedTask_t*));
			// Free old list
			if(scheduledTasksList != NULL)
				DBG_FREE(scheduledTasksList);
			// Assign new pointer and length
			scheduledTasksList = longerSchedulerList;
			taskListLen = longerListLen;
			// Return created space
			return firstUnusedIndex;
		}
		// Else, allocation failed...
	}
	#endif

	// No empty task slots
	#if defined (SCHEDULER_REPLACE_NONE)
		// Add task failed, no space
		return INVALID_TASK_INDEX;
	#elif defined (SCHEDULER_REPLACE_NEWEST)
	{
		// Check the list length is not zero
		if(taskListLen == 0)
		{
			// Add task failed, no list
			return INVALID_TASK_INDEX;			
		}
		// Return last added location
		#ifdef SCHEDULER_USES_MALLOC
			// If dynamic, free task first since only the slot is re-used
			DBG_FREE(scheduledTasksList[lastAddedTaskIndex]);
		#endif		
		return lastAddedTaskIndex;
	#elif defined(SCHEDULER_REPLACE_LAST)
	{
		// Search for oldest
		uint32_t timeLatest = 0;
		uint16_t indexLatest = 0;
		// Check the list length is not zero
		if(taskListLen == 0)
		{
			// Add task failed, no list
			return INVALID_TASK_INDEX;			
		}		
		for(index = 0; index < taskListLen; index++)
		{
			// For tasks with SAME time, uses highest index
			if(scheduledTasksList[index]->time >= timeLatest) 
			{
				timeLatest = scheduledTasksList[index]->time;
				indexLatest = index;
			}
		}
		#ifdef SCHEDULER_USES_MALLOC
			// If dynamic, free task first since only the slot is re-used
			DBG_FREE(scheduledTasksList[indexLatest]);
		#endif			
		return indexLatest;
	}
	#else
		#error "Specify list overflow behaviour!"
	#endif
}

static void ScheduleTaskProcessed(uint16_t index)
{
	TimedTask_t* task = scheduledTasksList[index];
	// Null check
	if(task != NULL)
	{
		// Reschedule?
		if(task->mode.repeat_task)
		{
			if(task->mode.rpt_one_hz)
			{
				// Scheduled to be called every second, time irrelevant
			}
			else if(task->interval == 1) 
			{
				task->mode.rpt_one_hz = 1;
			}
			else if(task->mode.rpt_from_time)
			{
				task->time += task->interval;
			}
			else if (task->mode.rpt_from_now)
			{
				task->time = SysTimeEpoch() + task->interval;
			}
		}
		// Remove
		else 
		{
			#ifdef SCHEDULER_USES_MALLOC
			// Free if not null
			if(task)DBG_FREE(task);
			#endif
			// Remove from queue
			DBG_INFO("\r\nTask #%u removed",index);
			scheduledTasksList[index] = NULL;
		}
	}
	return;	
}

// EOF

