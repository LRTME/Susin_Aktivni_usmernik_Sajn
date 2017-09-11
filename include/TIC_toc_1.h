/****************************************************************
* FILENAME:     TIC_toc_1.h
* DESCRIPTION:  struct declaration and macro definitions
* AUTHOR:       Mitja Nemec
* START DATE:   8.12.2010
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO         DETAIL
* 1.1       7.9.2017    Denis Sušin  Code changed for Timer 1 (not 0)
*
****************************************************************/
#ifndef     __TIC_TOC_1_H__
#define     __TIC_TOC_1_H__

volatile long TIC_time_1 = 0;

/*------------------------------------------------------------------------------
TIC & TOC macros
------------------------------------------------------------------------------*/
#define TIC_init_1()                        \
{                                           \
    CpuTimer1Regs.PRD.all  = 0xFFFFFFFF;    \
    CpuTimer1Regs.TPR.all  = 0;             \
    CpuTimer1Regs.TPRH.all = 0;             \
    CpuTimer1Regs.TCR.bit.TSS = 1;          \
    CpuTimer1Regs.TCR.bit.TRB = 1;          \
}


#define TIC_start_1()                   	\
{                                      	 	\
    CpuTimer1Regs.TCR.bit.TRB = 1;      	\
	CpuTimer1Regs.TCR.bit.TSS = 0;      	\
    TIC_time_1 = CpuTimer1Regs.TIM.all;   	\
}

#define TIC_stop_1()                      	\
{                                       	\
    CpuTimer1Regs.TCR.bit.TSS = 1;      	\
    TIC_time_1 = TIC_time_1                 \
             - CpuTimer1Regs.TIM.all;   	\
}

#define TIC_laptime_1   (CpuTimer1Regs.TIM.all)



#endif // __TIC_TOC_1_H__
