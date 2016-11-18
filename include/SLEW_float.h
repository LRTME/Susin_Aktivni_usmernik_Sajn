/****************************************************************
* FILENAME:     SLEW_float.h             
* DESCRIPTION:  declarations of Initialization & Support Functions.
* AUTHOR:       Mitja Nemec
* START DATE:   4.3.2014
*
****************************************************************/
#ifndef __SLEW_FLOAT_H__
#define __SLEW_FLOAT_H__

typedef struct {
    float In;           // Input
    float Out;          // Output
    float Slope_up;     // Up slope 
    float Slope_down;   // Down slope 
    } SLEW_float;              

/*-----------------------------------------------------------------------------
Default initalizer for the SL_float object.
-----------------------------------------------------------------------------*/                     
#define SLEW_FLOAT_DEFAULTS \
{                           \
    0.0,                    \
    0.0,                    \
    0.0,                    \
    0.0                     \
}  


/*------------------------------------------------------------------------------
 SLEW Macro Definition
------------------------------------------------------------------------------*/
#define SLEW_FLOAT_CALC(v)                              \
    if ((v.Out - v.In) < 0.0)                           \
    {                                                   \
        if ((-(v.Out - v.In)) > v.Slope_up)             \
            v.Out = v.Out + v.Slope_up;                 \
        else                                            \
            v.Out = v.In;                               \
    }                                                   \
    if ((v.Out - v.In) > 0.0)                           \
    {                                                   \
        if ((v.Out - v.In) > v.Slope_down)              \
            v.Out = v.Out - v.Slope_down;               \
        else                                            \
            v.Out = v.In;                               \
    }                                                   \

#endif // __SLEW_FLOAT_H__
