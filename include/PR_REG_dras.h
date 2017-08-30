/****************************************************************
* FILENAME:     PR_float.h
* DESCRIPTION:  Proporcionalno resonan�ni regulator
* AUTHOR:       Dra� Kun�i�
****************************************************************/
#ifndef INCLUDE_PR_FLOAT_H_
#define INCLUDE_PR_FLOAT_H_

	#include    "define.h"
	#include    "math.h"
    #ifndef PI
    #define PI  3.1415926535897932384626433832795
    #endif

	typedef struct PR_FLOAT_STRUCT
	{
		float Ref;            // (Vhod) �eljena vrednost
		float Kot;            // (Vhod) Normiran kot �eljene vrednosti
		float Fdb;            // (Vhod) Povratna informacija
		float Err;            //        Napaka
		float SinFi;		  //        Izra�un Sin(Kot)
		float CosFi;		  //        Izra�un Cos(Kot)
		float Kp;             // (Vhod) Proporcionalno oja�enje
		float Kr;             // (Vhod) Resonan�no oja�enje
		float Up;             //        Proporcionalni izhod
		float Uis;            //        Sin resonan�ni izhod
		float Uic;            //        Cos resonan�ni izhod
		float OutMax;         // (Vhod) Zgornja omejitev izhoda
		float OutMin;         // (Vhod) Spodnja omejitev izhoda
		float Out;            // (Izhod)Izhodnja vrednost
	} PR_float;

	#define PR_FLOAT_DEFAULTS   \
	{           				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,   				 	\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
		0.0,    				\
	}

	/*-----------------------------------------------------------------------------
	 Makro proporcionalno resonan�nega regulatorja
	-----------------------------------------------------------------------------*/
	#define PR_FLOAT_CALC(v)							\
	{													\
		v.SinFi = sin(2 * PI * v.Kot);					\
		v.CosFi = cos(2 * PI * v.Kot);					\
		v.Err = v.Ref - v.Fdb;							\
        v.Uis = v.Uis + (v.Err * v.SinFi * v.Kr);       \
        v.Uic = v.Uic + (v.Err * v.CosFi * v.Kr);       \
		v.Out = v.Kp * v.Err + (v.Uis * v.SinFi + v.Uic * v.CosFi); \
		if (v.Out > v.OutMax)                           \
		{                                               \
			v.Out = v.OutMax;                           \
	        v.Uis = v.Uis - (v.Err * v.SinFi * v.Kr);   \
	        v.Uic = v.Uic - (v.Err * v.CosFi * v.Kr);   \
		}												\
		else if (v.Out < v.OutMin)                      \
		{                                           	\
			v.Out = v.OutMin;                       	\
	        v.Uis = v.Uis - (v.Err * v.SinFi * v.Kr);   \
	        v.Uic = v.Uic - (v.Err * v.CosFi * v.Kr);   \
		}												\
	}

#endif
