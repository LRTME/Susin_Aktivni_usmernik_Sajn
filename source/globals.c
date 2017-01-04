/****************************************************************
* FILENAME:     globals.c
* DESCRIPTION:  global variables
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/
#include "globals.h"

// stevec prekinitev
float   interrupt_cnt = 0;

// spremenljivka stanja
volatile enum STATE state = Initialization;

volatile enum MODE mode = Control;

//struktura z zastavicami napake
struct FAULT_FLAGS fault_flags =
{
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    FALSE
};

// signaliziram, da je offset kalibriran
volatile bool calibration_done = FALSE;
volatile bool start_calibration = FALSE;




