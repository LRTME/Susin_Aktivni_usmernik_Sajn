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
#include "PWM_drv_fan.h"

// prototipi lokalnih funkcij
void    FAN_init(void);

// status ali delujemo ali ne
enum    FAN_STATE fan_status = DIS;


/**************************************************************
* Izklopi vse tranzistorje
* returns:
**************************************************************/
void FAN_disable(void)
{
    FAN_MODUL1.AQCSFRC.bit.CSFA = 1;

    fan_status = DIS;
}

/**************************************************************
* vklopi vse izhode
* returns:
**************************************************************/
void FAN_enable(void) //void FB_enable(void)
{
    FAN_MODUL1.AQCSFRC.bit.CSFA = 0;

    fan_status = EN;
}

/**************************************************************
* vrne status (delnam/ne delam)
* returns:
**************************************************************/
int FAN_status(void)
{
    return(fan_status);
}

/**************************************************************
* Funkcija, ki popiše registre za PWM1,2 in 3. Znotraj funkcije
* se omogoèi interrupt za proženje ADC, popiše se perioda, compare
* register, tripzone register, omogoèi se izhode za PWM...
* return:void
**************************************************************/
void FAN_init(void)
{
    // setup timer base
    FAN_MODUL1.TBPRD = FAN_PERIOD;       //nastavljeno na 25us, PWM_PERIOD = 50us

    // init timer
    FAN_MODUL1.TBCTL.bit.CLKDIV = 0;
    FAN_MODUL1.TBCTL.bit.HSPCLKDIV = 0;
    FAN_MODUL1.TBCTL.bit.PRDLD = 0;          // shadowed period reload on zero
    FAN_MODUL1.TBCTL.bit.PHSEN = 0;          // master timer does not sync
    FAN_MODUL1.TBCTL.bit.PHSDIR = 0;         // count up after sync
    FAN_MODUL1.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;       // sync out on zero

    // debug mode behafiour
    #if FAN_DEBUG_MODE == 0 //#if FB_DEBUG == 0
    FAN_MODUL1.TBCTL.bit.FREE_SOFT = 1;      // stop after current cycle
    #endif
    #if FAN_DEBUG_MODE == 1 //#if FB_DEBUG == 1
    FAN_MODUL1.TBCTL.bit.FREE_SOFT = 0;      // stop after current cycle
    #endif
    #if FAN_DEBUG_MODE == 2 //#if FB_DEBUG == 2
    FAN_MODUL1.TBCTL.bit.FREE_SOFT = 3;      // run free
    #endif

    // Init Timer-Base Phase Register for EPWM1-EPWM2

    // compare setup
    // Init Compare Control Register for EPWM1-EPWM2
    FAN_MODUL1.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    FAN_MODUL1.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    FAN_MODUL1.CMPA.bit.CMPA = FAN_PERIOD/8; //50% duty cycle

    // Init Action Qualifier Output A Register
    FAN_MODUL1.AQSFRC.bit.RLDCSF = 0;

    FAN_MODUL1.AQCTLA.bit.CAU = AQ_SET;      // set output on CMPA_UP
    FAN_MODUL1.AQCTLA.bit.CAD = AQ_CLEAR;    // clear output on CMPA_DOWN

    // Dead Time
    // trip zone functionality
//    FAN_MODUL1.TZSEL.bit.OSHT1 = 0;      // select which input triggers tripzone
//    FAN_MODUL1.TZCTL.bit.TZA = 2;        // force low
//    FAN_MODUL1.TZCTL.bit.TZB = 2;        // force low
//    FAN_MODUL1.TZCLR.bit.OST = 1;        // clear any pending flags


    // enable outputs by default
    FAN_disable();

    // output pin setup
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1;
    EDIS;

    // trip zone input pin setup
    EALLOW;
    //GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1
    EDIS;


}   //end of FB_PWM_init


/**************************************************************
* Funkcija, ki na podlagi vklopnega razmerja in izbranega vektorja
* vklopi doloèene tranzistorje
* return: void
* arg1: vklopno razmerje od -1.0 do +1.0 (format IQ)
**************************************************************/
#pragma CODE_SECTION(FAN_update, "ramfuncs");
void FAN_update(float duty1)
{
    unsigned int compare1;

    unsigned int perioda;

    // delam samo v primeru ce je mostic omogocen
    if (fan_status == EN)
    {
        perioda = FAN_MODUL1.TBPRD;

        // zašèita za duty cycle
        //(zašèita za sektor je narejena v default switch case stavku)
        if (duty1 < 0.0) duty1 = 0.0;
        if (duty1 > 1.0) duty1 = 1.0;

        // koda da naracunam vrednost, ki bo sla v CMPR register

        compare1 = perioda - perioda * duty1;

        FAN_MODUL1.AQCSFRC.bit.CSFA = 0;

        // vpisem v register
        FAN_MODUL1.CMPA.bit.CMPA = compare1;
    }
    else
    {
        FAN_MODUL1.CMPA.bit.CMPA = FAN_MODUL1.TBPRD;
    }
}  //end of BB_update

/**************************************************************
* Funkcija, ki starta PWM èasovnike. Znotraj funkcije nastavimo
* naèin štetja èasovnikov
* return: void
**************************************************************/
void FAN_start(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    FAN_MODUL1.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  //up-down count mode

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

}   //end of FB_start

