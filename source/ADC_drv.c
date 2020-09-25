/**************************************************************
* FILE:         ADC_drv.c
* DESCRIPTION:  A/D driver for piccolo devices
* AUTHOR:       Mitja Nemec
* DATE:         19.1.2012
*
****************************************************************/
#include "ADC_drv.h"

/**************************************************************
* inicializiramo ADC
**************************************************************/
void ADC_init(void)
{
    Uint16  acqps_min;
    float   acqps_set;
    float   t_sh;

    EALLOW;

    //write configurations
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdccRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcdRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    //Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //power up the ADCs
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    // enable ADCtemp measurement
    AnalogSubsysRegs.TSNSCTL.bit.ENABLE = 1;

    //delay for 1ms to allow ADC time to power up
    DELAY_US(1000);

    EDIS;
    
    //determine minimum acquisition window (in SYSCLKS) based on resolution
    if(ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
    {
        acqps_min = 14; //75ns
    }
    //resolution is 16-bit
    else 
    { 
        acqps_min = 63; //320ns
    }

    // naracunam acqps za moj hardware
    // B verzija CPU-ja -> èasonva konstanta = 30 ns.
    // za 12 bitni rezultat rabim vzorèiti dlje kot 250 ns
    t_sh = 250; //ns

    acqps_set = t_sh*CPU_FREQ/1000000000 - 1;
    // za zaokroževanje
    acqps_set = acqps_set + 0.5;

    if (acqps_set < acqps_min)
    {
        acqps_set = acqps_min;
    }

    // ADCA channel setup
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;          //SOC0 will convert pin A0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;          //SOC1 will convert pin A1
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 3;          //SOC2 will convert pin A3
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA

    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 4;          //SOC3 will convert pin A4
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 5;          //SOC4 will convert pin A5
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdcaRegs.ADCSOC5CTL.bit.CHSEL = 13;         //SOC5 will convert pin temperature
    AdcaRegs.ADCSOC5CTL.bit.ACQPS = 140;        //sample window 700 ns
    AdcaRegs.ADCSOC5CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 2;      //end of SOC2 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        //enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      //make sure INT1 flag is cleared

    // ADCB channel setup
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;          //SOC0 will convert pin B0
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA

    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 3;          //SOC1 will convert pin B3
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 1;          //SOC1 will convert pin B1
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    // ADCC channel setup
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 3;          //SOC0 will convert pin C3
    AdccRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdccRegs.ADCSOC1CTL.bit.CHSEL = 4;          //SOC1 will convert pin C4
    AdccRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdccRegs.ADCSOC1CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdccRegs.ADCSOC2CTL.bit.CHSEL = 2;          //SOC2 will convert pin C2
    AdccRegs.ADCSOC2CTL.bit.ACQPS = acqps_set;  //sample window
    AdccRegs.ADCSOC2CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCB

    AdccRegs.ADCINTSEL1N2.bit.INT2SEL = 4;      //end of SOC4 will set INT1 flag
    AdccRegs.ADCINTSEL1N2.bit.INT2E = 1;        //enable INT2 flag
    AdccRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;      //make sure INT2 flag is cleared

    // ADCD channel setup
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin D2
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcdRegs.ADCSOC1CTL.bit.CHSEL = 1;          //SOC1 will convert pin D3
    AdcdRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = 6;        //trigger on ePWM1 SOCA/C

    EDIS;

    // Proženje ADC-ja
    ADC_MODUL1.ETSEL.bit.SOCASEL = ET_CTR_ZERO;     //sproži prekinitev na periodo
    ADC_MODUL1.ETPS.bit.SOCAPRD = ADC_SAMP_RATIO;   //ob vsakem prvem dogodku
    ADC_MODUL1.ETCLR.bit.SOCA = 1;                  //clear possible flag
    ADC_MODUL1.ETSEL.bit.SOCAEN = 1;                //enable ADC Start Of conversion

    ADC_MODUL1.ETSEL.bit.SOCBSEL = ET_CTR_PRD;      //sproži prekinitev na periodo
    ADC_MODUL1.ETPS.bit.SOCBPRD = ADC_SAMP_RATIO;   //ob vsakem prvem dogodku
    ADC_MODUL1.ETCLR.bit.SOCB = 1;                  //clear possible flag
    ADC_MODUL1.ETSEL.bit.SOCBEN = 1;                //enable ADC Start Of conversion

    // pocakam, da se stabilizira
    DELAY_US(1000L);

}   //end of AP_ADC_init

/**************************************************************
* Funkcija, ki pocaka da ADC konca s pretvorbo
* vzorcimo...
* return: void
**************************************************************/
#pragma CODE_SECTION(ADC_wait, "ramfuncs");
void ADC_wait(void)
{
    while (AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0)
    {
        /* DO NOTHING */
    }
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1;
}
