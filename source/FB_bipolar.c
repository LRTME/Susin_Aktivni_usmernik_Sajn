/************************************************************** 
* FILE:         FB_drv.c
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
#include "FB_bipolar.h"


// status ali delujemo ali ne
enum FB_STATE fb_status = FB_DIS;

/**************************************************************
* nastavi mrtvi cas  
**************************************************************/
void FB_dead_time(float dead_time)
{
    float cpu_cycle_time = (1.0/CPU_FREQ);
    
    long cycle_number;
    
    // naracunam koliko ciklov to znese
    cycle_number = dead_time/cpu_cycle_time;

    FB_MODUL1.DBFED.bit.DBFED = cycle_number;
    FB_MODUL1.DBRED.bit.DBRED = cycle_number;
    FB_MODUL2.DBFED.bit.DBFED = cycle_number;
    FB_MODUL2.DBRED.bit.DBRED = cycle_number;
}

/**************************************************************
* Izklopi zgornja tranzistorja in vklopi spodnja, tako da je mostic kratkosticen
* in lahko napolneta bootstrap kondenzatorja
* returns:  
**************************************************************/
void FB_bootstrap(void)
{
    FB_MODUL1.AQCSFRC.bit.CSFA = 1;
    FB_MODUL1.AQCSFRC.bit.CSFB = 1;

    FB_MODUL2.AQCSFRC.bit.CSFA = 1;
    FB_MODUL2.AQCSFRC.bit.CSFB = 1;

    fb_status = FB_BOOTSTRAP;
}

/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns:
**************************************************************/
void FB_trip(void)
{
    FB_MODUL1.TZFRC.bit.OST = 1;
    FB_MODUL2.TZFRC.bit.OST = 1;

    fb_status = FB_TRIP;
}

/**************************************************************
* Izklopi vse tranzistorje
* returns:  
**************************************************************/
void FB_disable(void)
{
    FB_MODUL1.AQCSFRC.bit.CSFA = 1;
    FB_MODUL1.AQCSFRC.bit.CSFB = 2;

    FB_MODUL2.AQCSFRC.bit.CSFA = 1;
    FB_MODUL2.AQCSFRC.bit.CSFB = 2;

    fb_status = FB_DIS;
}

/**************************************************************
* vklopi vse izhode
* returns:  
**************************************************************/
void FB_enable(void)
{
    FB_MODUL1.AQCSFRC.bit.CSFA = 0;
    FB_MODUL1.AQCSFRC.bit.CSFB = 0;

    FB_MODUL2.AQCSFRC.bit.CSFA = 0;
    FB_MODUL2.AQCSFRC.bit.CSFB = 0;

    fb_status = FB_EN;
}

/**************************************************************
* vrne status (delnam/ne delam)
* returns:  
**************************************************************/
int FB_status(void)
{
    return(fb_status);
}

