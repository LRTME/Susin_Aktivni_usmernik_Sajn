/****************************************************************
* FILENAME:     PER_int.h
* DESCRIPTION:  periodic interrupt header file
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/
#ifndef     __PER_INT_H__
#define     __PER_INT_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "globals.h"

#include    "PWM_drv.h"
#include    "ADC_drv.h"
#include    "PCB_util.h"
#include    "math.h"

#include    "DLOG_gen.h"

/**************************************************************
* Funckija, ki pripravi vse potrebno za izvajanje
* prekinitvene rutine
**************************************************************/
extern void PER_int_setup(void);

#endif // end of __PER_INT_H__ definition
