/************************************************************** 
* FILE:         FB_phase_shift.c
* DESCRIPTION:  A/D in PWM driver for TMS320F2808
* AUTHOR:       Mitja Nemec
* START DATE:   19.8.2010
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO                 DETAIL 
* 1.0       21.12.2009  Mitja Nemec         Initial version
*
****************************************************************/
#include "BB_2phase_shift.h"

// prototipi lokalnih funkcij
void    BB_init(void);

// status ali delujemo ali ne
enum    BB_STATE bb1_status = DISABLE;
enum    BB_STATE bb2_status = DISABLE;

/**************************************************************
* nastavi mrtvi cas  
**************************************************************/
void BB_dead_time(float dead_time)
{
    float cpu_cycle_time = (1.0/CPU_FREQ);
    
    long cycle_number;
    
    // naracunam koliko ciklov to znese
    cycle_number = dead_time/cpu_cycle_time;

    BB_MODUL1.DBFED.bit.DBFED = cycle_number;
    BB_MODUL1.DBRED.bit.DBRED = cycle_number;
    BB_MODUL2.DBFED.bit.DBFED = cycle_number;
    BB_MODUL2.DBRED.bit.DBRED = cycle_number;
}

/**************************************************************
* Izklopi zgornja tranzistorja in vklopi spodnja, tako da je mostic kratkosticen
* in lahko napolneta bootstrap kondenzatorja
* returns:  
**************************************************************/
void BB_bootstrap(void)
{
    BB1_bootstrap();
    BB2_bootstrap();

}

/**************************************************************
* Izklopi zgornja tranzistorja in vklopi spodnja, tako da je mostic kratkosticen
* in lahko napolneta bootstrap kondenzatorja
* returns:  
**************************************************************/
void BB1_bootstrap(void)
{
    BB_MODUL1.AQCSFRC.bit.CSFA = 1;
    BB_MODUL1.AQCSFRC.bit.CSFB = 1;

    bb1_status = BOOTSTRAP;
}

/**************************************************************
* Izklopi zgornja tranzistorja in vklopi spodnja, tako da je mostic kratkosticen
* in lahko napolneta bootstrap kondenzatorja
* returns:  
**************************************************************/
void BB2_bootstrap(void)
{
    BB_MODUL2.AQCSFRC.bit.CSFA = 1;
    BB_MODUL2.AQCSFRC.bit.CSFB = 1;

    bb2_status = BOOTSTRAP;
}

/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns:
**************************************************************/
void BB_trip(void)
{
    BB1_trip();
    BB2_trip();
}
/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns:
**************************************************************/
void BB1_trip(void)
{
    BB_MODUL1.TZFRC.bit.OST = 1;

    bb1_status = TRIP;
}
/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns:
**************************************************************/
void BB2_trip(void)
{
    BB_MODUL1.TZFRC.bit.OST = 1;

    bb2_status = TRIP;
}

/**************************************************************
* Izklopi vse tranzistorje
* returns:  
**************************************************************/
void BB_disable(void)
{
    BB1_disable();
    BB2_disable();

}

/**************************************************************
* Izklopi prvo vejo
* returns:  
**************************************************************/
void BB1_disable(void)
{
    BB_MODUL1.AQCSFRC.bit.CSFA = 1;
    BB_MODUL1.AQCSFRC.bit.CSFB = 2;

    bb1_status = DISABLE;
}

/**************************************************************
* Izklopi drugo vejo
* returns:  
**************************************************************/
void BB2_disable(void)
{
    BB_MODUL2.AQCSFRC.bit.CSFA = 1;
    BB_MODUL2.AQCSFRC.bit.CSFB = 2;

    bb2_status = DISABLE;
}

/**************************************************************
* vklopi vse izhode
* returns:  
**************************************************************/
void BB_enable(void) //void FB_enable(void)
{
    BB1_enable();
    BB2_enable();
}

