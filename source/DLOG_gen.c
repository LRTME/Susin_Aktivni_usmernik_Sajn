/****************************************************************
* FILENAME:     DLOG_gen.c
* DESCRIPTION:  generic data logger module
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      1.1
*
* CHANGES : 
* VERSION   DATE        WHO         DETAIL 
* 1.0       16.1.2009   Mitja Nemec Initial version
* 1.1       23.2.2010   Mitja Nemec Added various trigger modes
*
****************************************************************/
#include "DLOG_gen.h"

// buffer data
DLOG_GEN_TYPE DLOG_b_1[DLOG_GEN_SIZE];
#if DLOG_GEN_NR > 1
DLOG_GEN_TYPE DLOG_b_2[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 2
DLOG_GEN_TYPE DLOG_b_3[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 3
DLOG_GEN_TYPE DLOG_b_4[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 4
DLOG_GEN_TYPE DLOG_b_5[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 5
DLOG_GEN_TYPE DLOG_b_6[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 6
DLOG_GEN_TYPE DLOG_b_7[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 7
DLOG_GEN_TYPE DLOG_b_8[DLOG_GEN_SIZE];
#endif

struct DLOG dlog = 
{
    0UL,
    0UL,
    #if DLOG_GEN_NR > 1
    0UL,
    #endif
    #if DLOG_GEN_NR > 2
    0UL,
    #endif
    #if DLOG_GEN_NR > 3
    0UL,
    #endif
    #if DLOG_GEN_NR > 4
    0UL,
    #endif
    #if DLOG_GEN_NR > 5
    0UL,
    #endif
    #if DLOG_GEN_NR > 6
    0UL,
    #endif
    #if DLOG_GEN_NR > 7
    0UL,
    #endif

    // common data
    0UL,
    1,
    0,
    0,
    DLOG_GEN_SIZE,
    Wait,
    Positive,
    DLOG_GEN_SIZE,
    0L,
    DLOG_GEN_SIZE,
    0L,
    Auto,
    0
};

#pragma CODE_SECTION(DLOG_GEN_update, "ramfuncs");
void DLOG_GEN_update(void)
{
    // èe delam v neprestanem naèinu, potem skozi spravljam vzorce
    if (dlog.mode == Continuous)
    {
        // ce je vzorec za shranit, ga shranim
        if (dlog.skip_cntr ==0)
        {
            // spravim prvi kanal
            DLOG_b_1[dlog.write_ptr] = *(dlog.iptr1);

            #if DLOG_GEN_NR > 1
            // spravim drugi kanal
            DLOG_b_2[dlog.write_ptr] = *(dlog.iptr2);
            #endif

            #if DLOG_GEN_NR > 2
            // spravim tretji kanal
            DLOG_b_3[dlog.write_ptr] = *(dlog.iptr3);
            #endif

            #if DLOG_GEN_NR > 3
            // spravim cetrti kanal
            DLOG_b_4[dlog.write_ptr] = *(dlog.iptr4);
            #endif

            #if DLOG_GEN_NR > 4
            // spravim peti kanal
            DLOG_b_5[dlog.write_ptr] = *(dlog.iptr5);
            #endif

            #if DLOG_GEN_NR > 5
            // spravim sesti kanal
            DLOG_b_6[dlog.write_ptr] = *(dlog.iptr6);
            #endif

            #if DLOG_GEN_NR > 6
            // spravim sedmi kanal
            DLOG_b_7[dlog.write_ptr] = *(dlog.iptr7);
            #endif

            #if DLOG_GEN_NR > 7
            // spravim osmi kanal
            DLOG_b_8[dlog.write_ptr] = *(dlog.iptr8);
            #endif

            // nastavim kazalec za bufferje
            (dlog.write_ptr)++;
            // ce pridem do konca
            if (dlog.write_ptr == dlog.write_length)
            {
                // pripravim za naslednjic
                dlog.write_ptr = 0;
            }
        }
        // za downsamplanje
        (dlog.skip_cntr)++;
        if ((dlog.skip_cntr) >= (dlog.prescalar))
        {
            dlog.skip_cntr = 0;
        }
    }
    else
    {
    // najprej pocakam da prirpavim trigger
    if ((dlog.state == Wait))
    {
        // ce sem zaustavljen po uspesni single shot pretvorbi
        // cakam na uporabnika, da spremeni nacin ali pa na SW trigger
        if (dlog.mode == Stop)
        {
            // ce je sw triger potem kar sproži
            if (dlog.sw_trigger != 0)
            {
                dlog.state = Ready;
                dlog.mode = Single;
            }
        }
        // sicer pa prozi triger ali pa triger vsaj pripravi
        else
        {
            // Auto trigger in Auto mode
            if (dlog.mode == Auto)
            {
                dlog.auto_cnt = dlog.auto_cnt + 1;
                if (dlog.auto_cnt == dlog.auto_time)
                {
                    dlog.state = Store;
                    dlog.auto_cnt = 0;
                }
            }
            // ready trigger if positive slope (value is below trigg value)
            if (dlog.slope == Positive)
            {
                if(*(dlog.trig) < dlog.trig_value) dlog.state = Ready;
            }
            // ready trigger if negative slope (value is above trigg value)
            else
            {
                if(*(dlog.trig) > dlog.trig_value) dlog.state = Ready;
            }
            // ce je sw triger potem kar sproži
            if (dlog.sw_trigger != 0)
            {
                dlog.state = Ready;
            }
        }
    }
    // cakam na trigger
    if (dlog.state == Ready)
    {
        // Auto trigger in Auto mode
        if (dlog.mode == Auto)
        {
            dlog.auto_cnt = dlog.auto_cnt + 1;
            if (dlog.auto_cnt == dlog.auto_time)
            {
                dlog.state = Store;
                dlog.auto_cnt = 0;
            }
        }

        //  Check for positive slope trigger event
        if (dlog.slope == Positive)
        {
            if(*(dlog.trig) >= dlog.trig_value)
            {
                dlog.state = Store;
                dlog.auto_cnt = 0;
            }
        }
        // check for negative slope trigger event 
        else
        {
            if(*(dlog.trig) <= dlog.trig_value)
            {
                dlog.state = Store;
                dlog.auto_cnt = 0;
            }

        }

        // ce pa je SW trigger pa kar sproži
        if (dlog.sw_trigger != 0)
        {
            dlog.state = Store;
            dlog.auto_cnt = 0;
            dlog.sw_trigger = 0;
        }
    }
    // ce delam, potem shranjujem v buffer
    if (dlog.state == Store)
    {
        // ce je vzorec za shranit, ga shranim
        if (dlog.skip_cntr ==0)
        {
            // spravim prvi kanal
            DLOG_b_1[dlog.write_ptr] = *(dlog.iptr1);

            #if DLOG_GEN_NR > 1
            // spravim drugi kanal
            DLOG_b_2[dlog.write_ptr] = *(dlog.iptr2);
            #endif

            #if DLOG_GEN_NR > 2
            // spravim tretji kanal
            DLOG_b_3[dlog.write_ptr] = *(dlog.iptr3);
            #endif

            #if DLOG_GEN_NR > 3
            // spravim cetrti kanal
            DLOG_b_4[dlog.write_ptr] = *(dlog.iptr4);
            #endif

            #if DLOG_GEN_NR > 4
            // spravim peti kanal
            DLOG_b_5[dlog.write_ptr] = *(dlog.iptr5);
            #endif

            #if DLOG_GEN_NR > 5
            // spravim sesti kanal
            DLOG_b_6[dlog.write_ptr] = *(dlog.iptr6);
            #endif

            #if DLOG_GEN_NR > 6
            // spravim sedmi kanal
            DLOG_b_7[dlog.write_ptr] = *(dlog.iptr7);
            #endif

            #if DLOG_GEN_NR > 7
            // spravim osmi kanal
            DLOG_b_8[dlog.write_ptr] = *(dlog.iptr8);
            #endif

            // nastavim kazalec za bufferje
            (dlog.write_ptr)++;
            // ce pridem do konca
            if (dlog.write_ptr == dlog.write_length)
            {
                // pripravim za naslednjic
                dlog.write_ptr = 0;
                dlog.skip_cntr = 0;
                // grem v holdoff
                if (dlog.mode != Single)
                {
                    dlog.state = Holdoff;
                    dlog.holdoff_cnt = 0;
                }
                // ustavim v primeru single - shota
                else
                {
                    dlog.mode = Stop;
                    dlog.state = Wait;
                }
            }
        }
        
        // za downsamplanje
        (dlog.skip_cntr)++;
        if ((dlog.skip_cntr) >= (dlog.prescalar))
        {
            dlog.skip_cntr = 0;
        }
    }

    // preden pripravim nov trigger pocakam
    if (dlog.state == Holdoff)
    {
        dlog.holdoff_cnt = dlog.holdoff_cnt + 1;
        if (dlog.holdoff_cnt == dlog.holdoff_time)
        {
            dlog.state = Wait;
        }
    }
    }
}
