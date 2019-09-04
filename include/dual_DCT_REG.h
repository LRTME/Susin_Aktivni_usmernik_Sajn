/****************************************************************
* FILENAME:     dual_DCT_REG.h
* DESCRIPTION:  dual DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   28.8.2019
* VERSION:      1.0
*
* CHANGES :
* VERSION   	DATE		WHO					DETAIL
* 1.0       	28.8.2019	Denis Sušin			Initial version based on DCT controller v3.4.
*
****************************************************************/

#ifndef INCLUDE_dual_DCT_REG_H_
#define INCLUDE_dual_DCT_REG_H_

#include    "math.h"
#include	"fpu_filter.h" // for FIR filter: max 512 samples in signal period, because of lack of Global Shared RAM

#ifndef PI
#define PI  3.1415926535897932384626433832795
#endif


// maximal length of buffer for saving history of accumulated error and number of coefficients for both DCT (FIR) filters
#define     FIR_FILTER_NUMBER_OF_COEFF2   	400

// maximal length of harmonics array
#define		LENGTH_OF_HARMONICS_ARRAY2		3
// harmonics selection at the beginning that passes through both DCT filters (i.e. "{1,5,7}" means that 1st, 5th and 7th harmonic passes through DCT filter, others are blocked)
#define		SELECTED_HARMONICS2				{1, 0, 0}
// amplitudes for each harmonic at the beginning
#define		AMPLITUDE_VALUES2				{1.0, 1.0, 1.0}
// phase delay compensation values for each harmonic at the beginning
#define		PHASE_LAG_COMPENSATION_VALUES2	{0, 0, 0}


typedef struct dual_DCT_REG_FLOAT_STRUCT
{
	FIR_FP FIR_filter_float1;		// Struct: FPU library FIR filter demand with phase delay compensation
	FIR_FP FIR_filter_float2;		// Struct: FPU library FIR filter demand without phase delay compensation
    float SamplingSignal;          	// Input: Signal that increments index [0, 1); CAUTION: SAMPLING SIGNAL MUST INCREMENT ONLY (UNTIL OVERFLOW)!
    float Ref;                      // Input: Reference input
    float Fdb;                      // Input: Feedback input
    float Kdct;                   	// Parameter: Gain for Err
    float ErrSumMax;        		// Parameter: Maximum error
    float ErrSumMin;        		// Parameter: Minimum error
    float OutMax;					// Parameter: Maximum output
    float OutMin;                   // Parameter: Minimum output
    int   BufferHistoryLength;    	// Variable: Length of buffer - must be the same as FIR_FILTER_NUMBER_OF_COEFF2, otherwise FIR filter won't work properly
    float Err;                      // Variable: Error
    float ErrSum;           		// Variable: Error that will be accumulated
    float Correction;               // Variable: Correction signal is actually unbounded output
    int   i;                        // Variable: Index i in buffer, representing sampling signal
	int   i_prev;                   // Variable: i from previous sample period
	int   j;                        // Variable: Index j for selection of the FIR filter coefficient buffer element
	int   HarmonicIndex;			// Variable: Index of the for loop (to access each harmonic's element)
	int   SumOfHarmonics;			// Variable: Sum of all elements of the "Harmonics"
	int   SumOfHarmonicsOld;		// Variable: History of sum of all elements of the "Harmonics"
	float SumOfAmplitudes;			// Variable: Sum of all elements of the amplitudes "A"
	float SumOfAmplitudesOld;		// Variable: Sum of all elements of the amplitudes "A"
	int   SumOfLagCompensation;		// Variable: Sum of all elements of the phase delay compensation "k"
	int   SumOfLagCompensationOld;	// Variable: History of sum of all elements of the phase delay compensation "k"
	int   CoeffCalcInProgressFlag;	// Variable: flag, indicating when the elements of the online FIR filter coefficient buffer are being calculated
    float Out;                      // Output: dual_DCT_REG output
    int	  HarmonicsBuffer[LENGTH_OF_HARMONICS_ARRAY2];	// Array of parameters: Selected harmonics that will pass through both DCT filters
    float A[LENGTH_OF_HARMONICS_ARRAY2];				// Array of parameters: Amplitude of each selected harmonic
    int	  k[LENGTH_OF_HARMONICS_ARRAY2];				// Array of parameters: Number of samples for each selected harmonic's phase delay compensation
} dual_DCT_REG_float;


