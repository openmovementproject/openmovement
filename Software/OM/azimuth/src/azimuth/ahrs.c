// AHRS - Attitude and heading reference system
// Dan Jackson, 2013
//
// Container wrapper and utility methods for Sebastian Madgwick's 2009 code.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms

//=====================================================================================================
// From MadgwickAHRS.c & MahonyAHRS.c
//=====================================================================================================
//
// Implementation of Madgwick's IMU and AHRS algorithms, and Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
// 19/02/2012	SOH Madgwick	[Madgwick] Magnetometer measurement is normalised
//
//=====================================================================================================


#ifdef _WIN32
#define _USE_MATH_DEFINES       // For M_PI
#endif

#include <math.h>

#include "azimuth/ahrs.h"
//#include "azimuth/mathutil.h"


// Fast inverse square-root. See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
static float invSqrt(float x)
{
#if 1
    float v = (float)sqrt(x);
    if (v == 0) { return v; }
    return (1.0f / v);
#else
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
#endif
}


//=====================================================================================================
// AHRS.c
// S.O.H. Madgwick
// 25th August 2010
// --- Modifications by Dan Jackson, 2011
//=====================================================================================================
// Description:
//
// Quaternion implementation of the 'DCM filter' [Mayhony et al].  Incorporates the magnetic distortion
// compensation algorithms from my filter [Madgwick] which eliminates the need for a reference
// direction of flux (bx bz) to be predefined and limits the effect of magnetic distortions to yaw
// axis only.
//
// User must define 'halfT' as the (sample period / 2), and the filter gains 'Kp' and 'Ki'.
//
// 'q0', 'q1', 'q2', 'q3' are the quaternion elements representing the estimated
// orientation.  See my report for an overview of the use of quaternions in this application.
//
// User must call 'AHRSupdate()' every sample period and parse calibrated gyroscope ('gx', 'gy', 'gz'),
// accelerometer ('ax', 'ay', 'ay') and magnetometer ('mx', 'my', 'mz') data.  Gyroscope units are
// degrees/second, accelerometer and magnetometer units are irrelevant as the vector is normalised.
//
//=====================================================================================================
void MayhonyOldAHRSupdate(ahrs_t *ahrs, float *gyro, float *accel, float *mag)
{
	float norm;
	float hx, hy, hz, bx, bz;
	float vx, vy, vz, wx, wy, wz;
	float ex, ey, ez;
	float gx =  gyro[0], gy =  gyro[1], gz =  gyro[2];
	float ax = accel[0], ay = accel[1], az = accel[2];
	float mx = (!mag) ? 0 : mag[0], my = (!mag) ? 0 : mag[1], mz = (!mag) ? 0 : mag[2];
    float *q = ahrs->q;
    float Ki = ahrs->twoKi / 2;
    float Kp = ahrs->twoKp / 2;
    float halfT = (1.0f / ahrs->sampleFreq) / 2;

	// auxiliary variables to reduce number of repeated operations
	float q0q0 = q[0]*q[0];
	float q0q1 = q[0]*q[1];
	float q0q2 = q[0]*q[2];
	float q0q3 = q[0]*q[3];
	float q1q1 = q[1]*q[1];
	float q1q2 = q[1]*q[2];
	float q1q3 = q[1]*q[3];
	float q2q2 = q[2]*q[2];   
	float q2q3 = q[2]*q[3];
	float q3q3 = q[3]*q[3];          

	// normalise the measurements
	norm = (float)sqrt(ax*ax + ay*ay + az*az); 
	if (norm != 0.0f) { ax = ax / norm; ay = ay / norm; az = az / norm; }

	norm = (float)sqrt(mx*mx + my*my + mz*mz);          
	if (norm != 0.0f) { mx = mx / norm; my = my / norm; mz = mz / norm; }

	// compute reference direction of flux
	hx = 2*mx*(0.5f - q2q2 - q3q3) + 2*my*(q1q2 - q0q3) + 2*mz*(q1q3 + q0q2);
	hy = 2*mx*(q1q2 + q0q3) + 2*my*(0.5f - q1q1 - q3q3) + 2*mz*(q2q3 - q0q1);
	hz = 2*mx*(q1q3 - q0q2) + 2*my*(q2q3 + q0q1) + 2*mz*(0.5f - q1q1 - q2q2);         
	bx = (float)sqrt((hx*hx) + (hy*hy));
	bz = hz;     

	// estimated direction of gravity and flux (v and w)
	vx = 2*(q1q3 - q0q2);
	vy = 2*(q0q1 + q2q3);
	vz = q0q0 - q1q1 - q2q2 + q3q3;
	wx = 2*bx*(0.5f - q2q2 - q3q3) + 2*bz*(q1q3 - q0q2);
	wy = 2*bx*(q1q2 - q0q3) + 2*bz*(q0q1 + q2q3);
	wz = 2*bx*(q0q2 + q1q3) + 2*bz*(0.5f - q1q1 - q2q2);  

    // [dgj] Missing magnetometer
    if (norm == 0.0f) { wx = 0.0f; wy = 0.0f; wz = 0.0f; }

	// error is sum of cross product between reference direction of fields and direction measured by sensors
	ex = (ay * vz - az * vy) + (my * wz - mz * wy);
	ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
	ez = (ax * vy - ay * vx) + (mx * wy - my * wx);

	// integral error scaled integral gain
	ahrs->integralFB[0] = ahrs->integralFB[0] + ex * Ki;
	ahrs->integralFB[1] = ahrs->integralFB[1] + ey * Ki;
	ahrs->integralFB[2] = ahrs->integralFB[2] + ez * Ki;

	// adjusted gyroscope measurements
	gx = gx + Kp * ex + ahrs->integralFB[0];
	gy = gy + Kp * ey + ahrs->integralFB[1];
	gz = gz + Kp * ez + ahrs->integralFB[2];

	// integrate quaternion rate and normalise
	q[0] = q[0] + (-q[1] * gx - q[2] * gy - q[3] * gz) * halfT;
	q[1] = q[1] + ( q[0] * gx + q[2] * gz - q[3] * gy) * halfT;
	q[2] = q[2] + ( q[0] * gy - q[1] * gz + q[3] * gx) * halfT;
	q[3] = q[3] + ( q[0] * gz + q[1] * gy - q[2] * gx) * halfT;  

	// normalise quaternion
	norm = (float)sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (norm != 0)
    {
	    q[0] = q[0] / norm;
	    q[1] = q[1] / norm;
	    q[2] = q[2] / norm;
	    q[3] = q[3] / norm;
    }
}


