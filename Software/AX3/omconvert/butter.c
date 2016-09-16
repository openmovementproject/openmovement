// Butterworth filter coefficient calculation.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#define _USE_MATH_DEFINES
#include <math.h>

#include "butter.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

// Calculates the coefficients for a 4th order Butterworth bandpass filter (now extended to low-pass, high-pass, band-stop).
// Based on http://www.exstrom.com/journal/sigproc/ Copyright (C) 2014 Exstrom Laboratories LLC
int CoefficientsButterworth(int order, double W1, double W2, double *B, double *A)
{
	int i, j;
	int tcof[BUTTERWORTH_MAX_ORDER + 1];

	// Pass-through filter
	if (W1 <= 0.0 && W2 <= 0.0)
	{
		return 0;
	}

	// Check filter type
	if (W1 > 0.0 && W2 > 0.0 && W2 < W1)
	{
		// (Important) Clear values
		memset(B, 0, sizeof(double) * (2 * order + 1));
		// For band-stop, calculate B coefficients directly
		double alpha = -2.0 * cos(M_PI * (W2 + W1) / 2.0) / cos(M_PI * fabs(W2 - W1) / 2.0);
		B[0] = 1.0;
		B[2] = 1.0;
		B[1] = alpha;
		for (i = 1; i < order; ++i)
		{
			B[2 * i + 2] += B[2 * i];
			for (j = 2 * i; j > 1; --j)
			{
				B[j + 1] += alpha * B[j] + B[j - 1];
			}
			B[2] += alpha * B[1] + 1.0;
			B[1] += alpha;
		}
	}
	else
	{
		// For low-pass, high-pass, band-pass: Calculate B coefficients as if for a Butterworth lowpass filter. 
		int prev = order;
		tcof[0] = 1;
		tcof[1] = order;
		for (i = 2; i <= (order / 2); i++)
		{
			prev = (order - i + 1) * prev / i;
			tcof[i] = prev;
			tcof[order - i] = prev;
		}
		tcof[order - 1] = order;
		tcof[order] = 1;
	}


	// If a high- or low-pass filter...
	if (W1 <= 0.0 || W2 <= 0.0)
	{
		// Calculate the scaling factor for the B coefficients of the Butterworth high- or low-pass filter (so the filter response has a maximum value of 1).
		double fcf = (W1 <= 0.0) ? W2 : W1;
		double omega = M_PI * fcf;
		double fomega = sin(omega);
		double parg0 = M_PI / (double)(2 * order);
		double sf = 1.0;
		for (i = 0; i < order / 2; ++i)
		{
			sf *= 1.0 + fomega * sin((double)(2 * i + 1) * parg0);
		}

		// High-pass / low-pass
		if (W2 <= 0.0)
		{
			fomega = cos(omega / 2.0);											// High-pass
			if (order % 2) { sf *= fomega + sin(omega / 2.0); }  // Odd order high-pass
		}
		else
		{
			fomega = sin(omega / 2.0);											// Low-pass
			if (order % 2) { sf *= fomega + cos(omega / 2.0); }	// Odd order low-pass
		}

		// Final scaling factor
		sf = pow(fomega, order) / sf;

		// Update the coefficients by applying the scaling factor
		for (i = 0; i < order; ++i)
		{
			B[i] = sf * tcof[i];
		}
		B[order] = sf * tcof[order];

		// Modify coefficients for a high-pass filter
		if (W2 <= 0.0)
		{
			for (i = 1; i <= order; i += 2) { B[i] = -B[i]; }
		}

	}
	else    // ...otherwise, a band-pass or band-stop filter
	{
		// Calculate the scaling factor for the B coefficients of the Butterworth band-stop or band-pass filter (so the filter response has a maximum value of 1).
		double sfr = 1.0;
		double sfi = 0.0;
		double ctt = tan(M_PI * fabs(W2 - W1) / 2.0);			// Band-stop scaling 
		if (W1 <= W2) { ctt = 1.0 / ctt; }						// Convert to band-pass scaling
		for (i = 0; i < order; i++)
		{
			double parg = M_PI * (double)(2 * i + 1) / (double)(2 * order);
			double a = (sfr + sfi) * ((ctt + sin(parg)) - cos(parg));
			double b = sfr * (ctt + sin(parg));
			double c = -sfi * cos(parg);
			sfr = b - c;
			sfi = a - b - c;
		}
		double sf_bwbp = (1.0 / sfr);

		// Band-pass or band-stop
		if (W1 <= W2)
		{
			// Update the coefficients for a Butterworth bandpass filter, and apply the scaling factor
			for (i = 0; i < order; ++i)
			{
				double sign = (i & 1) ? -1 : 1;
				B[2 * i] = sign * sf_bwbp * tcof[i];
				B[2 * i + 1] = 0;
			}
			B[2 * order] = sf_bwbp * tcof[order];
		}
		else
		{
			// Apply the scaling factor to the band-stop coefficients
			for (i = 0; i <= 2 * order; ++i)
			{
				B[i] = sf_bwbp * B[i];
			}
		}
	}

	// If a high- or low-pass filter...
	if (W1 <= 0.0 || W2 <= 0.0)
	{
		// Begin to calculate the A coefficients for a high-pass or low-pass Butterworth filter
		double theta = M_PI * ((W1 <= 0.0) ? W2 : W1);

		// Binomials
		double b[2 * BUTTERWORTH_MAX_ORDER];
		memset(b, 0, sizeof(double) * (2 * order));
		for (i = 0; i < order; i++)
		{
			double parg = M_PI * (double)(2 * i + 1) / (double)(2 * order);
			double a = 1.0 + sin(theta) * sin(parg);
			b[2 * i] = -cos(theta) / a;
			b[2 * i + 1] = -sin(theta) * cos(parg) / a;
		}

		// Multiply binomials together and returns the coefficents of the resulting polynomial.
		double a[2 * BUTTERWORTH_MAX_ORDER];
		memset(a, 0, sizeof(double) * (2 * order));
		for (i = 0; i < order; i++)
		{
			for (j = i; j > 0; --j)
			{
				a[2 * j] += b[2 * i] * a[2 * (j - 1)] - b[2 * i + 1] * a[2 * (j - 1) + 1];
				a[2 * j + 1] += b[2 * i] * a[2 * (j - 1) + 1] + b[2 * i + 1] * a[2 * (j - 1)];
			}
			a[0] += b[2 * i];
			a[1] += b[2 * i + 1];
		}

		// Read out results as A coefficients for high-pass or low-pass filter.
		A[1] = a[0];
		A[0] = 1.0;
		A[2] = a[2];
		for (i = 3; i <= order; ++i)
		{
			A[i] = a[2 * i - 2];
		}
	}
	else
	{
		// Begin to calculate the A coefficients for a band-pass or band-stop filter
		double cp = cos(M_PI * (W2 + W1) / 2.0);
		double theta = M_PI * fabs(W2 - W1) / 2.0;
		double s2t = 2.0 * sin(theta) * cos(theta);
		double c2t = 2.0 * cos(theta) * cos(theta) - 1.0;

		// Trinomials
		double c[BUTTERWORTH_MAX_ORDER * 2];
		double b[BUTTERWORTH_MAX_ORDER * 2];
		for (i = 0; i < order; i++)
		{
			double parg = M_PI * (double)(2 * i + 1) / (double)(2 * order);
			double z = 1.0 + s2t * sin(parg);
			c[2 * i] = c2t / z;
			if (W2 < W1) { c[2 * i + 1] = -s2t * cos(parg) / z; }					// Band-stop
			else { c[2 * i + 1] = s2t * cos(parg) / z; }							// Band-pass
			b[2 * i] = -2.0 * cp * (cos(theta) + sin(theta) * sin(parg)) / z;
			if (W2 < W1) { b[2 * i + 1] = 2.0 * cp * sin(theta) * cos(parg) / z; }	// Band-stop
			else { b[2 * i + 1] = -2.0 * cp * sin(theta) * cos(parg) / z; }			// Band-pass
		}


		// Multiply trinomials together and returns the coefficients of the resulting polynomial.
		double a[4 * BUTTERWORTH_MAX_ORDER];
		memset(a, 0, sizeof(double) * (4 * order));
		{
			a[2] = c[0]; a[3] = c[1]; a[0] = b[0]; a[1] = b[1];
			for (i = 1; i < order; i++)
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

		// Read out results as A coefficients for band-pass or band-stop filter
		A[1] = a[0];
		A[0] = 1.0;
		A[2] = a[2];
		for (i = 3; i <= 2 * order; i++)
		{
			A[i] = a[2 * i - 2];
		}
	}

	// Return the number of coefficients needed for the filter
	if (W1 > 0.0 && W2 > 0.0) { return (2 * order) + 1; }
	else { return order + 1; }
}


// Apply the filter, specified by the coefficients b & a, to count elements of data X, returning in data Y (can be same as X), where z[] tracks the final/initial conditions.
void filter(int numCoefficients, const double *b, const double *a, const double *X, double *Y, int count, double *z)
{
	if (numCoefficients > 0)
	{
		int m, i;
		z[numCoefficients - 1] = 0;
		for (m = 0; m < count; m++)
		{
			double oldXm = X[m];
			double newXm = b[0] * oldXm + z[0];
			for (i = 1; i < numCoefficients; i++)
			{
				z[i - 1] = b[i] * oldXm + z[i] - a[i] * newXm;
			}
			Y[m] = newXm;
		}
	}
	else if (X != Y) 
	{
		// Pass-through, and destination is not the source
		memmove(Y, X, sizeof(double) * count);
	}
	return;
}



/*
// !!!! TODO: Remove this
#define BUTTERWORTH4_ORDER 4
#define BUTTERWORTH4_NUM_COEFFICIENTS (BUTTERWORTH4_ORDER * 2 + 1)
void filterOrder4BP(double *b, double *a, double *X, int count, double *z)
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
*/
