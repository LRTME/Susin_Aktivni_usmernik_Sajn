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
#ifndef     __PWM_DRV_FAN_H__
#define     __PWM_DRV_FAN_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "main.h"

/* definicije */
// stevilke PWM modulov, ki krmilijo mostic
// ne pozabi spremeniti GPIO mux registrov
#define     FAN_MODUL1      EPwm5Regs


// perioda casovnika (v procesorskih cikilh)
#define     FAN_PERIOD      (((CPU_FREQ/2)/SWITCH_FREQ) - 1)


// nacin delovanja casovnika ob emulation stop
// (0-stop at zero, 1-stop immediately, 2-run free)
#define     FAN_DEBUG_MODE  2//

// definicije za status mostica
enum FAN_STATE { DIS=0, EN };


/**************************************************************
* Izklopi vse tranzistorje
* returns:
**************************************************************/
extern void FAN_disable(void);

/**************************************************************
* vklopi vse izhode
* returns:
**************************************************************/
extern void FAN_enable(void);

/**************************************************************
* vrne status
**************************************************************/
extern int FAN_status(void);

/**************************************************************
* Funkcija, ki popiše registre za PWM1,. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
extern void FAN_init(void);

/**************************************************************
* Funkcija, ki osveži registre za PWM
* return: void
**************************************************************/
extern void FAN_update(float duty1);

/**************************************************************
* Funkcija, ki starta PWM1. Znotraj funkcije nastavimo
* naèin štetja èasovnikov (up-down-count mode)
* return: void
**************************************************************/
extern void FAN_start(void);


#endif  // end of __BB_2PHASE_SHIFT_H__ definition

