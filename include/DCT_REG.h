/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   29.8.2017
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
*
****************************************************************/

#ifndef INCLUDE_DCT_REG_H_
#define INCLUDE_DCT_REG_H_

#include    "math.h"

    #ifndef PI
    #define PI  3.1415926535897932384626433832795
    #endif

// maximal length of buffer for saving history of accumulated error
#define     MAX_LENGTH_DCT_REG_BUFFER   400

// maximal length of harmonics array
#define		LENGTH_OF_HARMONICS_ARRAY	10


typedef struct DCT_REG_FLOAT_STRUCT
{
    int   BufferHistoryLength;    	// Input: Length of buffer
    float SamplingSignal;          	// Input: Signal that increments index [0, 1); CAUTION: SAMPLING SIGNAL MUST INCREMENT ONLY (UNTIL OVERFLOW)!
    float Ref;                      // Input: Reference input
    float Fdb;                      // Input: Feedback input
	float BaseFreq;					// Input: Frequency of basic harmonic
	int	  Harmonics[LENGTH_OF_HARMONICS_ARRAY];	// Input: Harmonics that will pass through DCT filter
	float LagCompensation;			// Input: Number of samples for phase lag compensation
	float FIRCoeff[MAX_LENGTH_DCT_REG_BUFFER];		// Input: FIR filter coefficients (so called DCT filter)
    float ErrSumMax;        		// Parameter: Maximum error
    float ErrSumMin;        		// Parameter: Minimum error
    float Kdct;                   	// Parameter: Gain for Err
    float w0;                       // Parameter: Weight for ErrSumHistory [i]
    float w1;                       // Parameter: Weight for ErrSumHistory [i + 1] and ErrSumHistory [i - 1]
    float w2;                       // Parameter: Weight for ErrSumHistory [i + 2] and ErrSumHistory [i - 2]
    float OutMax;                   // Parameter: Maximum output
    float OutMin;                   // Parameter: Minimum output
    float Err;                      // Variable: Error
    float ErrSum;           		// Variable: Error that will be accumulated
    float Correction;               // Variable: Correction that is summed with Ref
    int   i;                        // Variable: Index i in ErrSumHistory
	int   index;                    // Variable: Index build from i and LagCompensation
	int   j;                        // Variable: Index j in FIR filter coefficient and in for loop when performing convolution
	int   CircularBufferIndex;		// Variable: Index of circular buffer
    int   k;                        // Variable: Number of samples for compensation of delay
    float Out;                      // Output: DCT_REG output
	int   i_prev;                   // History: i from previous period
    float ErrSumHistory[MAX_LENGTH_DCT_REG_BUFFER];	// History: Buffer of errors from previous period
} DCT_REG_float;


#define DCT_REG_FLOAT_DEFAULTS  \
{           					\
    0,      					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0,      					\
    0,      					\
    0,      					\
    0,      					\
    0,      					\
    0,      					\
    0,      					\
    0,      					\
    0.0     					\
}

#define DCT_REG_INIT_MACRO(v)                          	\
{                                                       \
    for (v.i = 0; v.i < MAX_LENGTH_DCT_REG_BUFFER; v.i++)   \
    {                                                   \
        v.ErrSumHistory[v.i] = 0.0;                   	\
    }                                                   \
    v.i = 0;                                            \
}

#define DCT_REG_FIR_COEFF_CALC_MACRO(v)                	\
{                                                       \
    for (v.j = 0; v.j < v.BufferHistoryLength; v.j++)   \
    {                                                   \
        v.FIRCoeff[v.j] = cos(2 * PI * 1 * v.BaseFreq * (v.j + v.LagCompensation)/v.BufferHistoryLength);	\
    }                                                   \
    v.i = 0;                                            \
}

extern void DCT_REG (DCT_REG_float *v);

#endif /* INCLUDE_DCT_REG_H_ */
