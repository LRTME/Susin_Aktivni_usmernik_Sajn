/************************************************************** 
* FILE:         PWM_drv.c
* DESCRIPTION:  PWM driver for TMS320F28069
* AUTHOR:       Andraž Kontarèek
* START DATE:   21.12.2009
* VERSION:      1.1
*
* CHANGES : 
* VERSION   DATE        WHO                 DETAIL 
* 1.0       21.12.2009  Andraž Kontarèek    Initial version
* 1.1       5.3.2010    Mitja Nemec         Changed to generic
* 1.2       29.3.2012   Mitja Nemec         Decoupled ADC part into separate file
*
****************************************************************/
#ifndef     __PWM_DRV_H__
#define     __PWM_DRV_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "globals.h"

/* Definicije */
// delovanje modula ob debug-dogodkih
// (0-stop at zero, 1-stop immediately, 2-run free)
#define     PWM_DEBUG       0

// perioda casovnika (v procesorskih cikilh) 
#define     PWM_PERIOD      ((CPU_FREQ/2)/(SAMPLE_FREQ * SAMP_PRESCALE))

// prescaler za prekinitev
#define     PWM_INT_PSCL    SAMP_PRESCALE

/**************************************************************
* Funkcija, poklièe funkciji PWM_PWM_init in PWM_ADC_init; klièemo
* jo iz main-a
* return: void
**************************************************************/
extern void PWM_init(void);

/**************************************************************
* Funkcija, ki popiše registre za PWM1,. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
extern void PWM_update(float duty);

/**************************************************************
* Funkcija, ki starta PWM1. Znotraj funkcije nastavimo
* naèin štetja èasovnikov (up-down-count mode)
* return: void
**************************************************************/
extern void PWM_start(void);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel natancno
* return: void
* arg1: zelena perioda
**************************************************************/
extern void PWM_period(float perioda);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel natancno
* return: void
* arg1: zelena frekvenca
**************************************************************/
extern void PWM_frequency(float frekvenca);

#endif  // end of __PWM_DRV_H__ definition

