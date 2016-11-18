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
* Funckija ki brcne zunanji WatchDog se lahko
* kliče samo iz periodicne prekinitve
**************************************************************/
extern void PCB_kick_dog_int(void);

/**************************************************************
* Funckija, ki pove CPLD da je temperatura hladilnika normalna
**************************************************************/
extern void TEMP_normal(void);

/**************************************************************
* Funckija, ki pove CPLD da je temperatura hladilnika kritična
**************************************************************/
extern void TEMP_critical(void);

/**************************************************************
* Funckija, ki vklopi rele1
**************************************************************/
extern void PCB_in_relay_on(void);

/**************************************************************
* Funckija, ki izklopi rele1(za priklop omrežne napetosti)
**************************************************************/
extern void PCB_in_relay_off(void);

/**************************************************************
* Funckija, ki vklopi rele2
**************************************************************/
extern void PCB_res_relay_off(void);

/**************************************************************
* Funckija, ki izklopi rele2(premostitveni rele)
**************************************************************/
extern void PCB_res_relay_on(void);

/**************************************************************
* Funckija, ki vklopi rele3
**************************************************************/
extern void PCB_out_relay_on(void);

/**************************************************************
* Funckija, ki izklopi rele3(izhodni rele-priklop bremena)
**************************************************************/
extern void PCB_out_relay_off(void);

/**************************************************************
* Funckija, ki vklopi rele2
**************************************************************/
extern void PCB_load_relay_on(void);

/**************************************************************
* Funckija, ki izklopi rele2(premostitveni rele)
**************************************************************/
extern void PCB_load_relay_off(void);

/**************************************************************
* Funckija, ki vklopi LED indikacijo tokovnega režima
**************************************************************/
extern void PCB_mode_LED_on(void);

/**************************************************************
* Funckija, ki izklopi LED indikacijo tokovnega režima(delamo v napetostnem režimu)
**************************************************************/
extern void PCB_mode_LED_off(void);

/**************************************************************
* Funckija ki resetira zapah na prilagodilni tiskanini
*
* !!!!!!!!!!!!!!!!!!!!!
* je ne smeš klicati iz prekinitve
* !!!!!!!!!!!!!!!!!!!!
*
**************************************************************/
extern void PCB_lat_reset(void);

/**************************************************************
* Funckija ki prizge enable LED
**************************************************************/
extern void PCB_en_LED_on(void);

/**************************************************************
* Funckija ki ugasne enable LED
**************************************************************/
extern void PCB_en_LED_off(void);

/**************************************************************
* Funckija ki spremeni stanje fault LED
**************************************************************/
extern void PCB_en_LED_toggle(void);

/**************************************************************
* Funckija ki vrne stanje on tipke
**************************************************************/
extern bool PCB_en_SW(void);

/**************************************************************
* Funckija ki vrne stanje mode tipke
**************************************************************/
extern bool PCB_reset_SW(void);

/**************************************************************
* Funkcije ki manipulirajo LED diodo  na ControlCardu
**************************************************************/
extern void PCB_LEDcard_on(void);
extern void PCB_LEDcard_off(void);
extern void PCB_LEDcard_toggle(void);

/**************************************************************
* Funckija ki vrne stanje strojne za��ite
**************************************************************/
extern bool PCB_HW_trip(void);

/**************************************************************
* Funckija ki inicializira tiskanino
**************************************************************/
extern void PCB_init(void);


#endif  // end of PCB_UTIL_H definition

