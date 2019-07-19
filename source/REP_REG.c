/****************************************************************
* FILENAME:     REP_REG.c
* DESCRIPTION:  Repetitive controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   6.4.2016
* VERSION:      3.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016   Denis Sušin      Initial version
* 1.1		21.8.2017  Denis Sušin		Corrections of comments and names of variables
* 2.0		15.5.2019  Denis Sušin		Circular buffer compacted into function and
* 										circular buffer indexes handling upgraded
* 3.0		19.7.2019  Denis Sušin		Corrections to "i_delta" calculations. Before the function had bugs.
*
****************************************************************/

#include "REP_REG.h"

// deklaracija funkcij
int circular_buffer_transformation(int IndexLinearBuffer, int BufferSize);

// globalne spremenljivke








/****************************************************************************************************
* Funkcija, ki izvede algoritem repetitivnega regulatorja.
* Zelo zaželeno je, da je razmerje med vzorèno frekvenco in osnovno frekvenco reguliranega signala
* enako velikosti pomnilnika "BufferHistoryLength" (in veèje od 20), saj je regulator na to obèutljiv,
* kar lahko privede do nezanemarljivega pogreška v stacionarnem stanju.
****************************************************************************************************/
#pragma CODE_SECTION(REP_REG_CALC, "ramfuncs");
void REP_REG_CALC (REP_REG_float *v)
{
    // lokalne spremenljivke




    // program

    // omejitev dolžine circular bufferja
    if (v->BufferHistoryLength > MAX_LENGTH_REP_REG_BUFFER)
    {
        v->BufferHistoryLength = MAX_LENGTH_REP_REG_BUFFER;
    }
    else if (v->BufferHistoryLength < MIN_LENGTH_REP_REG_BUFFER)
    {
        v->BufferHistoryLength = MIN_LENGTH_REP_REG_BUFFER;
    }
	
	// omejitev kompenzacije zakasnitve, ki ne sme presegati dolžine bufferja
    if (v->k > v->BufferHistoryLength)
    {
        v->k = v->BufferHistoryLength;
    }
    else if (v->k < -v->BufferHistoryLength)
    {
        v->k = -v->BufferHistoryLength;
    }

    // omejitev vzorènega signala med 0.0 in 0.9999
    //(SamplingSignal ne sme biti enak ena, ker mora biti indeks i omejen od 0 do BufferHistoryLength-1)
    v->SamplingSignal = (v->SamplingSignal > 0.99999)? 0.99999: v->SamplingSignal;
    v->SamplingSignal = (v->SamplingSignal < 0.0)? 0.0: v->SamplingSignal;

	
	

    // izraèun trenutnega indeksa bufferja
    v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




    // èe se indeks spremeni, potem gre algoritem dalje:
    //   1. èe je "SamplingSignal" zunaj te funkcije natanèno sinhroniziran z vzorèno frekvenco ("i_delta" = 1)
    //      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval in se
    //      izkoristi celotna velikost pomnilnika, kar je optimalno
    //   2. èe je "SamplingSignal" prepoèasen ("i_delta" < 1) oz. osnovna frekvenca reguliranega signala prenizka,
    //      ni nujno, da se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval,
    //      kar pomeni, da ta algoritem lahko deluje s frekvenco nižjo od vzorène frekvence
    //   3. èe je "SamplingSignal" prehiter ("i_delta" > 1), oz. osnovna frekvenca reguliranega signala previsoka,
    //      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval,
    //      a se velikost pomnilnika umetno zmanjša za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
    //      kar pomeni, kot da bi bila velikost pomnilnika manjša za faktor "i_delta"
    //      OPOMBA: To zadnjo funkcionalnost je možno izklopiti z odkomentiranjem vrstice "v->i_delta = 1;"!
    if ((v->i != v->i_prev))
    {
		// izraèun razlike med trenutnim indeksom "i" in prejšnjim indeksom "i_prev"
		// (èe je "SamplingSignal" prehiter, lahko velikost pomnilnika
		// umetno zmanjšamo za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
		// ki pa ne sme presegati polovico velikosti pomnilnika)
		v->i_delta = v->i - v->i_prev;

		// manipuliranje z indeksi - zaradi circular bufferja; èe indeks narašèa - inkrementiranje
		if ( (v->i < v->i_prev) && (v->i_delta < -(v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = v->BufferHistoryLength + v->i_delta;
		}
		// manipuliranje z indeksi - zaradi circular bufferja; èe indeks pada - dekrementiranje
		else if ( (v->i > v->i_prev) && (v->i_delta > (v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = -(v->BufferHistoryLength - v->i_delta);
		}


		// èe funkcionalnost umetnega zmanjševanja velikosti pomnilnika ni zaželena ali ni potrebna (opis pod toèko 3.),
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
		
        // izraèunam trenutni error
        v->Err = v->Ref - v->Fdb;

        // izraèunam novi akumuliran error
        // (pri "ErrSumHistory" na mestu z indeksom i,
        // je zaenkrat še "ErrSumHistory" iz trenutka i - N
        // ker še ni prepisan)
        v->ErrSum = v->Krep * v->Err +
                    v->w0 * v->ErrSumHistory[v->i] +
                    v->w1 * v->ErrSumHistory[v->i_plus_one] +
                    v->w1 * v->ErrSumHistory[v->i_minus_one] +
                    v->w2 * v->ErrSumHistory[v->i_plus_two] +
                    v->w2 * v->ErrSumHistory[v->i_minus_two];

        // omejim trenutni error, da ne gre v nasièenje
        v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
        v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;

        // zapišem trenutni akumuliran error v buffer na trenutno mesto (v naslednjem ciklu bo to error v prejšnjem ciklu)
        v->ErrSumHistory[v->i] = v->ErrSum;

        // izraèunam korekcijo s pomoèjo zgodovine (upoštevana kompenzacija zakasnitve)
        v->Correction = v->ErrSumHistory[v->index];

        // izraèunam izhod
        v->Out = v->Correction;

        // shranim vrednost indeksa i, ki bo v naslednjem ciklu prejšnji i
        v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije void REP_REG_CALC (REP_REG_float *v)








/****************************************************************************************************
 * Realizacija funkcije krožnega pomnilnika (angl. circular buffer), s katero "index",
 * ki je lahko veèji od BufferSize-1 oz. manjši od 0 reducira na obmoèje [0,BufferSize-1].
 * OPOMBA: Na omejeno obmoèje [0,BufferSize-1] lahko funkcija transformira le števila,
 * ki so absolutno manjša od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * For zanka (namesto while) je implementirana zato, da omeji najveèje število iteracij zanke.
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