/**************************************************************
* Funkcija, ki popiše registre za PWM1,2 in 3. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, tripzone register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
void FB_init(void)
{
    // setup timer base 
    FB_MODUL1.TBPRD = FB_PERIOD;       //nastavljeno na 25us, PWM_PERIOD = 50us
    FB_MODUL2.TBPRD = FB_PERIOD;
    // init timer
    FB_MODUL1.TBCTL.bit.PHSDIR = 0;       // count up after sync
    FB_MODUL1.TBCTL.bit.CLKDIV = 0;
    FB_MODUL1.TBCTL.bit.HSPCLKDIV = 0;
    FB_MODUL1.TBCTL.bit.SYNCOSEL = 1;     // sync out on zero
    FB_MODUL1.TBCTL.bit.PRDLD = 0;        // shadowed period reload on zero
    FB_MODUL1.TBCTL.bit.PHSEN = 0;        // master timer does not sync

    FB_MODUL2.TBCTL.bit.PHSDIR = 0;       // count up after sync
    FB_MODUL2.TBCTL.bit.CLKDIV = 0;
    FB_MODUL2.TBCTL.bit.HSPCLKDIV = 0;
    FB_MODUL2.TBCTL.bit.SYNCOSEL = 1;     // sync out on zero
    FB_MODUL2.TBCTL.bit.PRDLD = 0;        // shadowed period reload
    FB_MODUL2.TBCTL.bit.PHSEN = 0;        // master timer does not sync
    
    // debug mode behafiour
    #if FB_DEBUG == 0
    FB_MODUL1.TBCTL.bit.FREE_SOFT = 1;  // stop after current cycle
    FB_MODUL2.TBCTL.bit.FREE_SOFT = 1;  // stop after current cycle
    #endif
    #if FB_DEBUG == 1
    FB_MODUL1.TBCTL.bit.FREE_SOFT = 0;  // stop after current cycle
    FB_MODUL2.TBCTL.bit.FREE_SOFT = 0;  // stop after current cycle
    #endif
    #if FB_DEBUG == 2
    FB_MODUL1.TBCTL.bit.FREE_SOFT = 3;  // run free
    FB_MODUL2.TBCTL.bit.FREE_SOFT = 3;  // run free
    #endif

    // Init Timer-Base Phase Register for EPWM1-EPWM2
    FB_MODUL1.TBPHS.bit.TBPHS = 0;
    FB_MODUL2.TBPHS.bit.TBPHS = 0;

    // compare setup
    // Init Compare Control Register for EPWM1-EPWM2
    FB_MODUL1.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    FB_MODUL1.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    FB_MODUL2.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    FB_MODUL2.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    FB_MODUL1.CMPA.bit.CMPA = FB_PERIOD/4; //50% duty cycle
    FB_MODUL2.CMPA.bit.CMPA = FB_PERIOD/4; //50% duty cycle

    // Init Action Qualifier Output A Register 
    FB_MODUL1.AQSFRC.bit.RLDCSF = 0;            // load AQ on zero
    FB_MODUL2.AQSFRC.bit.RLDCSF = 0;            // load AQ on zero

    FB_MODUL1.AQCTLA.bit.CAU = AQ_SET;      // set output on CMPA_UP
    FB_MODUL1.AQCTLA.bit.CAD = AQ_CLEAR;    // clear output on CMPA_DOWN
    FB_MODUL1.AQCTLB.bit.CAU = AQ_SET;      // set output on CMPA_UP
    FB_MODUL1.AQCTLB.bit.CAD = AQ_CLEAR;    // clear output on CMPA_DOWN
    
    FB_MODUL2.AQCTLA.bit.CAU = AQ_CLEAR;    // clear output on CMPA_UP
    FB_MODUL2.AQCTLA.bit.CAD = AQ_SET;      // set output on CMPA_DOWN
    FB_MODUL2.AQCTLB.bit.CAU = AQ_CLEAR;      // set output on CMPA_UP
    FB_MODUL2.AQCTLB.bit.CAD = AQ_SET;    // clear output on CMPA_DOWN

    // Dead Time
    FB_MODUL1.DBCTL.bit.IN_MODE = 2;    //
    FB_MODUL1.DBCTL.bit.POLSEL = 2;     // active high complementary mode
    FB_MODUL1.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    FB_MODUL2.DBCTL.bit.IN_MODE = 2;    //
    FB_MODUL2.DBCTL.bit.POLSEL = 2;     // active high complementary mode
    FB_MODUL2.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    FB_MODUL1.DBFED.bit.DBFED = FB_DEAD_TIME;
    FB_MODUL1.DBRED.bit.DBRED = FB_DEAD_TIME;
    FB_MODUL2.DBFED.bit.DBFED = FB_DEAD_TIME;
    FB_MODUL2.DBRED.bit.DBRED = FB_DEAD_TIME;

    // trip zone functionality
/*    FB_MODUL1.TZSEL.bit.OSHT1 = 0;      // select which input triggers tripzone
    FB_MODUL1.TZCTL.bit.TZA = 2;        // force low
    FB_MODUL1.TZCTL.bit.TZB = 2;        // force low
    FB_MODUL1.TZCLR.bit.OST = 1;        // clear any pending flags

    FB_MODUL2.TZSEL.bit.OSHT1 = 0;      // select which input triggers tripzone
    FB_MODUL2.TZCTL.bit.TZA = 2;        // force low
    FB_MODUL2.TZCTL.bit.TZB = 2;        // force low
    FB_MODUL2.TZCLR.bit.OST = 1;        // clear any pending flags
*/
    // disable outputs by default
    FB_disable();

    // output pin setup
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;
    EDIS;
    
    // trip zone input pin setup
    EALLOW;
    //GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1;
    EDIS;

}   //end of FB_PWM_init

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel natanèno
* return: void
* arg1: zelena perioda
**************************************************************/
#pragma CODE_SECTION(FB_period, "ramfuncs");
void FB_period(float perioda)
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
    FB_MODUL1.TBPRD = celi_del;
    FB_MODUL2.TBPRD = celi_del;
}   //end of FB_period

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel natanèno
* return: void
* arg1: zelena frekvenca
**************************************************************/
#pragma CODE_SECTION(FB_frequency, "ramfuncs");
void FB_frequency(float frekvenca)
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
    FB_MODUL1.TBPRD = celi_del - 1;
    FB_MODUL2.TBPRD = celi_del - 1;
}   //end of FB_frequency

/**************************************************************
* Funkcija, ki na podlagi vklopnega razmerja in izbranega vektorja
* vklopi doloèene tranzistorje
* return: void
* arg1: vklopno razmerje od -1.0 do +1.0 (format IQ)
**************************************************************/
#pragma CODE_SECTION(FB_update, "ramfuncs");
void FB_update(float duty)
{
    unsigned int compare;

    unsigned int perioda;

    // zašèita za duty cycle
    //(zašèita za sektor je narejena v default switch case stavku)
    if (duty < (-1.0)) duty = (-1.0);
    if (duty > (+1.0)) duty = (+1.0);

    perioda = FB_MODUL1.TBPRD;
    // koda da naracunam vrednost, ki bo sla v CMPR register

    compare = 0.5 * (perioda - perioda * duty);

    // vpisem v register
    FB_MODUL1.CMPA.bit.CMPA = compare;
    FB_MODUL2.CMPA.bit.CMPA = compare;

}  //end of FB_update
  
/**************************************************************
* Funkcija, ki starta PWM èasovnike. Znotraj funkcije nastavimo
* naèin štetja èasovnikov
* return: void
**************************************************************/
void FB_start(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    
    FB_MODUL1.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  //up-down count mode
    FB_MODUL2.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  //up-down count mode
    
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
    
}   //end of FB_start
