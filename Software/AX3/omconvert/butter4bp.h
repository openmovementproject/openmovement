// Butterworth 4th-order bandpass filter coefficient calculation.
//

#ifndef BUTTER4BP_H
#define BUTTER4BP_H

// Specifying a 4th order Butterworth bandpass filter
#define BUTTERWORTH4_ORDER 4
#define BUTTERWORTH4_NUM_COEFFICIENTS (BUTTERWORTH4_ORDER * 2 + 1)


// Where Fc1 = low cut-off frequency, Fc2 = high cut-off frequency, and Fs = sample frequency:
//   double W1 = Fc1 / (Fs / 2);
//   double W2 = Fc2 / (Fs / 2);
//   double B[BUTTERWORTH4_NUM_COEFFICIENTS];
//   double A[BUTTERWORTH4_NUM_COEFFICIENTS];
void CoefficientsButterworth4BP(double W1, double W2, double *B, double *A);


// Apply the filter, specified by the coefficients b & a, to count elements of data X, where z[BUTTERWORTH4_NUM_COEFFICIENTS] tracks the final/initial conditions.
void filter(double *b, double *a, double *X, int count, double *z);


#endif
