/**************************************************************
* FILE:         PCB_util.c 
* DESCRIPTION:  PCB initialization & Support Functions
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
**************************************************************/
#include "PCB_util.h"

/**************************************************************
* Funckija ki prizge LED diodo
**************************************************************/
#pragma CODE_SECTION(PCB_LEDcard_on, "ramfuncs");
void PCB_LEDcard_on(void)
{
	GpioDataRegs.GPCSET.bit.GPIO83 = 1;
}

/**************************************************************
* Funckija ki ugasne LED diodo
**************************************************************/
#pragma CODE_SECTION(PCB_LEDcard_off, "ramfuncs");
void PCB_LEDcard_off(void)
{
	GpioDataRegs.GPCCLEAR.bit.GPIO83 = 1;
}

/**************************************************************
* Funckija ki spremeni stanje LED diode
**************************************************************/
#pragma CODE_SECTION(PCB_LEDcard_toggle, "ramfuncs");
void PCB_LEDcard_toggle(void)
{
	GpioDataRegs.GPCTOGGLE.bit.GPIO83 = 1;
}



/**************************************************************
* Funckija ki inicializira PCB
**************************************************************/
void PCB_init(void)
{
    EALLOW;

    /* IZHODI */
    // LED na card-u
    GPIO_SetupPinMux(83, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(83, GPIO_OUTPUT, GPIO_PUSHPULL);


    EDIS;

}
