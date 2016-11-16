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

// frekvenca PWM-ja
#define     SWITCH_FREQ     20000L

// veèkratnik preklopne frekvence
#define     SAMP_PRESCALE   1

// Vzorèna frekvenca
#define     SAMPLE_FREQ     (SWITCH_FREQ/SAMP_PRESCALE)

// vzorèna perioda
#define     SAMPLE_TIME     (1.0/SAMPLE_FREQ)

// katero strojno opremo uporabljam
// 0 - FE sistem za vaje PMSM + DC + RM44,
// 1 - RLS naprava verzija 1, LM10 ali RM44 ali AKSIM
// 2 - RLC naprava verzija 2
// 3 - FE sistem za študije PMSM + RM44
// 4 - FE sistem za študeije PMSM 9 fazni + resolver + RMB28
#define     HW_TYPE         4

// frekvenca procesorja v Hz
#define     CPU_FREQ        200000000L

// definicije matematiènih konstant
#define     SQRT3           1.7320508075688772935274463415059
#define     SQRT2           1.4142135623730950488016887242097
#define     PI              3.1415926535897932384626433832795

// deklaracije za logicne operacije
//typedef enum {FALSE = 0, TRUE} bool;
#define     TRUE            true
#define     FALSE           false

// kako naj se obnašajo preriferne naprave, ko ustavimo izvajanje programa
// 0 stop immediately, 1 stop when finished, 2 run free
#define     DEBUG_STOP      2

#endif // end of __DEFINE_H__ definition
