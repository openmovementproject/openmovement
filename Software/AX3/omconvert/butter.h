// Butterworth filter coefficient calculation.
//

#ifndef BUTTER4BP_H
#define BUTTER4BP_H

// Maximum order
#define BUTTERWORTH_MAX_ORDER 16

// Number of coefficients for a given order
#define BUTTERWORTH_MAX_COEFFICIENTS(order) ((order) * 2 + 1)				// Maximum number of coefficients for a Butterworth filter (twice as many as the order, plus one)
#define BUTTERWORTH_NUM_COEFFICIENTS_BP(order) ((order) * 2 + 1)			// Number of coefficients for a band-pass filter (twice as many as the order, plus one)
#define BUTTERWORTH_NUM_COEFFICIENTS_BS(order) ((order) * 2 + 1)			// Number of coefficients for a band-stop filter (twice as many as the order, plus one)
#define BUTTERWORTH_NUM_COEFFICIENTS_LP(order) ((order) + 1)				// Number of coefficients for a high-pass filter (as many as the order, plus one)
#define BUTTERWORTH_NUM_COEFFICIENTS_HP(order) ((order) + 1)				// Number of coefficients for a low-pass filter (as many as the order, plus one)



// Where Fc1 = low cut-off frequency, Fc2 = high cut-off frequency, and Fs = sample frequency:
//   #define ORDER 4
//   double W1 = Fc1 / (Fs / 2);
//   double W2 = Fc2 / (Fs / 2);
//   double B[BUTTERWORTH_MAX_COEFFICIENTS(ORDER)];
//   double A[BUTTERWORTH_MAX_COEFFICIENTS(ORDER)];
//
// For band-stop, swap W1 and W2.
// For low-pass, set W1=0.
// For high-pass, set W2=0.
//
// Returns the number of coefficients
int CoefficientsButterworth(int order, double W1, double W2, double *B, double *A);


// Apply the filter, specified by the coefficients b & a, to count elements of data X, returning in data Y (can be same as X), where z[] tracks the final/initial conditions.
void filter(int numCoefficients, const double *b, const double *a, const double *X, double *Y, int count, double *z);



// !!!! TODO: Remove this
void filterOrder4BP(double *b, double *a, double *X, int count, double *z);

#endif
