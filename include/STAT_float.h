/****************************************************************
* FILENAME:     STAT_float.h
* DESCRIPTION:  izracuna srednjo vrednost, varianco, sigmo in R.M.S
*               signala
* AUTHOR:       Mitja Nemec
* START DATE:   25.7.2014
* VERSION:      1.0
*
****************************************************************/
#ifndef     __STAT_FLOAT_H__
#define     __STAT_FLOAT_H__

#include    "math.h"
#include    "define.h"

/* definicija konstant, ki jih potrebujemo  */

/* velikost našega okna - št vzorcev v periodi */
#define     STAT_FLOAT_SIZE    800

typedef struct STAT_FLOAT_STRUCT
{
    float   In;                     // Input
    float   Mean;                   // output
    float   Rms;
    float   Std;
    float   Var;
    float   Sum_square;
    int     m;                      // index
    float   Buffer[STAT_FLOAT_SIZE];  // buffer B
} STAT_float;

typedef STAT_float* STAT_float_handle;

/*-----------------------------------------------------------------------------
Default initalizer for the DC_fixed object.
-----------------------------------------------------------------------------*/                     
#define STAT_FLOAT_DEFAULTS   \
{                           \
    0.0,                     \
    0.0,                     \
    0.0,                     \
    0.0,                     \
    0.0,                     \
    0                       \
}

/*------------------------------------------------------------------------------
 VAR_stat macro Definition for main function
------------------------------------------------------------------------------*/
#define STAT_FLOAT_MACRO(v)                              \
{                                                       \
    v.Mean = v.Mean + ((1.0/STAT_FLOAT_SIZE) *            \
                             (v.In - v.Buffer[v.m]));   \
                                                        \
    v.Sum_square = v.Sum_square                         \
                 - (v.Buffer[v.m] * v.Buffer[v.m])      \
                 + (v.In * v.In);                       \
    v.Rms = sqrt(fabs(v.Sum_square/STAT_FLOAT_SIZE));     \
                                                        \
    v.Var = fabs((v.Mean * v.Mean) - (v.Rms * v.Rms));  \
    v.Std = sqrt(v.Var);                                \
    v.Buffer[v.m] = v.In;                               \
    v.m++;                                              \
    if (v.m == STAT_FLOAT_SIZE)                         \
    {                                                   \
        v.m = 0;                                        \
    }                                                   \
}

/*------------------------------------------------------------------------------
 VAR_stat macro Definition for initialization function
------------------------------------------------------------------------------*/
#define STAT_FLOAT_MACRO_INIT(v)                      \
{                                                   \
    for (v.m = 0; v.m < STAT_FLOAT_SIZE; v.m++)    \
    {                                               \
        v.Buffer[v.m] = (0.0);                      \
    }                                               \
    v.m = 0;                                        \
}

#endif // __STAT_FLOAT_H__
