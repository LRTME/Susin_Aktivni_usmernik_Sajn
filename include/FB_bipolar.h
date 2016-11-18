/************************************************************** 
* FILE:         FB_bipolar.h
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
#ifndef     __FB_BIPOLAR_H__
#define     __FB_BIPOLAR_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "globals.h"
#include	"main.h"

/* Definicije */
// stevilke PWM modulov, ki krmilijo mostic
// ne pozabi spremeniti GPIO mux registrov
#define     FB_MODUL1        EPwm1Regs
#define     FB_MODUL2        EPwm2Regs

// delovanje modula ob debug-dogodkih
// (0-stop at zero, 1-stop immediately, 2-run free)
#define     FB_DEBUG        2

// perioda casovnika (v procesorskih cikilh) 
#define     FB_PERIOD       ((CPU_FREQ/SWITCH_FREQ)/2 - 1)   //50us

// dead time (v procesorskih cikilh)
#define     FB_DEAD_TIME    (40)

// definicije za status mostica
enum FB_STATE { FB_DIS=0, FB_EN, FB_BOOTSTRAP, FB_TRIP};

/**************************************************************
* nastavi mrtvi cas
**************************************************************/
extern void FB_dead_time(float dead_time);

/**************************************************************
* Izklopi zgornja tranzistorja in vklopi spodnja, tako da je mostic kratkosticen
* in lahko napolneta bootstrap kondenzatorja
* returns:  
**************************************************************/
extern void FB_bootstrap(void);

/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns:
**************************************************************/
extern void FB_trip(void);

/**************************************************************
* Izklopi vse tranzistorje
* returns:  
**************************************************************/
extern void FB_disable(void);

/**************************************************************
* vklopi vse izhode
* returns:  
**************************************************************/
extern void FB_enable(void);

/**************************************************************
* Funkcija, ki popiše registre za PWM1,. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
extern void FB_init(void);

/**************************************************************
* Funkcija, ki osveži registre za PWM1
* return: void
**************************************************************/
extern void FB_update(float duty);

/**************************************************************
* Funkcija, ki starta PWM1. Znotraj funkcije nastavimo
* naèin štetja èasovnikov (up-down-count mode)
* return: void
**************************************************************/
extern void FB_start(void);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel natanèno
* return: void
* arg1: zelena perioda
**************************************************************/
extern void FB_period(float perioda);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel natanèno
* return: void
* arg1: zelena frekvenca
**************************************************************/
extern void FB_frequency(float frekvenca);

/**************************************************************
* return:status
**************************************************************/
extern int FB_status(void);


#endif  // end of __FB_BIPOLAR_H__ definition

