/****************************************************************
* FILENAME:     REP_REG.c
* DESCRIPTION:  Repetitive controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Su�in
* START DATE:   6.4.2016
* VERSION:      3.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016   Denis Su�in      Initial version
* 1.1		21.8.2017  Denis Su�in		Corrections of comments and names of variables
* 2.0		15.5.2019  Denis Su�in		Circular buffer compacted into function and
* 										circular buffer indexes handling upgraded
* 3.0		19.7.2019  Denis Su�in		Corrections to "i_delta" calculations. Before the function had bugs.
*
****************************************************************/

#ifndef INCLUDE_REP_REG_H_
#define INCLUDE_REP_REG_H_

// minimal length of buffer for saving history of accumulated error (minimalno 20)
#define     MIN_LENGTH_REP_REG_BUFFER   20
// maximal length of buffer for saving history of accumulated error
#define     MAX_LENGTH_REP_REG_BUFFER   400


typedef struct REP_REG_FLOAT_STRUCT
{
    float SamplingSignal;          	// Input: Signal that increments or decrements [0, 1)
    float Ref;                      // Input: Reference input
    float Fdb;                      // Input: Feedback input
    int   BufferHistoryLength;    	// Parameter: Length of buffer
    float Krep;                   	// Parameter: Gain for Err
    int   k;                        // Parameter: Number of samples for compensation of delay
    float w0;                       // Parameter: Weight for ErrSumHistory [i]
    float w1;                       // Parameter: Weight for ErrSumHistory [i + 1] and ErrSumHistory [i - 1]
    float w2;                       // Parameter: Weight for ErrSumHistory [i + 2] and ErrSumHistory [i - 2]
    float ErrSumMax;        		// Parameter: Maximum error
    float ErrSumMin;        		// Parameter: Minimum error
    float OutMax;                   // Parameter: Maximum output
    float OutMin;                   // Parameter: Minimum output
    float Err;                      // Variable: Error
    float ErrSum;           		// Variable: Error that will be accumulated
    float Correction;               // Variable: Correction that is summed with Ref
    int   i;                        // Variable: Index i in ErrSumHistory
    int   i_prev;                   // Variable: i from previous period - history
    int   i_delta;                  // Variable: difference between i and i_prev
    int   i_plus_one;               // Variable: i + 1 sample in history
    int   i_minus_one;              // Variable: i - 1 sample in history
    int   i_plus_two;               // Variable: i + 2 sample in history
    int   i_minus_two;              // Variable: i - 2 sample in history
    int   index;                    // Variable: Index (i + k) in ErrSumHistory (used in Correction) - includes k
    float Out;                      // Output: REP_REG output
    float ErrSumHistory[MAX_LENGTH_REP_REG_BUFFER]; // History: Circular buffer of errors from previous period
} REP_REG_float;


#define REP_REG_FLOAT_DEFAULTS  \
{           					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0,      					\
    0.0,	   					\
    0,    						\
    0.0,    					\
    0.0,    					\
    0.0,	    				\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
    0.0,      					\
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

#define REP_REG_INIT_MACRO(v)                          		\
{                                                       	\
    for (v.i = 0; v.i < MAX_LENGTH_REP_REG_BUFFER; v.i++)   \
    {                                                   	\
        v.ErrSumHistory[v.i] = 0.0;                   		\
    }                                                   	\
	/* v.i and v.i_prev must not be equal at the start */   \
    v.i = 0;                                            	\
    v.i_prev = -1;						 					\
}


extern void REP_REG_CALC (REP_REG_float *v);

#endif /* INCLUDE_REP_REG_H_ */
