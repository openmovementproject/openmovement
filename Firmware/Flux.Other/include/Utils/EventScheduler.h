// Karim Ladha 23-11-2016
// Created to add timed events to the SysTime module similar to TaskScheduler.c
// SysTime permits callbacks to run at high interrupt priority at a given rate, period or every second
// The TaskScheduler creates tasks that run at any scope in the Tasks() call at a given time in seconds or periodically
// The EventScheduler enables multiple callbacks from the RTC interrupt or from a Task() call with sub-millisecond accuracy  
// N.b. Compatibility to Nordic's SDK "app_timer.c" is by design to facilitate library reuse
#ifndef EVENT_SCHEDULER_H
#define EVENT_SCHEDULER_H

// Headers
#include <stdint.h>			 

// Definitions - Used for reschedValue to determine behaviour on expiry
#define EVTSCHED_RESCHEDULE_NO_REPEAT		(0UL)	// Used for period argument if event is not recurring
#define EVTSCHED_RESCHEDULE_NO_CB_IN_ISR	(-1L)	// Used where the event tick should never expire

// Types
// Event callbacks
typedef void (*EventSchedulerCB_t)(void* ctx);
// Private, event scheduler stateEVT_SCHED_STATE_ADD
typedef enum {
	// State of event timer
	EVT_SCHED_STATE_UNUSED	= 0,	// Reset, un-initialized or unused state
	EVT_SCHED_STATE_INIT	= 1,	// Initialized inactive state, used event	
	EVT_SCHED_STATE_ADD		= 2,	// Event configured ready to add to the queue
	EVT_SCHED_STATE_REMOVE	= 3,	// The event should be removed from queue		
	EVT_SCHED_STATE_CB_DUE	= 4,	// Timer expired. Due to call-back externally
	EVT_SCHED_STATE_QUEUED	= 5,	// The event was added into the queue
	EVT_SCHED_STATE_ACTIVE	= 6,	// The next event due. The timer is for this event
} EventSchedulerState_t;
// Private, scheduled event instance. Do not modify...
typedef struct ScheduledEvent_tag {
	EventSchedulerState_t state;	// Scheduled event state
	uint32_t scheduledTick;			// RTC counter value when event callback becomes required
	uint32_t reschedValue;			// Set behavior after timer expiry e.g. restart timer, call callback 
	EventSchedulerCB_t cb;			// Scheduled event callback pointer
	void* ctx;						// Context pointer used for callback
} ScheduledEvent_t;

// Globals
// List of scheduled event handles (For debug visibility only)
extern ScheduledEvent_t* scheduledEvents[];
extern volatile uint16_t gEventQueueLength;

// Prototypes
// Initialise the event scheduler module
void EventSchedulerInit(void);

// Only required if events are set to use external CB context
void EventSchedulerTasks(void);

// Stop the scheduler module
void EventSchedulerShutdown(void);

// Initialise the event structure. It's instance is instantiated externally
void EventSchedulerSetup(ScheduledEvent_t* event, uint32_t reschedValue, EventSchedulerCB_t cb, void* ctx);

// Add the event to the scheduler queue to complete after specified ticks
uint8_t EventSchedulerStart(ScheduledEvent_t* event, uint32_t delay);

// Remove a pending or recurring event from the queueuint8_t EventSchedulerStart(ScheduledEvent_t* event, uint32_t delay)
uint8_t EventSchedulerStop(ScheduledEvent_t* evt);

#endif
