/*************************************************************
* FILE:         LRTME_com.c
* DESCRIPTION:  LRTME komunikacijski driver
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
*************************************************************/
#include "SCI_drv.h"

// internal variables for signaling between interrupts and code

// wheather I'm sending bytes or words
int SCI_tx_send_type = 0;

// number of bytes/words sent
volatile int SCI_tx_sent = 0;
volatile int *SCI_tx_ptr = 0;
volatile int SCI_tx_nr_bytes = 0;

// number of bytes/words received
volatile int SCI_rx_received = 0;
volatile int *SCI_rx_ptr = 0;
volatile int SCI_rx_nr_bytes = 0;
volatile int SCI_rx_packets = 0;

// rx queue
unsigned int SCI_Q_first_byte;
unsigned int SCI_Q_last_byte;
unsigned int SCI_Q_nr_bytes;
int SCI_Q_buffer[SCI_RX_BUFFER_SIZE];


// interupt routines prototypes
interrupt void SCI_rx_interrupt(void);
interrupt void SCI_tx_interrupt(void);

// timeout function prototypes
void SCI_timer_init(void);
void SCI_timer_start(long timeout);
int  SCI_timer_check_timeout(void);

// rx queue function prototypes
void SCI_queue_init(void);
void SCI_queue_put(int element);
int  SCI_queue_get(void);
int SCI_get_packet(int *dst);


/**************************************************************
* prekinitvena rutina za prejemanje
* returns:
**************************************************************/
interrupt void SCI_rx_interrupt(void)
{
    int data = 0;

    // enable all higher priority interrupts
    EINT;

    // transfer all the data from FIFO to the queue
    while(SCI_MODUL.SCIFFRX.bit.RXFFST != 0)
    {
        // read byte
        data = SCI_MODUL.SCIRXBUF.bit.SAR;

        // èe sem prejel 0x0000, potem naznanim, da sem prejel en COBS paket
        if (data == 0x0000)
        {
            SCI_rx_packets = SCI_rx_packets + 1;
        }
        //and put in queue
        SCI_queue_put(data);
    }

    // acknowledge the interrupt
    SCI_MODUL.SCIFFRX.bit.RXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
    
    // Disable all higher priority interrupts
    // to prevent RB corruption
    DINT;
}

void SCI_flush_fifo(void)
// transfer all the data from FIFO to the queue
{
    int data;

    SCI_MODUL.SCIFFRX.bit.RXFFIENA = 0;
    while(SCI_MODUL.SCIFFRX.bit.RXFFST != 0)
    {
        // read byte
        data = SCI_MODUL.SCIRXBUF.bit.SAR;

        // èe sem prejel 0x0000, potem naznanim, da sem prejel en COBS paket
        if (data == 0x0000)
        {
            SCI_rx_packets = SCI_rx_packets + 1;
        }
        //and put in queue
        SCI_queue_put(data);
    }
    SCI_MODUL.SCIFFRX.bit.RXFFIENA = 1;
}
/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
int SCI_receive_byte(void *data, int nr_bytes, long timeout)
{
    SCI_rx_received = 0;
    SCI_rx_ptr = data;
    SCI_rx_nr_bytes = nr_bytes;

    // start timeout - only if required
    if (timeout != 0)
    {
        SCI_timer_start(timeout);
    }

    // transfer data from queue until you get all the data
    // or timeout occurs
    while (   (SCI_timer_check_timeout() == FALSE)
            &&(SCI_rx_received < SCI_rx_nr_bytes) )
    {
        // if data available get it out
        if (SCI_Q_nr_bytes != 0)
        {
            *SCI_rx_ptr = SCI_queue_get();
            SCI_rx_received = SCI_rx_received + 1;
            SCI_rx_ptr = SCI_rx_ptr + 1;
        }
        // if waiting for less then FIFO interrupt level, then
        // reduce FIFO interrupt level
        if (   (SCI_rx_received < SCI_rx_nr_bytes)
                &&((SCI_rx_nr_bytes - SCI_rx_received) < SCI_FIFO_DEPTH))
        {
                SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_rx_nr_bytes - SCI_rx_received;
        }
    }

    // whene received all the data reset FIFO interrupt level
    SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_FIFO_DEPTH;

    return(SCI_rx_received);
}

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
int SCI_receive_word(void *data, int nr_bytes, long timeout)
{
    SCI_rx_received = 0;
    SCI_rx_ptr = data;
    SCI_rx_nr_bytes = nr_bytes;

    // start timeout - only if required
    if (timeout != 0)
    {
        SCI_timer_start(timeout);
    }

    // transfer data from queue until you get all the data
    // or timeout occurs
    while (   (SCI_timer_check_timeout() == FALSE)
            &&(SCI_rx_received < SCI_rx_nr_bytes) )
    {
        // if data available get it out
        if (SCI_Q_nr_bytes != 0)
        {
            *SCI_rx_ptr = SCI_queue_get();
            SCI_rx_received = SCI_rx_received + 1;
            SCI_rx_ptr = SCI_rx_ptr + 1;
        }
        // if waiting for less then FIFO interrupt level, then
        // reduce FIFO interrupt level
        if (   (SCI_rx_received < SCI_rx_nr_bytes)
                &&(((SCI_rx_nr_bytes - SCI_rx_received)*2) < SCI_FIFO_DEPTH))
        {
                SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_rx_nr_bytes - SCI_rx_received;
        }
    }

    // whene received all the data reset FIFO interrupt level
    SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_FIFO_DEPTH;

    return(SCI_rx_received);
}