// [Madgwick] IMU algorithm update
static void MadgwickAHRSupdateIMU(ahrs_t *ahrs, float *gyro, float *accel) 
{
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;
    float *q = ahrs->q;
    float ax = accel[0], ay = accel[1], az = accel[2];
    float gx = gyro[0], gy = gyro[1], gz = gyro[2];

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q[1] * gx - q[2] * gy - q[3] * gz);
	qDot2 = 0.5f * (q[0] * gx + q[2] * gz - q[3] * gy);
	qDot3 = 0.5f * (q[0] * gy - q[1] * gz + q[3] * gx);
	qDot4 = 0.5f * (q[0] * gz + q[1] * gy - q[2] * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
    {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * q[0];
		_2q1 = 2.0f * q[1];
		_2q2 = 2.0f * q[2];
		_2q3 = 2.0f * q[3];
		_4q0 = 4.0f * q[0];
		_4q1 = 4.0f * q[1];
		_4q2 = 4.0f * q[2];
		_8q1 = 8.0f * q[1];
		_8q2 = 8.0f * q[2];
		q0q0 = q[0] * q[0];
		q1q1 = q[1] * q[1];
		q2q2 = q[2] * q[2];
		q3q3 = q[3] * q[3];

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q[1] - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * q[2] + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * q[3] - _2q1 * ax + 4.0f * q2q2 * q[3] - _2q2 * ay;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= ahrs->twoKp * s0;
		qDot2 -= ahrs->twoKp * s1;
		qDot3 -= ahrs->twoKp * s2;
		qDot4 -= ahrs->twoKp * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q[0] += qDot1 * (1.0f / ahrs->sampleFreq);
	q[1] += qDot2 * (1.0f / ahrs->sampleFreq);
	q[2] += qDot3 * (1.0f / ahrs->sampleFreq);
	q[3] += qDot4 * (1.0f / ahrs->sampleFreq);

	// Normalise quaternion
	recipNorm = invSqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	q[0] *= recipNorm;
	q[1] *= recipNorm;
	q[2] *= recipNorm;
	q[3] *= recipNorm;
}


// [Madgwick] AHRS algorithm update
static void MadgwickAHRSupdate(ahrs_t *ahrs, float *gyro, float *accel, float *mag)
{
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
    float *q = ahrs->q;
    float ax = accel[0], ay = accel[1], az = accel[2];
    float gx = gyro[0], gy = gyro[1], gz = gyro[2];
    float mx = mag[0], my = mag[1], mz = mag[2];

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))
    {
		MadgwickAHRSupdateIMU(ahrs, gyro, accel);
		return;
	}

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q[1] * gx - q[2] * gy - q[3] * gz);
	qDot2 = 0.5f * (q[0] * gx + q[2] * gz - q[3] * gy);
	qDot3 = 0.5f * (q[0] * gy - q[1] * gz + q[3] * gx);
	qDot4 = 0.5f * (q[0] * gz + q[1] * gy - q[2] * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
    {
		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = 2.0f * q[0] * mx;
		_2q0my = 2.0f * q[0] * my;
		_2q0mz = 2.0f * q[0] * mz;
		_2q1mx = 2.0f * q[1] * mx;
		_2q0 = 2.0f * q[0];
		_2q1 = 2.0f * q[1];
		_2q2 = 2.0f * q[2];
		_2q3 = 2.0f * q[3];
		_2q0q2 = 2.0f * q[0] * q[2];
		_2q2q3 = 2.0f * q[2] * q[3];
		q0q0 = q[0] * q[0];
		q0q1 = q[0] * q[1];
		q0q2 = q[0] * q[2];
		q0q3 = q[0] * q[3];
		q1q1 = q[1] * q[1];
		q1q2 = q[1] * q[2];
		q1q3 = q[1] * q[3];
		q2q2 = q[2] * q[2];
		q2q3 = q[2] * q[3];
		q3q3 = q[3] * q[3];

		// Reference direction of Earth's magnetic field
		hx = mx * q0q0 - _2q0my * q[3] + _2q0mz * q[2] + mx * q1q1 + _2q1 * my * q[2] + _2q1 * mz * q[3] - mx * q2q2 - mx * q3q3;
		hy = _2q0mx * q[3] + my * q0q0 - _2q0mz * q[1] + _2q1mx * q[2] - my * q1q1 + my * q2q2 + _2q2 * mz * q[3] - my * q3q3;
		_2bx = (float)sqrt(hx * hx + hy * hy);
		_2bz = -_2q0mx * q[2] + _2q0my * q[1] + mz * q0q0 + _2q1mx * q[3] - mz * q1q1 + _2q2 * my * q[3] - mz * q2q2 + mz * q3q3;
		_4bx = 2.0f * _2bx;
		_4bz = 2.0f * _2bz;

		// Gradient decent algorithm corrective step
		s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q[2] * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q[3] + _2bz * q[1]) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q[2] * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s1 =  _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q[1] * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q[3] * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q[2] + _2bz * q[0]) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q[3] - _4bz * q[1]) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q[2] * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q[2] - _2bz * q[0]) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q[1] + _2bz * q[3]) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q[0] - _4bz * q[2]) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s3 =  _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q[3] + _2bz * q[1]) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q[0] + _2bz * q[2]) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q[1] * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= ahrs->twoKp * s0;
		qDot2 -= ahrs->twoKp * s1;
		qDot3 -= ahrs->twoKp * s2;
		qDot4 -= ahrs->twoKp * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q[0] += qDot1 * (1.0f / ahrs->sampleFreq);
	q[1] += qDot2 * (1.0f / ahrs->sampleFreq);
	q[2] += qDot3 * (1.0f / ahrs->sampleFreq);
	q[3] += qDot4 * (1.0f / ahrs->sampleFreq);

	// Normalise quaternion
	recipNorm = invSqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	q[0] *= recipNorm;
	q[1] *= recipNorm;
	q[2] *= recipNorm;
	q[3] *= recipNorm;
}


