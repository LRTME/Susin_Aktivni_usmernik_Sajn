/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Su�in
* START DATE:   29.8.2017
* VERSION:      3.4
*
* CHANGES :
* VERSION   DATE		WHO					DETAIL
* 1.0       6.4.2016	Denis Su�in			Initial version
* 1.1		21.8.2017	Denis Su�in			Corrections of comments and names of variables
* 2.0		15.5.2019	Denis Su�in			Circular buffer compacted into function and
* 											circular buffer indexes handling upgraded
* 3.0		9.7.2019	Denis Su�in			FPU FIR filter struct implemented within DCT controller struct,
* 											FIR coefficient buffer and delay buffer must be declared externally,
*											however, the DCT controller struct manipulates with those two buffers.
* 3.1		18.7.2019   Denis Su�in			Minor changes to prevent unwanted changes to "k", right after initialization.
*
* 3.2		19.7.2019   Denis Su�in			Corrections to "i_delta" calculations. Before the function had bugs.
*
* 3.3		19.7.2019   Denis Su�in			Corrections to phase delay compensation "k". One additional sample must be 
*											compensated when calculating "index", because of calculation delay 
*											(one sample exactly). However, this extra sample must not be implemented in 
*											FIR (DCT) filter coefficients calculation.
* 3.4		29.8.2019   Denis Su�in			Added pragma in front of two internal functions, cleared unnecessary init value
*											and static attribute.
*
****************************************************************/

#include "DCT_REG.h"

// deklaracija funkcij
int circular_buffer_transformation2(int IndexLinearBuffer, int BufferSize);

// globalne spremenljivke




