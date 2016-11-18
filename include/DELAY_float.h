/****************************************************************
* FILENAME:     DELAY_float.h
* DESCRIPTION:  declarations of Initialization & Support Functions.
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
****************************************************************/
#ifndef __DELAY_FLOAT_H__
#define __DELAY_FLOAT_H__

// kakšna je najveèja zakasnitev
#define     DELAY_LINE_LENGTH   100

typedef struct DELAY_FLOAT_STRUCT
{
    float           in;
    float           out;
    int             delay;
    int             index;
    int             read_index;
    float           buffer[DELAY_LINE_LENGTH+1];
} DELAY_float;

/*-----------------------------------------------------------------------------
Default initalizer for the DELAY_float object.
-----------------------------------------------------------------------------*/                     
#define DELAY_FLOAT_DEFAULTS    \
{                               \
    0.0,                        \
    0.0,                        \
    0,                          \
    0,                          \
    0                           \
}

/*------------------------------------------------------------------------------
 DELAY Macro Definition
------------------------------------------------------------------------------*/
#define DELAY_FLOAT_CALC(v)                                 \
{                                                           \
    v.buffer[v.index] = v.in;                               \
    if (v.delay > DELAY_LINE_LENGTH)                        \
    {                                                       \
        v.delay = DELAY_LINE_LENGTH;                        \
    }                                                       \
    v.read_index = v.index - v.delay;                       \
    if (v.read_index < 0)                                   \
    {                                                       \
        v.read_index = v.read_index + DELAY_LINE_LENGTH;    \
    }                                                       \
    v.out = v.buffer[v.read_index];                         \
    v.index = v.index + 1;                                  \
    if (v.index >= DELAY_LINE_LENGTH)                        \
    {                                                       \
        v.index = 0;                                        \
    }                                                       \
}

/*------------------------------------------------------------------------------
 DELAY Macro Definition for initialization function
------------------------------------------------------------------------------*/
#define DELAY_FLOAT_INIT(v)                                         \
{                                                                   \
    for (v.index = 0; v.index < (DELAY_LINE_LENGTH+1); v.index++)   \
    {                                                               \
        v.buffer[v.index] = 0.0;                                    \
    }                                                               \
    v.index = 0;                                                    \
}

#endif // __DELAY_FLOAT_H__
