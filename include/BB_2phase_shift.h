/************************************************************** 
* FILE:         FB_phase_shift.h
* DESCRIPTION:  header file for full bridg driver
* AUTHOR:       Mitja Nemec
* START DATE:   18.8.2010
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO                 DETAIL 
* 1.0       21.12.2009  Mitja Nemec         Initial version
*
****************************************************************/
#ifndef     __BB_2PHASE_SHIFT_H__
#define     __BB_2PHASE_SHIFT_H__

#include    "F28x_Project.h"

#include    "define.h"
#include	"main.h"

/* definicije */
// stevilke PWM modulov, ki krmilijo mostic
// ne pozabi spremeniti GPIO mux registrov
#define     BB_MODUL1      EPwm3Regs
#define     BB_MODUL2      EPwm4Regs

// perioda casovnika (v procesorskih cikilh) 
#define     BB_PERIOD      (((CPU_FREQ/2)/SWITCH_FREQ)/2 - 1)

// dead time (v procesorskih cikilh)
#define     BB_DEAD_TIME   40 //5

// nacin delovanja casovnika ob emulation stop
// (0-stop immediately, 1-stop at zero, 2-run free)
#define     BB_DEBUG_MODE  2

// definicije za status mostica
enum BB_STATE { DISABLE=0, ENABLE, BOOTSTRAP, TRIP };

/**************************************************************
* nastavi mrtvi cas  
**************************************************************/
extern void BB_dead_time(float dead_time);

/**************************************************************
* Izklopi zgornja tranzistorja in vklopi spodnja, tako da je mostic kratkosticen
* in lahko napolneta bootstrap kondenzatorja
* returns:  
**************************************************************/
extern void BB_bootstrap(void);
extern void BB1_bootstrap(void);
extern void BB2_bootstrap(void);

/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns:
**************************************************************/
extern void BB_trip(void);
extern void BB1_trip(void);
extern void BB2_trip(void);

/**************************************************************
* Izklopi vse tranzistorje
* returns:  
**************************************************************/
extern void BB_disable(void);
extern void BB1_disable(void);
extern void BB2_disable(void);

/**************************************************************
* vklopi vse izhode
* returns:  
**************************************************************/
extern void BB_enable(void);
extern void BB1_enable(void);
extern void BB2_enable(void);

/**************************************************************
* vrne status
**************************************************************/
extern int BB1_status(void);
extern int BB2_status(void);

/**************************************************************
* Funkcija, ki popiše registre za PWM1,. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
extern void BB_init(void);

/**************************************************************
* Funkcija, ki osveži registre za PWM
* return: void
**************************************************************/
extern void BB_update(float duty1, float duty2);

/**************************************************************
* Funkcija, ki starta PWM1. Znotraj funkcije nastavimo
* naèin štetja èasovnikov (up-down-count mode)
* return: void
**************************************************************/
extern void BB_start(void);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel
* return: void
* arg1: zelena perioda
**************************************************************/
extern void BB_period(float perioda);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel
* return: void
* arg1: zelena frekvenca
**************************************************************/
extern void BB_frequency(float frekvenca);

#endif  // end of __BB_2PHASE_SHIFT_H__ definition

