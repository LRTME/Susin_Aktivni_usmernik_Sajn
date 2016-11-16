/**************************************************************
* FILE:         DAC_drv.c
* DESCRIPTION:  DAC driver for 28377
* AUTHOR:       Mitja Nemec
* DATE:         15.7.2015
*
****************************************************************/
#ifndef     __DAC_DRV_H__
#define     __DAC_DRV_H__

#include    "F28x_Project.h"
#include    "define.h"

/**************************************************************
* inicializiramo DAC
**************************************************************/
extern void DAC_init(bool use_external_ref);

/**************************************************************
* osvežimo vrednost na DAC-u
**************************************************************/
extern void DAC_update_a(float napetost);

extern void DAC_update_a_signed(float napetost);

extern void DAC_update_b_signed(float napetost);

/**************************************************************
* osvežimo vrednost na DAC-u
**************************************************************/
extern void DAC_update_b(float napetost);

#endif /* __ADC_DRV_H__ */