/**************************************************************
* vklopi prvo vejo
* returns:  
**************************************************************/
void BB1_enable(void) //void FB_enable(void)
{
    BB_MODUL1.AQCSFRC.bit.CSFA = 0;
    BB_MODUL1.AQCSFRC.bit.CSFB = 0;

    bb1_status = ENABLE;
}

/**************************************************************
* vklopi drugo vejo
* returns:  
**************************************************************/
void BB2_enable(void) //void FB_enable(void)
{
    BB_MODUL2.AQCSFRC.bit.CSFA = 0;
    BB_MODUL2.AQCSFRC.bit.CSFB = 0;

    bb2_status = ENABLE;
}


/**************************************************************
* vrne status (delnam/ne delam)
* returns:  
**************************************************************/
int BB1_status(void)
{
    return(bb2_status);
}

/**************************************************************
* vrne status (delnam/ne delam)
* returns:  
**************************************************************/
int BB2_status(void)
{
    return(bb1_status);
}

/**************************************************************
* Funkcija, ki popiše registre za PWM1,2 in 3. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, tripzone register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
void BB_init(void)
{
    // setup timer base 
    BB_MODUL1.TBPRD = BB_PERIOD;       //nastavljeno na 25us, PWM_PERIOD = 50us
    BB_MODUL2.TBPRD = BB_PERIOD;
    // init timer
    BB_MODUL1.TBCTL.bit.CLKDIV = 0;
    BB_MODUL1.TBCTL.bit.HSPCLKDIV = 0;
    BB_MODUL1.TBCTL.bit.PRDLD = 0;          // shadowed period reload on zero
    BB_MODUL1.TBCTL.bit.PHSEN = 0;          // master timer does not sync
    BB_MODUL1.TBCTL.bit.PHSDIR = 0;         // count up after sync
    BB_MODUL1.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;       // sync out on zero

    BB_MODUL2.TBCTL.bit.CLKDIV = 0;
    BB_MODUL2.TBCTL.bit.HSPCLKDIV = 0;
    BB_MODUL2.TBCTL.bit.SYNCOSEL = 1;       // sync out on zero
    BB_MODUL2.TBCTL.bit.PRDLD = 0;          // shadowed period reload
    BB_MODUL2.TBCTL.bit.PHSEN = 1;          // master timer does sync
    BB_MODUL2.TBCTL.bit.PHSDIR = TB_DOWN;   // count down on sync

    // debug mode behafiour
    #if BB_DEBUG_MODE == 0 //#if FB_DEBUG == 0
    BB_MODUL1.TBCTL.bit.FREE_SOFT = 0;      // stop after current cycle
    BB_MODUL2.TBCTL.bit.FREE_SOFT = 0;      // stop after current cycle
    #endif
    #if BB_DEBUG_MODE == 1 //#if FB_DEBUG == 1
    BB_MODUL1.TBCTL.bit.FREE_SOFT = 1;      // stop after current cycle
    BB_MODUL2.TBCTL.bit.FREE_SOFT = 1;      // stop after current cycle
    #endif
    #if BB_DEBUG_MODE == 2 //#if FB_DEBUG == 2
    BB_MODUL1.TBCTL.bit.FREE_SOFT = 3;      // run free
    BB_MODUL2.TBCTL.bit.FREE_SOFT = 3;      // run free
    #endif

    // Init Timer-Base Phase Register for EPWM1-EPWM2
    BB_MODUL2.TBCTR = BB_PERIOD/2 - 1;
    BB_MODUL2.TBPHS.bit.TBPHS = BB_PERIOD/2; // by default 180°

    // compare setup
    // Init Compare Control Register for EPWM1-EPWM2
    BB_MODUL1.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    BB_MODUL1.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    BB_MODUL2.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    BB_MODUL2.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    BB_MODUL1.CMPA.bit.CMPA = BB_PERIOD/4; //50% duty cycle
    BB_MODUL2.CMPA.bit.CMPA = BB_PERIOD/4; //50% duty cycle

    // Init Action Qualifier Output A Register 
    BB_MODUL1.AQSFRC.bit.RLDCSF = 0;
    BB_MODUL2.AQSFRC.bit.RLDCSF = 0;

    BB_MODUL1.AQCTLA.bit.CAU = AQ_SET;      // set output on CMPA_UP
    BB_MODUL1.AQCTLA.bit.CAD = AQ_CLEAR;    // clear output on CMPA_DOWN
    BB_MODUL1.AQCTLB.bit.CAU = AQ_SET;      // set output on CMPA_UP
    BB_MODUL1.AQCTLB.bit.CAD = AQ_CLEAR;    // clear output on CMPA_DOWN
    
    BB_MODUL2.AQCTLA.bit.CAU = AQ_SET;      // clear output on CMPA_UP
    BB_MODUL2.AQCTLA.bit.CAD = AQ_CLEAR;    // set output on CMPA_DOWN
    BB_MODUL2.AQCTLB.bit.CAU = AQ_SET;      // set output on CMPA_UP
    BB_MODUL2.AQCTLB.bit.CAD = AQ_CLEAR;    // clear output on CMPA_DOWN

    // Dead Time
    BB_MODUL1.DBCTL.bit.IN_MODE = 2;    //
    BB_MODUL1.DBCTL.bit.POLSEL = 2;     // active high complementary mode
    BB_MODUL1.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    BB_MODUL2.DBCTL.bit.IN_MODE = 2;    //
    BB_MODUL2.DBCTL.bit.POLSEL = 2;     // active high complementary mode
    BB_MODUL2.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    BB_MODUL1.DBFED.bit.DBFED = BB_DEAD_TIME;
    BB_MODUL1.DBRED.bit.DBRED = BB_DEAD_TIME;
    BB_MODUL2.DBFED.bit.DBFED = BB_DEAD_TIME;
    BB_MODUL2.DBRED.bit.DBRED = BB_DEAD_TIME;

    // trip zone functionality
/*    BB_MODUL1.TZSEL.bit.OSHT1 = 0;      // select which input triggers tripzone
    BB_MODUL1.TZCTL.bit.TZA = 2;        // force low
    BB_MODUL1.TZCTL.bit.TZB = 2;        // force low
    BB_MODUL1.TZCLR.bit.OST = 1;        // clear any pending flags

    BB_MODUL2.TZSEL.bit.OSHT1 = 0;      // select which input triggers tripzone
    BB_MODUL2.TZCTL.bit.TZA = 2;        // force low
    BB_MODUL2.TZCTL.bit.TZB = 2;        // force low
    BB_MODUL2.TZCLR.bit.OST = 1;        // clear any pending flags
*/
    // enable outputs by default
    BB_disable();

    // output pin setup
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;
    EDIS;
    
    // trip zone input pin setup
    EALLOW;
