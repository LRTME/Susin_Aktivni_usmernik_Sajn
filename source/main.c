/****************************************************************
* FILENAME:     main.c
* DESCRIPTION:  initialization code
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "main.h"

/**************************************************************
* Funkcija, ki se izvede inicializacijo
**************************************************************/
void main(void)
{
    // PLL, in ura
    InitSysCtrl();
    
    // GPIO - najprej
labela:
    InitGpio();
    // inicializiram PCB
    PCB_init();

    // generic init of PIE
    InitPieCtrl();

    // basic vector table
    InitPieVectTable();

    // inicializiram SD kartico
    SD_init();

    // inicializacija komunikacije
    COMM_initialization();

    // inicializiram ADC
    ADC_init();

    // inicializiram PWM enote
    FB_init();
    BB_init();
    FAN_init();
    FAN_update(0.1);

    // inicializiram periodièno prekinitev
    PER_int_setup();

    // pozenem PWM enote
    FB_start();
    BB_start();
    FAN_start();

    // omogocim prekinitve
    EINT;
    ERTM;

    // pocakam, da se izvede par prekinitev, da zacnem brcati psa cuvaja
    DELAY_US(1000);

    // resetiram zapahe
    PCB_lat_reset();

    // resetiram vse morebitne napake
    fault_flags.overcurrent_bb = FALSE;
    fault_flags.overcurrent_grid = FALSE;
    fault_flags.undervoltage_dc = FALSE;
    fault_flags.overvoltage_dc = FALSE;
    fault_flags.undervoltage_grid = FALSE;
    fault_flags.overvoltage_grid = FALSE;
    fault_flags.cpu_overrun = FALSE;
    fault_flags.fault_registered = FALSE;
    fault_flags.HW_trip = FALSE;

    // inicializiram uptime števec
    UP_init();

    // inicializiram zašèitno prekinitev,
    // in sicer po tem ko resetiram latch
    // saj bi se mi v nasprotnem primeru izvedla zašèitna prekinitev
    // FLT_int_setup();

    // pocakam, da se izvede kalibracija tokovnih sond
    DELAY_US(10000);
    start_calibration = TRUE;
    while(calibration_done == FALSE)
    {
        /* DO NOTHING */
    }

    /* vklopna procedura */
    state = Startup;
    // najprej se priklopin na omrežje in poèasi napolnim DC-link
    PCB_in_relay_on();
    DELAY_US(1000000);

    // èe se je vklopila strojna zašèita se je zelo verjetno vklopila zaradi prevelikega toka
    if (PCB_HW_trip() == TRUE)
    {
        // zato kar resetiram MCU, da se zaženemo še enkrat
        asm(" ESTOP0");
        DINT;
        goto labela;
        /*
        asm(" ESTOP0");
        EALLOW;
        SysCtrlRegs.WDCR = 0x0040;
        EDIS;
        */
    }


    // pocakam, da napetost na enosmernem tokokrogu naraste
    while (nap_dc < nap_grid_rms * SQRT2 * 0.95)
    {
        /* DO NOTHING */
    }

    // kratkostièim zagonski upor
    PCB_res_relay_on();
    DELAY_US(1000000);

    // in pocakam, da napetost na enosmernem tokokrogu naraste do konca
    while (nap_dc < nap_grid_rms * SQRT2 * 0.98)
    {
        /* DO NOTHING */
    }

    // vklopim moènostno stopnjo in povem regulaciji da zaène delati
    DINT;
    FAN_enable();
    FAN_update(0.25);
    FB_enable();
    state = Standby;

    DisableDog();

    // zeljeno vrednost enacim s trenutno, da se lepo zapeljem po rampi
    nap_dc_slew.Out = nap_dc;
    EINT;
    // pocakam da se napetost enosmernega kroga zapelje na nastavljeno vrednost
    while(fabs(nap_dc_reg.Fdb - nap_dc_reg.Ref) > 0.1)
    {
        /* DO NOTHING */
    }

    // grem v neskoncno zanko, ki se izvaja v ozadju
    BACK_loop();
}   // end of main
