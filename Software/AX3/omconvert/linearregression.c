/*
* Copyright (c) 2014, Newcastle University, UK.
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

// Linear Regression
// Dan Jackson, 2014 (but see below)




#include <stdlib.h>
//#include <math.h>

#include "linearregression.h"


// Linear regression with one independent variable
double *LinearModelFitOneIndependent(int n, double *y, double *x)
{
	static double coef[3];		// offset(intersect), scale(gradient), spare(to be compatible with two-variable version)
	int i;

	// sum(Xi * Yi)
	// sum(Xi^2)
	double sumx = 0, sumy = 0;
	double sumxy = 0, sumxsq = 0;
	for (i = 0; i < n; i++)
	{
		sumx += x[i];
		sumy += y[i];
		sumxy += x[i] * y[i];
		sumxsq += x[i] * x[i];
	}

	// Calculate mean of x and y
	double xmean = n > 0 ? sumx / n : 0;
	double ymean = n > 0 ? sumy / n : 0;

	//                 sum(x*y) - (n * xm * ym)
	// Slope (b):  b = ------------------------
	//                  sum(x^2) - (n * xm^2)
	double bNumerator = sumxy - (n * xmean * ymean);
	double bDenominator = sumxsq - (n * xmean * xmean);
	double b = bDenominator != 0.0 ? bNumerator / bDenominator : 0.0;

	// Intercept (a):  a = ym - b * xm
	double a = ymean - (b * xmean);

	// Line of best fit: y = a + b * X
	coef[0] = a;
	coef[1] = b;
	coef[2] = 0.0;		// just to be compatible with two-variable version

	return &coef[0];
}


// Linear regression with two independent variables
double *LinearModelFitTwoIndependent(int n, double *y, double *x1, double *x2)
{
	// Implemented from information from: http://faculty.cas.usf.edu/mbrannick/regression/Reg2IV.html
	static double coef[3];		// offset (intersect), scale 1 (gradient 1), scale 2 (gradient 2)
	int i;

	double sumx1 = 0, sumx2 = 0, sumy = 0;
	double sumx1y = 0, sumx2y = 0;
	double sumx1x2 = 0;
	double sumx1sq = 0, sumx2sq = 0;
	for (i = 0; i < n; i++)
	{
		sumx1 += x1[i];
		sumx2 += x2[i];
		sumy += y[i];
		sumx1y += x1[i] * y[i];
		sumx2y += x2[i] * y[i];
		sumx1x2 += x1[i] * x2[i];
		sumx1sq += x1[i] * x1[i];
		sumx2sq += x2[i] * x2[i];
	}

	// Calculate mean of x1, x2, and y
	double x1mean = n > 0 ? sumx1 / n : 0;
	double x2mean = n > 0 ? sumx2 / n : 0;
	double ymean = n > 0 ? sumy / n : 0;

	//                   sum(x2^2).sum(x1*y) - sum(x1*x2).sum(x2*y)
	// Slope (b1):  b1 = ------------------------------------------
	//                       sum(x1^2).sum(x2^2) - sum(x1*x2)^2
	double b1Numerator = (sumx2sq * sumx1y) - (sumx1x2 * sumx2y);
	double b1Denominator = (sumx1sq * sumx2sq) - (sumx1x2 * sumx1x2);
	double b1 = b1Numerator != 0.0 ? b1Numerator / b1Denominator : 0.0;

	//                   sum(x1^2).sum(x2*y) - sum(x1*x2).sum(x1*y)
	// Slope (b2):  b2 = ------------------------------------------
	//                       sum(x1^2).sum(x2^2) - sum(x1*x2)^2
	double b2Numerator = (sumx1sq * sumx2y) - (sumx1x2 * sumx1y);
	double b2Denominator = b1Denominator;		// Same as denominator of b1
	double b2 = b2Denominator != 0.0 ? b2Numerator / b2Denominator : 0.0;

	// Intercept (a):  a = ym - b1 * x1m - b2 * x2m
	double a = ymean - (b1 * x1mean) - (b2 * x2mean);

	// Line of best fit: y = a + b1 * x1 + b2 * x2
	coef[0] = a;
	coef[1] = b1;
	coef[2] = b2;

	return &coef[0];
}


#ifdef ENABLE_APPROXIMATE
// Linear regression with two independent variables, weighted
// NOTE: This is not really correct...
double *LinearModelFitTwoIndependentWeightedApproximately(int n, double *y, double *x1, double *x2, double *weights)
{
	double *weightedY = (double *)malloc(sizeof(double) * n);
	double *weightedX1 = (double *)malloc(sizeof(double) * n);
	double *weightedX2 = (double *)malloc(sizeof(double) * n);
	int i;

	// HACK: To reduce influence on intercept
	double influenceSum = 0;
	for (i = 0; i < n; i++)
	{
		double sw = sqrt(weights[i]);
		weightedY[i] = y[i] * sw;
		weightedX1[i] = x1[i] * sw;
		weightedX2[i] = x2[i] * sw;
		influenceSum += sw;
	}

	double *coef = LinearModelFitTwoIndependent(n, weightedY, weightedX1, weightedX2);
	double influence = n != 0 ? influenceSum / n : 0;
	if (influence != 0.0) { coef[0] /= influence; }

	free(weightedY);
	free(weightedX1);
	free(weightedX2);
	return coef;
}
#endif



#ifdef ENABLE_GSL


#include <gsl/gsl_errno.h>
#include <gsl/gsl_multifit.h>
//#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

/* General weighted case in gsl/multifit/multilinear.c */



double *LinearModelFitTwoIndependentWeighted(int n, double *y, double *x1, double *x2, double *weights)
{
	#define NPARAMS 2

	// Allocate
	gsl_matrix *matrixX = gsl_matrix_alloc(n, NPARAMS);
	gsl_vector *vectorW = gsl_vector_alloc(n);
	gsl_vector *vectorY = gsl_vector_alloc(n);
	gsl_vector *vectorC = gsl_vector_alloc(1 + NPARAMS);

	// Copy data
	int i;
	for (i = 0; i < n; i++)
	{
		// NPARAMS
		gsl_matrix_set(matrixX, i, 0, x1[i]);
		gsl_matrix_set(matrixX, i, 1, x2[i]);
		gsl_vector_set(vectorW, i, weights[i]);
		gsl_vector_set(vectorY, i, y[i]);
	}

	
	// Compute
	gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(n, NPARAMS);
	int ret = gsl_multifit_wlinear(matrixX, vectorW, vectorY, vectorC, /*gsl_matrix * cov*/ NULL, /*double * chisq*/ NULL, work);
	gsl_multifit_linear_free(work);

	static double coef[NPARAMS + 1];
	// NPARAMS+1
	coef[0] = gsl_vector_get(vectorC, 0);
	coef[1] = gsl_vector_get(vectorC, 1);
	coef[2] = gsl_vector_get(vectorC, 2);

	// Free
	gsl_matrix_free(matrixX);
	gsl_vector_free(vectorW);
	gsl_vector_free(vectorY);
	gsl_vector_free(vectorC);

	return coef;
}
#endif
