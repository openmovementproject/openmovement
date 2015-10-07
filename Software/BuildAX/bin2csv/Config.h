
// Configuration file for BaxTestHarness.exe

#ifndef CONFIG_H_
#define CONFIG_H_

// Includes
#include <stdio.h>

// Debugging
#define ERROR_FILE	"ERRORS.TXT"
#define GLOBAL_DEBUG_LEVEL 2

// Send command list to radio, null or SI44_CMD_EOL terminated
#define Si44CommandList(...)
// Send single command to radio
#define Si44Command(...) NULL
// Callback for events
#define Si44SetEventCB(...)

#define Si44RadioState 0

// Generic state type
typedef enum {
	NOT_PRESENT = -2,
	ERROR_STATE = -1,
	OFF_STATE = 0,		
	STATE_OFF = 0,	/*Settings - OFF*/
	PRESENT = 1,
	STATE_ON = 1,	/*Settings - ON*/
	INITIALISED = 2,
	ACTIVE_STATE = 3
} state_t;



#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

// Settings struct
typedef struct Settings_tag{
	// Input/Output
	FILE* inputFile;
	FILE* outputFile;

	unsigned char printEncryptedPackets;
} Settings_t;

typedef struct {
	state_t app_state;
	state_t radio_state;
}Status_t;

// Globals
extern Settings_t gSettings;
extern Status_t gStatus;

// Types
typedef int (*GetByte_t)(struct Settings_tag* settings);
typedef int (*PutByte_t)(struct Settings_tag*, unsigned char b);

// Exit error handler
void ErrorExit(const char* fmt,...);

#endif
