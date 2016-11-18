/****************************************************************
* FILENAME:     ABF_float.c
* DESCRIPTION:  Alfa-Beta filter
* AUTHOR:       David Kavreèiè
* START DATE:   16.1.2009
****************************************************************/

#include "ABF_float.h"


#pragma CODE_SECTION(ABF_float_calc, "ramfuncs");
void ABF_float_calc(ABF_float *v)
{
    // PREDIKCIJA
	v->u_cap_estimated = v->u_cap_estimated_1 + v->i_cap_estimated_1 * ABF_SAMP_TIME / v->Capacitance;
	v->i_cap_estimated = v->i_cap_estimated_1;

	// NAPAKA
	v->err_ucap = v->u_cap_measured - v->u_cap_estimated;

	// KOREKCIJA
	v->u_cap_estimated = v->u_cap_estimated + v->Alpha * v->err_ucap;
	v->i_cap_estimated = v->i_cap_estimated + v->Beta * v->err_ucap * v->Capacitance * ABF_SAMP_FREQ;

	// SHRANI VREDNOSTI ZA NASLEDNJO ITERACIJO
	v->u_cap_estimated_1 = v->u_cap_estimated;
	v->i_cap_estimated_1 = v->i_cap_estimated;

}
