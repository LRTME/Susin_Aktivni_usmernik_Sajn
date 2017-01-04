/****************************************************************
* FILENAME:     UP_time.c
* DESCRIPTION:  UP time counter initialization and data
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include    "UP_time.h"

// fat file system result code
static  FIL     fp_uptime;
static  FRESULT fresult;
static  UINT    bytes_read_written;
static  unsigned char    uptime_string[] = "0000";

// stevec uptime
long    uptime = 0;

/**************************************************************
 * poveèanje UP time števca
 **************************************************************/
void UP_inc(void)
{
    // poveèam števec
    uptime = uptime + 1;

    // odprem
    fresult = f_open(&fp_uptime, "uptime.bin", FA_OPEN_EXISTING | FA_WRITE);
    // zapisem
    uptime_string[0] = (uptime >> 24) & 0x00FF;
    uptime_string[1] = (uptime >> 16) & 0x00FF;
    uptime_string[2] = (uptime >> 8) & 0x00FF;
    uptime_string[3] = (uptime >> 0) & 0x00FF;
    fresult = f_write(&fp_uptime, uptime_string, sizeof(uptime_string)-1, &bytes_read_written);
    // zaprem
    fresult = f_close(&fp_uptime);
}

/**************************************************************
 * inicializacija UP time števca
 **************************************************************/
void UP_init(void)
{
    // odprem uptime.bin èe obstaja
    // in naložim vrednost v spremenljivko uptime
    fresult = f_open(&fp_uptime, "uptime.bin", FA_OPEN_EXISTING | FA_READ);
    // ce datoteke ni, jo ustvarim
    if (fresult == FR_NO_FILE)
    {
        // odprem
        fresult = f_open(&fp_uptime, "uptime.bin", FA_CREATE_NEW | FA_WRITE);
        // zapisem
        uptime_string[0] = 0x00; // (uptime >> 24) & 0x00FF
        uptime_string[1] = 0x00; // (uptime >> 16) & 0x00FF
        uptime_string[2] = 0x00; // (uptime >> 8) & 0x00FF
        uptime_string[3] = 0x00; // (uptime >> 0) & 0x00FF
        fresult = f_write(&fp_uptime, uptime_string, sizeof(uptime_string)-1, &bytes_read_written);
        // zaprem
        fresult = f_close(&fp_uptime);
    }
    // preberem v string
    fresult = f_read(&fp_uptime, uptime_string, 4, &bytes_read_written);
    fresult = f_close(&fp_uptime);
    // iz stringa postavim v uptime
    uptime = ((long)uptime_string[0] << 24)
           + ((long)uptime_string[1] << 16)
           + ((long)uptime_string[2] << 8)
           + ((long)uptime_string[3]);
}



