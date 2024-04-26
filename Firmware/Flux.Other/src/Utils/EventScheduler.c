// Karim Ladha 23-11-2016
// Created to add timed events to the SysTime module similar to TaskScheduler.c
// SysTime permits callbacks to run at high interrupt priority at a given rate, period or every second
// The TaskScheduler creates tasks that run at any scope in the Tasks() call at a given time in seconds or periodically
// The EventScheduler enables multiple callbacks from the RTC interrupt or from a Task() call with sub-millisecond accuracy  
// N.b. Compatibility to Nordic's SDK "app_timer.c" is by design to facilitate library reuse

// Headers
#include <stdint.h>			
#include <string.h> 
#include "Compiler.h"
#include "Utils/EventScheduler.h"
#include "Peripherals/SysTime.h"
#include "Config.h"

// Debugging
#define DEBUG_LEVEL_LOCAL	DEBUG_SYS
#define DBG_FILE			"evt-sched.c"	
#include "Utils/debug.h"

// Definitions - private and internal
#ifndef EVENT_SCHEDULER_MAX_ENTRIES
	#define EVENT_SCHEDULER_MAX_ENTRIES (1)
#elif (EVENT_SCHEDULER_MAX_ENTRIES < 1)
	#error "Event queue minimum length is 1."
#endif
#ifndef SYSTIME_TIMER_MIN_TICKS		/* Minimum ticks remaining before event considered expired in tasks() */
#define SYSTIME_TIMER_MIN_TICKS		5	
#endif
#ifndef SYSTIME_ENABLE_RTC_TIMER
	#error "No provision made for event scheduler in SysTimer.c"
#endif
#define EVTSCHED_INDEX_ACTIVE			(0)
#define EVTSCHED_INDEX_INSERT			(EVENT_SCHEDULER_MAX_ENTRIES - 1)

// Globals
// The event queue active entry count and used for indicating queue update required
volatile uint16_t gEventQueueLength = 0;
volatile uint8_t gQueueStopFlag = 0;
volatile uint8_t gQueueStartFlag = 0;

// Private prototypes
static void EventSchedulerHandler(void);
static inline void EventSchedulerTimerTasks(void); 
static inline void EventSchedulerStartTasks(void);
static inline void EventSchedulerStopTasks(void);
static inline void EventSchedulerCheckActive(void);
static inline void EventSchedulerQueueSetLast(uint16_t index);
static inline void EventSchedulerQueueRemove(uint16_t index);
static inline void EventSchedulerQueueReorder(uint16_t position);

// List of scheduled event handles as a list of pointers (first and last reserved)
ScheduledEvent_t* scheduledEvents[(EVENT_SCHEDULER_MAX_ENTRIES)] = {0};

// Prototypes
// Initialise the event scheduler module
void EventSchedulerInit(void)
{
	// Initialise queue entries and its length variables 
	memset(scheduledEvents, 0, sizeof(scheduledEvents));
	gEventQueueLength = 0;
	gQueueStopFlag = 0;
	gQueueStartFlag = 0;	
	// Install the timer callback
	SysTimeTimerSetCB(EventSchedulerHandler);
}

// Stop the scheduler module - also stop user calls to tasks()
void EventSchedulerShutdown(void)
{
	// First, prevent callback from timer ISR if active
	SysTimeTimerSetCB(NULL);	
	// Check if timer is running and stop it (first/active queue entry uses timer) 
	if(scheduledEvents[EVTSCHED_INDEX_ACTIVE]->state == EVT_SCHED_STATE_ACTIVE)
		SysTimeTimerStop();
	// Initialise queue entries and its length variables 
	memset(scheduledEvents, 0, sizeof(scheduledEvents));
	gEventQueueLength = 0;	
	gQueueStopFlag = 0;		
	gQueueStartFlag = 0;	
}

// Initialise the event structure. It is instantiated externally
void EventSchedulerSetup(ScheduledEvent_t* event, uint32_t reschedValue, EventSchedulerCB_t cb, void* ctx)
{
	// Checks - User creates event instance
	if(event == NULL) return;
	// Initialise the scheduled event
	event->state = EVT_SCHED_STATE_INIT;
	event->scheduledTick = 0;
	event->reschedValue = reschedValue;
	event->cb = cb;
	event->ctx = ctx;	
	return;
}

