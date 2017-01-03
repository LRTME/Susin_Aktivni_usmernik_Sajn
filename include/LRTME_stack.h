/****************************************************************
* FILENAME:     LRTME_stack.h
* DESCRIPTION:  LRTME stack code
* AUTHOR:       Mitja Nemec
* START DATE:   3.12.2015
*
****************************************************************/
#ifndef     __LETMR_STACK_H__
#define     __LETMR_STACK_H__

#include    "F28x_Project.h"
#include    "define.h"

#include    "COBS_drv.h"
#include    "SCI_drv.h"

// najdaljši paket, ki ga bom še poslal
#define     LRTME_TX_BUFF_SIZE      2000

// najveèje število paketov ki èaka na pošiljanje
#define     LRTME_TX_QUEUE_SIZE     20

// najdaljši paket, ki ga bom še sprejel
#define     LRTME_RX_BUFF_SIZE      40

// koliko RX handlerjev lahko registriram
#define     LRTME_RX_NR_HANDLERS    40

extern void LRTME_init(void);

extern void LRTME_stack(void);

extern int LRTME_tx_queue_poll(void);

// place a frame to the send queue
extern void LRTME_send(int code, int *data, int length, void    (*tx_callback)(void));

// register a receive frame handler
extern void LRTME_receive_register(int code, void (*function)(int *));

#endif // end of __LETMR_STACK_H__
