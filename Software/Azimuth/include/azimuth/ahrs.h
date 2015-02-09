// AHRS - Attitude and heading reference system
// Dan Jackson, 2013
//
// Container wrapper and utility methods for Sebastian Madgwick's 2009 code.


#ifndef AHRS_H
#define AHRS_H

#ifdef __cplusplus
extern "C" {
#endif


// AHRS Container Structure
typedef struct ahrs_struct_t
{
    // Public (read-only)
    char mode;              // Mode 0 is Madgwick, mode 1 is Mahony
    float sampleFreq;       // Sample frequency in Hz
    float q[4];             // Quaternion of sensor frame relative to auxiliary frame
    
    // Private
    float twoKp;            // 2 * proportional gain ("beta" in Madgwick, "Kp" in Mahony)
    float twoKi;            // [Mahony] 2 * integral gain (not used in Madgwick, "Ki" in Mahony)
    float integralFB[3];    // [Mahony] Integral error terms scaled by Ki (not used in Madgwick)
} ahrs_t;


// Initialize AHRS tracker
void AhrsInit(ahrs_t *ahrs, char mode, float frequency, float beta);

// Reset AHRS tracker
void AhrsReset(ahrs_t *ahrs);

// Get the quaternion from the AHRS tracker
float *AhrsGetQuaternion(ahrs_t *ahrs);

// Update the AHRS tracker with accelerometer, gyroscope and (optional) magnetometer data
void AhrsUpdate(ahrs_t *ahrs, float *gyro, float *accel, float *mag);


#ifdef __cplusplus
}
#endif

#endif