//    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;
    EDIS;


}   //end of FB_PWM_init

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na en procesorski cikel
* return: void
* arg1: zelena perioda
* arg2: format zelene periode
**************************************************************/
#pragma CODE_SECTION(BB_period, "ramfuncs");
void BB_period(float perioda)
{
    // spremenljivke
    float   temp_tbper;
    static float ostanek = 0;
    long celi_del;

    // naracunam TBPER (CPU_FREQ * perioda)
    temp_tbper = perioda * CPU_FREQ/2;
    
    // izlocim celi del in ostanek
    celi_del = (long)temp_tbper;
    ostanek = temp_tbper - celi_del;
    // povecam celi del, ce je ostanek veji od 1
    if (ostanek > 1.0)
    {
        ostanek = ostanek - 1.0;
        celi_del = celi_del + 1;
    }

    // nastavim TBPER
    BB_MODUL1.TBPRD = celi_del;
    BB_MODUL2.TBPRD = celi_del;
}   //end of FB_period

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na 10ns
* return: void
* arg1: zelena frekvenca
* arg2: format zelene periode
**************************************************************/
#pragma CODE_SECTION(BB_frequency, "ramfuncs");
void BB_frequency(float frekvenca)
{
    // spremenljivke
    float   temp_tbper;
    static float ostanek = 0;
    long celi_del;

    // naracunam TBPER (CPU_FREQ / SAMPLING_FREQ) - 1
    temp_tbper = ((CPU_FREQ/2)/2)/frekvenca;

    // izlocim celi del in ostanek
    celi_del = (long)temp_tbper;
    ostanek = temp_tbper - celi_del;
    // povecam celi del, ce je ostanek veji od 1
    if (ostanek > 1.0)
    {
        ostanek = ostanek - 1.0;
        celi_del = celi_del + 1;
    }

    // nastavim TBPER
    BB_MODUL1.TBPRD = celi_del;
    BB_MODUL2.TBPRD = celi_del;
}   //end of BB_frequency