/**************************************************************
* prekinitvena rutina za posiljanje
* returns:
**************************************************************/
interrupt void SCI_tx_interrupt(void)
{
    // enable all higher priority interrupts
    EINT;

    // I'm sending bytes
    if (SCI_tx_send_type == 0)
    {
        // fill the fifo, while there is data to send and place in fifo
        while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < SCI_FIFO_DEPTH)
               &&(SCI_tx_sent < SCI_tx_nr_bytes) )
        {
            SCI_MODUL.SCITXBUF.bit.TXDT = *SCI_tx_ptr;
            SCI_tx_ptr = SCI_tx_ptr + 1;
            SCI_tx_sent = SCI_tx_sent + 1;
        }

        // if all data was send, disable TX FIFO interrupt
        if (SCI_tx_sent >= SCI_tx_nr_bytes)
        {
            SCI_MODUL.SCIFFTX.bit.TXFFIENA = 0;
        }
    }
    // I'm sending words
    else
    {
        // fill the fifo, while there is data to send and place in fifo
        while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < (SCI_FIFO_DEPTH - 1))
               &&(SCI_tx_sent < SCI_tx_nr_bytes) )
        {
            // first send MSB
            #pragma diag_push
            #pragma diag_suppress 169
            SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
            #pragma diag_pop
            SCI_tx_sent = SCI_tx_sent + 1;

            // ce je slucajno konec potem grem ven iz zanke
            if (SCI_tx_sent >= SCI_tx_nr_bytes)
            {
                break;
            }

            // then send LSB
            #pragma diag_push
            #pragma diag_suppress 169
            SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
            #pragma diag_pop
            SCI_tx_sent = SCI_tx_sent + 1;
        }

        // if all data was send, disable TX FIFO interrupt
        if (SCI_tx_sent >= SCI_tx_nr_bytes)
        {
            SCI_MODUL.SCIFFTX.bit.TXFFIENA = 0;
        }
    }

    // acknowledge interrupt
    SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
    
    // Disable all higher priority interrupts
    // to prevent RB corruption
    DINT;
}

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* primerno samo za pošiljanje stringov za direkten prikaz v terminalu
* returns:  number of bytes to sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
int SCI_send_byte(void *data, int nr_bytes, long timeout)
{
    SCI_tx_send_type = 0;
    SCI_tx_sent = 0;
    SCI_tx_ptr = data;
    SCI_tx_nr_bytes = nr_bytes;

    // fill the fifo, while there is data to send and place in fifo
    while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < SCI_FIFO_DEPTH)
           &&(SCI_tx_sent < SCI_tx_nr_bytes) )
    {
        SCI_MODUL.SCITXBUF.bit.TXDT = *SCI_tx_ptr;
        SCI_tx_ptr = SCI_tx_ptr + 1;
        SCI_tx_sent = SCI_tx_sent + 1;
        // reset TX fifo flag, to prevent triggering spurious interrupts
        SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    }

    // if there is more data to send enable interrupts
    if (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        SCI_MODUL.SCIFFTX.bit.TXFFIENA = 1;
    }

    // start timeout


    // either yield (if using RTOS) or wait until sent, or timeout runs out
    while (SCI_tx_sent != SCI_tx_nr_bytes)
    {
        /* DO NOTHING */
    }

    // if ended due to timeout, cancel transmission in progress

    // and signal how many bytes/words were sent
    return(SCI_tx_sent);
}

