/****************************************************************
* FILENAME:     DLOG_gen.h             
* DESCRIPTION:  struct and func declarations for data logger module  
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.1
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
* 1.1       23.2.2010   Mitja Nemec Added various trigger modes
*
****************************************************************/
#ifndef     __DLOG_GEN_H__
#define     __DLOG_GEN_H__

#include    "define.h"

// type of element to buffer
#define     DLOG_GEN_TYPE   float
// number of channels
#define     DLOG_GEN_NR     4
// size of each buffer
#define     DLOG_GEN_SIZE   1000

// function prototype
void DLOG_GEN_update(void);

enum DLOG_MODE { Auto, Normal, Single, Stop, Continuous};

enum DLOG_SLOPE { Positive, Negative};

enum DLOG_STATE { Wait, Ready, Store, Holdoff};

extern DLOG_GEN_TYPE DLOG_b_1[];

#if DLOG_GEN_NR > 1
extern DLOG_GEN_TYPE DLOG_b_2[];
#endif

#if DLOG_GEN_NR > 2
extern DLOG_GEN_TYPE DLOG_b_3[];
#endif
#if DLOG_GEN_NR > 3
extern DLOG_GEN_TYPE DLOG_b_4[];
#endif
#if DLOG_GEN_NR > 4
extern DLOG_GEN_TYPE DLOG_b_5[];
#endif
#if DLOG_GEN_NR > 5
extern DLOG_GEN_TYPE DLOG_b_6[];
#endif
#if DLOG_GEN_NR > 6
extern DLOG_GEN_TYPE DLOG_b_7[];
#endif
#if DLOG_GEN_NR > 7
extern DLOG_GEN_TYPE DLOG_b_8[];
#endif

struct DLOG
{
    DLOG_GEN_TYPE   *trig;      // pointer to trigger value
    DLOG_GEN_TYPE   *iptr1;     // Input: First input pointer (Q15)
    #if DLOG_GEN_NR > 1
    DLOG_GEN_TYPE   *iptr2;     // Input: Second input pointer (Q15)
    #endif
    #if DLOG_GEN_NR > 2
    DLOG_GEN_TYPE   *iptr3;     // Input: Third input pointer (Q15)
    #endif
    #if DLOG_GEN_NR > 3
    DLOG_GEN_TYPE   *iptr4;     // Input: Fourth input pointer (Q15)
    #endif
    #if DLOG_GEN_NR > 4
    DLOG_GEN_TYPE   *iptr5;     // Input: Fifth input pointer (Q15)
    #endif
    #if DLOG_GEN_NR > 5
    DLOG_GEN_TYPE   *iptr6;     // Input: Sixth input pointer (Q15)
    #endif
    #if DLOG_GEN_NR > 6
    DLOG_GEN_TYPE   *iptr7;     // Input: Seventh input pointer (Q15)
    #endif
    #if DLOG_GEN_NR > 7
    DLOG_GEN_TYPE   *iptr8;     // Input: Eighth input pointer (Q15)
    #endif

    // common data
    DLOG_GEN_TYPE   trig_value;     // Input: Trigger point
    unsigned int    prescalar;      // Parameter: Data log prescale
    unsigned int    skip_cntr;      // Variable:  Data log skip counter
    unsigned int    write_ptr;      // Variable:  Graph address pointer
    unsigned int    write_length;   //
    enum DLOG_STATE      state;          // state of internal state machine
    enum DLOG_SLOPE      slope;          //
    long            auto_time;
    long            auto_cnt;       // counter for auto trigger in auto mode
    long            holdoff_time;
    long            holdoff_cnt;
    enum DLOG_MODE       mode;           // mode selection
    int             sw_trigger;
};

extern struct DLOG dlog;

#endif // __DLOG_GEN_H__
