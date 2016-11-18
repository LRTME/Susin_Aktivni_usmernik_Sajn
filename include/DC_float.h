/****************************************************************
* FILENAME:     DC_float.h             
* DESCRIPTION:  izracuna srednjo vrednost signala  
* AUTHOR:       Mitja Nemec
* START DATE:   12.8.2010
* VERSION:      1.0
*
****************************************************************/
#ifndef     __DC_FLOAT_H__
#define     __DC_FLOAT_H__

#include    "define.h"

/* definicija konstant, ki jih potrebujemo  */

/* velikost našega okna - št vzorcev v periodi */
#define     DC_FLOAT_SIZE  SAMPLE_POINTS

typedef struct DC_FLOAT_STRUCT
{
    float   In;                    // Input
    float   Mean;                  // srednja vrednost
    int     m;                     // sample index
    float   Buffer[DC_FLOAT_SIZE]; // buffer B
} DC_float;

typedef DC_float* DC_float_handle;

/*-----------------------------------------------------------------------------
Default initalizer for the DC_fixed object.
-----------------------------------------------------------------------------*/                     
#define DC_FLOAT_DEFAULTS   \
{                           \
    0.0,                    \
    0.0,                    \
    0                      \
}

/*------------------------------------------------------------------------------
 DC Macro Definition for main function
------------------------------------------------------------------------------*/
#define DC_FLOAT_MACRO(v)                           	\
{                                                   	\
    v.Mean = v.Mean + ((1.0/DC_FLOAT_SIZE) * 			\
                             (v.In - v.Buffer[v.m])); 	\
    v.Buffer[v.m] = v.In;                           	\
    v.m++;                                          	\
    if (v.m == DC_FLOAT_SIZE)                  			\
    {                                               	\
        v.m = 0;                                    	\
    }                                               	\
}

/*------------------------------------------------------------------------------
 DFT Macro Definition for initialization function
------------------------------------------------------------------------------*/
#define DC_FLOAT_MACRO_INIT(v)                  \
{                                               \
    for (v.m = 0; v.m < DC_FLOAT_SIZE; v.m++)   \
    {                                           \
        v.Buffer[v.m] = (0.0);               	\
    }                                           \
    v.m = 0;                                    \
}

#endif // __DC_FLOAT_H__
