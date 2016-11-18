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

// s kakšnim taktom PWM-ja naj prožim ADC
#define     ADC_SAMP_RATIO  SAMPLING_RATIO

// doloci kateri PWM modul prozi ADC
#define     ADC_MODUL1      EPwm1Regs

// kje se nahajajo rezultati

// startani na SOCA
#define     TEMPERATURA     (AdcdResultRegs.ADCRESULT0)
#define     TOK_GRID        (AdcaResultRegs.ADCRESULT0)
#define     TOK_BB1         (AdcaResultRegs.ADCRESULT1)
#define     TOK_BB2         (AdcbResultRegs.ADCRESULT0)
#define     TOK_OUT         (AdcaResultRegs.ADCRESULT2)

// startani na SOCB
#define     NAP_CAP         (AdcaResultRegs.ADCRESULT3)
#define     NAP_OUT         (AdcbResultRegs.ADCRESULT1)
#define     NAP_DC          (AdcbResultRegs.ADCRESULT2)
#define     NAP_GRID        (AdcdResultRegs.ADCRESULT1)

#define     POT_I_FINE      (AdccResultRegs.ADCRESULT0)
#define     POT_I_COARSE    (AdccResultRegs.ADCRESULT1)
#define     POT_U_FINE      (AdcaResultRegs.ADCRESULT4)
#define     POT_U_COARSE    (AdccResultRegs.ADCRESULT2)
#define     TEMP_CPU        (AdcdResultRegs.ADCRESULT0)

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
