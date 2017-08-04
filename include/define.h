/****************************************************************
* FILENAME:     define.h           
* DESCRIPTION:  file with global define macros
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/
#ifndef     __DEFINE_H__
#define     __DEFINE_H__

#include	"stddef.h"
#include	"stdbool.h"
#include	"stdint.h"

// kako deluje izhodni pretovrnik 1 - eno vejno, 2 - dvovejno
#define     BB_LEGS         2

// frekvenca PWM ja v Hz
#define     SWITCH_FREQ     40000

// razmerje med preklopno in vzorèno frekvenco
#define     SAMPLING_RATIO  2

// frekvenca vzorèenja v HZ
#define     SAMP_FREQ       (SWITCH_FREQ/SAMPLING_RATIO)

// vzorèna perioda
#define     SAMPLE_TIME     (1.0/SAMP_FREQ)

// frekvenca omrežja
#define		GRID_FREQ		50

// število vzorcev v eni periodi
#define		SAMPLE_POINTS	(SAMP_FREQ/GRID_FREQ)

// frekvenca procesorja v Hz
#define     CPU_FREQ        200000000L

// definicije matematiènih konstant
#define     SQRT3           1.7320508075688772935274463415059
#define     SQRT2           1.4142135623730950488016887242097
#define     ZSQRT2          0.70710678118654752440084436210485
#define     PI              3.1415926535897932384626433832795

// maksimalna izhodna moc v wattih
#define     P_MAX           400
// maksimalni bremenski tok v amperih
#define     I_MAX           20
// maksimalna izhodna napetost
#define     U_MAX           36
// nastavljena napetost enosmernega tokokroga
#define     U_DC_REF        40

#define     CURRENT_GRID_LIM    20
#define     CURRENT_BB_LIM      20
#define     CURRENT_BB_LIM      20
#define     NAP_GRID_RMS_MIN    10
#define     NAP_GRID_RMS_MAX    30
#define     U_DC_MAX            48
#define     U_DC_MIN            0

// deklaracije za logicne operacije
// typedef enum {FALSE = 0, TRUE} bool;
#define     TRUE            true
#define     FALSE           false

// kako naj se obnašajo preriferne naprave, ko ustavimo izvajanje programa
// 0 stop immediately, 1 stop when finished, 2 run free
#define     DEBUG_STOP      2

#endif // end of __DEFINE_H__ definition
