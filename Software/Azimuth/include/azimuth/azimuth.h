// Azimuth - IMU Processor
// Dan Jackson, 2013
// 

#ifndef AZIMUTH_H
#define AZIMUTH_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>

#include "azimuth/ahrs.h"
#include "azimuth/mathutil.h"


	struct azimuth_struct_t;

	// Azimuth Update Callback Function
	typedef void(*azimuth_update_callback_t)(struct azimuth_struct_t *azimuth, void *reference);

	// Azimuth Container Structure
	typedef struct azimuth_struct_t
	{
		// Public
		float accel[3];
		float gyro[3];
		float mag[3];
		float magMin[3];
		float magMax[3];
		float quat[4];
		float euler[3];
		float ypr[3];
		float gravity[3];
		float matrixRotation[16];
		float matrixInvRotation[16];
		float relAccel[3];
		float linearAccel[3];
		float screen[3];        // 

		int temperature;
		unsigned int pressure, pressureOffset;
		unsigned int battery;

		// Private
		int fd;                 // Open port
		ahrs_t ahrs;            // AHRS tracker
		FILE *outfp;            // Output file
		int deviceMode;         // Hint for USB/Bluetooth optimizations
		float frequency;
		int sampleCount;

		// Background update
		void *backgroundThread;
		char quitBackgroundPoll;
		azimuth_update_callback_t updateCallback;
		void *updateCallbackReference;
	} azimuth_t;


#define AZIMUTH_DEFAULT_BETA 0.1f

	// Initialize
	int AzimuthInit(azimuth_t *azimuth, const char *infile, const char *initString, int sampleRate, float beta);

	// Close
	void AzimuthClose(azimuth_t *azimuth);

	// Reset
	void AzimuthReset(azimuth_t *azimuth);

	// Poll
	int AzimuthPoll(azimuth_t *azimuth);

	// Update callback function
	void AzimuthSetCallback(azimuth_t *azimuth, azimuth_update_callback_t callback, void *reference);

	// Background polling
	void AzimuthStartBackgroundPoll(azimuth_t *azimuth);
	void AzimuthStopBackgroundPoll(azimuth_t *azimuth);

	// Send command
	int AzimuthSend(azimuth_t *azimuth, const unsigned char *buffer, size_t length);



	// Utility
	unsigned long long TicksNow(void);

#ifdef __cplusplus
}
#endif


#endif
