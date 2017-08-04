/****************************************************************
* FILENAME:     REF_gen.c
* DESCRIPTION:  generator razliènih oblik signala
* AUTHOR:       Mitja Nemec
* START DATE:   1.8.2016
****************************************************************/
#include    "REF_gen.h"
#include    "define.h"

struct REF_GEN  ref_gen =
{
        0.0,
        0.0,
        0.0,
        0.0,
        SAMPLE_TIME,
        1.0,
        1.0,
        0.0,
        Step
};

#pragma CODE_SECTION(REF_gen, "ramfuncs");
void REF_gen(void)
{
    // vrednost, ki jo pošljem ven
    float   ref_internal = 0.0;

    // najprej generiram kot
    ref_gen.kot = ref_gen.kot + ref_gen.frequency * ref_gen.sampling_period;
    if (ref_gen.kot > 1.0)
    {
        ref_gen.kot = ref_gen.kot - 1.0;
    }
    if (ref_gen.kot < 0.0)
    {
        ref_gen.kot = ref_gen.kot + 1.0;
    }

    // potem pa generiram referenèni signal glede na izbrano obliko
    switch(ref_gen.type)
    {
    case Step:
        // generiram signal zaradi zašèite elektronike dodam omejitev naklona
        // najprej generiram step
        if (ref_gen.kot < ref_gen.duty)
        {
            ref_internal = ref_gen.amp + ref_gen.offset;
        }
        else
        {
            ref_internal = ref_gen.offset;
        }
        ref_gen.out = ref_internal;

        break;
    case Slew:
        // generiram signal
        if (ref_gen.kot < ref_gen.duty)
        {
            ref_internal = ref_gen.amp + ref_gen.offset;
        }
        else
        {
            ref_internal = ref_gen.offset;
        }
        // omejim naklon
        if ((ref_gen.slew_out - ref_internal) < 0.0)
        {
            if ((-(ref_gen.slew_out - ref_internal)) > ref_gen.slew_up * ref_gen.sampling_period)
                ref_gen.slew_out = ref_gen.slew_out + ref_gen.slew_up * ref_gen.sampling_period;
            else
            ref_gen.slew_out = ref_internal;
        }
        if ((ref_gen.slew_out - ref_internal) > 0.0)
        {
            if ((ref_gen.slew_out - ref_internal) > ref_gen.slew_down * ref_gen.sampling_period)
                ref_gen.slew_out = ref_gen.slew_out - ref_gen.slew_down * ref_gen.sampling_period;
            else
                ref_gen.slew_out = ref_internal;
        }

        ref_gen.out = ref_gen.slew_out;

        break;

    case Konst:
        // generiram signal
        ref_gen.out = ref_gen.amp;
        break;

    case Sine:
        // generiram signal
        ref_gen.out = ref_gen.offset + ref_gen.amp * sin(2*PI*ref_gen.kot);
        break;

    default:
        ref_gen.out = 0.0;
        break;

    }
}

