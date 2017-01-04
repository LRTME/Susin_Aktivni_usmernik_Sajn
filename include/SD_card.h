/****************************************************************
* FILENAME:     SD_card.h
* DESCRIPTION:  declarations for SD card support
* AUTHOR:       Mitja Nemec
****************************************************************/

#ifndef   __SD_CARD_H__
#define   __SD_CARD_H__

#include    "ff.h"
#include    "diskio.h"

#define     SD_tick_timer   disk_timerproc

/**************************************************************
 * inicializacija SD kartice
 **************************************************************/
extern void SD_init(void);



#endif  // __SD_CARD_H__

