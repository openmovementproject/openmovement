// Butterworth 4th-order bandpass filter coefficient calculation.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "butter4bp.h"


// Calculates the coefficients for a 4th order Butterworth bandpass filter.
// Based on http://www.exstrom.com/journal/sigproc/ Copyright (C) 2014 Exstrom Laboratories LLC
void CoefficientsButterworth4BP(double W1, double W2, double *B, double *A)
{
	int i, j;
	
	// Calculate B coefficients as if for a Butterworth lowpass filter. 
	int prev = BUTTERWORTH4_ORDER;
	int tcof[BUTTERWORTH4_ORDER + 1];
	tcof[0] = 1;
	tcof[1] = BUTTERWORTH4_ORDER;
	for (i = 2; i <= (BUTTERWORTH4_ORDER / 2); i++)
	{
		prev = (BUTTERWORTH4_ORDER - i + 1) * prev / i;
		tcof[i] = prev;
		tcof[BUTTERWORTH4_ORDER - i] = prev;
	}
	tcof[BUTTERWORTH4_ORDER - 1] = BUTTERWORTH4_ORDER;
	tcof[BUTTERWORTH4_ORDER] = 1;

	// Calculate the scaling factor for the B coefficients of the Butterworth bandpass filter (so the filter response has a maximum value of 1).
	double ctt = 1.0 / tan(M_PI * (W2 - W1) / 2.0);
	double sfr = 1.0;
	double sfi = 0.0;
	for (i = 0; i < BUTTERWORTH4_ORDER; i++)
	{
		double parg = M_PI * (double)(2 * i + 1) / (double)(2 * BUTTERWORTH4_ORDER);
		double a = (sfr + sfi) * ((ctt + sin(parg)) - cos(parg));
		double b = sfr * (ctt + sin(parg));
		double c = -sfi * cos(parg);
		sfr = b - c;
		sfi = a - b - c;
	}
	double sf_bwbp = (1.0 / sfr);

	// Update the coefficients for a Butterworth bandpass filter, and apply the scaling factor
	for (i = 0; i < BUTTERWORTH4_ORDER; ++i)
	{
		double sign = (i & 1) ? -1 : 1;
		B[2 * i] = sign * sf_bwbp * tcof[i];
		B[2 * i + 1] = 0;
	}
	B[2 * BUTTERWORTH4_ORDER] = sf_bwbp * tcof[BUTTERWORTH4_ORDER];


	// Begin to calculate the A coefficients
	double cp = cos(M_PI * (W2 + W1) / 2.0);
	double theta = M_PI * (W2 - W1) / 2.0;
	double s2t = 2.0 * sin(theta) * cos(theta);
	double c2t = 2.0 * cos(theta) * cos(theta) - 1.0;

	// Trinomials
	double c[BUTTERWORTH4_ORDER * 2];
	double b[BUTTERWORTH4_ORDER * 2];
	for (i = 0; i < BUTTERWORTH4_ORDER; i++)
	{
		double parg = M_PI * (double)(2 * i + 1) / (double)(2 * BUTTERWORTH4_ORDER);
		double z = 1.0 + s2t * sin(parg);
		c[2 * i] = c2t / z;
		c[2 * i + 1] = s2t * cos(parg) / z;
		b[2 * i] = -2.0 * cp * (cos(theta) + sin(theta) * sin(parg)) / z;
		b[2 * i + 1] = -2.0 * cp * sin(theta) * cos(parg) / z;
	}

	// Multiply trinomials together and returns the coefficients of the resulting polynomial.
	double a[4 * BUTTERWORTH4_ORDER];
	memset(a, 0, sizeof(a));
	{
		a[2] = c[0]; a[3] = c[1]; a[0] = b[0]; a[1] = b[1];
		for (i = 1; i < BUTTERWORTH4_ORDER; i++)
		{
			a[2 * (2 * i + 1)] += c[2 * i] * a[2 * (2 * i - 1)] - c[2 * i + 1] * a[2 * (2 * i - 1) + 1];
			a[2 * (2 * i + 1) + 1] += c[2 * i] * a[2 * (2 * i - 1) + 1] + c[2 * i + 1] * a[2 * (2 * i - 1)];
			for (j = 2 * i; j > 1; j--)
			{
				a[2 * j] += b[2 * i] * a[2 * (j - 1)] - b[2 * i + 1] * a[2 * (j - 1) + 1] + c[2 * i] * a[2 * (j - 2)] - c[2 * i + 1] * a[2 * (j - 2) + 1];
				a[2 * j + 1] += b[2 * i] * a[2 * (j - 1) + 1] + b[2 * i + 1] * a[2 * (j - 1)] + c[2 * i] * a[2 * (j - 2) + 1] + c[2 * i + 1] * a[2 * (j - 2)];
			}
			a[2] += b[2 * i] * a[0] - b[2 * i + 1] * a[1] + c[2 * i];
			a[3] += b[2 * i] * a[1] + b[2 * i + 1] * a[0] + c[2 * i + 1];
			a[0] += b[2 * i];
			a[1] += b[2 * i + 1];
		}
	}

	// Read out results as A coefficients
	A[1] = a[0];
	A[0] = 1.0;
	A[2] = a[2];
	for (i = 3; i <= 2 * BUTTERWORTH4_ORDER; i++)
	{
		A[i] = a[2 * i - 2];
	}

	return;
}


// Apply the filter, specified by the coefficients b & a, to count elements of data X, where z[BUTTERWORTH4_NUM_COEFFICIENTS] tracks the final/initial conditions.
void filter(double *b, double *a, double *X, int count, double *z)
{
	int m, i;
	z[BUTTERWORTH4_NUM_COEFFICIENTS - 1] = 0;
	for (m = 0; m < count; m++)
	{
		double oldXm = X[m];
		double newXm = b[0] * oldXm + z[0];
		for (i = 1; i < BUTTERWORTH4_NUM_COEFFICIENTS; i++)
		{
			z[i - 1] = b[i] * oldXm + z[i] - a[i] * newXm;
		}
		X[m] = newXm;
	}
	return;
}


