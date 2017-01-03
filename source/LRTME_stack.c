/****************************************************************
* FILENAME:     LRTME_stack.c
* DESCRIPTION:  LRTME stack code
* AUTHOR:       Mitja Nemec
* START DATE:   3.12.2015
*
****************************************************************/

#include "LRTME_stack.h"

// deklaracije statiènih lokalnih spremenljivk
long    LRTME_crc_error_count = 0;
long	LRTME_packets_received = 0;
long	LRTME_packets_sent = 0;

// spremenljivke, ki jih potrebujemo pri pošiljanju
int     LRTME_raw_send[LRTME_TX_BUFF_SIZE];
int     LRTME_raw_send_length;
int     LRTME_encoded_send[LRTME_TX_BUFF_SIZE];
int     LRTME_encoded_send_length;

int     LRTME_raw_send_code;
int     *LRTME_raw_send_data;

void    (*LRTME_raw_tx_callback)(void);

struct  LRTME_tx_struct
{
    int     code;
    int     *data;
    int     length;
    void    (*LRTME_tx_callback)(void);
};

struct  LRTME_tx_struct LRTME_tx_send_queue[LRTME_TX_QUEUE_SIZE];
int     LRTME_tx_Q_first = 0;
int     LRTME_tx_Q_last = 0;
int     LRTME_tx_Q_number = 0;



int     LRTME_raw_received[LRTME_RX_BUFF_SIZE];         // bufer za podatke, ki so še zakodirani s COBS-om
int     LRTME_raw_received_length;      // dolžina prejetega zakodiranega paketa (v bajtih)
int     LRTME_decoded_received[LRTME_RX_BUFF_SIZE];     // buffer za podatke, ki so že odkodirani
int     LRTME_decoded_received_length;  // dolžina prejetega paketa (v bajtih)

int     LRTME_received_code;                // prejeti ukaz
void    (*LRTME_rx_handler)(int *);  // funkcija, ki jo je potrebno klicati

// prototip strukture, ki jih postavljam v vrsto ob registraciji handlerjev za prejete ukaze
struct  LRTME_rx_struct
{
    int code;
    void (*function)(int *);
};

// bufer, kamor se bo registiralo vse handler funkcije
struct  LRTME_rx_struct LRTME_rx_handler_list[LRTME_RX_NR_HANDLERS];
int     LRTME_rx_list_elements = 0;



// pototipi lokalnih funkcij
void LRTME_receive(void);
void LRTME_receive_register(int code, void (*function)(int *));
int LRTME_tx_queue_poll(void);
void LRTME_transmit(void);
void LRTME_tx_queue_put(struct  LRTME_tx_struct *to_send);
bool LRTME_tx_queue_get(struct  LRTME_tx_struct *to_send);

void LRTME_stack(void)
{
    // send
    LRTME_transmit();
    // receiva and call hanldes
    LRTME_receive();
}

#pragma CODE_SECTION(LRTME_transmit, "ramfuncs");
void LRTME_transmit(void)
{
    struct  LRTME_tx_struct za_poslat;
    int i;

    // èe sploh lahko pošiljam
    if (SCI_data_sent() == TRUE)
    {
        // in èe je kaj za poslati
        if (LRTME_tx_queue_poll() != 0)
        {
            // poberem, kar je za poslati
            LRTME_tx_queue_get(&za_poslat);
            LRTME_raw_send_code = za_poslat.code;
            LRTME_raw_send_data = za_poslat.data;
            LRTME_raw_send_length = za_poslat.length;
            LRTME_raw_tx_callback = za_poslat.LRTME_tx_callback;

            // naložim v buffer
            LRTME_raw_send[0] = LRTME_raw_send_code;
            for (i = 1; i <= LRTME_raw_send_length/2; i = i + 1)
            {
                LRTME_raw_send[i] = *LRTME_raw_send_data;
                LRTME_raw_send_data = LRTME_raw_send_data + 1;
            }

            // zakodiram
            LRTME_encoded_send_length = COBS_encode(LRTME_raw_send, LRTME_raw_send_length + 2, LRTME_encoded_send);

            // ker so podatki že zakodirani lahko že sedaj izvedem callback
            // èe ga je sploh treba
            if (LRTME_raw_tx_callback != NULL)
            {
                (*LRTME_raw_tx_callback)();
            }

            // javim, da je uspešno poslan
            LRTME_packets_sent = LRTME_packets_sent + 1;

            // potem pa podatke pošljem po serijskem portu
            SCI_send_word(LRTME_encoded_send, LRTME_encoded_send_length);
        }
    }
}