/****************************************************************************************************
* Funkcija, ki izvede algoritem DCT regulatorja.
* Zelo za�eleno je, da je razmerje med vzor�no frekvenco in osnovno frekvenco reguliranega signala
* enako velikosti pomnilnika "BufferHistoryLength" (in ve�je od 20), saj je regulator na to ob�utljiv,
* kar lahko privede do nezanemarljivega pogre�ka v stacionarnem stanju.
****************************************************************************************************/
#pragma CODE_SECTION(DCT_REG_CALC, "ramfuncs");
void DCT_REG_CALC (DCT_REG_float *v)
{
	// lokalne spremenljivke




	// program

	// omejitev dol�ine circular bufferja
	if (v->BufferHistoryLength > FIR_FILTER_NUMBER_OF_COEFF)
	{
		v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
	}
	else if (v->BufferHistoryLength < 0)
	{
		v->BufferHistoryLength = 0;
	}

	// to�no dolo�ena velikost kro�nega pomnilnika in �t. koeficientov FIR filtra
	v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;

	
	
	
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




	// izra�un vsote bufferja, kjer so dolo�eni harmoniki, ki jih reguliramo
	v->SumOfHarmonics = 0;
	for(v->SumOfHarmonicsIndex = 0; v->SumOfHarmonicsIndex < LENGTH_OF_HARMONICS_ARRAY; v->SumOfHarmonicsIndex++)
	{
		v->SumOfHarmonics = v->SumOfHarmonics + v->HarmonicsBuffer[v->SumOfHarmonicsIndex];
	}

	// zaznavanje spremembe parametrov DCT regulatorja s strani uporabnika, ki zahteva vnovi�ni izra�un koeficientov DCT oz. FIR filtra.
	if( (v->k != v->k_old) || (v->SumOfHarmonics != v->SumOfHarmonicsOld) )
	{
		v->CoeffCalcInProgressFlag = 1;
		v->j = 0;
	}

	// izra�un koeficientov FIR oz. DCT filtra; vsako vzor�no periodo en koeficient
	if(v->CoeffCalcInProgressFlag != 0)
	{
		FIR_FILTER_COEFF_CALC(v);
	}

	
	
	
	// zapis trenutnih vrednosti, ki bodo v naslednji iteraciji �e zgodovina
	v->k_old = v->k;
	v->SumOfHarmonicsOld = v->SumOfHarmonics;




    // izra�un trenutnega indeksa bufferja
	v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




	// �e se indeks spremeni, potem gre algoritem dalje:
	//   1. �e je "SamplingSignal" zunaj te funkcije natan�no sinhroniziran z vzor�no frekvenco (|i_delta| = 1)
	//      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval in se
	//      izkoristi celotna velikost pomnilnika, kar je optimalno
	//   2. �e je "SamplingSignal" prepo�asen (|i_delta| < 1) oz. osnovna frekvenca reguliranega signala prenizka,
	//      ni nujno, da se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval,
	//      kar pomeni, da ta algoritem lahko deluje s frekvenco ni�jo od vzor�ne frekvence
	//   3. �e je "SamplingSignal" prehiter (|i_delta| > 1), oz. osnovna frekvenca reguliranega signala previsoka,
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
		// OPOMBA: v->k se mora izjemoma od�teti, saj je nasproten predznak pri kompenzaciji zakasnitve "k" tudi
		//         pri izra�unu koeficientov DCT filtra
		v->index = circular_buffer_transformation2(v->i - (v->k + 1)*v->i_delta,v->BufferHistoryLength);




		/***************************************************/
		/* koda DCT regulatorja */
		/***************************************************/

		// izra�unam trenutni error
		v->Err = v->Ref - v->Fdb;

		// izra�unam novi akumuliran error
		v->ErrSum = v->Kdct * v->Err +						\
					v->CorrectionHistory[v->index];

		// omejim trenutni error, da ne gre v nasi�enje
		v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
		v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;




		/* DCT filter - FIR filter */
		v->FIR_filter_float.input = v->ErrSum;
		v->FIR_filter_float.calc(&(v->FIR_filter_float));
		v->Correction = v->FIR_filter_float.output;

		// zapi�em trenutno vrednost korekcijskega signala v buffer na trenutno mesto
		v->CorrectionHistory[v->i] = v->Correction;

		// izra�unam izhod
		v->Out = v->Correction;

	    // shranim vrednost indeksa i, ki bo v naslednjem ciklu prej�nji i
	    v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije








/****************************************************************************************************
 * Izra�un koeficientov DCT filtra med delovanjem. V eni vzor�ni periodi je izra�unan le en koeficient.
 * OPOMBA: Za izra�un vseh N koeficientov, je potrebnih N vzor�nih period.
 * OPOMBA: FIR filter iz FPU knji�nice izvede konvolucijo le delno, ker ne obrne signala, zato ima
 *         kompenzacija zakasnitve ravno nasproten predznak.
****************************************************************************************************/
#pragma CODE_SECTION(FIR_FILTER_COEFF_CALC, "ramfuncs");
void FIR_FILTER_COEFF_CALC (DCT_REG_float *v)
{
	int harmonic_index;


	*(v->FIR_filter_float.coeff_ptr + v->j) = 0.0;
	/* LAG COMPENSATION HAS NEGATIVE SIGN, BECAUSE OF REALIZATION OF DCT FILTER WITH FPU LIBRARY */
	for(harmonic_index = 0; harmonic_index < LENGTH_OF_HARMONICS_ARRAY; harmonic_index++)
	{
		if(v->HarmonicsBuffer[harmonic_index] != 0)
		{
			*(v->FIR_filter_float.coeff_ptr + v->j) = *(v->FIR_filter_float.coeff_ptr + v->j) + 	\
					      2.0/(FIR_FILTER_NUMBER_OF_COEFF) *  										\
					      cos( 2 * PI * v->HarmonicsBuffer[harmonic_index] * 						\
						  ( (float)(v->j - v->k) ) / (FIR_FILTER_NUMBER_OF_COEFF) );				\
		}

	}
	/* FIR FILTER FROM FPU LIBRARY DOESN'T FLIP SIGNAL FROM LEFT TO RIGHT WHILE PERFORMING CONVOLUTION */

	v->j = v->j + 1;

    if(v->j >= FIR_FILTER_NUMBER_OF_COEFF)
    {
    	v->j = 0;
    	v->CoeffCalcInProgressFlag = 0;
    }
}








/****************************************************************************************************
 * Realizacija funkcije kro�nega pomnilnika (angl. circular buffer), s katero "index",
 * ki je lahko ve�ji od BufferSize-1 oz. manj�i od 0 reducira na obmo�je [0,BufferSize-1].
 * OPOMBA: Na omejeno obmo�je [0,BufferSize-1] lahko funkcija transformira le �tevila,
 *         ki so absolutno manj�a od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * OPOMBA: For zanka (namesto while) je implementirana zato, da omeji najve�je �tevilo iteracij zanke.
****************************************************************************************************/
#pragma CODE_SECTION(circular_buffer_transformation2, "ramfuncs");
int circular_buffer_transformation2(IndexLinearBuffer,BufferSize)
{
	int IndexCircularBuffer = IndexLinearBuffer;
	int i;

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
