/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   29.8.2017
* VERSION:      1.0
* ADD:			FPU lib is used for realization of DCT (FIR) filter
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
*
****************************************************************/

#ifndef INCLUDE_DCT_REG_H_
#define INCLUDE_DCT_REG_H_

#include    "math.h"
#include	"fpu_filter.h" // for FIR filter: max 512 samples in period, because of lack of Global Shared RAM

    #ifndef PI
    #define PI  3.1415926535897932384626433832795
    #endif



// maximal length of buffer for saving history of accumulated error and number of coefficients for DCT (FIR) filter
#define     FIR_FILTER_NUMBER_OF_COEFF   	400

// maximal length of harmonics array
#define		LENGTH_OF_HARMONICS_ARRAY		10


typedef struct DCT_REG_FLOAT_STRUCT
{
    float SamplingSignal;          	// Input: Signal that increments index [0, 1); CAUTION: SAMPLING SIGNAL MUST INCREMENT ONLY (UNTIL OVERFLOW)!
    float Ref;                      // Input: Reference input
    float Fdb;                      // Input: Feedback input
    int   BufferHistoryLength;    	// Parameter: Length of buffer - base frequency definition, which must be the same as FIR_FILTER_NUMBER_OF_COEFF, otherwise FIR filter won't work properly
    float Kdct;                   	// Parameter: Gain for Err
	int	  Harmonics[LENGTH_OF_HARMONICS_ARRAY];	// Parameter: Harmonics that will pass through DCT filter
	float FIRCoeff[FIR_FILTER_NUMBER_OF_COEFF];	// Parameter: FIR filter coefficients (so called DCT filter)
    int   k;                        // Parameter: Number of samples for compensation of delay
    float ErrSumMax;        		// Parameter: Maximum error
    float ErrSumMin;        		// Parameter: Minimum errorfloat OutMax;
    float OutMax;					// Parameter: Maximum output
    float OutMin;                   // Parameter: Minimum output
    float Err;                      // Variable: Error
    float ErrSum;           		// Variable: Error that will be accumulated
    float Correction;               // Variable: Correction that is summed with Ref
    int   i;                        // Variable: Index i in ErrSumHistory
	int   i_prev;                   // Variable: i from previous period
	int   index;                    // Variable: Index build from i and LagCompensation
	int   j;                        // Variable: Index j in FIR filter coefficient and in for loop when performing convolution
	int   CircularBufferIndex;		// Variable: Index of circular buffer
    float Out;                      // Output: DCT_REG output
    float ErrSumHistory[FIR_FILTER_NUMBER_OF_COEFF];	// History: Buffer of errors from previous period
} DCT_REG_float;


#define DCT_REG_FLOAT_DEFAULTS  \
{           					\
    0.0,     					\
    0.0,    					\
    0.0,    					\
    0,  						\
    0.0,    					\
    0,	    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,   						\
    0.0,    					\
    0,	    					\
    0,      					\
    0,      					\
    0,      					\
    0,      					\
    0.0      					\
}

#define DCT_REG_INIT_MACRO(v)                          		\
{                                                       	\
    for (v.i = 0; v.i < FIR_FILTER_NUMBER_OF_COEFF; v.i++)   	\
    {                                                   	\
    	v.ErrSumHistory[v.i] = 0.0;                   		\
    	v.FIRCoeff[v.i] = 0.0;                   			\
    }                                                   	\
    v.i = 0;                                            	\
}

#define DCT_REG_FIR_COEFF_CALC_MACRO(v)                							\
{                                                       						\
    for (v.j = 0; v.j < FIR_FILTER_NUMBER_OF_COEFF; v.j++)   						\
    {                                                   						\
        v.FIRCoeff[v.j] = 2.0/FIR_FILTER_NUMBER_OF_COEFF * 							\
						cos(2 * PI * 1.0 * (v.j + v.k)/v.BufferHistoryLength);	\
    }                                                   						\
    v.j = 0;                                            						\
}

extern void DCT_REG_CALC (DCT_REG_float *v);

#endif /* INCLUDE_DCT_REG_H_ */
