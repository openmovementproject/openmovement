// Karim Ladha, 2015
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

// Universal state type
typedef enum {
	STATE_UNKNOWN = -2,
	STATE_ERROR = -1,
	STATE_OFF	= 0,
	STATE_ON 	= 1,	
	STATE_ENABLED = 2,
	STATE_ACTIVE = 3
}state_t;

// Device settings structure used in eeprom and ram
typedef struct
{
	// Device
    unsigned short deviceId;            // Device identifier 

	// Streaming settings
	unsigned char sleepMode;			// 0. No sleep + run on USB power, 1. Low power discoverable, 2. Wake on movement, 3. Wake on rotate, 4. Hibernate until USB connect
	unsigned short inactivityThreshold;	// Uses movement to detect inactivity
    unsigned short sampleRate;          // Sample rate code
    unsigned char dataMode;             // Data output mode 0. ascii, 1. binary
	unsigned char highPerformance;		// Use PLL oscillator esp for ascii mode + high rate
	unsigned short txLenThreshold;		// Must be 512,256,128,64 or 0

	// Bluetooth
	unsigned char runBt;

	// Stream setting
	unsigned char doStream;

	// Sensor settings
	unsigned short accelOn;
	unsigned short accelRate;
	unsigned short accelRange;
	unsigned short gyroOn;
	unsigned short gyroRate;
	unsigned short gyroRange;
	unsigned short magOn;
	unsigned short magRate;

	// Set by selected sleep mode 
	unsigned short accelInts;

	// Checksum
	unsigned short checksum;
}settings_t;

typedef struct {
	// State
	unsigned char sleepMode;			// 0. No sleep + run on USB power, 1. Low power discoverable, 2. Wake on movement, 3. Wake on rotate, 4. Hibernate until USB connect
	unsigned char streaming;

	// Sensor data	
	unsigned char accelIntPending;
	unsigned long pressure;
	signed short temperature;
	unsigned short battmv;
	unsigned short inactivity;
	unsigned char newXdata;

	// LED
	signed char ledOverride;

	// Bluetooth
	state_t			btState;

	// State of main loop
	unsigned char runMain;

	// USB
	unsigned char 	usbAtStart;
	state_t 		usbState;
}status_t;

extern settings_t settings;
extern status_t status;


char SettingsCommand(const char *line);
char SettingsCommandNumber(unsigned short num);
void SettingsInitialize(unsigned char wipe);
void SettingsSave(void);

// Helper functions
unsigned short FindChecksum (void* ptr, unsigned short len);
signed long ReadStringInteger(char* ptr,char ** end);

#endif