// Called from the ISR on scheduled event timer event
static void EventSchedulerHandler(void)
{
	// Remove any stopped timers
	EventSchedulerStopTasks();	
	// Add any new timers
	EventSchedulerStartTasks();
	// Handle expired events
	EventSchedulerTimerTasks();
	// Check timer running state
	EventSchedulerCheckActive();
}

// Start an existing, pre-initialised timed event
uint8_t EventSchedulerStart(ScheduledEvent_t* event, uint32_t delay)
{
	// Check event settings, queue space and other values
	if(	(event == NULL) ||
		(event->cb == NULL) ||
		(event->state != EVT_SCHED_STATE_INIT) || 
		(scheduledEvents[EVTSCHED_INDEX_INSERT] != NULL) ||
		(gEventQueueLength >= EVENT_SCHEDULER_MAX_ENTRIES) ) 
		return 0;	
	// Calculate the tick time value using RTC counter. tickTime = delay - (ticks_to_zero)
	event->scheduledTick = (delay + SysTimeTimerGetTick()) & sysTimeTickMask;	
	// Set state indicating the event needs positioning in the queue
	event->state = EVT_SCHED_STATE_ADD;
	// Set the last queue entry to the event
	scheduledEvents[EVTSCHED_INDEX_INSERT] = event;
	// Set the flag indicating event added
	gQueueStartFlag = 1;	
	// Invoke the interrupt now to add the event to the queue
	SysTimeTimerTriggerISR();
	// The event timer is started from the ISR
	return 1;
}

// The following function is called in the ISR context when the add flag is set
static inline void EventSchedulerStartTasks(void)	
{
	// If there are events requiring scheduling
	if(	(gQueueStartFlag != 0) &&
		(scheduledEvents[EVTSCHED_INDEX_INSERT] != NULL) &&
		(scheduledEvents[EVTSCHED_INDEX_INSERT]->state == EVT_SCHED_STATE_ADD) )
	{
		// Adjust the length to include the new event (although not 'within' queue)
		gEventQueueLength++;
		// Update the event state to queued
		scheduledEvents[EVTSCHED_INDEX_INSERT]->state = EVT_SCHED_STATE_QUEUED;
		// If the current position is not already the end of the queue
		if(EVTSCHED_INDEX_INSERT != (gEventQueueLength - 1))
		{
			// Make sure the event is in the queue by moving to the newly created last entry
			scheduledEvents[(gEventQueueLength - 1)] = scheduledEvents[EVTSCHED_INDEX_INSERT];
			// Free up the queue 'add-new' queue position again
			scheduledEvents[EVTSCHED_INDEX_INSERT] = NULL;
		}
		// Correctly order the new event within the queue
		EventSchedulerQueueReorder((gEventQueueLength - 1));
	}
	// Clear the event-added flag 
	gQueueStartFlag = 0;	
}

// Remove any pending or recurring events from the queue
uint8_t EventSchedulerStop(ScheduledEvent_t* event)
{
	// Check event settings, queue space and other values
	if(event == NULL) return 0;	
	// Check if timer is active and stop it
	if(event->state == EVT_SCHED_STATE_ACTIVE)
		SysTimeTimerStop();
	// Set the state to remove value
	event->state = EVT_SCHED_STATE_REMOVE;
	// Increment the remove flag
	gQueueStopFlag = 1;
	// Invoke the interrupt now to remove the event from queue
	SysTimeTimerTriggerISR();
	// The event timer is removed in the ISR
	return 1;
}

// The following function is called if the remove pointer is set from the ISR
static inline void EventSchedulerStopTasks(void)	
{
	uint16_t index;	
	// Check remove flag is set
	if(gQueueStopFlag != 0)
	{
		// Find events flagged for removal
		for(index = 0; index < gEventQueueLength; index++)
		{
			// Remove events in removal state and update states
			if(scheduledEvents[index]->state == EVT_SCHED_STATE_REMOVE)
			{
				// Update the event state before removing
				scheduledEvents[index]->state = EVT_SCHED_STATE_INIT;
				// Remove the event from the queue
				EventSchedulerQueueRemove(index);
			}
		}
	}
	// Clear the remove-events flag
	gQueueStopFlag = 0;
}

