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
// kdaj stejemo tipko za pritisnjeno

/**************************************************************
 * Funkcija, ki se izvaja v ozadju med obratovanjem
 **************************************************************/
#pragma CODE_SECTION(BACK_loop, "ramfuncs");
void BACK_loop(void)
{
    // lokalne spremenljivke
    while (1)
    {
        DELAY_US(1000000L);
        PCB_LEDcard_toggle();

        asm(" NOP");
    }   // end of while(1)
}       // end of BACK_loop

