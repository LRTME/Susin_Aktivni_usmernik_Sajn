/****************************************************************
* FILENAME:     BACK_loop.c
* DESCRIPTION:  background code
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/

#include "BACK_loop.h"

// deklaracije lokalnih spremenljivk
bool en_tipka = FALSE;  // pulz, ko pritisnemo na tipko
bool reset_tipka = FALSE;  // pulz, ko pritisnemo na tipko

bool pulse_1000ms = FALSE;
bool pulse_500ms = FALSE;
bool pulse_100ms = FALSE;
bool pulse_50ms = FALSE;
bool pulse_10ms = FALSE;


// stevec, za pavzo po inicializaciji
int init_done_cnt = 0;
int fault_sensed_cnt = 0;

// deklaracije zunanjih spremenljivk
extern int      interrupt_cnt;
extern float    nap_dc;

// pototipi funkcij
void scan_keys(void);
void pulse_gen(void);

void startup_fcn(void);
void standby_fcn(void);
void ramp_up_fcn(void);
void work_fcn(void);
void ramp_down_fcn(void);
void fault_sensed_fcn(void);
void fault_fcn(void);

float   cutoff_freq_out = 1500;
float   damping_out = 0.707;
float   cutoff_freq_dc = 500;
float   damping_dc = 0.707;
extern  ABF_float   i_cap_abf;
extern  ABF_float   i_cap_dc;
/**************************************************************
* Funkcija, ki se izvaja v ozadju med obratovanjem
**************************************************************/
void BACK_loop(void)
{

    // lokalne spremenljivke
    while (1)
    {
        // na podlagi mejne frekvence in dušenj izraèunam a,b koeficienta
        i_cap_abf.Beta = (cutoff_freq_out*2*PI/SAMP_FREQ) * (cutoff_freq_out*2*PI/SAMP_FREQ);
        i_cap_abf.Alpha = 2*damping_out*sqrt(i_cap_abf.Beta)- i_cap_abf.Beta/2;

        i_cap_dc.Beta = (cutoff_freq_dc*2*PI/SAMP_FREQ) * (cutoff_freq_dc*2*PI/SAMP_FREQ);
        i_cap_dc.Alpha = 2*damping_dc*sqrt(i_cap_dc.Beta)- i_cap_dc.Beta/2;

        // tukaj pride koda neskoncne zanke, ki tece v ozadju
        // generiranje pulzov in branje tipk
        pulse_gen();
        scan_keys();

        /*****************/
        /* state machine */
        switch (state)
        {
        case Standby:
            standby_fcn();
            break;
        case Ramp_up:
            ramp_up_fcn();
            break;
        case Work:
            work_fcn();
            break;
        case Ramp_down:
            ramp_down_fcn();
            break;
        case Fault_sensed:
            fault_sensed_fcn();
            break;
        case Fault:
            fault_fcn();
            break;
        }

    }   // end of while(1)
}       // end of BACK_loop

void fault_fcn(void)
{
    // pobrišem napako, in grem v standby
    if (reset_tipka == TRUE)
    {
        // resetiram MCU - preko WD-ja
        EALLOW;
        WdRegs.WDCR.all = 0x0040;
        EDIS;
    }
    // signalizacija
    PCB_en_LED_off();
}

void fault_sensed_fcn(void)
{
    // izklopim mostic
    FB_disable();
    BB_disable();

    // izklopim vse kontaktorjev
    PCB_in_relay_off();
    PCB_res_relay_off();
    PCB_out_relay_off();

    FLT_int_disable();

    if (fault_flags.fault_registered == FALSE)
    {
        fault_flags.fault_registered = TRUE;
    }
    // signalizacija
    PCB_en_LED_off();
    PCB_mode_LED_off();

    state = Fault;
}

