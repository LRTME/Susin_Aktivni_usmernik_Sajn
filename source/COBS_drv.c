/*************************************************************
* FILE:         COBS_drv.c
* DESCRIPTION:  COBS(Consistent Overhead Byte Stuffing) encoder in decoder
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
*************************************************************/
#include    "COBS_drv.h"

/**************************************************************
* funkcija dekodira string podatkov, ki je bil zakodiran s COBS algoritmu
* returns:  number bytes decoded
* arg1:     pointer to data for decoding
* arg2:     number of bytes to decode
* arg3:     pointer to decoded data
**************************************************************/
int COBS_decode(int* src, int length, int* dst, bool* data_correct)
{
    int input_str_index = 0;
    int output_str_index = 0;
    int input_current_byte;
    int code;
    int stevec;

    // za zapis in kodiranje CRC-ja
    int crc;

    CRC_init();

    while (input_str_index < length)
    {
        // preberem koliko bajtov je v paketku

        // preberem en byte iz vhoda
        input_current_byte = __byte(src,input_str_index);

        // povecam index
        input_str_index = input_str_index + 1;

        code = input_current_byte;
        // skopiram ustrezno st bajtov
        for (stevec = 1; stevec < code; stevec++)
        {
            // najpej preberem en bajt
            input_current_byte = __byte(src,input_str_index);

            // povecam index
            input_str_index = input_str_index + 1;

            // potem pa zapisem ta bajt
            __byte(dst,output_str_index) = input_current_byte;

            // in izraèunam CRC za ta bajt
            CRC_add_byte(input_current_byte);
            // povecam index
            output_str_index = output_str_index + 1;
        }

        // ko pa pridem do konca, pa zapisem še nulo, ce je treba
        if (code < 0xFF)
        {
            __byte(dst,output_str_index) = 0x00;

            // in izraèunam CRC za ta bajt
            CRC_add_byte(0x0000);
            // povecam index
            output_str_index = output_str_index + 1;
        }
    }

    // pogledam, kakšen CRC sem naraèunal
    crc = CRC_get();

    // ker sem pri izraèunu CRC-ja dodal tudi poslan CRC bi moral biti rezultat
    // enak 0x0000, v kolikor so podatki pravilni
    if (crc == 0x0000)
    {
        *data_correct = TRUE;
    }
    else
    {
        *data_correct = FALSE;
    }

    return(output_str_index-1-2);
}

/**************************************************************
* funkcija zakodira string podatkov s COBS algoritmu
* returns:  number bytes encoded
* arg1:     pointer to data for encoding
* arg2:     number of bytes to encode
* arg3:     pointer to encoded data
**************************************************************/
int COBS_encode(int* src, int length, int* dst)
{
    int input_str_index = 0;
    int output_str_index = 1;
    int input_current_byte;
    int code_str_index = 0;

    // stevilo bitov v delnem paketicu
    int code = 1;

    // za zapis in kodiranje CRC-ja
    int crc, crc_lsb, crc_msb;

    CRC_init();

    // grem èez vhodni string
    while(input_str_index < length)
    {
        // preberem en byte iz vhoda
        input_current_byte = __byte(src,input_str_index);

        CRC_add_byte(input_current_byte);

        // preverim ali je byte enak 0
        if (input_current_byte == 0)
        {
            // zapisem stevilo nenicelnih bajtov v paketicu
            // zapišem trenutni byte na izhod
            __byte(dst,code_str_index) = code;

            // resetiram kazalec na polje kamor bom naslednjic
            // zapisal st bajtov v paketicu
            code_str_index = output_str_index;
            output_str_index = output_str_index + 1;

            // resetiram stevec nenicelnih bajtov
            code = 1;
        }
        else
        {
            // zapišem trenutni byte na izhod
            __byte(dst,output_str_index) = input_current_byte;

            output_str_index = output_str_index + 1;
            // povecam stevilo nenicelnih bajtov v tem paketicu
            code = code + 1;
            // ce je slucajno vseh bajtov ze 255 potem zakljucim paketic in zacnem novega
            if (code == 0x00FF)
            {
                // zapisem stevilo nenicelnih bajtov v paketicu
                // zapišem trenutni byte na izhod
                __byte(dst,code_str_index) = code;

                // resetiram kazalec na polje kamor bom naslednjic
                // zapisal st bajtov v paketicu
                code_str_index = output_str_index;
                output_str_index = output_str_index + 1;

                // resetiram stevec nenicelnih bajtov
                code = 1;
            }
        }
        // grem na naslednji bajt
        input_str_index = input_str_index + 1;
    }

    // sedaj zakodiram še CRC - najprej lsb
    crc = CRC_get();
    crc_lsb = crc & 0x00FF;
    crc_msb = (crc >> 8) & 0x00FF;

    if (crc_lsb == 0)
    {
        // zapisem stevilo nenicelnih bajtov v paketicu
        // zapišem trenutni byte na izhod
        __byte(dst,code_str_index) = code;

        // resetiram kazalec na polje kamor bom naslednjic
        // zapisal st bajtov v paketicu
        code_str_index = output_str_index;
        output_str_index = output_str_index + 1;

        // resetiram stevec nenicelnih bajtov
        code = 1;
    }
    else
    {
        // zapišem trenutni byte na izhod
        __byte(dst,output_str_index) = crc_lsb;

        output_str_index = output_str_index + 1;
        // povecam stevilo nenicelnih bajtov v tem paketicu
        code = code + 1;
        // ce je slucajno vseh bajtov ze 255 potem zakljucim paketic in zacnem novega
        if (code == 0x00FF)
        {
            // zapisem stevilo nenicelnih bajtov v paketicu
            // zapišem trenutni byte na izhod
            __byte(dst,code_str_index) = code;

            // resetiram kazalec na polje kamor bom naslednjic
            // zapisal st bajtov v paketicu
            code_str_index = output_str_index;
            output_str_index = output_str_index + 1;

            // resetiram stevec nenicelnih bajtov
            code = 1;
        }
    }

    if (crc_msb == 0)
    {
        // zapisem stevilo nenicelnih bajtov v paketicu
        // zapišem trenutni byte na izhod
        __byte(dst,code_str_index) = code;

        // resetiram kazalec na polje kamor bom naslednjic
        // zapisal st bajtov v paketicu
        code_str_index = output_str_index;
        output_str_index = output_str_index + 1;

        // resetiram stevec nenicelnih bajtov
        code = 1;
    }
    else
    {
        // zapišem trenutni byte na izhod
        __byte(dst,output_str_index) = crc_msb;

        output_str_index = output_str_index + 1;
        // povecam stevilo nenicelnih bajtov v tem paketicu
        code = code + 1;
        // ce je slucajno vseh bajtov ze 255 potem zakljucim paketic in zacnem novega
        if (code == 0x00FF)
        {
            // zapisem stevilo nenicelnih bajtov v paketicu
            // zapišem trenutni byte na izhod
            __byte(dst,code_str_index) = code;

            // resetiram kazalec na polje kamor bom naslednjic
            // zapisal st bajtov v paketicu
            code_str_index = output_str_index;
            output_str_index = output_str_index + 1;

            // resetiram stevec nenicelnih bajtov
            code = 1;
        }
    }

    // še zakljucim
    // zapisem stevilo nenicelnih bajtov v paketicu
    // zapišem trenutni byte na izhod
    __byte(dst,code_str_index) = code;

    // Èisto na koncu še zakluèim paket z 0x00, za potrebe sinhronizacije
    __byte(dst,output_str_index) = 0x0000;

    output_str_index = output_str_index + 1;

    // vrnem stevilo bajtov
    return(output_str_index);
}

