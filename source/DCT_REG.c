/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   29.8.2017
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE		WHO					DETAIL
* 1.0       6.4.2016	Denis Sušin			Initial version
* 1.1		21.8.2017	Denis Sušin			Corrections of comments and names of variables
* 2.0		15.5.2019	Denis Sušin			Circular buffer compacted into function and
* 											circular buffer indexes handling upgraded
* 3.0		9.7.2019	Denis Sušin			FPU FIR filter struct implemented within DCT controller struct,
* 											FIR coefficient buffer and delay buffer must be declared externally,
*											however, the DCT controller struct manipulates with those two buffers.
* 3.1		18.7.2019   Denis Sušin			Minor changes to prevent unwanted changes to "k", right after initialization.
*
* 3.2		19.7.2019   Denis Sušin			Corrections to "i_delta" calculations. Before the function had bugs.
*
* 3.3		19.7.2019   Denis Sušin			Corrections to phase delay compensation "k". One additional sample must be 
*											compensated when calculating "index", because of calculation delay 
*											(one sample exactly). However, this extra sample must not be implemented in 
*											FIR (DCT) filter coefficients calculation.
*
****************************************************************/

#include "DCT_REG.h"

// deklaracija funkcij
int circular_buffer_transformation2(int IndexLinearBuffer, int BufferSize);

// globalne spremenljivke




/****************************************************************************************************
* Funkcija, ki izvede algoritem DCT regulatorja.
* Zelo zaželeno je, da je razmerje med vzorèno frekvenco in osnovno frekvenco reguliranega signala
* enako velikosti pomnilnika "BufferHistoryLength" (in veèje od 20), saj je regulator na to obèutljiv,
* kar lahko privede do nezanemarljivega pogreška v stacionarnem stanju.
****************************************************************************************************/
#pragma CODE_SECTION(DCT_REG_CALC, "ramfuncs");
void DCT_REG_CALC (DCT_REG_float *v)
{
	// lokalne spremenljivke




	// program

	// omejitev dolžine circular bufferja
	if (v->BufferHistoryLength > FIR_FILTER_NUMBER_OF_COEFF)
	{
		v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
	}
	else if (v->BufferHistoryLength < 0)
	{
		v->BufferHistoryLength = 0;
	}

	// toèno doloèena velikost krožnega pomnilnika in št. koeficientov FIR filtra
	v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;

	
	
	
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




	// izraèun vsote bufferja, kjer so doloèeni harmoniki, ki jih reguliramo
	v->SumOfHarmonics = 0;
	for(v->SumOfHarmonicsIndex = 0; v->SumOfHarmonicsIndex < LENGTH_OF_HARMONICS_ARRAY; v->SumOfHarmonicsIndex++)
	{
		v->SumOfHarmonics = v->SumOfHarmonics + v->HarmonicsBuffer[v->SumOfHarmonicsIndex];
	}

	// zaznavanje spremembe parametrov DCT regulatorja s strani uporabnika, ki zahteva vnovièni izraèun koeficientov DCT oz. FIR filtra.
	if( (v->k != v->k_old) || (v->SumOfHarmonics != v->SumOfHarmonicsOld) )
	{
		v->CoeffCalcInProgressFlag = 1;
		v->j = 0;
	}

	// izraèun koeficientov FIR oz. DCT filtra; vsako vzorèno periodo en koeficient
	if(v->CoeffCalcInProgressFlag != 0)
	{
		FIR_FILTER_COEFF_CALC(v);
	}

	
	
	
	// zapis trenutnih vrednosti, ki bodo v naslednji iteraciji že zgodovina
	v->k_old = v->k;
	v->SumOfHarmonicsOld = v->SumOfHarmonics;




    // izraèun trenutnega indeksa bufferja
	v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




	// èe se indeks spremeni, potem gre algoritem dalje:
	//   1. èe je "SamplingSignal" zunaj te funkcije natanèno sinhroniziran z vzorèno frekvenco (|i_delta| = 1)
	//      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval in se
	//      izkoristi celotna velikost pomnilnika, kar je optimalno
	//   2. èe je "SamplingSignal" prepoèasen (|i_delta| < 1) oz. osnovna frekvenca reguliranega signala prenizka,
	//      ni nujno, da se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval,
	//      kar pomeni, da ta algoritem lahko deluje s frekvenco nižjo od vzorène frekvence
	//   3. èe je "SamplingSignal" prehiter (|i_delta| > 1), oz. osnovna frekvenca reguliranega signala previsoka,
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
		// OPOMBA: v->k se mora izjemoma odšteti, saj je nasproten predznak pri kompenzaciji zakasnitve "k" tudi
		//         pri izraèunu koeficientov DCT filtra
		v->index = circular_buffer_transformation2(v->i - (v->k + 1)*v->i_delta,v->BufferHistoryLength);




		/***************************************************/
		/* koda DCT regulatorja */
		/***************************************************/

		// izraèunam trenutni error
		v->Err = v->Ref - v->Fdb;

		// izraèunam novi akumuliran error
		v->ErrSum = v->Kdct * v->Err +						\
					v->CorrectionHistory[v->index];

		// omejim trenutni error, da ne gre v nasièenje
		v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
		v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;




		/* DCT filter - FIR filter */
		v->FIR_filter_float.input = v->ErrSum;
		v->FIR_filter_float.calc(&(v->FIR_filter_float));
		v->Correction = v->FIR_filter_float.output;

		// zapišem trenutno vrednost korekcijskega signala v buffer na trenutno mesto
		v->CorrectionHistory[v->i] = v->Correction;

		// izraèunam izhod
		v->Out = v->Correction;

	    // shranim vrednost indeksa i, ki bo v naslednjem ciklu prejšnji i
	    v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije








/****************************************************************************************************
 * Izraèun koeficientov DCT filtra med delovanjem. V eni vzorèni periodi je izraèunan le en koeficient.
 * OPOMBA: Za izraèun vseh N koeficientov, je potrebnih N vzorènih period.
 * OPOMBA: FIR filter iz FPU knjižnice izvede konvolucijo le delno, ker ne obrne signala, zato ima
 *         kompenzacija zakasnitve ravno nasproten predznak.
****************************************************************************************************/
void FIR_FILTER_COEFF_CALC (DCT_REG_float *v)
{
	int static harmonic_index = 0;


	*(v->FIR_filter_float.coeff_ptr + v->j) = 0.0;
	/* LAG COMPENSATION HAS NEGATIVE SIGN, BECAUSE OF REALIZATION OF DCT FILTER WITH FPU LIBRARY */
	for(harmonic_index = 0; harmonic_index < LENGTH_OF_HARMONICS_ARRAY; harmonic_index++)
	{
		if(v->HarmonicsBuffer[harmonic_index] != 0)
		{
			*(v->FIR_filter_float.coeff_ptr + v->j) = *(v->FIR_filter_float.coeff_ptr + v->j) + 							\
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
    	harmonic_index = 0;
    	v->CoeffCalcInProgressFlag = 0;
    }
}








/****************************************************************************************************
 * Realizacija funkcije krožnega pomnilnika (angl. circular buffer), s katero "index",
 * ki je lahko veèji od BufferSize-1 oz. manjši od 0 reducira na obmoèje [0,BufferSize-1].
 * OPOMBA: Na omejeno obmoèje [0,BufferSize-1] lahko funkcija transformira le števila,
 *         ki so absolutno manjša od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * OPOMBA: For zanka (namesto while) je implementirana zato, da omeji najveèje število iteracij zanke.
****************************************************************************************************/
int circular_buffer_transformation2(IndexLinearBuffer,BufferSize)
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
