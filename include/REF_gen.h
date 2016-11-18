/****************************************************************
* FILENAME:     REF_gen.h
* DESCRIPTION:  generator tazliènih oblik signala
* AUTHOR:       Mitja Nemec
* START DATE:   1.8.2016
****************************************************************/
#include    "define.h"
#include    "math.h"

enum    REF_TYPE { Step, Slew, Konst, Sine};

struct  REF_GEN
{
    float   out;
    float   kot;
    float   amp;
    float   duty;
    float   offset;
    float   frequency;
    float   sampling_period;
    float   slew_up;
    float   slew_down;
    float   slew_out;
    enum    REF_TYPE type;
};

extern struct REF_GEN ref_gen;

extern void REF_gen(void);
