/****************************************************************
* FILENAME:     TIC_toc.h             
* DESCRIPTION:  struct declaration and macro definitions
* AUTHOR:       Mitja Nemec
* START DATE:   8.12.2010
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       8.12.2010   Mitja Nemec Initial version
*
****************************************************************/
#ifndef     __TIC_TOC_H__
#define     __TIC_TOC_H__

volatile long TIC_time = 0;

/*------------------------------------------------------------------------------
TIC & TOC macros
------------------------------------------------------------------------------*/
#define TIC_init()                          \
{                                           \
    CpuTimer0Regs.PRD.all  = 0xFFFFFFFF;    \
    CpuTimer0Regs.TPR.all  = 0;             \
    CpuTimer0Regs.TPRH.all = 0;             \
    CpuTimer0Regs.TCR.bit.TSS = 1;          \
    CpuTimer0Regs.TCR.bit.TRB = 1;          \
}


#define TIC_start()                     \
{                                       \
    CpuTimer0Regs.TCR.bit.TSS = 0;      \
    TIC_time = CpuTimer0Regs.TIM.all;   \
}
        
#define TIC_stop()                      \
{                                       \
    CpuTimer0Regs.TCR.bit.TSS = 1;      \
    TIC_time = TIC_time                 \
             - CpuTimer0Regs.TIM.all;   \
}

#define TIC_laptime   (CpuTimer0Regs.TIM.all)
    

        
#endif // __TIC_TOC_H__
