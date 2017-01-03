/****************************************************************
* FILENAME:     globals.h
* DESCRIPTION:  global variables header file
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/
#ifndef     __GLOBALS_H__
#define     __GLOBALS_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "SLEW_float.h"
#include    "PID_float.h"

// stevec prekinitev
extern float    interrupt_cnt;

// seznam globalnih spremenljivk
extern volatile enum STATE { Initialization = 0, Startup, Standby, Ramp_up, Work, Ramp_down, Fault, Fault_sensed} state;

extern volatile enum MODE { Open_loop = 0, Control} mode;

//struktura z zastavicami napake
extern struct FAULT_FLAGS
{
    bool    overcurrent_bb:1;
    bool    overcurrent_grid:1;
    bool    HW_trip:1;
    bool    undervoltage_dc:1;
    bool    overvoltage_dc:1;
    bool    undervoltage_grid:1;
    bool    overvoltage_grid:1;
    bool    cpu_overrun:1;
    bool    fault_registered:1;
} fault_flags;

// signaliziram, da je offset kalibriran
extern volatile bool calibration_done;
extern volatile bool start_calibration;


// za zagon in delovanje
extern SLEW_float   nap_dc_slew;
extern SLEW_float   tok_bb_slew;
extern SLEW_float   nap_out_slew;
extern PID_float    nap_out_reg;
extern PID_float    nap_dc_reg;

extern float tok_bb1;
extern float tok_bb2;

extern float nap_dc;
extern float nap_grid_rms;
extern float nap_out;
extern float nap_cap;


#endif // end of __GLOBALS_H__ definition
