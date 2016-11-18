/****************************************************************
* FILENAME:     ABF_float.h
* DESCRIPTION:  Alfa-Beta filter
* AUTHOR:       David Kavreèiè
* START DATE:   16.1.2009
****************************************************************/
#ifndef     __ABF_FLOAT__
#define     __ABF_FLOAT__

#include    "define.h"

#define     ABF_SAMP_FREQ       SAMP_FREQ

// Diskretizacijski korak trackerja
#define     ABF_SAMP_TIME       (1.0 / ABF_SAMP_FREQ)


typedef struct ABF_FLOAT_STRUCT
{
	float u_cap_measured;       // izmerjena napetost
	float u_cap_estimated;      // ocenjena napetost
	float u_cap_estimated_1;    // prejsnja ocenjena napetost
	float i_cap_estimated;      // ocenjeni tok
	float i_cap_estimated_1;    // prejsnji ocecnjeni tok
	float err_ucap;             // napaka med meritvijo in oceno
	float Alpha;                // Parametra trackerja
	float Beta;
	float Capacitance;          // kapacitivnost

} ABF_float;

#define ABF_FLOAT_DEFAULTS  \
{                           \
	0.0,                    \
	0.0,                    \
	0.0,                    \
	0.0,                    \
	0.0,                    \
	0.0,                    \
	0.0,                    \
    0.0,                    \
	0.0                     \
}

extern void ABF_float_calc(ABF_float *v);

#endif  // __ABF_FLOAT__
