/**************************************************************
* FILE:         PCB_util.c 
* DESCRIPTION:  PCB initialization & Support Functions
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
**************************************************************/
#include "PCB_util.h"

/**************************************************************
* WatchDog
**************************************************************/

// frekvenca s katero brcnemo psa (Hz)
#define PCB_DOG_FREQ    1000
// trajanje brce (us)
#define PCB_DOG_WIDTH   100L

#define PCB_DOG_KICK_ON ((SAMP_FREQ * PCB_DOG_WIDTH) / 1000000L) //tukaj sem spremenil iz SAMPLE_FREQ v SWITCH_FREQ
// za merjenje casa za brcanje psa cuvaja
static unsigned int  counter = 0;

/**************************************************************
* Funckija ki brcne zunanji WatchDog se lahko
* kliče samo iz periodicne prekinitve
**************************************************************/
#pragma CODE_SECTION(PCB_kick_dog_int, "ramfuncs");
void PCB_kick_dog_int(void)
{
    // brcnemo psa
    if (counter <= PCB_DOG_KICK_ON)
    {
        GpioDataRegs.GPBCLEAR.bit.GPIO38 = 1;
    }
    else
    {
        GpioDataRegs.GPBSET.bit.GPIO38 = 1;
    }

    // povecamo stevec prekinitev
    counter = counter + 1;
    if (counter > (SAMP_FREQ / PCB_DOG_FREQ) ) //tukaj sem spremenil iz SAMPLE_FREQ v SWITCH_FREQ
    {
        counter = 0;
    }
}

/**************************************************************
* Funckija, ki pove CPLD da je temperatura hladilnika normalna
**************************************************************/
void TEMP_normal(void)
{
    GpioDataRegs.GPASET.bit.GPIO28 = 1;
}

/**************************************************************
* Funckija, ki pove CPLD da je temperatura hladilnika kritična
**************************************************************/
void TEMP_critical(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO28 = 1;
}

/**************************************************************
* Funckija, ki vklopi rele1
**************************************************************/
void PCB_in_relay_on(void)
{
    GpioDataRegs.GPCSET.bit.GPIO70 = 1;
}

/**************************************************************
* Funckija, ki izklopi rele1(za priklop omrežne napetosti)
**************************************************************/
void PCB_in_relay_off(void)
{
    GpioDataRegs.GPCCLEAR.bit.GPIO70 = 1;
}

/**************************************************************
* Funckija, ki vklopi rele2
**************************************************************/
void PCB_load_relay_on(void)
{
    GpioDataRegs.GPASET.bit.GPIO23 = 1;
}

/**************************************************************
* Funckija, ki izklopi rele2(premostitveni rele)
**************************************************************/
void PCB_load_relay_off(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;
}

/**************************************************************
* Funckija, ki vklopi rele2
**************************************************************/
void PCB_res_relay_on(void)
{
    GpioDataRegs.GPCSET.bit.GPIO72 = 1;
}

/**************************************************************
* Funckija, ki izklopi rele2(premostitveni rele)
**************************************************************/
void PCB_res_relay_off(void)
{
    GpioDataRegs.GPCCLEAR.bit.GPIO72 = 1;
}

/**************************************************************
* Funckija, ki vklopi rele3
**************************************************************/
void PCB_out_relay_on(void)
{
    GpioDataRegs.GPASET.bit.GPIO21 = 1;
}

/**************************************************************
* Funckija, ki izklopi rele3(izhodni rele-priklop bremena)
**************************************************************/
void PCB_out_relay_off(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;
}

/**************************************************************
* Funckija, ki vklopi LED indikacijo tokovnega režima
**************************************************************/
void PCB_mode_LED_on(void)
{
    GpioDataRegs.GPASET.bit.GPIO27 = 1;
}

/**************************************************************
* Funckija, ki izklopi LED indikacijo tokovnega režima(delamo v napetostnem režimu)
**************************************************************/
void PCB_mode_LED_off(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO27 = 1;
}

/**************************************************************
* Funckija ki resetira zapah na prilagodilni tiskanini
*
* !!!!!!!!!!!!!!!!!!!!!
* je ne smeš klicati iz prekinitve
* !!!!!!!!!!!!!!!!!!!!
*
**************************************************************/
#pragma CODE_SECTION(PCB_lat_reset, "ramfuncs");
void PCB_lat_reset(void)
{
    GpioDataRegs.GPASET.bit.GPIO29 = 1;
    DELAY_US(500L);
    GpioDataRegs.GPACLEAR.bit.GPIO29 = 1;
}


