/****************************************************************
* FILENAME:     REP_REG.c
* DESCRIPTION:  Repetitive controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Su�in
* START DATE:   6.4.2016
* VERSION:      3.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016   Denis Su�in      Initial version
* 1.1		21.8.2017  Denis Su�in		Corrections of comments and names of variables
* 2.0		15.5.2019  Denis Su�in		Circular buffer compacted into function and
* 										circular buffer indexes handling upgraded
* 3.0		19.7.2019  Denis Su�in		Corrections to "i_delta" calculations. Before the function had bugs.
*
****************************************************************/

#include "REP_REG.h"

// deklaracija funkcij
int circular_buffer_transformation(int IndexLinearBuffer, int BufferSize);

// globalne spremenljivke








/****************************************************************************************************
* Funkcija, ki izvede algoritem repetitivnega regulatorja.
* Zelo za�eleno je, da je razmerje med vzor�no frekvenco in osnovno frekvenco reguliranega signala
* enako velikosti pomnilnika "BufferHistoryLength" (in ve�je od 20), saj je regulator na to ob�utljiv,
* kar lahko privede do nezanemarljivega pogre�ka v stacionarnem stanju.
****************************************************************************************************/
#pragma CODE_SECTION(REP_REG_CALC, "ramfuncs");
void REP_REG_CALC (REP_REG_float *v)
{
    // lokalne spremenljivke




    // program

    // omejitev dol�ine circular bufferja
    if (v->BufferHistoryLength > MAX_LENGTH_REP_REG_BUFFER)
    {
        v->BufferHistoryLength = MAX_LENGTH_REP_REG_BUFFER;
    }
    else if (v->BufferHistoryLength < MIN_LENGTH_REP_REG_BUFFER)
    {
        v->BufferHistoryLength = MIN_LENGTH_REP_REG_BUFFER;
    }
	
	// omejitev kompenzacije zakasnitve, ki ne sme presegati dol�ine bufferja
    if (v->k > v->BufferHistoryLength)
    {
        v->k = v->BufferHistoryLength;
    }
    else if (v->k < -v->BufferHistoryLength)
    {
        v->k = -v->BufferHistoryLength;
    }

    // omejitev vzor�nega signala med 0.0 in 0.9999
    //(SamplingSignal ne sme biti enak ena, ker mora biti indeks i omejen od 0 do BufferHistoryLength-1)
    v->SamplingSignal = (v->SamplingSignal > 0.99999)? 0.99999: v->SamplingSignal;
    v->SamplingSignal = (v->SamplingSignal < 0.0)? 0.0: v->SamplingSignal;

	
	

    // izra�un trenutnega indeksa bufferja
    v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




    // �e se indeks spremeni, potem gre algoritem dalje:
    //   1. �e je "SamplingSignal" zunaj te funkcije natan�no sinhroniziran z vzor�no frekvenco ("i_delta" = 1)
    //      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval in se
    //      izkoristi celotna velikost pomnilnika, kar je optimalno
    //   2. �e je "SamplingSignal" prepo�asen ("i_delta" < 1) oz. osnovna frekvenca reguliranega signala prenizka,
    //      ni nujno, da se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval,
    //      kar pomeni, da ta algoritem lahko deluje s frekvenco ni�jo od vzor�ne frekvence
    //   3. �e je "SamplingSignal" prehiter ("i_delta" > 1), oz. osnovna frekvenca reguliranega signala previsoka,
    //      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval,
    //      a se velikost pomnilnika umetno zmanj�a za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
    //      kar pomeni, kot da bi bila velikost pomnilnika manj�a za faktor "i_delta"
    //      OPOMBA: To zadnjo funkcionalnost je mo�no izklopiti z odkomentiranjem vrstice "v->i_delta = 1;"!
    if ((v->i != v->i_prev))
    {
		// izra�un razlike med trenutnim indeksom "i" in prej�njim indeksom "i_prev"
		// (�e je "SamplingSignal" prehiter, lahko velikost pomnilnika
		// umetno zmanj�amo za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
		// ki pa ne sme presegati polovico velikosti pomnilnika)
		v->i_delta = v->i - v->i_prev;

		// manipuliranje z indeksi - zaradi circular bufferja; �e indeks nara��a - inkrementiranje
		if ( (v->i < v->i_prev) && (v->i_delta < -(v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = v->BufferHistoryLength + v->i_delta;
		}
		// manipuliranje z indeksi - zaradi circular bufferja; �e indeks pada - dekrementiranje
		else if ( (v->i > v->i_prev) && (v->i_delta > (v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = -(v->BufferHistoryLength - v->i_delta);
		}


		// �e funkcionalnost umetnega zmanj�evanja velikosti pomnilnika ni za�elena ali ni potrebna (opis pod to�ko 3.),
		// odkomentiraj naslednji blok programske kode
		if(v->i_delta > 1)
		{
			v->i_delta = 1;
		}
		if(v->i_delta < -1)
		{
			v->i_delta = -1;
		}



		// manipuliranje z indeksi - zaradi circular bufferja
		v->index = circular_buffer_transformation(v->i + v->k*v->i_delta,v->BufferHistoryLength);
		v->i_plus_one = circular_buffer_transformation(v->i + 1*v->i_delta,v->BufferHistoryLength);
		v->i_minus_one = circular_buffer_transformation(v->i - 1*v->i_delta,v->BufferHistoryLength);
		v->i_plus_two = circular_buffer_transformation(v->i + 2*v->i_delta,v->BufferHistoryLength);
		v->i_minus_two = circular_buffer_transformation(v->i - 2*v->i_delta,v->BufferHistoryLength);




		/***************************************************
        * koda repetitivnega regulatorja
		***************************************************/
		
        // izra�unam trenutni error
        v->Err = v->Ref - v->Fdb;

        // izra�unam novi akumuliran error
        // (pri "ErrSumHistory" na mestu z indeksom i,
        // je zaenkrat �e "ErrSumHistory" iz trenutka i - N
        // ker �e ni prepisan)
        v->ErrSum = v->Krep * v->Err +
                    v->w0 * v->ErrSumHistory[v->i] +
                    v->w1 * v->ErrSumHistory[v->i_plus_one] +
                    v->w1 * v->ErrSumHistory[v->i_minus_one] +
                    v->w2 * v->ErrSumHistory[v->i_plus_two] +
                    v->w2 * v->ErrSumHistory[v->i_minus_two];

        // omejim trenutni error, da ne gre v nasi�enje
        v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
        v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;

        // zapi�em trenutni akumuliran error v buffer na trenutno mesto (v naslednjem ciklu bo to error v prej�njem ciklu)
        v->ErrSumHistory[v->i] = v->ErrSum;

        // izra�unam korekcijo s pomo�jo zgodovine (upo�tevana kompenzacija zakasnitve)
        v->Correction = v->ErrSumHistory[v->index];

        // izra�unam izhod
        v->Out = v->Correction;

        // shranim vrednost indeksa i, ki bo v naslednjem ciklu prej�nji i
        v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije void REP_REG_CALC (REP_REG_float *v)








/****************************************************************************************************
 * Realizacija funkcije kro�nega pomnilnika (angl. circular buffer), s katero "index",
 * ki je lahko ve�ji od BufferSize-1 oz. manj�i od 0 reducira na obmo�je [0,BufferSize-1].
 * OPOMBA: Na omejeno obmo�je [0,BufferSize-1] lahko funkcija transformira le �tevila,
 * ki so absolutno manj�a od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * For zanka (namesto while) je implementirana zato, da omeji najve�je �tevilo iteracij zanke.
****************************************************************************************************/
int circular_buffer_transformation(IndexLinearBuffer,BufferSize)
{
	int IndexCircularBuffer = IndexLinearBuffer;
	static int i;

	// omejim stevilo iteracij na 10
	for(i = 0; i < 10; i++)
	{
		if(IndexCircularBuffer > BufferSize - 1)
		{
			IndexCircularBuffer = IndexCircularBuffer - BufferSize;
		}
		else if(IndexCircularBuffer < 0)
		{
			IndexCircularBuffer = IndexCircularBuffer + BufferSize;
		}
		else
		{
			break;
		}
	}

	return(IndexCircularBuffer);
}
