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
// Define the Delay buffer for the 50th order filter and place it in "firldb" section
#pragma DATA_SECTION(dbuffer, "firldb")
float dbuffer[LENGTH_DCT_REG_BUFFER];
// Define Constant Co-efficient Array  and place the .constant section in ROM memory
#pragma DATA_SECTION(coeff, "coefffilt");
float const coeff[LENGTH_DCT_REG_BUFFER] = FIR_FP_LPF256;




// funkcija
#pragma CODE_SECTION(DCT_REG_CALC, "ramfuncs");
void DCT_REG_CALC (DCT_REG_float *v)
{
    // lokalne spremenljivke








    // program


	// TEST FIR FILTRA





    // omejitev bufferja
    if (v->BufferHistoryLength > LENGTH_DCT_REG_BUFFER)
    {
        v->BufferHistoryLength = LENGTH_DCT_REG_BUFFER;
    }
    else if (v->BufferHistoryLength < 1)
    {
        v->BufferHistoryLength = 1;
    }




    // izraèun trenutnega indeksa bufferja
    v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




	/* circular buffer */
/*
    // èe se indeks spremeni, potem gre algoritem dalje (vsako periodo signala, ne pa vsako vzorèno periodo/interval)
    if ((v->i != v->i_prev) || (v->i == 0 && v->i_prev == 0))
    {

		
        // manipuliranje z indeksi - zaradi circular bufferja
        if (v->i > v->i_prev)
        {
            // indeks, ki kaže v preteklost (potrebujem za ponovno zakasnitev, ki je že kompenzirana z DCT filtrom)
            v->index = v->i - v->k;

			// omejitve zaradi circular bufferja
            if (v->index < 0)
            {
                v->index = v->index + v->BufferHistoryLength;
            }
        } // end of if (v->i > v->i_prev)
        else if (v->i < v->i_prev)
        {
            // indeks, ki kaže v preteklost (potrebujem za ponovno zakasnitev, ki je že kompenzirana z DCT filtrom)
            v->index = v->i + v->k;

            // omejitve zaradi circular bufferja
            if (v->index > (v->BufferHistoryLength - 1))
            {
                v->index = v->index - v->BufferHistoryLength;
            }
        } // end of else if (v->i < v->i_prev)			



		
		// izraèunam trenutni error
        v->Err = v->Ref - v->Fdb;
		
		
		
		
*/
        /* DCT filter - FIR filter */
		
//		v->ErrSum = v->Kdct * v->Err;
/*
		for (v->j = 0; v->j < 200; v->j++)
			{
				v->CircularBufferIndex = v->i - v->j; // predvidevam, da indeks i vedno le narašèa
				if (v->CircularBufferIndex < 0)
				{
					v->CircularBufferIndex = v->CircularBufferIndex + v->BufferHistoryLength;
				}

				v->ErrSum = v->ErrSum + v->FIRCoeff[v->j] * v->ErrSumHistory[v->CircularBufferIndex];
			}
*/

/*
        // omejim trenutni error, da ne gre v nasièenje
        v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
        v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;
		
		
		
				
		// zapišem trenutni akumuliran error v buffer (ponovno uvedem zakasnitev, ki je bila kompenzirana v DCT filtru)
        v->ErrSumHistory[v->index] = v->ErrSum;
		
		
		
		
        // izraèunam korekcijo (upoštevana kompenzacija zakasnitve v DCT filtru)
        v->Correction = v->ErrSum;

        // izraèunam izhod
        v->Out = v->Correction;

        // shranim vrednost indeksa i, ki bo v naslednjem ciklu prejšnji i
        v->i_prev = v->i;




    } // end of if (i != i_prev)



		
    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;
*/



    firFP.input= v->Ref;
    firFP.calc(&firFP);
    v->Out = firFP.output;

} // konec funkcije
