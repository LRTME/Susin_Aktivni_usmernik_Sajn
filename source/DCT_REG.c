/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
* START DATE:   29.8.2017
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
*
****************************************************************/

#include "DCT_REG.h"

// globalne spremenljivke

// spremenljivke za FIR filter iz FPU modula

// Create an Instance of FIRFILT_GEN module and place the object in "firfilt" section
#pragma DATA_SECTION(firFP, "firfilt")
FIR_FP  firFP = FIR_FP_DEFAULTS;
// Define the Delay buffer for the 50th order filterfilter and place it in "firldb" section
#pragma DATA_SECTION(dbuffer, "firldb")
// Align the delay buffer for max 1024 words (512 float variables)
#pragma DATA_ALIGN (dbuffer,0x400)
float dbuffer[FIR_FILTER_NUMBER_OF_COEFF];
// Define Constant Co-efficient Array  and place the .constant section in ROM memory
#pragma DATA_SECTION(coeff, "coefffilt");
// Align the coefficent buffer for max 1024 words (512 float coeff)
#pragma DATA_ALIGN (coeff,0x400)
float coeff[FIR_FILTER_NUMBER_OF_COEFF];


// funkcija
#pragma CODE_SECTION(DCT_REG_CALC, "ramfuncs");
void DCT_REG_CALC (DCT_REG_float *v)
{
    // lokalne spremenljivke
	static int first_start = 0;




    // program

	/* no need for limiting k and BufferHistoryLength, because they are defined at initialisation and must stay constant

    // omejitev dolžine circular bufferja
    if (v->BufferHistoryLength > FIR_FILTER_NUMBER_OF_COEFF)
    {
        v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
    }
    else if (v->BufferHistoryLength < 1)
    {
        v->BufferHistoryLength = 1;
    }

    // omejitev kompenzacije zakasnitve, ki ne sme presegati dolžine bufferja
    if (v->k > FIR_FILTER_NUMBER_OF_COEFF)
    {
        v->k = FIR_FILTER_NUMBER_OF_COEFF;
    }
    else if (v->k < 0)
    {
        v->k = 0;
    }

	*/

	v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
    v->k = LAG_COMPENSATION;

    // omejitev vzorènega signala med 0.0 in 0.9999 (SamplingSignal ne sme biti enak ena, ker mora biti indeks i omejen od 0 do BufferHistoryLength-1)
    v->SamplingSignal = (v->SamplingSignal > 0.99999)? 0.99999: v->SamplingSignal;
    v->SamplingSignal = (v->SamplingSignal < 0.0)? 0.0: v->SamplingSignal;




    // izraèun trenutnega indeksa bufferja
    v->i = (int)(v->SamplingSignal * v->BufferHistoryLength);




    // èe se indeks spremeni, potem gre algoritem dalje (vsako periodo signala, ne pa vsako vzorèno periodo/interval)
    if ((v->i != v->i_prev) || (first_start == 0))
    {
    	if(v->i != v->i_prev)
    	{
			// ko je program prviè na tem mestu, dvignemo zastavico
			first_start = 1;
    	}

    	/***************************************************/
		/* circular buffer */
    	/***************************************************/

		// manipuliranje z indeksi - zaradi circular bufferja
    	if ( (v->i > v->i_prev) || (v->i - v->i_prev == -(v->BufferHistoryLength - 1)) || (first_start == 0) )
		{
			// indeks, ki kaže v preteklost (potrebujem za ponovno zakasnitev, ki je že kompenzirana z DCT filtrom)
			v->index = v->i - 1 - v->k;

			// omejitve zaradi circular bufferja
			if (v->index < 0)
			{
				v->index = v->index + v->BufferHistoryLength;
			}
		} // end of if (v->i > v->i_prev)
        else if ( (v->i < v->i_prev) || (v->i - v->i_prev == (v->BufferHistoryLength - 1)) )
		{
			// indeks, ki kaže v preteklost (potrebujem za ponovno zakasnitev, ki je že kompenzirana z DCT filtrom)
			v->index = v->i + 1 + v->k;

			// omejitve zaradi circular bufferja
			if (v->index > (v->BufferHistoryLength - 1))
			{
				v->index = v->index - v->BufferHistoryLength;
			}
		} // end of else if (v->i < v->i_prev)




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
		firFP.input = v->ErrSum;
		firFP.calc(&firFP);
		v->Correction = firFP.output;

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
