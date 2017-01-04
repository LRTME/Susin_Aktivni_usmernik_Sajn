/****************************************************************
* FILENAME:     UP_time.h
* DESCRIPTION:  declarations for uptime counter
* AUTHOR:       Mitja Nemec
****************************************************************/

#ifndef   __UP_TIME_H__
#define   __UP_TIME_H__

#include    "SD_card.h"

// stevec uptime
extern long     uptime;

/**************************************************************
* inicializacija UP time števca
**************************************************************/
extern void UP_init(void);

/**************************************************************
 * poveèanje UP time števca
 **************************************************************/
extern void UP_inc(void);

#endif  // __SD_CARD_H__

