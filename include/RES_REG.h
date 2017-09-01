/****************************************************************
* FILENAME:     RES_REG.h
* DESCRIPTION:  Resonant controller (regulator) which is reducing periodic error (only one harmonic component)
* AUTHOR:       Denis Sušin
* START DATE:   6.4.2016
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016   Denis Sušin      Initial version
*
****************************************************************/

#ifndef __RES_REG_H__
#define __RES_REG_H__

#include    "math.h"

    #ifndef PI
    #define PI  3.1415926535897932384626433832795
    #endif

typedef struct RES_REG_FLOAT_STRUCT
{
    float Ref;            // Input: Reference input 
    float Fdb;            // Input: Feedback input 
    float Kot;            // Input: Angle that controls resonant controller [0,1)
    float Ff;             // Input: Feedforward input
    float Kres;           // Parameter: Resonant controller gain
    float Kff;            // Parameter: Feedforward gain
	float OutMax;         // Parameter: Maximum output 
    float OutMin;         // Parameter: Minimum output
    float Out;            // Output: Controller output 
	float Err;            // Variable: Error
    float Cos;            // Variable: cosine of angle
    float Sin;            // Variable: sine of angle
	float Ui1;            // Variable: Output of I controller in cosine branch
    float Ui2;            // Variable: Output of I controller in sine branch
	float Ucos;           // Variable: Output of cosine branch
    float Usin;           // Variable: Output of sine branch
    float Ures;			  // Variable: Resonant controller output
    float Uff;            // Variable: Feedforward output
} RES_REG_float;


#define RES_REG_FLOAT_DEFAULTS  \
{           					\
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
    0.0,     					\
	0.0,     					\
	0.0,     					\
	0.0,     					\
	0.0     					\
}

#define RES_REG_CALC(v)                       		\
{                                                   \
	if (v.Kot > 1.0)								\
    {                                               \
        v.Kot = 1.0;                           		\
    }                                               \
	if (v.Kot < 0.0)								\
    {                                               \
        v.Kot = 0.0;                           		\
    }                                               \
    v.Err = v.Ref - v.Fdb;                          \
	v.Cos = cos(2 * PI * v.Kot);                    \
	v.Sin = sin(2 * PI * v.Kot);                    \
	v.Ucos = v.Ui1 * v.Cos;                    	    \
	v.Usin = v.Ui2 * v.Sin;                    		\
	v.Ures = v.Kres * (v.Ucos + v.Usin);			\
	v.Uff = v.Kff * v.Ff;                           \
    v.Out = v.Ures + v.Uff;         				\
    if (v.Out > v.OutMax)                           \
    {                                               \
        v.Out = v.OutMax;                           \
    }                                               \
    else if (v.Out < v.OutMin)                      \
    {                                               \
        v.Out = v.OutMin;                           \
    }                                               \
    else                                            \
    {                                               \
		v.Ui1 = v.Ui1 + v.Err * v.Cos;            	\
		v.Ui2 = v.Ui2 + v.Err * v.Sin;            	\
    }                                               \
} 
#endif // __RES_REG_H__