#pragma CODE_SECTION(LRTME_send, "ramfuncs");
void LRTME_send(int code, int *data, int length, void    (*tx_callback)(void))
{
    struct  LRTME_tx_struct za_poslat;

    za_poslat.code = code;
    za_poslat.data = data;
    za_poslat.length = length;
    za_poslat.LRTME_tx_callback = tx_callback;

    LRTME_tx_queue_put(&za_poslat);
}

int LRTME_tx_queue_poll(void)
{
    return (LRTME_tx_Q_number);
}
/**************************************************************
* Inicializacija vrste uporabljene za sprejemni buffer
* returns:
**************************************************************/

void LRTME_tx_queue_init(void)
{
    // rx queue
    int i = 0;

    LRTME_tx_Q_first = 0;
    LRTME_tx_Q_last = 0;
    LRTME_tx_Q_number = 0;

    // inicializacija vrste
    for (i = 0; i < SCI_RX_BUFFER_SIZE; i++)
    {
        LRTME_tx_send_queue[i].code = 0;
        LRTME_tx_send_queue[i].data = 0;
        LRTME_tx_send_queue[i].length = 0;
        LRTME_tx_send_queue[i].LRTME_tx_callback = 0;
    }
}

/**************************************************************
* vpisovanje novega elementa v vrsto
* returns:
* arg1:     element to put to the queue
**************************************************************/
#pragma CODE_SECTION(LRTME_tx_queue_put, "ramfuncs");
void LRTME_tx_queue_put(struct  LRTME_tx_struct *to_send)
{
    // blokiram komunikacijske prekinitve, da sluèajno ne bo kdo ravno prebiral iz vrste
	SCI_disable_interrupts();

    //dam v buffer
    LRTME_tx_send_queue[LRTME_tx_Q_last].code = to_send->code;
    LRTME_tx_send_queue[LRTME_tx_Q_last].data = to_send->data;
    LRTME_tx_send_queue[LRTME_tx_Q_last].length = to_send->length;
    LRTME_tx_send_queue[LRTME_tx_Q_last].LRTME_tx_callback = to_send->LRTME_tx_callback;

    // povecam kazalec in stevec elementov
    LRTME_tx_Q_last = LRTME_tx_Q_last + 1;
    LRTME_tx_Q_number = LRTME_tx_Q_number + 1;

    // ce sem ze prisel okoli kroznega bufferja
    if (LRTME_tx_Q_last >= LRTME_TX_QUEUE_SIZE)
    {
        LRTME_tx_Q_last = 0;
    }

    // ce je vrst polna vrzi ven najstarejsi element
    if (LRTME_tx_Q_number >= LRTME_TX_QUEUE_SIZE)
    {
        LRTME_tx_Q_first = LRTME_tx_Q_first + 1;
    }
    if (LRTME_tx_Q_first >= LRTME_TX_QUEUE_SIZE)
    {
        LRTME_tx_Q_first = 0;
    }

    // odblokiram prekinitve
    SCI_enable_interrupts();
}

