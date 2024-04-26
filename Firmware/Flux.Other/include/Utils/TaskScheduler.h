// Karim Ladha, 2015
// Light weight flexible task scheduler for various tasks
// For normal usage call the tasks function from main scope
#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

// Headers
#include <stdint.h>

/* 	Required definitions (config.h):
	SCHEDULER_USES_MALLOC		- Dynamic memory usage instead of fixed
	SCHEDULER_MAX_TASKS			- Number of simultaneous scheduled tasks ()
	Define *ONE* of these to define task overflow behaviour
	SCHEDULER_REPLACE_NONE		- Discard new task
	SCHEDULER_REPLACE_NEWEST	- Replace the last added task
	SCHEDULER_REPLACE_LAST		- Replace the task that runs last
*/
// Definitions
#define SCHEDULER_INVALID_UID	0xFF

// Scheduled task mode flags and optional uid
typedef union {
	uint16_t val;					// Full 16 bit value
	struct {
	uint8_t repeat_task		: 1;	// Task is re-scheduled after it occurs
	uint8_t rpt_from_now	: 1;	// Re-scheduled time is from now (may drift)
	uint8_t rpt_from_time	: 1;	// Task is re-scheduled exactly from last time
	uint8_t rpt_one_hz		: 1;	// Task is called every second
	uint8_t sched_at_time	: 1;	// The time value is an epoch time
	uint8_t sched_at_offset	: 1;	// The time value is a offset from now
	uint8_t use_cb_aptr 	: 1;	// Callback prototype is void fptr(void* aptr)
	uint8_t use_cb_uid 		: 1;	// Callback prototype is void fptr(uint8_t uid)
	uint8_t uid : 8;};				// Optional user id value
}TaskMode_t;

// The scheduled task structure
typedef struct {
	uint32_t time;					// Execute at time
	uint32_t interval;				// Re-schedule interval
	void* fptr;						// Call back func ptr
	void* aptr;						// Call back arg ptr
	TaskMode_t mode;				// Task mode setting
} TimedTask_t;

// Call first, once
void SchedulerInit(void);
// Call at main scope, returns true once a second
uint16_t SchedulerTasks(void);
// Construct a task and add it to the queue
void SchedulerAdd(uint32_t time, uint32_t interval, uint16_t taskMode ,void* fptr, void* aptr);
// Remove matching uids, function pointers and argument pointers
void SchedulerRemove(void* fptr, void* aptr, uint8_t uid);
// Add a raw task to queue, the task is copied internally
void SchedulerAddRaw(TimedTask_t* task);


// Debugging visibility
#ifdef __DEBUG
#include "config.h"
extern uint16_t taskListLen;
extern uint16_t lastAddedTaskIndex;
#ifdef SCHEDULER_USES_MALLOC 
extern TimedTask_t** scheduledTasksList;
#else
extern TimedTask_t* scheduledTasksList[] ;
#endif
extern TimedTask_t scheduledTasksItems[];
#endif

#endif // ifdef TASK_SCHEDULER_H
// EOF
