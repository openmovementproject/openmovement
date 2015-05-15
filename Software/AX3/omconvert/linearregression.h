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
// Dan Jackson, 2014 (but see source code)


#ifndef LINEARREGRESSION_H
#define LINEARREGRESSION_H


//#define ENABLE_APPROXIMATE
//#define ENABLE_GSL


// WARNING: These return a statically allocated array so are not safe (as is) in multi-threaded contexts
// TODO: Change the API to take a pointer for the return coefficients (will be thread safe)


double *LinearModelFitOneIndependent(int n, double *y, double *x);
double *LinearModelFitTwoIndependent(int n, double *y, double *x1, double *x2);

#ifdef ENABLE_APPROXIMATE
double *LinearModelFitTwoIndependentWeightedApproximately(int n, double *y, double *x1, double *x2, double *weights);
#endif


#ifdef ENABLE_GSL
double *LinearModelFitTwoIndependentWeighted(int n, double *y, double *x1, double *x2, double *weights);
#endif


#endif

