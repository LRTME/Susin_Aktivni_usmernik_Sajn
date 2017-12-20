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
#define     TEMPERATURA     (AdcdResultRegs.ADCRESULT0) // A5 -> D2
#define     TOK_BB1         (AdcaResultRegs.ADCRESULT1) // B1 -> A1
#define     TOK_BB2         (AdcbResultRegs.ADCRESULT0) // A0 -> B0
#define     TOK_OUT         (AdcaResultRegs.ADCRESULT2) // B2 -> A3

// startani na SOCB
#define     TOK_GRID        (AdcaResultRegs.ADCRESULT0) // B0 -> A0
#define     NAP_CAP         (AdcaResultRegs.ADCRESULT3) // B3 -> A4
#define     NAP_OUT         (AdcbResultRegs.ADCRESULT1) // A2 -> B3
#define     NAP_DC          (AdcbResultRegs.ADCRESULT2) // A1 -> B1
#define     NAP_GRID        (AdcdResultRegs.ADCRESULT1) // A4 -> D1

#define     POT_I_FINE      (AdccResultRegs.ADCRESULT0) // B6 -> C3
#define     POT_I_COARSE    (AdccResultRegs.ADCRESULT1) // B7 -> C4
#define     POT_U_FINE      (AdcaResultRegs.ADCRESULT4) // B4 -> A5
#define     POT_U_COARSE    (AdccResultRegs.ADCRESULT2) // B5 -> C2

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