// [Mahony] IMU algorithm update
static void MahonyAHRSupdateIMU(ahrs_t *ahrs, float *gyro, float *accel)
{
	float recipNorm;
	float halfvx, halfvy, halfvz;
	float halfex, halfey, halfez;
	float qa, qb, qc;
    float *q = ahrs->q;
    float ax = accel[0], ay = accel[1], az = accel[2];
    float gx = gyro[0], gy = gyro[1], gz = gyro[2];

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
    {
		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;        

		// Estimated direction of gravity and vector perpendicular to magnetic flux
		halfvx = q[1] * q[3] - q[0] * q[2];
		halfvy = q[0] * q[1] + q[2] * q[3];
		halfvz = q[0] * q[0] - 0.5f + q[3] * q[3];
	
		// Error is sum of cross product between estimated and measured direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if (ahrs->twoKi > 0.0f)
        {
			ahrs->integralFB[0] += ahrs->twoKi * halfex * (1.0f / ahrs->sampleFreq);	// integral error scaled by Ki
			ahrs->integralFB[1] += ahrs->twoKi * halfey * (1.0f / ahrs->sampleFreq);
			ahrs->integralFB[2] += ahrs->twoKi * halfez * (1.0f / ahrs->sampleFreq);
			gx += ahrs->integralFB[0];	// apply integral feedback
			gy += ahrs->integralFB[1];
			gz += ahrs->integralFB[2];
		}
		else 
        {
			ahrs->integralFB[0] = 0.0f;	// prevent integral windup
			ahrs->integralFB[1] = 0.0f;
			ahrs->integralFB[2] = 0.0f;
		}

		// Apply proportional feedback
		gx += ahrs->twoKp * halfex;
		gy += ahrs->twoKp * halfey;
		gz += ahrs->twoKp * halfez;
	}
	
	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / ahrs->sampleFreq));		// pre-multiply common factors
	gy *= (0.5f * (1.0f / ahrs->sampleFreq));
	gz *= (0.5f * (1.0f / ahrs->sampleFreq));
	qa = q[0];
	qb = q[1];
	qc = q[2];
	q[0] += (-qb * gx - qc * gy - q[3] * gz);
	q[1] += (qa * gx + qc * gz - q[3] * gy);
	q[2] += (qa * gy - qb * gz + q[3] * gx);
	q[3] += (qa * gz + qb * gy - qc * gx); 
	
	// Normalise quaternion
	recipNorm = invSqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	q[0] *= recipNorm;
	q[1] *= recipNorm;
	q[2] *= recipNorm;
	q[3] *= recipNorm;
}