/**************************************************************
* Funkcija, ki na podlagi vklopnega razmerja in izbranega vektorja
* vklopi doloèene tranzistorje
* return: void
* arg1: vklopno razmerje od -1.0 do +1.0 (format IQ)
**************************************************************/
#pragma CODE_SECTION(BB_update, "ramfuncs");
void BB_update(float duty1, float duty2)
{
    unsigned int compare1;
    unsigned int compare2;

    unsigned int perioda;
    
    perioda = BB_MODUL1.TBPRD;

    if (bb1_status == ENABLE)
    {
        // zašèita za duty cycle
        //(zašèita za sektor je narejena v default switch case stavku)
        if (duty1 < 0.0) duty1 = 0.0;
        if (duty1 > 1.0) duty1 = 1.0;

        // koda da naracunam vrednost, ki bo sla v CMPR register
        compare1 = perioda - perioda * duty1;

        BB_MODUL1.AQCSFRC.bit.CSFA = 0;
        BB_MODUL1.AQCSFRC.bit.CSFB = 0;

        if (compare1 < (1 * BB_DEAD_TIME))
        {
        	compare1 = 0;
        	BB_MODUL1.AQCSFRC.bit.CSFA = 2;
        	BB_MODUL1.AQCSFRC.bit.CSFB = 2;
        }
        if (compare1 > (perioda - BB_DEAD_TIME))
        {
        	compare1 = perioda + 1;
        	BB_MODUL1.AQCSFRC.bit.CSFA = 1;
        	BB_MODUL1.AQCSFRC.bit.CSFB = 1;
        }
        // vpisem v register
    	BB_MODUL1.CMPA.bit.CMPA = compare1;
    }
    if (bb2_status == ENABLE)
    {
        // zašèita za duty cycle
        //(zašèita za sektor je narejena v default switch case stavku)
        if (duty2 < 0.0) duty2 = 0.0;
        if (duty2 > 1.0) duty2 = 1.0;

        // koda da naracunam vrednost, ki bo sla v CMPR register
        compare2 = perioda - perioda * duty2;

        BB_MODUL2.AQCSFRC.bit.CSFA = 0;
        BB_MODUL2.AQCSFRC.bit.CSFB = 0;

        if (compare2 < (1 * BB_DEAD_TIME))
        {
        	compare2 = 0;
        	BB_MODUL2.AQCSFRC.bit.CSFA = 2;
        	BB_MODUL2.AQCSFRC.bit.CSFB = 2;
        }
        if (compare2 > (perioda - BB_DEAD_TIME))
        {
        	compare2 = perioda + 1;
        	BB_MODUL2.AQCSFRC.bit.CSFA = 1;
        	BB_MODUL2.AQCSFRC.bit.CSFB = 1;
        }
        // vpisem v register
        BB_MODUL2.CMPA.bit.CMPA = compare2;
    }


}  //end of BB_update
  
/**************************************************************
* Funkcija, ki starta PWM èasovnike. Znotraj funkcije nastavimo
* naèin štetja èasovnikov
* return: void
**************************************************************/
void BB_start(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    
    BB_MODUL1.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  //up-down count mode
    BB_MODUL2.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  //up-down count mode
    
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
    
}   //end of FB_start