/**************************************************************
* funkcija ki poslje n bytov po serijskem vodilu
* returns:  number of bytes sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
int SCI_send_word(void *data, int nr_bytes)
{
    // preper for begining of the send sequence
    SCI_tx_send_type = 1;
    SCI_tx_sent = 0;

    SCI_tx_ptr = data;
    SCI_tx_nr_bytes = nr_bytes;

    // fill the fifo, while there is data to send and place in fifo
    while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < (SCI_FIFO_DEPTH - 1))
           &&(SCI_tx_sent < SCI_tx_nr_bytes) )
    {
        // first send MSB
        #pragma diag_push
        #pragma diag_suppress 169
        SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
        #pragma diag_pop
        SCI_tx_sent = SCI_tx_sent + 1;
        // ce je slucajno konec potem grem ven iz zanke
        if (SCI_tx_sent >= SCI_tx_nr_bytes)
        {
            break;
        }
		// then send LSB
        #pragma diag_push
        #pragma diag_suppress 169
        SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
        #pragma diag_pop
        SCI_tx_sent = SCI_tx_sent + 1;

        // clear int flag
        SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    }

    // if there is more data to send enable interrupts
    if (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        SCI_MODUL.SCIFFTX.bit.TXFFIENA = 1;
    }
    // and signal how many bytes/words were sent
    return(SCI_tx_sent);
}

int SCI_send_word_blocking(void *data, int nr_bytes, long timeout)
{
    SCI_send_word(data, nr_bytes);
    // wait until sent
    while (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        /* DO NOTHING */
    }
    return(SCI_tx_sent);
}

bool SCI_data_sent(void)
{
    if (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        return (FALSE);
    }
    else
    {
        return( TRUE);
    }
}

