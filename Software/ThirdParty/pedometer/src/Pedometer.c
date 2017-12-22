// Pedometer code
//#ifdef _WIN32
//	#define FALSE	0
//	#define TRUE	!FALSE
//	#include <stdio.h>
//	#include <stdlib.h>
//	#include <string.h>
//	#include <windows.h>
//#else
//	#include <Compiler.h>
//	#include "HardwareProfile.h"
//	#include "GenericTypeDefs.h"
//	#include "Utils/Util.h"
//#endif

#include <string.h>
#include "Pedometer.h"

// Types

// Globals
ped_t ped;

// Prototypes
static unsigned short SquareRootRounded(unsigned short a_nInput);
static unsigned short AccelToSvm(ped_accel_t* sample);

void PedInit(ped_accel_t* initialiser)
{
	// Initialise vars
	memset(&ped,0,sizeof(ped_t));
	// Init the svm 1g offset with +8bit precision
	ped.iirAveSvm = (AccelToSvm(initialiser)<<8);
}

void PedTasks(ped_accel_t* data, unsigned short num)
{
	unsigned int i;
	unsigned short acc;

	// Now process the samples
	for(i=0;i<num;i++, data++)
	{
		// Timing in samples
		ped.tick++;

		// Acceleration *above* gravity
		acc = AccelToSvm(data);

		// IIR smoothing filter of average acceleration
		ped.iirAveAcc = (ped.iirAveAcc * ((1<<SMOOTH_LPF_BW)-1) + (acc<<8))>>SMOOTH_LPF_BW;

		// AC couple the accelleration data by subtracting the DC from the LPF average (effective BP filtering by SMOOTH_LPF and DC_LPF)
		ped.aveAcc = (unsigned short)(ped.iirAveAcc>>8); // Note: signed, this is the variable for the pedometer algorithm

		// Tracked smoothed max with decay
		if(ped.peakTracker < ped.iirAveAcc) {ped.peakTracker = (unsigned short)ped.iirAveAcc;}
		else if(ped.peakTracker > THRESHOLD_DECAY_VAL){ped.peakTracker -= THRESHOLD_DECAY_VAL;}
		else ped.peakTracker = 0;

		// Thresholds at 50% and 75%
		ped.lowThresh = ped.highThresh = ped.peakTracker>>9;
		ped.highThresh += ped.peakTracker>>10;

		// Threshold crossing detect
		if(ped.aveAcc > ped.highThresh)									{ped.state = A_RISING;}
		else if ((ped.state == A_RISING)&&(ped.aveAcc < ped.lowThresh))	{ped.state = A_FALLING;}

		if (ped.state == A_FALLING) 	// Falling edge
		{
			// Has it dropped below *half* the current maximum and min avtivity level exceeded?
			if((ped.peakTracker>>8 > ACTIVITY_THRESHOLD))
			{
				// Find step interval
				ped.lastInterval = ped.tick - ped.lastStep; 	// Time since last step
				ped.lastStep = ped.tick;						// Save the time anyway

				// Check it is a valid step interval
				if (	(ped.lastInterval <= MAX_STEP_INTERVAL)  && // Max interval
						(ped.lastInterval >= MIN_STEP_INTERVAL))	// Min interval						
				{
					// Compare gait interval
					signed short jitter;
					ped.aveInterval = (unsigned short)(ped.iirAveInterval>>8);
					jitter = ped.aveInterval - ped.lastInterval;
					if(jitter < 0) jitter = -jitter;
					if(jitter < (ped.lastInterval>>1))
					{
						// Within range of 50% average interval
						ped.steps++;
						ped.state = A_LATCHED;
					}				

					// IIR smoothing filter of average step frequency
					ped.iirAveInterval = (ped.iirAveInterval * ((1<<INTERVAL_FILTER_CONST)-1) + (ped.lastInterval<<8))>>INTERVAL_FILTER_CONST;
				}
			}
		}
	} // For each sample
	return;
}

static unsigned short AccelToSvm(ped_accel_t* sample)
{
	unsigned short sum;
	signed short svm;
	signed short x, y, z;
	// Stack var and normalise to one g = 32
	x = sample->x>>ONE_G_SHIFT;	x*=x;
	y = sample->y>>ONE_G_SHIFT;	y*=y;
	z = sample->z>>ONE_G_SHIFT;	z*=z;
	// Get svm^2, this *could* overflow
	sum = x+y+z;
	// Get svm (~32) and clamp
	svm = SquareRootRounded(sum);
	if(svm > 255) svm = 255;
	// Filter svm
	ped.iirAveSvm = (ped.iirAveSvm * ((1<<SVM_FILTER_VAL)-1) + (svm<<8))>>SVM_FILTER_VAL;
	// Subtract svm
	ped.aveSvm = (unsigned short)(ped.iirAveSvm>>8);
	ped.acc = svm - ped.aveSvm;
	if(ped.acc > 0)
	{
		// Sum accelerations
		ped.eeSum += ped.acc;
		return ped.acc;
	}
	// Return zero if negative svm (acceleration with gravity)
	return 0;
}

/** KL - Someones integer square root with rounding 
found at "http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2"
 * \brief    Fast Square root algorithm, with rounding
 *
 * This does arithmetic rounding of the result. That is, if the real answer
 * would have a fractional part of 0.5 or greater, the result is rounded up to
 * the next integer.
 *      - SquareRootRounded(2) --> 1
 *      - SquareRootRounded(3) --> 2
 *      - SquareRootRounded(4) --> 2
 *      - SquareRootRounded(6) --> 2
 *      - SquareRootRounded(7) --> 3
 *      - SquareRootRounded(8) --> 3
 *      - SquareRootRounded(9) --> 3
 *
 * \param[in] a_nInput - unsigned integer for which to find the square root
 *
 * \return Integer square root of the input value.
 */
static unsigned short SquareRootRounded(unsigned short a_nInput)
{
    unsigned short op  = a_nInput;
    unsigned short res = 0;
    unsigned short one = 1uL << 14; // The second-to-top bit is set

    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    /* Do arithmetic rounding to nearest integer */
    if (op > res)
    {
        res++;
    }

    return res;
}
//EOF