// Use only if scheduling events with external callback setting
void EventSchedulerTasks(void)
{
	uint16_t index;
	// The queue is volatile, event pointers must be cached
	for(index = 0; index < gEventQueueLength; index++)
	{
		// Get an event pointer from the queue
		ScheduledEvent_t* event = scheduledEvents[index];
		// Check the event state for external callback entries
		if(event->state == EVT_SCHED_STATE_CB_DUE)
		{
			// Update the state to indicate it should be removed from queue
			event->state = EVT_SCHED_STATE_REMOVE;
			// Callback will be called at current scope (can be rescheduled) 
			if(event->cb)event->cb(event->ctx);		
			// Set the flag indicating events need removal
			gQueueStopFlag = 1;
		}
	}
	// If the event removal flag is set, invoke the ISR to remove events
	if(gQueueStopFlag != 0)
		SysTimeTimerTriggerISR();
}

static inline void EventSchedulerTimerTasks(void)
{	
	uint16_t index;
	// Handle all expired events from queue start
	for(index = 0; index < gEventQueueLength; index++)
	{
		// Get a pointer to each event in the queue
		ScheduledEvent_t* event = scheduledEvents[index];		
		int32_t ticksRemaining;
		// Check for timer expiry on all events up to the first non-queued one
		if(event->state >= EVT_SCHED_STATE_QUEUED)
		{
			// Get the remaining time until event
			ticksRemaining = (event->scheduledTick - SysTimeTimerGetTick()) & sysTimeTickMask;
			// Check if entry has expired
			if(ticksRemaining <= SYSTIME_TIMER_MIN_TICKS)
			{
				// Check reschedule value for correct action on expiry
				if(event->reschedValue == EVTSCHED_RESCHEDULE_NO_CB_IN_ISR)
				{
					// Callback to be called outside ISR. Set external callback state
					event->state = EVT_SCHED_STATE_CB_DUE;
					// Remove from the active queue and place last in the queue
					EventSchedulerQueueSetLast(index);					
				}
				else if(event->reschedValue == EVTSCHED_RESCHEDULE_NO_REPEAT)
				{
					// The event is to be removed from the queue
					event->state = EVT_SCHED_STATE_INIT;
					// Just remove the event from the queue
					EventSchedulerQueueRemove(index);
				}
				else // Event reschedule required 
				{
					// Rescheduling is required for the event
					event->scheduledTick = (event->reschedValue + SysTimeTimerGetTick()) & sysTimeTickMask;
					// Set state to queued so it will be re-ordered within queue
					event->state = EVT_SCHED_STATE_QUEUED;			
					// Re-insert into the event queue
					EventSchedulerQueueReorder(index);
				}
				// Call the callback from ISR context
				if(event->cb)event->cb(event->ctx);
				// Check next queue entry for timer expiry
				continue;
			}
		}
		// Stop checking events after first non-queued or non-expired entry
		break;
	}// For;;
}	