#define dual_DCT_REG_FLOAT_DEFAULTS		\
{           							\
	FIR_FP_DEFAULTS,					\
	FIR_FP_DEFAULTS,					\
    0.0,     							\
    0.0,    							\
    0.0,    							\
    0.0,  								\
    0.0,    							\
    0.0,    							\
    0.0,    							\
    0.0,    							\
	0,    								\
    0.0,    							\
    0.0,   								\
    0.0,    							\
    0,	    							\
    0,      							\
    0,      							\
    0,      							\
	0,      							\
	0,      							\
	0.0,      							\
	0.0,      							\
	0,      							\
	0,      							\
	0,      							\
	0.0,      							\
}

#define dual_DCT_REG_INIT_MACRO(v)                          				\
{                                                       					\
	v.BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF2;					\
																			\
    for(v.j = 0; v.j < FIR_FILTER_NUMBER_OF_COEFF2; v.j++)   				\
    {                                                   					\
    	*(v.FIR_filter_float1.dbuffer_ptr + v.j) = 0.0;                   	\
    	*(v.FIR_filter_float1.coeff_ptr + v.j) = 0.0;                   	\
    	*(v.FIR_filter_float2.dbuffer_ptr + v.j) = 0.0;                   	\
    	*(v.FIR_filter_float2.coeff_ptr + v.j) = 0.0;                   	\
    }                                                   					\
    v.j = 0;                                            					\
}

#define dual_DCT_REG_FIR_COEFF_INIT_MACRO(v)                												\
{                                                       													\
	int temp_array[LENGTH_OF_HARMONICS_ARRAY2] = SELECTED_HARMONICS2; 										\
	float temp_array2[LENGTH_OF_HARMONICS_ARRAY2] = AMPLITUDE_VALUES2;										\
	int temp_array3[LENGTH_OF_HARMONICS_ARRAY2] = PHASE_LAG_COMPENSATION_VALUES2;							\
																											\
	for(v.i = 0; v.i < LENGTH_OF_HARMONICS_ARRAY2; v.i++)													\
	{																										\
		v.HarmonicsBuffer[v.i] =  temp_array[v.i];															\
		v.A[v.i] =  temp_array2[v.i];																		\
		v.k[v.i] =  temp_array3[v.i];																		\
	}																										\
	/* LAG COMPENSATION HAS NEGATIVE SIGN, BECAUSE OF REALIZATION OF DCT FILTER WITH FPU LIBRARY */			\
    for(v.j = 0; v.j < FIR_FILTER_NUMBER_OF_COEFF2; v.j++)   												\
    {                                                   													\
    	*(v.FIR_filter_float1.coeff_ptr + v.j) = 0.0;														\
    	*(v.FIR_filter_float2.coeff_ptr + v.j) = 0.0;														\
        for(v.i = 0; v.i < LENGTH_OF_HARMONICS_ARRAY2; v.i++)												\
		{																									\
        	if(v.HarmonicsBuffer[v.i] != 0)																	\
			{																								\
        		*(v.FIR_filter_float1.coeff_ptr + v.j) = *(v.FIR_filter_float1.coeff_ptr + v.j) + 			\
							 2.0/FIR_FILTER_NUMBER_OF_COEFF2 *  											\
							 cos( 2.0 * PI * v.HarmonicsBuffer[v.i] * 										\
							 ( (float)(v.j) ) / (FIR_FILTER_NUMBER_OF_COEFF2) );							\
							 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	\
				*(v.FIR_filter_float2.coeff_ptr + v.j) = *(v.FIR_filter_float2.coeff_ptr + v.j) + 			\
							 2.0/FIR_FILTER_NUMBER_OF_COEFF2 *  											\
							 v.A[v.i] * cos( 2.0 * PI * v.HarmonicsBuffer[v.i] * 							\
							 ( (float)(v.j - v.k[v.i]) ) / (FIR_FILTER_NUMBER_OF_COEFF2) );					\
			}																								\
		}																									\
	/* FIR FILTER FROM FPU LIBRARY DOESN'T FLIP SIGNAL FROM LEFT TO RIGHT WHILE PERFORMING CONVOLUTION */	\
    }                                                   													\
    v.j = 0;                                            													\
    v.i = 0;																								\
}

extern void dual_DCT_REG_CALC (dual_DCT_REG_float *v);

extern void FIR_FILTER_COEFF_CALC2 (dual_DCT_REG_float *v);


#endif /* INCLUDE_dual_DCT_REG_H_ */
