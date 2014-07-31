/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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

// Configuration settings, status and command handling
// Dan Jackson, 2011-2012
#include "HardwareProfile.h"

// Sensor settings
typedef struct
{
    char stream;
    char enabled;
    unsigned short frequency;
    unsigned short sensitivity;
    unsigned short options;
} sensor_config_t;

// Sensor index - IMPORTANT: Must be in the same order as structures in Record.c and bitfield dataStreamFlags_t in Sampler.h
#define SENSOR_INDEX_ACCEL  0
#define SENSOR_INDEX_GYRO   1
#define SENSOR_INDEX_MAG    2
#define SENSOR_INDEX_ALT    3
#define SENSOR_INDEX_ADC    4
#define NUM_SENSORS         5

#ifdef SYNCHRONOUS_SAMPLING
	// Update number fo sensors to 6 if supporting synchronous sampling
	#define SENSOR_INDEX_ALLAXIS 5
	#undef NUM_SENSORS
	#define NUM_SENSORS          6
#endif

#define STREAM_MODE_NONE 0
#define STREAM_MODE_WAX_SLIP 1
#define STREAM_MODE_TEXT 2
//#define STREAM_MODE_OSC  3
//#define STREAM_MODE_HEX  4
#define STREAM_MODE_OMX_SLIP 5

// Device settings for startup
typedef struct
{
	// Device 
    unsigned short deviceId;            // Device identifier (stored in ROM)

	// Logging: Settings
    unsigned long sessionId;            // Session identifier
    unsigned long loggingStartTime;     // Date/time to start logging
    unsigned long loggingEndTime;       // Date/time to end logging
    sensor_config_t sensorConfig[NUM_SENSORS];  // Sensor configuration
    //unsigned long lastChangeTime;       // Date/time of last metadata change
    //short timeZone;                     // RTC timezone offset from UTC in minutes (-1 = unknown)
    #define ANNOTATION_SIZE 32				// bank size
    #define ANNOTATION_COMPATABILITY_COUNT 14	// 14 banks from old implementation
    #define ANNOTATION_COUNT 6					// 6 (192/32) banks actually stored

    char annotation[ANNOTATION_COUNT][ANNOTATION_SIZE]; // 14x 32-byte annotation chunks

	// Misc
    unsigned char debuggingInfo;        // Debug flashing of state (0 = off, 1 = constant, 2 = on tap)
    short streamFormat;                 // 0 = old, 1 = WAX SLIP, 2 = TEXT, 5 = OMX SLIP

    unsigned short batteryLowTimer;     // Battery low counter
} Settings;

extern Settings settings;




// === SYSTEM STATES ===

// USB state: not connected, powered, connected
typedef enum { CONNECTION_NONE, CONNECTION_POWER, CONNECTION_COMPUTER } StateConnection;

// Battery state: low, normal, charging, full
typedef enum { BATTERY_EMPTY = -2, BATTERY_WARNING = -1, BATTERY_NORMAL = 0, BATTERY_CHARGING = 1, BATTERY_FULL = 2 } StateBattery;

// Stay in discoverable wireless state: never, for a while after last movement, always.
typedef enum { DISCOVERABLE_NEVER = 0, DISCOVERABLE_ALWAYS = 1, DISCOVERABLE_AUTO = 2 } StateDiscoverable;

// Wireless state: discoverable, off, connected
typedef enum { WIRELESS_UNKNOWN = -1, WIRELESS_OFF = 0, WIRELESS_DISCOVERABLE = 1, WIRELESS_CONNECTED = 2 } StateWireless;

// Drive state: unknown state, normal state, disk full, mounted by OS
typedef enum { DRIVE_ERROR = -1, DRIVE_UNOWNED = 0, DRIVE_OWNED, DRIVE_OWNED_FULL, DRIVE_MOUNTED } StateDrive;


// State changing functions
char StateSetDrive(StateDrive newState, char force, unsigned short stopReason);
void StateSetWireless(StateWireless newState);



// Device logging status
typedef struct
{
    // Status
    //char attached;                    // USB attached (>0 = configured)
    unsigned short initialBattery;      // Battery level at start (used for battery health)

    // System settings
    StateDiscoverable discoverable;

    // System state
    StateConnection connection;
    StateBattery battery;
    StateWireless wireless;
    StateDrive drive;
    int batteryLowTimer;

    // Status: attached
    char batteryFullTimer;              // Battery fully charged timer
    char ledOverride;                   // Set LED colour (-1 = automatic)

    // Delayed action
    char actionFlags;                   // Settings for delayed actions
    char actionCountdown;               // Countdown to delayed action

    // Timers
    volatile unsigned short inactivity;      // Time since last motion (seconds)
    unsigned short disconnectTimer;          // Time since last disconnect (seconds)
    unsigned short alertTimer;          	 // Time since last alert (seconds)

    // Configuration read from NVM
    unsigned short lockCode;            // Comms lock when non-zero
    char dataEcc;                       // Apply error-correction code to recorded data sectors
    char filespec[16];					// Filename "DATA????.OMX\0" (keep NULL-padded)
    char folder[16];					// Folder "DATA\0"
	short calibration[4][8];			// 4x 8x calibration words
} Status;