// [Mahony] AHRS algorithm update
static void MahonyAHRSupdate(ahrs_t *ahrs, float *gyro, float *accel, float *mag)
{
	float recipNorm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;  
	float hx, hy, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float qa, qb, qc;
    float *q = ahrs->q;
    float ax = accel[0], ay = accel[1], az = accel[2];
    float gx = gyro[0], gy = gyro[1], gz = gyro[2];
    float mx = mag[0], my = mag[1], mz = mag[2];

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) 
    {
		MahonyAHRSupdateIMU(ahrs, gyro, accel);
		return;
	}

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) 
    {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;     

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;   

        // Auxiliary variables to avoid repeated arithmetic
        q0q0 = q[0] * q[0];
        q0q1 = q[0] * q[1];
        q0q2 = q[0] * q[2];
        q0q3 = q[0] * q[3];
        q1q1 = q[1] * q[1];
        q1q2 = q[1] * q[2];
        q1q3 = q[1] * q[3];
        q2q2 = q[2] * q[2];
        q2q3 = q[2] * q[3];
        q3q3 = q[3] * q[3];   

        // Reference direction of Earth's magnetic field
        hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
        hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
        bx = (float)sqrt(hx * hx + hy * hy);
        bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

		// Estimated direction of gravity and magnetic field
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - 0.5f + q3q3;
        halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
        halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
        halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);  
	