void standby_fcn(void)
{
    // sem pripravljen, sedaj samo cakam da uporabnik vklopi regulacijo
    if ((en_tipka == TRUE))
    {
        // postavim generator rampe v izhodišèe
        DINT;
        nap_out_slew.Out = 0;
        state = Ramp_up;
        EINT;

        // omogocim mocnostno stopnjo
		#if	BB_LEGS == 2
        BB_enable();
		#endif
		#if	BB_LEGS == 1
		BB1_enable();
		#endif
    }
    // signaliziram
    PCB_en_LED_off();
    PCB_mode_LED_off();
}

void ramp_up_fcn(void)
{
    // ko sta napetosti zelo blizu
    if (fabs(nap_out-nap_cap) < 0.1)
    {
        //  vklopim rele
        PCB_out_relay_on();

        // in preidem v normalni nacin delovanja
        DINT;
        state = Work;
        EINT;
    }
    // ce grem v tokovni rezim, potem to signaliziram
    if (  (nap_out_reg.Out == nap_out_reg.OutMax)
       || (nap_out_reg.Out == nap_out_reg.OutMin))
    {
        PCB_mode_LED_on();
    }
    else
    {
        PCB_mode_LED_off();
    }

    // signaliziram
    if (pulse_100ms == TRUE)
    {
        PCB_en_LED_toggle();
    }
    PCB_en_LED_off();
}

void work_fcn(void)
{
    // normalno obratovanje
    // preverjam ce sem znotraj mejnih vrednost, ce je napetost DC-lika dovolj visoka
    // in ce uporabnik hoce izklop
    if ((en_tipka == TRUE))
    {
        // postavim generator rampe v izhodišèe
        DINT;
        tok_bb_slew.Out = tok_bb1 + tok_bb2;
        state = Ramp_down;
        EINT;
    }

    // signaliziram da delujem
    PCB_en_LED_on();

    // ce grem v tokovni rezim, potem to signaliziram
    if (  (nap_out_reg.Out == nap_out_reg.OutMax)
       || (nap_out_reg.Out == nap_out_reg.OutMin))
    {
        PCB_mode_LED_on();
    }
    else
    {
        PCB_mode_LED_off();
    }
}

void ramp_down_fcn(void)
{
    // ko je tok na 0
    if (tok_bb_slew.Out == 0)
    {
        // izklopim izhodni rele
        PCB_out_relay_off();

        // in onemogocim izhodno stopnjo
		#if	BB_LEGS == 1
        BB_disable();
		#endif
		#if	BB_LEGS == 2
        BB1_disable();
		#endif

        // in preidem v stanje pripravljenosti
        DINT;
        state = Standby;
        EINT;

    }

    // signaliziram da delujem
    if (pulse_100ms == TRUE)
    {
        PCB_en_LED_toggle();
    }
    PCB_mode_LED_on();

}