/**************************************************************
* Inicializacija SCI modula
* returns:
* arg1:     baud rate
**************************************************************/
void SCI_init(long baud_rate)
{
    // za izracuna baud rate registra
    float baud_f;
    int baud;
    int baud_H;
    int baud_L;
    long sci_freq;

    // configure inputs/ouputs
    EALLOW;
    GPIO_SetupPinMux(86, GPIO_MUX_CPU1, 5);
    GPIO_SetupPinMux(87, GPIO_MUX_CPU1, 5);
    EDIS;

    // calculate baudrate
    if(ClkCfgRegs.LOSPCP.bit.LSPCLKDIV == 0)
    {
        sci_freq = CPU_FREQ;
    }
    else
    {
        sci_freq = CPU_FREQ / (2 * ClkCfgRegs.LOSPCP.bit.LSPCLKDIV);
    }

    baud_f = (sci_freq / ((float)(baud_rate * 8L))) - 1L;
    baud = baud_f;
    baud_H = (int)(baud >> 8) ;
    baud_L = (int)(baud & 0x00FF);

    SCI_MODUL.SCIHBAUD.bit.BAUD = baud_H;
    SCI_MODUL.SCILBAUD.bit.BAUD = baud_L;

    // configure SCI peripherals to use FIFO
    SCI_MODUL.SCICTL1.bit.SWRESET = 0;

    SCI_MODUL.SCICCR.bit.STOPBITS = 1;	// two stop bits
    SCI_MODUL.SCICCR.bit.PARITY = 0;
    SCI_MODUL.SCICCR.bit.PARITYENA = 0;
    SCI_MODUL.SCICCR.bit.LOOPBKENA = 0;
    SCI_MODUL.SCICCR.bit.ADDRIDLE_MODE = 0;
    SCI_MODUL.SCICCR.bit.SCICHAR = 7;

    SCI_MODUL.SCICTL1.bit.RXERRINTENA = 0;
    SCI_MODUL.SCICTL1.bit.TXWAKE = 0;
    SCI_MODUL.SCICTL1.bit.SLEEP = 0;

    SCI_MODUL.SCICTL2.bit.TXINTENA = 0;
    SCI_MODUL.SCICTL1.bit.RXERRINTENA = 0;

    #if SCI_DEBUG_MODE == 0
    SCI_MODUL.SCIPRI.bit.FREESOFT = 0;
    #else
    SCI_MODUL.SCIPRI.bit.FREESOFT = 3;
    #endif

    // setup FIFO registers
    SCI_MODUL.SCIFFTX.bit.SCIFFENA = 1; // FIFO je omogocen

    SCI_MODUL.SCIFFCT.bit.ABDCLR = 0;
    SCI_MODUL.SCIFFCT.bit.CDC = 0;
    SCI_MODUL.SCIFFCT.bit.FFTXDLY = 2;

    SCI_MODUL.SCIFFTX.bit.SCIFFENA = 1;
    SCI_MODUL.SCIFFTX.bit.TXFIFORESET = 1;
    SCI_MODUL.SCIFFTX.bit.TXFFINT = 0;
    SCI_MODUL.SCIFFTX.bit.TXFFIL = 0;
    SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    SCI_MODUL.SCIFFTX.bit.TXFFIENA = 0;

    SCI_MODUL.SCIFFRX.bit.RXFIFORESET = 0;
    SCI_MODUL.SCIFFRX.bit.RXFFOVRCLR = 1;
    SCI_MODUL.SCIFFRX.bit.RXFFINTCLR = 1;
    SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_FIFO_DEPTH; // kdaj naj se prozi prekinitev
    SCI_MODUL.SCIFFRX.bit.RXFFIENA = 1; // omogocim prekinitev, ko je FIFO poln
    SCI_MODUL.SCIFFRX.bit.RXFIFORESET = 1;

    SCI_MODUL.SCIFFCT.bit.FFTXDLY = 3;

    // enable receiver and transmiter
    SCI_MODUL.SCICTL1.bit.RXENA = 1;
    SCI_MODUL.SCICTL1.bit.TXENA = 1;

    SCI_MODUL.SCICTL1.bit.SWRESET = 1;   // enable SCI-module

    // register interrupt routines
    EALLOW;
    PieVectTable.SCIB_RX_INT = &SCI_rx_interrupt;
    PieVectTable.SCIB_TX_INT = &SCI_tx_interrupt;
    EDIS;
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1;
    PieCtrlRegs.PIEIER9.bit.INTx4 = 1;
    IER |= M_INT9;
    // da mi prekinitev tece  tudi v real time nacinu
    // (za razhorscevanje main zanke in BACK_loop zanke)
    SetDBGIER(M_INT9);

    // initialize timer
    SCI_timer_init();

    //initialize queue
    SCI_queue_init();
}

void SCI_disable_interrupts(void)
{
	PieCtrlRegs.PIEIER9.bit.INTx3 = 0;
	PieCtrlRegs.PIEIER9.bit.INTx4 = 0;
}

void SCI_enable_interrupts(void)
{
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx4 = 1;
}


/**************************************************************
* Inicializacija vrste uporabljene za sprejemni buffer
* returns:
**************************************************************/
void SCI_queue_init(void)
{
    // rx queue
    int i = 0;

    SCI_Q_first_byte = 0;
    SCI_Q_last_byte = 0;
    SCI_Q_nr_bytes = 0;

    // inicializacija vrste
    for (i = 0; i < SCI_RX_BUFFER_SIZE; i++)
    {
        SCI_Q_buffer[i] = 0;
    }
}

