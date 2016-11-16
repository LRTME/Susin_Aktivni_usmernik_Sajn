/****************************************************************
* FILENAME:     PER_int.c
* DESCRIPTION:  periodic interrupt code
* AUTHOR:       Mitja Nemec
* DATE:         16.1.2009
*
****************************************************************/
#include    "PER_int.h"
#include    "TIC_toc.h"


// generiranje željene vrednosti
float ref_counter = 0;
float ref_counter_prd = SAMPLE_FREQ;
float ref_counter_cmpr = 500;

float ref_value = 0;
float ref_value_high = 0.5;
float ref_value_low = 0.05;

// za oceno obremenjenosti CPU-ja
float   cpu_load  = 0.0;
long    interrupt_cycles = 0;

// spremenljikva s katero štejemo kolikokrat se je prekinitev predolgo izvajala
int interrupt_overflow_counter = 0;

/**************************************************************
* Prekinitev, ki v kateri se izvaja regulacija
**************************************************************/
#pragma CODE_SECTION(PER_int, "ramfuncs");
void interrupt PER_int(void)
{
    /* lokalne spremenljivke */
    
    // najprej povem da sem se odzzval na prekinitev
    // Spustimo INT zastavico casovnika ePWM1
    EPwm1Regs.ETCLR.bit.INT = 1;
    // Spustimo INT zastavico v PIE enoti
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
    
    // pozenem stoprico
    interrupt_cycles = TIC_time;
    TIC_start();

    // izracunam obremenjenost procesorja
    cpu_load = (float)interrupt_cycles / (CPU_FREQ/SAMPLE_FREQ);

    // generiram spreenjljivo želeno vrednost
    ref_counter = ref_counter + 1;
    if (ref_counter >= ref_counter_prd)
    {
        ref_counter = 0;
    }

    // stopnicasta zeljena vrednost
    if (ref_counter > ref_counter_cmpr)
    {
        ref_value = ref_value_low;
    }
    else
    {
        ref_value = ref_value_high;
    }

    // pocakam da ADC konca s pretvorbo
    ADC_wait();


    // spavim vrednosti v buffer za prikaz - samo dokler ne pride do napake
    DLOG_GEN_update();

    
    /* preverim, èe me sluèajno èaka nova prekinitev.
       èe je temu tako, potem je nekaj hudo narobe
       saj je èas izvajanja prekinitve predolg
       vse skupaj se mora zgoditi najmanj 10krat,
       da reèemo da je to res problem
    */
    if (EPwm1Regs.ETFLG.bit.INT == TRUE)
    {
        // povecam stevec, ki steje take dogodke
        interrupt_overflow_counter = interrupt_overflow_counter + 1;
        
        // in ce se je vse skupaj zgodilo 10 krat zapored se ustavim
        // v kolikor uC krmili kakšen resen HW, potem moèno
        // proporoèam lepše "hendlanje" takega dogodka
        // beri:ugasni moènostno stopnjo, ...
        if (interrupt_overflow_counter >= 10)
        {
            asm(" ESTOP0");
        }
    }
    else
    {
        interrupt_overflow_counter = 0;
    }
    
    // stopam
    TIC_stop();

}   // end of PWM_int

/**************************************************************
* Funckija, ki pripravi vse potrebno za izvajanje
* prekinitvene rutine
**************************************************************/
void PER_int_setup(void)
{
    // inicializiram data logger
    dlog.mode = Single;
    dlog.auto_time = SAMPLE_FREQ;          // number of calls to update function
    dlog.holdoff_time = SAMPLE_FREQ;       // number of calls to update function

    dlog.prescalar = 20;                    // store every  sample

    dlog.slope = Positive;                 // trigger on positive slope
    dlog.trig = &ref_counter;
    dlog.trig_value = SAMPLE_FREQ - 10;

    dlog.iptr1 = &ref_counter;

    // Proženje prekinitve
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;    //sproži prekinitev na periodo
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;         //ob vsakem prvem dogodku
    EPwm1Regs.ETCLR.bit.INT = 1;                //clear possible flag
    EPwm1Regs.ETSEL.bit.INTEN = 1;              //enable interrupt

    // registriram prekinitveno rutino
    EALLOW;
    PieVectTable.EPWM1_INT = &PER_int;
    EDIS;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    IER |= M_INT3;
    // da mi prekinitev teèe  tudi v real time naèinu
    // (za razhoršèevanje main zanke in BACK_loop zanke)
    SetDBGIER(M_INT3);
}