void pulse_gen(void)
{
    static long interrupt_cnt_old_500ms = 0;
    static long interrupt_cnt_old_100ms = 0;
    static long interrupt_cnt_old_50ms = 0;
    static long interrupt_cnt_old_10ms = 0;
    static int  pulse_1000ms_cnt = 0;

    long delta_cnt_500ms;
    long delta_cnt_100ms;
    long delta_cnt_50ms;
    long delta_cnt_10ms;

    if ( (interrupt_cnt - interrupt_cnt_old_500ms) < 0)
    {
        interrupt_cnt_old_500ms = interrupt_cnt_old_500ms - (SAMP_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_100ms) < 0)
    {
        interrupt_cnt_old_100ms = interrupt_cnt_old_100ms - (SAMP_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_50ms) < 0)
    {
        interrupt_cnt_old_50ms = interrupt_cnt_old_50ms - (SAMP_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_10ms) < 0)
    {
        interrupt_cnt_old_10ms = interrupt_cnt_old_10ms - (SAMP_FREQ);
    }

    delta_cnt_500ms = interrupt_cnt - interrupt_cnt_old_500ms;
    delta_cnt_100ms = interrupt_cnt - interrupt_cnt_old_100ms;
    delta_cnt_50ms = interrupt_cnt - interrupt_cnt_old_50ms;
    delta_cnt_10ms = interrupt_cnt - interrupt_cnt_old_10ms;

    // generiraj pulza vsakih 0.01
    if (delta_cnt_10ms > ((SAMP_FREQ)/100))
    {
        pulse_10ms = TRUE;
        interrupt_cnt_old_10ms = interrupt_cnt;
    }
    else
    {
        pulse_10ms = FALSE;
    }

    // generiraj pulze vsakih 0.05
    if (delta_cnt_50ms > ((SAMP_FREQ)/50))
    {
        pulse_50ms = TRUE;
        interrupt_cnt_old_50ms = interrupt_cnt;
    }
    else
    {
        pulse_50ms = FALSE;
    }


    // generiraj pulza vsakih 0.1
    if (delta_cnt_100ms > ((SAMP_FREQ)/10))
    {
        pulse_100ms = TRUE;
        interrupt_cnt_old_100ms = interrupt_cnt;
    }
    else
    {
        pulse_100ms = FALSE;
    }

    // generiraj pulza vsakih 0.5
    if (delta_cnt_500ms > ((SAMP_FREQ)/2))
    {
        pulse_500ms = TRUE;
        interrupt_cnt_old_500ms = interrupt_cnt;
    }
    else
    {
        pulse_500ms = FALSE;
    }

    // stejem pulze po 100ms
    if (pulse_100ms == TRUE)
    {
        pulse_1000ms_cnt = pulse_1000ms_cnt + 1;
        if (pulse_1000ms_cnt == 10)
        {
            pulse_1000ms = TRUE;
            pulse_1000ms_cnt = FALSE;
        }
        else
        {
            pulse_1000ms = FALSE;
        }
    }
    // da pulz traja samo in samo eno iteracijo
    if ((pulse_100ms != TRUE)&&(pulse_1000ms == TRUE))
    {
        pulse_1000ms = FALSE;
    }
}

void scan_keys(void)
{
    // lokalne spremenljivke
    const int sw_on_cnt_limit = 10;

    static int en_tipka_cnt = 0;
    static int reset_tipka_cnt = 0;

    bool en_tipka_new;
    bool reset_tipka_new;

    // scan every cca 0.05s
    if (pulse_10ms == 1)
    {
        // preberem stanja tipk
        en_tipka_new = PCB_en_SW();
        reset_tipka_new = PCB_reset_SW();

        // ali smo pritisnili na tipko 1
        // ce je tipka pritisnjena, stopaj koliko casa je prisitsnjena
        if (en_tipka_new == TRUE)
        {
            en_tipka_cnt = en_tipka_cnt + 1;
        }
        // ce ni pritisnjena resetiraj stevec
        else
        {
            en_tipka_cnt = 0;
        }

        // ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
        if (en_tipka_cnt == sw_on_cnt_limit)
        {
            en_tipka = TRUE;
        }
        // sicer pa ne javi
        else
        {
            en_tipka = FALSE;
        }

        // ali smo pritisnili na tipko 2
        // ce je tipka pritisnjena, stopaj koliko casa je prisitsnjena
        if (reset_tipka_new == TRUE)
        {
            reset_tipka_cnt = reset_tipka_cnt + 1;
        }
        // ce ni pritisnjena resetiraj stevec
        else
        {
            reset_tipka_cnt = 0;
        }

        // ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
        if (reset_tipka_cnt == sw_on_cnt_limit)
        {
            reset_tipka = TRUE;
        }
        // sicer pa ne javi
        else
        {
            reset_tipka = FALSE;
        }
    }
    // da je pulz dolg res samo in samo eno iteracijo
    else
    {
        en_tipka = FALSE;
        reset_tipka = FALSE;
    }
}
