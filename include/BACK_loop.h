/****************************************************************
* FILENAME:     BACK_loop.h             
* DESCRIPTION:  background code header file
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/
#ifndef     __BACK_LOOP_H__
#define     __BACK_LOOP_H__

#include    "F28x_Project.h"
#include    "define.h"
#include    "globals.h"

#include    "PWM_drv.h"
#include 	"PCB_util.h"
#include    "PER_int.h"
    
/**************************************************************
* Funkcija, ki se izvaja v ozadju med obratovanjem
**************************************************************/
extern void BACK_loop(void);
#endif // end of __BACK_LOOP_H__
