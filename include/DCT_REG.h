/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   29.8.2017
* VERSION:      3.3
*
* CHANGES :
* VERSION   DATE		WHO					DETAIL
* 1.0       6.4.2016	Denis Sušin			Initial version
* 1.1		21.8.2017	Denis Sušin			Corrections of comments and names of variables
* 2.0		15.5.2019	Denis Sušin			Circular buffer compacted into function and
* 											circular buffer indexes handling upgraded
* 3.0		9.7.2019	Denis Sušin			FPU FIR filter struct implemented within DCT controller struct,
* 											FIR coefficient buffer and delay buffer must be declared externally,
*											however, the DCT controller struct manipulates with those two buffers.
* 3.1		18.7.2019   Denis Sušin			Minor changes to prevent unwanted changes to "k", right after initialization.
*
* 3.2		19.7.2019   Denis Sušin			Corrections to "i_delta" calculations. Before the function had bugs.
*
* 3.3		19.7.2019   Denis Sušin			Corrections to phase delay compensation "k". One additional sample must be 
*											compensated when calculating "index", because of calculation delay 
*											(one sample exactly). However, this extra sample must not be implemented in 
*											FIR (DCT) filter coefficients calculation.
* 3.4		29.8.2019   Denis Sušin			Added pragma in front of two internal functions, cleared unnecessary init value.
*
****************************************************************/

#ifndef INCLUDE_DCT_REG_H_
#define INCLUDE_DCT_REG_H_

#include    "math.h"
#include	"fpu_filter.h" // for FIR filter: max 512 samples in signal period, because of lack of Global Shared RAM

#ifndef PI
#define PI  3.1415926535897932384626433832795
#endif


// maximal length of buffer for saving history of accumulated error and number of coefficients for DCT (FIR) filter
#define     FIR_FILTER_NUMBER_OF_COEFF   	400

// maximal length of harmonics array
#define		LENGTH_OF_HARMONICS_ARRAY		10
// harmonics selection at the begining that passes through DCT filter (i.e. "{1,5,7}" means that 1st, 5th and 7th harmonic passes through DCT filter, others are blocked)
#define		SELECTED_HARMONICS				{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}


typedef struct DCT_REG_FLOAT_STRUCT
{
	FIR_FP FIR_filter_float;		// Struct: FPU library FIR filter demand
    float SamplingSignal;          	// Input: Signal that increments index [0, 1); CAUTION: SAMPLING SIGNAL MUST INCREMENT ONLY (UNTIL OVERFLOW)!
    float Ref;                      // Input: Reference input
    float Fdb;                      // Input: Feedback input
    float Kdct;                   	// Parameter: Gain for Err
    float ErrSumMax;        		// Parameter: Maximum error
    float ErrSumMin;        		// Parameter: Minimum error
    float OutMax;					// Parameter: Maximum output
    float OutMin;                   // Parameter: Minimum output
    int   k;                        // Parameter: Number of samples for delay, which is already compensated within DCT filter (must be the same as lag compensation "k")
    int   k_old;                    // Variable: k from previous sample period
    int   BufferHistoryLength;    	// Variable: Length of buffer - must be the same as FIR_FILTER_NUMBER_OF_COEFF, otherwise FIR filter won't work properly
    float Err;                      // Variable: Error
    float ErrSum;           		// Variable: Error that will be accumulated
    float Correction;               // Variable: Correction that is summed with Ref
    int   i;                        // Variable: Index i in CorrectionHistory
	int   i_prev;                   // Variable: i from previous sample period
    int   i_delta;                  // Variable: difference between i and i_prev
	int   index;                    // Variable: Index build from i and LagCompensation
	int   j;                        // Variable: Index j for selection of the FIR filter coefficient buffer element
	int   SumOfHarmonicsIndex;		// Variable: Index of the for loop needed to calculate the sum
	int   SumOfHarmonics;			// Variable: Sum of all elements of the "Harmonics"
	int   SumOfHarmonicsOld;		// Variable: History of sum of all elements of the "Harmonics"
	int   CoeffCalcInProgressFlag;	// Variable: flag, inidicating when the elements of the online FIR filter coefficient buffer are being calculated
    float Out;                      // Output: DCT_REG output
    float CorrectionHistory[FIR_FILTER_NUMBER_OF_COEFF]; // History: Circular buffer of errors from previous signal period
	int	  HarmonicsBuffer[LENGTH_OF_HARMONICS_ARRAY];	 // Array: Harmonics that will pass through DCT filter
} DCT_REG_float;


#define DCT_REG_FLOAT_DEFAULTS  \
{           					\
	FIR_FP_DEFAULTS,			\
    0.0,     					\
    0.0,    					\
    0.0,    					\
    0.0,  						\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0,    						\
	0,    						\
	0,    						\
    0.0,    					\
    0.0,   						\
    0.0,    					\
    0,	    					\
    0,      					\
    0,      					\
    0,      					\
	0,      					\
	0,      					\
	0,      					\
	0,      					\
	0,      					\
	0.0,      					\
}

#define DCT_REG_INIT_MACRO(v)                          						\
{                                                       					\
	v.BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;						\
																			\
    for(v.j = 0; v.j < FIR_FILTER_NUMBER_OF_COEFF; v.j++)   				\
    {                                                   					\
    	v.CorrectionHistory[v.j] = 0.0;                     				\
    	*(v.FIR_filter_float.dbuffer_ptr + v.j) = 0.0;                   	\
    	*(v.FIR_filter_float.coeff_ptr + v.j) = 0.0;                   		\
    }                                                   					\
    v.j = 0;                                            					\
}

#define DCT_REG_FIR_COEFF_INIT_MACRO(v)                														\
{                                                       													\
	int temp_array[LENGTH_OF_HARMONICS_ARRAY] = SELECTED_HARMONICS; 										\
																											\
	for(v.i = 0; v.i < LENGTH_OF_HARMONICS_ARRAY; v.i++)													\
	{																										\
		v.HarmonicsBuffer[v.i] =  temp_array[v.i];															\
	}																										\
	/* LAG COMPENSATION HAS NEGATIVE SIGN, BECAUSE OF REALIZATION OF DCT FILTER WITH FPU LIBRARY */			\
    for(v.j = 0; v.j < FIR_FILTER_NUMBER_OF_COEFF; v.j++)   												\
    {                                                   													\
    	*(v.FIR_filter_float.coeff_ptr + v.j) = 0.0;														\
        for(v.i = 0; v.i < LENGTH_OF_HARMONICS_ARRAY; v.i++)												\
		{																									\
        	if(v.HarmonicsBuffer[v.i] != 0)																	\
			{																								\
        		*(v.FIR_filter_float.coeff_ptr + v.j) = *(v.FIR_filter_float.coeff_ptr + v.j) + 			\
							 2.0/FIR_FILTER_NUMBER_OF_COEFF *  												\
							 cos( 2 * PI * v.HarmonicsBuffer[v.i] * 										\
							 ( (float)(v.j - v.k) ) / (FIR_FILTER_NUMBER_OF_COEFF) );						\
			}																								\
		}																									\
	/* FIR FILTER FROM FPU LIBRARY DOESN'T FLIP SIGNAL FROM LEFT TO RIGHT WHILE PERFORMING CONVOLUTION */	\
    }                                                   													\
    v.j = 0;                                            													\
    v.i = 0;																								\
}

extern void DCT_REG_CALC (DCT_REG_float *v);

extern void FIR_FILTER_COEFF_CALC (DCT_REG_float *v);


#endif /* INCLUDE_DCT_REG_H_ */