extern Status status;





#define BATT_FULL_INTERVAL 60       // 60 seconds over 'full' level before reporting battery as full
#define BATT_EMPTY_INTERVAL 5       // 5 seconds below 'empty' level before reporting battery as low
#define CONFIG_ECC_DEFAULT 1        // 0 = ECC off for data sectors, 1 = ECC on for data sectors


// Logging format
//#define FORMAT_NONE            0x00
//#define FORMAT_CSV             0x01
//#define FORMAT_CWA_PACKED      0x02
//#define FORMAT_CWA_UNPACKED    0x04
//#define FORMAT_SINGLE_FILE     0x10
//#define FORMAT_MASK_TYPE       0x0f
//#define FORMAT_MASK_OPTIONS    0xf0



typedef enum { SETTINGS_BATCH, SETTINGS_USB, SETTINGS_ALTERNATE } SettingsMode;

void SettingsInitialize(void);
char SettingsReadFile(const char *filename);
char SettingsCommand(const char *line, SettingsMode mode);


#define ACTION_FORMAT_WIPE     0x01
#define ACTION_FORMAT_QUICK    0x02
#define ACTION_DELETE          0x04
#define ACTION_SAVE            0x08
#define ACTION_CREATE          0x10
#define ACTION_REMOUNT         0x20
#define ACTION_RESTART         0x40

char SettingsAction(char flags);


#define LOG_SIZE 32                 // "*TTTT##########################0"
#define LOG_COUNT 15
/*
// Device history/log
typedef struct
{
    unsigned short restartCount;        // Restart count
    unsigned short rechargeCount;       // Number of recharge cycles
    unsigned short reserved[14];        // 28 bytes of reserved space

    char log[LOG_COUNT][LOG_SIZE];      // 15x 32-byte log chunks
} Log;
*/
#define LOG_VALUE_RESTART 0
#define LOG_VALUE_BATTERY 1

#define LOG_CATEGORY_USER 0x0100
#define LOG_CATEGORY_STOP 0x0200

unsigned short SettingsGetLogValue(unsigned int index);
unsigned short SettingsIncrementLogValue(unsigned int index);
void SettingsAddLogEntry(unsigned short status, unsigned long timestamp, const char *message);
const char *SettingsGetLogEntry(int index, unsigned short *status, unsigned long *timestamp);


// Configuration
//#define CONFIG_LOCK 0
//#define CONFIG_ECC  1
//#define CONFIG_UNDEFINED 0xffff

// Configuration structure
typedef struct
{
	unsigned short lockCode;			// @ 0 lock code
    unsigned short _reserved1;          // @ 2(old systems was inverse lock code)
	unsigned short ecc;					// @ 4 ecc parameter
    unsigned short _reserved2;          // @ 6 (old systems was inverse ecc)
    short discoverable;                 // @ 8 Discoverable
	unsigned short reserved[11];		// @10 11*2 -- reserved-short values
	char filespec[16];					// @32 8.3 (keep null-padded)
	char folder[16];					// @48 8.3
	short calibration[4][8];			// @64 4x 8x calibration words
	// @112 ...
} config_t;
// Configuration macros (offset similar to stddef.h's offsetof() macro)
#define CONFIG_SIZEOF(_structure, _member)   ((size_t)sizeof(((_structure*)0)->_member))
#define CONFIG_OFFSETOF(_structure, _member) ((size_t)&(((_structure*)0)->_member))
#define CONFIG_GET(_member, _value) SettingsGetConfig(CONFIG_OFFSETOF(config_t, _member), (_value), CONFIG_SIZEOF(config_t, _member))
#define CONFIG_SET(_member, _value) SettingsSetConfig(CONFIG_OFFSETOF(config_t, _member), (_value), CONFIG_SIZEOF(config_t, _member))
char SettingsGetConfig(size_t offset, void *buffer, size_t len);
char SettingsSetConfig(size_t offset, const void *buffer, size_t len);
//unsigned short SettingsGetConfigValue(unsigned int index);
//char SettingsSetConfigValue(unsigned int index, unsigned short value);

    

// CWA constants
#define SETTINGS_FILE "\\SETTINGS.INI"
#define DATA_FILE "DATA????.OMX"
#define DATA_FOLDER "" //"\\DATA"
//#define METADATA_FILE "METADATA.BIN"