/**************************************************************
* pobiranje elementa iz vrste
* returns:  element from queue
**************************************************************/
#pragma CODE_SECTION(LRTME_tx_queue_get, "ramfuncs");
bool LRTME_tx_queue_get(struct  LRTME_tx_struct *to_send)
{
    // poberem lahko samo ce je kaj za pobrat
    if (LRTME_tx_Q_number > 0)
    {
        // blokiram komunikacijske prekinitve, da mi sluèajno ne bo kdo hkrati pisal ali v Queue
        SCI_disable_interrupts();

        // preberem podatek
        to_send->code = LRTME_tx_send_queue[LRTME_tx_Q_first].code;
        to_send->data = LRTME_tx_send_queue[LRTME_tx_Q_first].data;
        to_send->length = LRTME_tx_send_queue[LRTME_tx_Q_first].length;
        to_send->LRTME_tx_callback = LRTME_tx_send_queue[LRTME_tx_Q_first].LRTME_tx_callback;

        // povecam kazalec
        LRTME_tx_Q_first = LRTME_tx_Q_first + 1;
        LRTME_tx_Q_number = LRTME_tx_Q_number - 1;

        // po potrebi ga obrnem okoli
        if (LRTME_tx_Q_first >= LRTME_TX_QUEUE_SIZE)
        {
            LRTME_tx_Q_first = 0;
        }

        // odblokiram prekinitve
        SCI_enable_interrupts();

        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

#pragma CODE_SECTION(LRTME_receive, "ramfuncs");
void LRTME_receive(void)
{
    int i;

    bool data_correct;

    // branje z vodila
    if (SCI_chk_packet_ready() == TRUE)
    {
        LRTME_raw_received_length = SCI_get_packet(LRTME_raw_received);

        // potem pa še paketek dekodiram
        LRTME_decoded_received_length = COBS_decode(LRTME_raw_received, LRTME_raw_received_length, LRTME_decoded_received, &data_correct);

        // èe je izraèunani CRC enak 0x0000 potem je podatek pravilen
        if (data_correct == TRUE)
        {
        	// še en uspešno prejet paket
        	LRTME_packets_received = LRTME_packets_received + 1;

            // preberem ukaz
            LRTME_received_code = LRTME_decoded_received[0];

            // potem pa pogledam po listu, katero funkcijo naj klièem
            for (i = 0; i < LRTME_rx_list_elements; i = i + 1)
            {
                // èe jo najdem, potem nastavim kazalec na fukcijo
                if (LRTME_rx_handler_list[i].code == LRTME_received_code)
                {
                    LRTME_rx_handler = LRTME_rx_handler_list[i].function;

                    // èe so v paketu tudi podatki, potem pokažem na njih
                    if (LRTME_decoded_received_length > 2)
                    {
                        (*LRTME_rx_handler)(&LRTME_decoded_received[1]);
                    }
                    // sicer pa dam null pointer
                    else
                    {
                        (*LRTME_rx_handler)(0);
                    }
                }
            }
        }
        else
        {
            LRTME_crc_error_count = LRTME_crc_error_count + 1;
        }
    }
}

// registriram nov handler
void LRTME_receive_register(int code, void (*function)(int *))
{
    LRTME_rx_handler_list[LRTME_rx_list_elements].code = code;
    LRTME_rx_handler_list[LRTME_rx_list_elements].function = function;
    LRTME_rx_list_elements = LRTME_rx_list_elements + 1;
}

void LRTME_init(void)
{
    size_t i;
    LRTME_tx_queue_init();
    for (i = 0; i < sizeof(LRTME_raw_send); i = i + 1)
    {
        LRTME_raw_send[i] = 0;
    }
    for (i = 0; i < sizeof(LRTME_encoded_send); i = i + 1)
    {
        LRTME_encoded_send[i] = 0;
    }

    for (i = 0; i < sizeof(LRTME_raw_received); i = i + 1)
    {
        LRTME_raw_received[i] = 0;
    }
    for (i = 0; i < sizeof(LRTME_decoded_received); i = i + 1)
    {
        LRTME_decoded_received[i] = 0;
    }


}

