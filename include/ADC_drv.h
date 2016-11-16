/**************************************************************
* FILE:         ADC_drv.c
* DESCRIPTION:  A/D driver for piccolo devices
* AUTHOR:       Mitja Nemec
* DATE:         19.1.2012
*
****************************************************************/
#ifndef     __ADC_DRV_H__
#define     __ADC_DRV_H__

#include    "F28x_Project.h"
#include    "define.h"
#include    "DAC_drv.h"

// doloci kateri PWM modul prozi ADC
#define     ADC_MODUL1      EPwm1Regs

// kje se nahajajo rezultati
#define     ADC_ENC1_SIN    (AdcaResultRegs.ADCRESULT0)
#define     ADC_ENC3_SIN    (AdcaResultRegs.ADCRESULT1)
#define     ADC_NAP_1       (AdcaResultRegs.ADCRESULT2)
#define     ADC_NAP_2       (AdcaResultRegs.ADCRESULT3)
#define     ADC_TEMP        (AdcaResultRegs.ADCRESULT5)

#define     ADC_ENC1_COS    (AdcbResultRegs.ADCRESULT0)
#define     ADC_ENC3_COS    (AdcbResultRegs.ADCRESULT1)
#define     ADC_NAP_DC      (AdcbResultRegs.ADCRESULT2)
#define     ADC_NAP_3       (AdcbResultRegs.ADCRESULT3)

#define     ADC_TOK_3       (AdccResultRegs.ADCRESULT0)

#define     ADC_TOK_2       (AdcdResultRegs.ADCRESULT0)
#define     ADC_TOK_1       (AdcdResultRegs.ADCRESULT1)
#define     ADC_POT_2       (AdcdResultRegs.ADCRESULT2)
#define     ADC_POT_1       (AdcdResultRegs.ADCRESULT3)


/**************************************************************
* inicializiramo ADC
**************************************************************/
extern void ADC_init(void);

/**************************************************************
* Funkcija, ki pocaka da ADC konca s pretvorbo
* vzorcimo...
* return: void
**************************************************************/
extern void ADC_wait(void);

#endif /* __ADC_DRV_H__ */
