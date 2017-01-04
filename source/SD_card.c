/****************************************************************
* FILENAME:     SD_card.c
* DESCRIPTION:  SD_card initialization and data
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include    "SD_card.h"

static FATFS    g_sFatFs;

/**************************************************************
 * inicializacija SD kartice
 **************************************************************/
void SD_init(void)
{
    // fat file system result code
    FRESULT fresult;

    // priklopim kartico
    fresult = f_mount(&g_sFatFs, "0", 1);
    if(fresult != FR_OK)
    {
        asm(" ESTOP0");
    }
}



