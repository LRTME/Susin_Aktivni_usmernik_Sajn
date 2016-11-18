/****************************************************************
* FILENAME:     DFT_float.h
* DESCRIPTION:  izracuna harmonik vhodnega signala
* AUTHOR:       Bogdana Leban, Bojan Adamic, Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.4
*
****************************************************************/
#ifndef     __DFT_FLOAT_H__
#define     __DFT_FLOAT_H__

#include    "define.h"
#include    "math.h"


/* definicija konstant, ki jih potrebujemo  */

/* velikost našega okna - št vzorcev v periodi */
#define     DFT_FLOAT_SIZE  SAMPLE_POINTS

// tip strukture
typedef struct DFT_FLOAT_STRUCT
{
    float   In;                    // Input
    float   Out;                   // Output
    float   SumA;                  // vsota A
    float   SumB;                  // vsota B
    float   kot;
    float   sin;
    float   cos;
    int     m;                     // buffer pointer
    float   Buffer[DFT_FLOAT_SIZE];// buffer A
} DFT_float;


/*-----------------------------------------------------------------------------
Default initalizer for the DFT_fixed object.
-----------------------------------------------------------------------------*/                     
#define DFT_FLOAT_DEFAULTS      \
{                               \
    0.0,                         \
    0.0,                         \
    0.0,                         \
    0.0,                         \
    0.0,                         \
    0.0,                         \
    0.0,                         \
    0,                          \
}

/*------------------------------------------------------------------------------
 DFT Macro Definition for main function
------------------------------------------------------------------------------*/
#define DFT_FLOAT_MACRO(v)                                  \
{                                                           \
    v.kot = ((2*PI/DFT_FLOAT_SIZE) * (v.m)); \
    v.sin = sin(v.kot);                                     \
    v.cos = cos(v.kot);                                     \
    v.SumA = v.SumA                                         \
           + ((v.In - v.Buffer[v.m]) * v.sin) *             \
             (2.0/DFT_FLOAT_SIZE);                          \
    v.SumB = v.SumB                                         \
           + ((v.In - v.Buffer[v.m]) * v.cos) *             \
             (2.0/DFT_FLOAT_SIZE);                          \
    v.Buffer[v.m] = v.In;                                   \
    v.Out = v.SumA * v.sin + v.SumB * v.cos;                \
    v.m++;                                                  \
    if (v.m == DFT_FLOAT_SIZE) v.m = 0;                     \
}

/*------------------------------------------------------------------------------
 DFT Macro Definition for initialization function
------------------------------------------------------------------------------*/
#define DFT_FLOAT_MACRO_INIT(v)                 \
{                                               \
    for (v.m = 0; v.m < DFT_FLOAT_SIZE; v.m++)  \
    {                                           \
        v.Buffer[v.m] = 0.0;                    \
    }                                           \
    v.m = 0;                                    \
}


#endif // __DFT_FIXED_H__