/**************************************************************
* Funckija ki prizge fault LED
**************************************************************/
void PCB_fault_LED_on(void)
{
    GpioDataRegs.GPASET.bit.GPIO25 = 1;
}

/**************************************************************
* Funckija ki ugasne fault LED
**************************************************************/
void PCB_fault_LED_off(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO25 = 1;
}

/**************************************************************
* Funckija ki prizge fault LED
**************************************************************/
void PCB_en_LED_on(void)
{
    GpioDataRegs.GPASET.bit.GPIO25 = 1;
}

/**************************************************************
* Funckija ki ugasne fault LED
**************************************************************/
void PCB_en_LED_off(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO25 = 1;
}

/**************************************************************
* Funckija ki spremeni stanje fault LED
**************************************************************/
void PCB_en_LED_toggle(void)
{
    GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1;
}
/**************************************************************
* Funckija ki vrne stanje on tipke
**************************************************************/
bool PCB_en_SW(void)
{
    if (GpioDataRegs.GPADAT.bit.GPIO24 == 1)
    {
        return (FALSE);
    }
    else
    {
        return (TRUE);
    }
}

/**************************************************************
* Funckija ki vrne stanje mode tipke
**************************************************************/
bool PCB_reset_SW(void)
{
    if (GpioDataRegs.GPADAT.bit.GPIO26 == 1)
    {
        return (FALSE);
    }
    else
    {
        return (TRUE);
    }
}

/**************************************************************
* Funckija ki vrne stanje strojne za��ite
**************************************************************/
bool PCB_HW_trip(void)
{
    if (GpioDataRegs.GPADAT.bit.GPIO16 == 1)
    {
        return (FALSE);
    }
    else
    {
        return (TRUE);
    }

}

/**************************************************************
* Funckija ki inicializira PCB
**************************************************************/
void PCB_init(void)
{
    /* IZHODI */
    // GPIO25 - enable led
    GPIO_SetupPinMux(25, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(25, GPIO_OUTPUT, GPIO_PUSHPULL);
    
    // GPIO27 - mode led
    GPIO_SetupPinMux(27, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(27, GPIO_OUTPUT, GPIO_PUSHPULL);
    
    // GPIO29 - latch reset
    GPIO_SetupPinMux(29, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(29, GPIO_OUTPUT, GPIO_PUSHPULL);
    
    // GPIO21 - out relay
    GPIO_SetupPinMux(21, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(21, GPIO_OUTPUT, GPIO_PUSHPULL);
    PCB_out_relay_off();
    
    // GPIO72  res relay
    GPIO_SetupPinMux(72, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(72, GPIO_OUTPUT, GPIO_PUSHPULL);
    PCB_res_relay_off();
    
    // GPIO70 - in relay
    GPIO_SetupPinMux(70, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(70, GPIO_OUTPUT, GPIO_PUSHPULL);
    PCB_in_relay_off();
    
    // GPIO23 load relay
    GPIO_SetupPinMux(23, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(23, GPIO_OUTPUT, GPIO_PUSHPULL);
    PCB_load_relay_off();

    // GPIO28 - Temp status
    GPIO_SetupPinMux(28, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(28, GPIO_OUTPUT, GPIO_PUSHPULL);
    
    // GPIO38 - WD kick
    GPIO_SetupPinMux(38, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(38, GPIO_OUTPUT, GPIO_PUSHPULL);

    /* IZHODI */
    // LED na card-u
    GPIO_SetupPinMux(83, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(83, GPIO_OUTPUT, GPIO_PUSHPULL);

    /* vhodi */
    // GPIO24 - enable tipka
    GPIO_SetupPinMux(24, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(24, GPIO_INPUT, GPIO_INPUT);

    // GPIO26 - reset tipka
    GPIO_SetupPinMux(26, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(26, GPIO_INPUT, GPIO_INPUT);

    // postavim v privzeto stanje
    PCB_mode_LED_off();     // napetostna regulacija
    PCB_fault_LED_off();
    PCB_out_relay_off();
    PCB_res_relay_off();
    PCB_in_relay_off();
}
