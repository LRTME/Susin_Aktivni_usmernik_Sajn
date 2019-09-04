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

#include    "FB_bipolar.h"
#include    "ADC_drv.h"

#include    "DLOG_gen.h"
#include    "REF_gen.h"

#include    "PID_float.h"
#include    "DC_float.h"
#include    "DFT_float.h"
#include    "SLEW_float.h"
#include    "STAT_float.h"
#include    "ABF_float.h"
#include    "DELAY_float.h"

#include    "SD_card.h"

#include	"PI_REG.h"
#include	"RES_REG.h"
#include	"DCT_REG.h"
#include 	"dual_DCT_REG.h"
#include	"REP_REG.h"

/**************************************************************
* Funckija, ki pripravi vse potrebno za izvajanje
* prekinitvene rutine
**************************************************************/
extern void PER_int_setup(void);

#endif // end of __PER_INT_H__ definition
