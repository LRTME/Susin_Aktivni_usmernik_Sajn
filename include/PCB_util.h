/**************************************************************
* FILE:         PCB_util.h 
* DESCRIPTION:  definitions for PCB Initialization & Support Functions
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
**************************************************************/
#ifndef   PCB_UTIL_H
#define   PCB_UTIL_H

#include    "F28x_Project.h"
#include    "define.h"

/**************************************************************
* Funkcije ki manipulirajo LED diodo  na ControlCardu
**************************************************************/
extern void PCB_LEDcard_on(void);
extern void PCB_LEDcard_off(void);
extern void PCB_LEDcard_toggle(void);

/**************************************************************
* Funkcija ki inicializa MCU in tiskanino
**************************************************************/
extern void PCB_init(void);


#endif  // end of PCB_UTIL_H definition

