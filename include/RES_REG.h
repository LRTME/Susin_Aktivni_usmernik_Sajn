/****************************************************************
* FILENAME:     RES_REG.h
* DESCRIPTION:  Resonant controller (regulator) which is reducing periodic error (only one harmonic component)
* AUTHOR:       Denis Sušin
* START DATE:   29.3.2018
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016	Denis Sušin     Initial version
* 2.0		29.3.2018	Denis Sušin     Phase lag compensation added, feedforward removed
* 3.0		3.1.2019	Denis Sušin     Added limitation of integral parts
* 3.1		15.5.2019   Denis Sušin     Added some comments
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
    float Angle;          // Input: Angle that controls resonant controller [0,1)
    int	  Harmonic;		  // Input: Harmonic
    float Kres;           // Parameter: Resonant controller gain
    float PhaseCompDeg;	  // Parameter: Phase lag compensation in degrees
	float OutMax;         // Parameter: Maximum output
    float OutMin;         // Parameter: Minimum output
    float Out;            // Output: Controller output 
	float Err;            // Variable: Error
    float Cos;            // Variable: cosine of angle
    float Sin;            // Variable: sine of angle
    float CosComp;        // Variable: cosine of angle and phase lag compensation
    float SinComp;        // Variable: sine of angle and phase lag compensation
	float Ui1;            // Variable: Output of I controller in cosine branch
    float Ui2;            // Variable: Output of I controller in sine branch
	float Ucos;           // Variable: Output of cosine branch with compensation
    float Usin;           // Variable: Output of sine branch with compensation
} RES_REG_float;


#define RES_REG_FLOAT_DEFAULTS  \
{           					\
    0.0,    					\
    0.0,    					\
    0.0,    					\
	0,    						\
	0.0,      					\
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
}




/****************************************************************************************************
* Makro, ki izvede algoritem resonanènega regulatorja.
* Zelo zaželeno je, da je razmerje med vzorèno frekvenco in osnovno frekvenco reguliranega signala
* enako celemu številu (in veèjemu od 20), saj je regulator na to obèutljiv,
* kar lahko privede do nezanemarljivega pogreška v stacionarnem stanju.
****************************************************************************************************/
#define RES_REG_CALC(v)                            									\
{                                                   								\
	if (v.Angle > 1.0)																\
    {                                               								\
        v.Angle = 1.0;                           									\
    }                                               								\
	if (v.Angle < 0.0)																\
    {                                               								\
        v.Angle = 0.0;                           									\
    }                                               								\
    v.Err = v.Ref - v.Fdb;    			            								\
	v.Cos = cos(2 * PI * v.Harmonic * v.Angle);    									\
	v.Sin = sin(2 * PI * v.Harmonic * v.Angle);    									\
	v.CosComp = cos(2 * PI * v.Harmonic * v.Angle + v.PhaseCompDeg * PI / 180.0);	\
	v.SinComp = sin(2 * PI * v.Harmonic * v.Angle + v.PhaseCompDeg * PI / 180.0);	\
	v.Ucos = v.Ui1 * v.CosComp;                 	    							\
	v.Usin = v.Ui2 * v.SinComp;                  									\
	v.Out = v.Ucos + v.Usin;														\
    if (v.Out > v.OutMax)                           								\
    {                                               								\
        v.Out = v.OutMax;                           								\
    }                                               								\
    else if (v.Out < v.OutMin)                      								\
    {                                               								\
        v.Out = v.OutMin;                           								\
    }                                               								\
    else                                            								\
    {                                               								\
		v.Ui1 = v.Ui1 + v.Kres * v.Err * v.Cos;    									\
		v.Ui2 = v.Ui2 + v.Kres * v.Err * v.Sin;     								\
    }                                               								\
																					\
	if(v.Ui1 > v.OutMax)															\
    {                                               								\
		v.Ui1 = v.OutMax;    														\
    }                                               								\
	else if(v.Ui1 < v.OutMin)														\
    {                                               								\
		v.Ui1 = v.OutMin;    														\
    }                                               								\
																					\
	if(v.Ui2 > v.OutMax)															\
    {                                               								\
		v.Ui2 = v.OutMax;    														\
    }                                               								\
	else if(v.Ui2 < v.OutMin)														\
    {                                               								\
		v.Ui2 = v.OutMin;    														\
    }                                               								\
} 
#endif // __RES_REG_H__
