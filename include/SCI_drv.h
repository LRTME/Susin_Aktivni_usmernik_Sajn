/*************************************************************
* FILE:         SCI_drv.h
* DESCRIPTION:  SCI driver
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
*************************************************************/
#ifndef     __SCI_DRV_H__
#define     __SCI_DRV_H__
        
#include    "F28x_Project.h"
#include    "define.h"


// which peripheral is in use
#define     SCI_MODUL       ScibRegs

// which timer to use for timeout
#define     SCI_TIMER       CpuTimer2Regs

// receive mode (0-byte, 1-word)
#define     SCI_MODE        1

//globina FIFO bufferja
#define     SCI_FIFO_DEPTH  4

// globina RX bufferja
#define     SCI_RX_BUFFER_SIZE  40

// delovanje SCI modula ob debug ustavitvi
// 0 - stop immediatelly, 1 - run free
#define     SCI_DEBUG_MODE      1

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
extern int SCI_receive_byte(void *data, int nr_bytes, long timeout);

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* returns:  number of words received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
extern int SCI_receive_word(void *data, int nr_bytes, long timeout);

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* primerno samo za pošiljanje stringov za direkten prikaz v terminalu
* returns:  number of bytes to sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
extern int SCI_send_byte(void *data, int nr_bytes, long timeout);

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* returns:  number of bytes sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
extern int SCI_send_word(void *data, int nr_bytes);
extern int SCI_send_word_blocking(void *data, int nr_bytes, long timeout);
extern bool SCI_data_sent(void);

/**************************************************************
* pobere paket
* returns:  element from queue
**************************************************************/
extern int SCI_get_packet(int *dst);

/**************************************************************
* preveri ali je kak paket na voljo
* returns:  element from queue
**************************************************************/
extern bool SCI_chk_packet_ready(void);

/**************************************************************
* Inicializacija SCI modula
* returns:
* arg1:     baud rate
**************************************************************/
extern void SCI_init(long baud_rate);

extern void SCI_disable_interrupts(void);
extern void SCI_enable_interrupts(void);



#endif      // __SCI_DRV_H__
