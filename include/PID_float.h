/****************************************************************
* FILENAME:     PID_float.h             
* DESCRIPTION:  declarations of Initialization & Support Functions.
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
****************************************************************/
#ifndef __PID_FLOAT_H__
#define __PID_FLOAT_H__

typedef struct PID_FLOAT_STRUCT
{
    float Ref;            // Input: Reference input 
    float Fdb;            // Input: Feedback input 
    float Ff;             // Variable: Feedforward input
    float Err;            // Variable: Error
    float Kp;             // Parameter: Proportional gain
    float Ki;             // Parameter: Integral gain
    float Kff;            // Feedforward gain
    float Up;             // Variable: Proportional output 
    float Ui;             // Variable: Integral output 
    float Uff;            // Variable: Feedforward output
    float OutMax;         // Parameter: Maximum output 
    float OutMin;         // Parameter: Minimum output
    float Out;            // Output: PID output 
} PID_float;            

/*-----------------------------------------------------------------------------
Default initalizer for the PID_float object.
-----------------------------------------------------------------------------*/                     
#define PID_FLOAT_DEFAULTS  \
{           \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0     \
}

/*------------------------------------------------------------------------------
 PID Macro Definition
------------------------------------------------------------------------------*/
#define PID_FLOAT_CALC(v)                           \
{                                                   \
    v.Err = v.Ref - v.Fdb;                          \
    v.Up= v.Kp * v.Err;                             \
    v.Uff = v.Ff * v.Kff;                           \
    v.Out = v.Up + v.Ui + v.Uff;                    \
    if (v.Out > v.OutMax)                           \
    {                                               \
        v.Out = v.OutMax;                           \
        if  (v.Ui < 0.0)                            \
        {                                           \
            v.Ui = v.Ki * v.Err + v.Ui;             \
        }                                           \
    }                                               \
    else if (v.Out < v.OutMin)                      \
    {                                               \
        v.Out = v.OutMin;                           \
        if  (v.Ui > 0.0)                            \
        {                                           \
            v.Ui = v.Ki * v.Err + v.Ui;             \
        }                                           \
    }                                               \
    else                                            \
    {                                               \
        v.Ui = v.Ki * v.Err + v.Ui;                 \
    }                                               \
}    
#endif // __PID_FLOAT_H__