/**************************************************************
* vpisovanje novega elementa v vrsto
* returns:
* arg1:     element to put to the queue
**************************************************************/
void SCI_queue_put(int element)
{
    //dam v buffer
    __byte(SCI_Q_buffer,SCI_Q_last_byte) = element;

    // povecam kazalec in stevec elementov
    SCI_Q_last_byte = SCI_Q_last_byte + 1;
    SCI_Q_nr_bytes = SCI_Q_nr_bytes + 1;


    // ce sem ze prisel okoli kroznega bufferja
    if ((SCI_Q_last_byte >> 1) >= SCI_RX_BUFFER_SIZE)
    {
        SCI_Q_last_byte = 0;
    }

    // ce je vrst polna vrzi ven najstarejsi element
    if ((SCI_Q_nr_bytes >> 1) >= SCI_RX_BUFFER_SIZE)
    {
        SCI_Q_first_byte = SCI_Q_first_byte + 1;
    }
    if ((SCI_Q_first_byte >> 1) >= SCI_RX_BUFFER_SIZE)
    {
        SCI_Q_first_byte = 0;
    }
}

/**************************************************************
* pobiranje elementa iz vrste
* returns:  element from queue
**************************************************************/
int SCI_queue_get(void)
{
    int element;
    // poberem lahko samo ce je kaj za pobrat
    if (SCI_Q_nr_bytes > 0)
    {
        element = __byte(SCI_Q_buffer,SCI_Q_first_byte);
        // povecam kazalec
        SCI_Q_first_byte = SCI_Q_first_byte + 1;
        SCI_Q_nr_bytes = SCI_Q_nr_bytes - 1;

        // po potrebi ga obrnem okoli
        if ((SCI_Q_first_byte >> 1) >= SCI_RX_BUFFER_SIZE)
        {
            SCI_Q_first_byte = 0;
        }

    }
    return (element);
}

/**************************************************************
* pobere paket
* returns:  element from queue
**************************************************************/
int SCI_get_packet(int *dst)
{
    unsigned int index = 0;
    int bajt;

    // èe je paket sploh na voljo
    if (SCI_rx_packets != 0)
    {
        // pobiram, dokler ne naletim na konec paketa
        do
        {
            // poberem en bajt
            bajt = SCI_queue_get();

            // potem pa zapisem ta bajt
            __byte(dst,index) = bajt;
            
            index = index + 1;
        } while (bajt != 0);

        // ko poberem paket, je seveda sedaj en paket manj na voljo
        SCI_MODUL.SCIFFRX.bit.RXFFIENA = 0;
        SCI_rx_packets  = SCI_rx_packets - 1;
        SCI_MODUL.SCIFFRX.bit.RXFFIENA = 1;
    }
    return (index - 1);
}

/**************************************************************
* preveri ali je kak paket na voljo
* returns:  element from queue
**************************************************************/
bool SCI_chk_packet_ready(void)
{
    // spraznem, kar je ostalo v FIFO-tu
    SCI_flush_fifo();

    if (SCI_rx_packets != 0)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

/**************************************************************
* Inicializacija casovnika uporabljenega pri sprejemanju
* returns:
**************************************************************/
void SCI_timer_init(void)
{
    SCI_TIMER.PRD.all  = 0xFFFFFFFF;
    SCI_TIMER.TPR.all  = 0;
    SCI_TIMER.TPRH.all = 0;
    SCI_TIMER.TCR.bit.TSS = 1;
    SCI_TIMER.TCR.bit.TRB = 1;
    SCI_TIMER.TCR.bit.FREE = 0;
    SCI_TIMER.TCR.bit.SOFT = 1;
}

/**************************************************************
* pozene casovnik ki odsteva cas
* returns:
* arg1:     cas ki ga casovnik steje (v us)
**************************************************************/
void SCI_timer_start(long timeout)
{
    long timer;

    timer = timeout * (CPU_FREQ / 1000000L);
    SCI_TIMER.PRD.all  = timer;
    SCI_TIMER.TCR.bit.TRB = 1;
    SCI_TIMER.TCR.bit.TIF = 0;
    SCI_TIMER.TCR.bit.TSS = 0;

}

/**************************************************************
* preveri aje casovnik ze prisel do konca
* returns:  TURE or FALSE regardles if timer has counted the time
**************************************************************/
int  SCI_timer_check_timeout(void)
{
    int return_value = FALSE;
    if (SCI_TIMER.TCR.bit.TIF == 1)
    {
        return_value = TRUE;
        SCI_TIMER.TCR.bit.TSS = 1;
    }
    return (return_value);
}


