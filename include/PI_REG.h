/****************************************************************
* FILENAME:     PI_float.h             
* DESCRIPTION:  PI controller (regulator); Declarations of Initialization & Support Functions.
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
*
* CHANGES:
* VERSION   DATE        WHO             DETAIL
* 1.1       28.8.2017   Denis Sušin     Changed name PID into PI and corrected spaces and similar.
****************************************************************/
#ifndef __PI_REG_H__
#define __PI_REG_H__

typedef struct PI_REG_FLOAT_STRUCT
{
    float Ref;            // Input: Reference input 
    float Fdb;            // Input: Feedback input 
    float Ff;             // Input: Feedforward input
    float Kp;             // Parameter: Proportional gain
    float Ki;             // Parameter: Integral gain
    float Kff;            // Parameter: Feedforward gain
	float OutMax;         // Parameter: Maximum output 
    float OutMin;         // Parameter: Minimum output
    float Out;            // Output: PI output 
	float Err;            // Variable: Error
    float Up;             // Variable: Proportional output 
    float Ui;             // Variable: Integral output 
    float Uff;            // Variable: Feedforward output
} PI_float;            

/*-----------------------------------------------------------------------------
Default initalizer for the PI_float object.
-----------------------------------------------------------------------------*/                     
#define PI_FLOAT_DEFAULTS  	\
{           			 	\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0,    				\
    0.0     				\
}

/*------------------------------------------------------------------------------
 PI Macro Definition
------------------------------------------------------------------------------*/
#define PI_FLOAT_CALC(v)                            \
{                                                   \
    v.Err = v.Ref - v.Fdb;                          \
    v.Up = v.Kp * v.Err;                            \
    v.Uff = v.Ff * v.Kff;                           \
    v.Out = v.Up + v.Ui + v.Uff;                    \
    if (v.Out > v.OutMax)                           \
    {                                               \
        v.Out = v.OutMax;                           \
        if  (v.Ui < 0.0)         					\
        {                                           \
            v.Ui = v.Ki * v.Err + v.Ui;             \
        }                                           \
    }                                               \
    else if (v.Out < v.OutMin)                      \
    {                                               \
        v.Out = v.OutMin;                           \
        if  (v.Ui > 0.0)         					\
        {                                           \
            v.Ui = v.Ki * v.Err + v.Ui;             \
        }                                           \
    }                                               \
    else                                            \
    {                                               \
        v.Ui = v.Ki * v.Err + v.Ui;                 \
    }                                               \
}    
#endif // __PI_REG_H__