// Make sure there is a timer running for events if there should be
static inline void EventSchedulerCheckActive(void)
{
	// If there are scheduled events in the queue and there is no active timer
	if((gEventQueueLength != 0) && (scheduledEvents[EVTSCHED_INDEX_ACTIVE]->state == EVT_SCHED_STATE_QUEUED))
	{
		// Update the active timer state
		scheduledEvents[EVTSCHED_INDEX_ACTIVE]->state = EVT_SCHED_STATE_ACTIVE;
		// Start the new active timer
		SysTimeTimerSetTick(scheduledEvents[EVTSCHED_INDEX_ACTIVE]->scheduledTick);	
	}	
}
// Quickly move an item in the queue to the last position
static inline void EventSchedulerQueueSetLast(uint16_t index)
{
	// Coopy the event pointer before the deletion
	ScheduledEvent_t* event	= scheduledEvents[index];
	// Move queue to close the gap
	memmove(&scheduledEvents[index], &scheduledEvents[index + 1], (sizeof(ScheduledEvent_t*) * (gEventQueueLength - index - 1)));
	// Replace the event at the queue end
	scheduledEvents[(gEventQueueLength - 1)] = event;
}
// Method to remove an event from the queue
static inline void EventSchedulerQueueRemove(uint16_t index)
{
	// If the index is not the end of the queue
	if(index < (gEventQueueLength - 1))
	{
		// Move queue to close the gap
		memmove(&scheduledEvents[index], &scheduledEvents[index + 1], (sizeof(ScheduledEvent_t*) * (gEventQueueLength - index - 1)));
		// Clear the duplicate created at queue end
		scheduledEvents[(gEventQueueLength - 1)] = NULL;
	}
	else if(index == (gEventQueueLength - 1))
	{
		// Indexing end of queue, just set to null to remove
		scheduledEvents[index] = NULL;
	}
	else
	{
		// Can't remove event not in queue
		DBG_ASSERT(0);
	}
	// Reduce the event list length to reflect the change, check for error
	if(gEventQueueLength > 0){gEventQueueLength--;}
	else {DBG_ASSERT(0);}
}
// The following function is called to move a queued event to the correct position
static inline void EventSchedulerQueueReorder(uint16_t position)	
{
	ScheduledEvent_t* event	= scheduledEvents[position];
	uint16_t index;	
	// Check value of position is within range
	DBG_ASSERT(position < EVENT_SCHEDULER_MAX_ENTRIES);
	// Short circuit first queue entry
	if(gEventQueueLength <= 1)
	{
		// Can't re-order with zero entries, shouldn't call on zero
		DBG_ASSERT(gEventQueueLength != 0);
		// The position should be at queue head if one entry
		DBG_ASSERT(position == 0);
		return;
	}
	// Find correct position in the queue
	for(index = 0; index < gEventQueueLength; index++)
	{
		// Check entries in the queue
		if(scheduledEvents[index] != NULL) 
		{
			// If the current position is in the time-ordered part of the queue
			if(scheduledEvents[index]->state >= EVT_SCHED_STATE_QUEUED)
			{
				int32_t difference;
				uint32_t remaining;
				// If the re-ordered event is not timed
				if(event->state < EVT_SCHED_STATE_QUEUED) 
				{
					// The event can be anywhere in the queue *after* the timed events
					continue;				
				}
				// Calculate difference in ticks from the event to this position, to see which is first
				difference = event->scheduledTick - scheduledEvents[index]->scheduledTick;
				// Determine which event is sooner, if event being inserted is later then skip over
				if(difference > 0)
				{
					// Get the current ticks remaining to the *later* time value
					remaining = (event->scheduledTick - SysTimeTimerGetTick()) & sysTimeTickMask;	
					if(remaining < difference)
					{
						// Exception case, current tick is *between* event times reversing order - insert event here
					}
					else
					{
						// The event should be ordered *after* this one, skip and check next queued event
						continue;					
					}
				}
				else // The event being inserted has a lower tick value than this position, earlier - insert here
				{
					// Get the current ticks remaining to the later position tick value
					remaining = (scheduledEvents[index]->scheduledTick - SysTimeTimerGetTick()) & sysTimeTickMask;	
					if(remaining < (-difference))
					{
						// Exception case, current tick is *between* event times reversing order - skip over instead
						continue;
					}
					else
					{
						// The event at position is later, insert the event at this queue position
					}
				}
			}
			// The new queue position found. If the new position requires an active timer to be stopped	
			if( (index == EVTSCHED_INDEX_ACTIVE) && 
				(scheduledEvents[index]->state == EVT_SCHED_STATE_ACTIVE) )
			{
				// Stop the running timer
				SysTimeTimerStop();
				// Update the displaced event state
				scheduledEvents[index]->state = EVT_SCHED_STATE_QUEUED;
			}
		}
		// New event position found or null reached. If earlier in the queue
		if(position > index)
		{
			// Shift the queue down one position, overwriting the entry being rescheduled
			memmove(&scheduledEvents[(index + 1)], &scheduledEvents[index], (sizeof(ScheduledEvent_t*) * (position - index)));
		}
		// If the new position is later in the queue. Otherwise, position has not changed
		else if(index < position)
		{
			// Move the queue up one position overwriting the current position to re-schedule
			memmove(&scheduledEvents[position], &scheduledEvents[(position + 1)], (sizeof(ScheduledEvent_t*) * (index - position)));
		}
		// Insert the event at the new position after any required queue shifts
		scheduledEvents[index] = event;	
		// The queue position was correctly re-ordered, return
		return;
	}
	// Position was not re-ordered successfully. An error occurred
	DBG_ASSERT(0);
}

// Check the list variables for errors
static inline void EventSchedulerQueueChecks(void)
{
	// Check queue length, should not exceed capacity
	if(gEventQueueLength > EVENT_SCHEDULER_MAX_ENTRIES)
	{
		// No way to resolve this issue, can't add event
		gEventQueueLength = EVENT_SCHEDULER_MAX_ENTRIES;
		DBG_ASSERT(0); 
	}	
	// Check queue head for validity errors
	if((gEventQueueLength > 0) && (scheduledEvents[EVTSCHED_INDEX_ACTIVE]->state != EVT_SCHED_STATE_ACTIVE))
	{
		// The queue should have an active event
		DBG_ASSERT(0);
	}	
	
}