// [dgj] Missing magnetometer
if (mx == 0 && my == 0 && mz == 0)
{
	halfwx = 0.0f; halfwy = 0.0f; halfwz = 0.0f;
}

		// Error is sum of cross product between estimated direction and measured direction of field vectors
		halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
		halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
		halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

		// Compute and apply integral feedback if enabled
		if (ahrs->twoKi > 0.0f) 
        {
			ahrs->integralFB[0] += ahrs->twoKi * halfex * (1.0f / ahrs->sampleFreq);	// integral error scaled by Ki
			ahrs->integralFB[1] += ahrs->twoKi * halfey * (1.0f / ahrs->sampleFreq);
			ahrs->integralFB[2] += ahrs->twoKi * halfez * (1.0f / ahrs->sampleFreq);
			gx += ahrs->integralFB[0];	// apply integral feedback
			gy += ahrs->integralFB[1];
			gz += ahrs->integralFB[2];
		}
		else 
        {
			ahrs->integralFB[0] = 0.0f;	// prevent integral windup
			ahrs->integralFB[1] = 0.0f;
			ahrs->integralFB[2] = 0.0f;
		}

		// Apply proportional feedback
		gx += ahrs->twoKp * halfex;
		gy += ahrs->twoKp * halfey;
		gz += ahrs->twoKp * halfez;
	}
	
	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / ahrs->sampleFreq));		// pre-multiply common factors
	gy *= (0.5f * (1.0f / ahrs->sampleFreq));
	gz *= (0.5f * (1.0f / ahrs->sampleFreq));
	qa = q[0];
	qb = q[1];
	qc = q[2];
	q[0] += (-qb * gx - qc * gy - q[3] * gz);
	q[1] += (qa * gx + qc * gz - q[3] * gy);
	q[2] += (qa * gy - qb * gz + q[3] * gx);
	q[3] += (qa * gz + qb * gy - qc * gx); 
	
	// Normalise quaternion
	recipNorm = invSqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	q[0] *= recipNorm;
	q[1] *= recipNorm;
	q[2] *= recipNorm;
	q[3] *= recipNorm;
}


// Initialize AHRS tracker
void AhrsInit(ahrs_t *ahrs, char mode, float frequency, float beta)
{
    ahrs->mode = mode;
    ahrs->sampleFreq = frequency;
    ahrs->twoKp = beta;     // e.g. For 512Hz: 0.1 in Madgwick, 1.0 in Mahony (old: 10*2)
    ahrs->twoKi = 0.0f;     // e.g. 0 in Mahony (old: 0.005*2)
    AhrsReset(ahrs);
}


// Reset AHRS tracker
void AhrsReset(ahrs_t *ahrs)
{
    ahrs->q[0] = 1.0f; ahrs->q[1] = 0.0f; ahrs->q[2] = 0.0f; ahrs->q[3] = 0.0f; 
    ahrs->integralFB[0] = 0.0f; ahrs->integralFB[1] = 0.0f; ahrs->integralFB[2] = 0.0f; 
}


// Get the quaternion from the AHRS tracker
float *AhrsGetQuaternion(ahrs_t *ahrs)
{
    return ahrs->q;
}


// Update the AHRS tracker with accelerometer, gyroscope and (optional) magnetometer data
void AhrsUpdate(ahrs_t *ahrs, float *gyro, float *accel, float *mag)
{
    if (ahrs->mode == -1)
    {
        MayhonyOldAHRSupdate(ahrs, gyro, accel, mag);
    }
    else if (ahrs->mode == 1)    // 1 = Mayhony
    {
        if (!mag)
        {
            MahonyAHRSupdateIMU(ahrs, gyro, accel);
        }
        else
        {
            MahonyAHRSupdate(ahrs, gyro, accel, mag);
        }
    }
    else                    // Default = Madgwick
    {
        if (!mag)
        {
            MadgwickAHRSupdateIMU(ahrs, gyro, accel);
        }
        else
        {
            MadgwickAHRSupdate(ahrs, gyro, accel, mag);
        }
    }
}

