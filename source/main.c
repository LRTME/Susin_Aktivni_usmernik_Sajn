/****************************************************************
* FILENAME:     main.c
* DESCRIPTION:  initialization code
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.0
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
*
****************************************************************/
#include    "main.h"

/**************************************************************
* Funkcija, ki se izvede inicializacijo
**************************************************************/
void main(void)
{
    // PLL, in ura
    InitSysCtrl();

    // GPIO - najprej
    InitGpio();

    // inicializiram vhodno izhodne pine
    PCB_init();

    // generic init of PIE
    InitPieCtrl();

    // basic vector table
    InitPieVectTable();

    // inicializiram DAC - do use internal reference
    DAC_init(TRUE);

    // inicializiram ADC in PWM modul
    ADC_init();
    PWM_init();

    // inicializiram peridoièno prekinitev za regulacijo motorja
    PER_int_setup();

    // omogocim prekinitve
    EINT;
    ERTM;

    // pozenem casovnik, ki bo prozil ADC in prekinitev
    PWM_start();

    // grem v neskoncno zanko, ki se izvaja v ozadju
    BACK_loop();
}   // end of main

