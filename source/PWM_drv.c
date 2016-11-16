/************************************************************** 
* FILE:         PWM_drv.c
* DESCRIPTION:  A/D in PWM driver for TMS320F2808
* AUTHOR:       Andraž Kontarèek, Mitja Nemec
* DATE:         21.12.2009
*
****************************************************************/
#include "PWM_drv.h"

// prototipi lokalnih funkcij


/**************************************************************
* Funkcija, ki popiše registre za PWM1,2 in 3. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, tripzone register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
void PWM_init(void)
{
//EPWM Module 1
    // setup timer base 
    EPwm1Regs.TBPRD = PWM_PERIOD/2;       //nastavljeno na 25us, PWM_PERIOD = 50us
    EPwm1Regs.TBCTL.bit.PHSDIR = 0;       // count up after sync
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;     // sync out on zero
    EPwm1Regs.TBCTL.bit.PRDLD = 0;        // shadowed period reload
    EPwm1Regs.TBCTL.bit.PHSEN = 0;        // master timer does not sync
    EPwm1Regs.TBCTR = 1;

        // debug mode behafiour
    #if PWM_DEBUG == 0
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 0;  // stop immediately
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 0;  // stop immediately
    #endif
    #if PWM_DEBUG == 1
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 1;  // stop when finished
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 1;  // stop when finished
    #endif
    #if FB_DEBUG == 2
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;  // run free
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;  // run free
    #endif
    
    // Compare registers
    EPwm1Regs.CMPA.bit.CMPA = PWM_PERIOD/4;                 //50% duty cycle

    // Init Action Qualifier Output A Register 
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;  // clear output on CMPA_UP
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;    // set output on CMPA_DOWN

    // Dead Time
    
    // Trip zone 

    // Event trigger
    // ADC trigger setup
    EPwm1Regs.CMPB.bit.CMPB = 0x0000;
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;
    EPwm1Regs.ETCLR.bit.SOCA = 1;
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;

    EPwm1Regs.ETSEL.bit.SOCBSEL = ET_CTRD_CMPA;
    EPwm1Regs.ETPS.bit.SOCBPRD = ET_1ST;
    EPwm1Regs.ETCLR.bit.SOCB = 1;
    EPwm1Regs.ETSEL.bit.SOCBEN = 1;
    // Proženje prekinitve 
    EPwm1Regs.ETSEL.bit.INTSEL = 2;             //sproži prekinitev na periodo
    EPwm1Regs.ETPS.bit.INTPRD = PWM_INT_PSCL;   //ob vsakem prvem dogodku
    EPwm1Regs.ETCLR.bit.INT = 1;                //clear possible flag
    EPwm1Regs.ETSEL.bit.INTEN = 1;              //enable interrupt

    PWM_update(0.0);

    // output pin setup
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // GPIO0 pin is under ePWM control
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // GPIO1 pin is under ePWM control

    EDIS;                                 // Disable EALLOW

}   //end of PWM_PWM_init

/**************************************************************
* Funkcija, ki na podlagi vklopnega razmerja in izbranega vektorja
* vklopi doloèene tranzistorje
* return: void
* arg1: vklopno razmerje od 0.0 do 1.0 (format IQ)
**************************************************************/
void PWM_update(float duty)
{
   int compare;

    // zašèita za duty cycle 
    //(zašèita za sektor je narejena v default switch case stavku)
    if (duty < -0.99) duty = -0.99;
    if (duty > +0.99) duty = +0.99;

    // po potrebi spremenim smer
    if (duty == 0.0)
    {
        GpioDataRegs.GPADAT.bit.GPIO3 = 0;
    }
    else
    {
        GpioDataRegs.GPADAT.bit.GPIO3 = 1;
    }


    //izraèunam vrednost compare registra(iz periode in preklopnega razmerja)
    compare = EPwm1Regs.TBPRD/2 + EPwm1Regs.TBPRD * duty/2;

    // vpisem vrednost v register
    EPwm1Regs.CMPA.bit.CMPA = compare;
    

}  //end of PWM_update

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel natancno
* return: void
* arg1: zelena perioda
**************************************************************/
void PWM_period(float perioda)
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
    EPwm1Regs.TBPRD = celi_del;
}   //end of FB_period

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel natancno
* return: void
* arg1: zelena frekvenca
**************************************************************/
void PWM_frequency(float frekvenca)
{
    // spremenljivke
    float   temp_tbper;
    static float ostanek = 0;
    long celi_del;

    // naracunam TBPER (CPU_FREQ / SAMPLING_FREQ) - 1
    temp_tbper = (CPU_FREQ/2) / frekvenca;

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
    EPwm1Regs.TBPRD = celi_del - 1;
}   //end of FB_frequency
  
/**************************************************************
* Funkcija, ki starta PWM1. Znotraj funkcije nastavimo
* naèin štetja èasovnikov (up-down-count mode)
* return: void
**************************************************************/
void PWM_start(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  //up-down-count mode
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
    
}   //end of AP_PWM_start



