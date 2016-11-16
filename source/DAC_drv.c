/**************************************************************
* FILE:         DAC_drv.c
* DESCRIPTION:  DAC driver for 28377
* AUTHOR:       Mitja Nemec
* DATE:         15.7.2015
*
****************************************************************/
#include "DAC_drv.h"

/**************************************************************
* inicializiramo DAC
**************************************************************/
void DAC_init(bool use_external_ref)
{
    // Enable DACOUTA
    EALLOW;

    //Use VDAC as the reference for DAC
    if (use_external_ref == TRUE)
    {
        DacaRegs.DACCTL.bit.DACREFSEL  = 0;
        DacbRegs.DACCTL.bit.DACREFSEL  = 0;
    }
    else
    {
        DacaRegs.DACCTL.bit.DACREFSEL  = 1;
        DacbRegs.DACCTL.bit.DACREFSEL  = 1;
    }

    //Enable DAC output
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;

    EDIS;

}

#pragma CODE_SECTION(DAC_update_a, "ramfuncs");
void DAC_update_a(float napetost)
{
    int code;

    code = napetost * 4096;

    // preverim meje
    if (code >= 4096)
    {
        code = 4095;
    }
    if (code < 0.0)
    {
        code = 0.0;
    }
    
    DacaRegs.DACVALS.bit.DACVALS = code;
}

#pragma CODE_SECTION(DAC_update_b, "ramfuncs");
void DAC_update_b(float napetost)
{
    int code;

    code = napetost * 4096;

    // preverim meje
    if (code > 4096)
    {
        code = 1.0;
    }
    if (code < 0.0)
    {
        code = 0.0;
    }

    DacbRegs.DACVALS.bit.DACVALS = code;
}

#pragma CODE_SECTION(DAC_update_a_signed, "ramfuncs");
void DAC_update_a_signed(float napetost)
{
    int code;

    code = napetost * 2048 + 2048;

    // preverim meje
    if (code > 4096)
    {
        code = 1.0;
    }
    if (code < 0.0)
    {
        code = 0.0;
    }

    DacaRegs.DACVALS.bit.DACVALS = code;
}

#pragma CODE_SECTION(DAC_update_b_signed, "ramfuncs");
void DAC_update_b_signed(float napetost)
{
    int code;

    code = napetost * 2048 + 2048;

    // preverim meje
    if (code > 4096)
    {
        code = 1.0;
    }
    if (code < 0.0)
    {
        code = 0.0;
    }

    DacbRegs.DACVALS.bit.DACVALS = code;
}
