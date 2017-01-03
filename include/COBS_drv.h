/*************************************************************
* FILE:         COBS_drv.c
* DESCRIPTION:  COBS(Consistent Overhead Byte Stuffing) encoder in decoder
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
*************************************************************/
#ifndef     __COBS_DRV_H__
#define     __COBS_DRV_H__

#include    "CRC_mitja.h"
#include    "define.h"

/**************************************************************
* funkcija dekodira string podatkov, ki je bil zakodiran s COBS algoritmu
* returns:  number bytes decoded
* arg1:     pointer to data for decoding
* arg2:     number of bytes to decode
* arg3:     pointer to decoded data
**************************************************************/
int COBS_decode(int* src, int length, int* dst, bool* data_correct);

/**************************************************************
* funkcija zakodira string podatkov s COBS algoritmu
* returns:  number bytes encoded
* arg1:     pointer to data for encoding
* arg2:     number of bytes to encode
* arg3:     pointer to encoded data
**************************************************************/
extern int COBS_encode(int* src, int length, int* dst);


#endif      // __COBS_DRV_H__
