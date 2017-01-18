/****************************************************************
* FILENAME:     PER_int.c
* DESCRIPTION:  periodic interrupt code
* AUTHOR:       Mitja Nemec
* DATE:         16.1.2009
*
****************************************************************/
#include    "PER_int.h"
#include    "TIC_toc.h"

// definicije za uporabniski vmesnik - potenciometri
// širina mrtve cone
#define     POT_DEAD_BAND_WIDTH     0.05
//  širina nasièenja
#define     POT_SATURATION_WIDTH    0.02

// tokovi
float   tok_grid = 0.0;
float   tok_bb1 = 0.0;
float   tok_bb2 = 0.0;
float   tok_out = 0.0;

float   tok_grid_offset = 2048;
float   tok_bb1_offset = 2048;
float   tok_bb2_offset = 2048;
float   tok_out_offset = 2048;

float   tok_grid_gain = -(15.0 / 0.625 ) * (7.5 / 6.2) * (3.3 / 4096);
float   tok_bb1_gain = -(15.0 / 0.625 ) * (7.5 / 6.2) * (3.3 / 4096);
float   tok_bb2_gain = -(15.0 / 0.625 ) * (7.5 / 6.2) * (3.3 / 4096);
float   tok_out_gain = -(15.0 / 0.625 ) * (7.5 / 6.2) * (3.3 / 4096);

long    current_offset_counter = 0;

// napetosti
float   nap_grid = 0.0;
float   nap_dc = 0.0;
float   nap_cap = 0.0;
float   nap_out = 0.0;

float   nap_grid_offset = 2048.0;
float   nap_dc_offset = 0.0;
float   nap_cap_offset = 0.0;
float   nap_out_offset = 0.0;

float   nap_grid_gain = ((1.8 + 36.0) / 1.8) * (-1.0) * (3.3 / 4096);
float   nap_dc_gain = ((3.6 + 62.0) / 3.6) * (3.3 / 4096);
float   nap_cap_gain = ((3.6 + 62.0) / 3.6) * (3.3 / 4096);
float   nap_out_gain = ((3.6 + 62.0) / 3.6) * (3.3 / 4096);

// filtrirana napetost DC linka
DC_float    napetost_dc_f = DC_FLOAT_DEFAULTS;
float   napetost_dc_filtered = 0.0;

// prvi harmonik in RMS omrežne napetosti
DFT_float   nap_grid_dft = DFT_FLOAT_DEFAULTS;
float   nap_grid_rms = 0.0;
float   nap_grid_form = 0.0;

// regulacija napetosti enosmernega tokokroga
PID_float   nap_dc_reg = PID_FLOAT_DEFAULTS;
SLEW_float  nap_dc_slew = SLEW_FLOAT_DEFAULTS;

// regulacija omreznega toka
PID_float   tok_grid_reg = PID_FLOAT_DEFAULTS;

// regulacija izhodne napetosti
PID_float   nap_out_reg = PID_FLOAT_DEFAULTS;
SLEW_float  nap_out_slew = SLEW_FLOAT_DEFAULTS;

// regulacija bb tokov
PID_float   tok_bb1_reg = PID_FLOAT_DEFAULTS;
PID_float   tok_bb2_reg = PID_FLOAT_DEFAULTS;
SLEW_float  tok_bb_slew = SLEW_FLOAT_DEFAULTS;

// sinhronizacija na omrežje
float       sync_base_freq = SWITCH_FREQ;
PID_float   sync_reg    = PID_FLOAT_DEFAULTS;
float       sync_switch_freq = SWITCH_FREQ;
float       sync_grid_freq = ((SWITCH_FREQ/SAMPLING_RATIO)/SAMPLE_POINTS);
bool        sync_use = TRUE;

// samo za statistiko meritev
STAT_float  statistika = STAT_FLOAT_DEFAULTS;

// za oceno bremenskega toka
ABF_float   i_cap_abf = ABF_FLOAT_DEFAULTS;
float       tok_out_abf = 0.0;

// za oceno DC-link toka
ABF_float   i_cap_dc = ABF_FLOAT_DEFAULTS;
float       tok_dc_abf = 0.0;

// za zakasnitev omreznega toka
DELAY_float i_grid_delay = DELAY_FLOAT_DEFAULTS;

// filtriranje izhoda ocene
DC_float 	i_dc_f = DC_FLOAT_DEFAULTS;

// filtriranje meritve
DC_float	tok_out_f = DC_FLOAT_DEFAULTS;

// izbira ocene izhodnega toka
volatile enum   {Meas_out = 0, ABF_out, KF_out, None_out } tok_out_source = Meas_out;

// izbira ocene dc toka
volatile enum   {Meas_dc = 0, ABF_dc, KF_dc, None_dc } tok_dc_source = None_dc;

// izhodna moc
float   power_out = 0.0;

// temperatura hladilnika
float   temperatura = 0.0;

// potenciometri
float   pot_i_coarse = 0.0;
float   pot_i_fine = 0.0;
float   pot_u_coarse = 0.0;
float   pot_u_fine = 0.0;

float	pot_u_coarse_old = 0.0;
float	pot_u_fine_old = 0.0;
float   pot_i_coarse_old = 0.0;
float   pot_i_fine_old = 0.0;

// za oceno obremenjenosti CPU-ja
float   cpu_load  = 0.0;
long    interrupt_cycles = 0;

long 	sd_card_cnt = 0;

// temperatura procesorja
float	cpu_temp = 0.0;

// za vklop bremena
float	ref_gen_load = 0.1;

// kdo generira željene vrednosti
volatile enum    {Pots = 0, Ref_generator} ref_select = Pots;

// spremenljikva s katero štejemo kolikokrat se je prekinitev predolgo izvajala
int     interrupt_overflow_counter = 0;

void check_limits(void);
void get_electrical(void);
void sync(void);
void input_bridge_control(void);
void output_bb_control(void);

/**************************************************************
 * Prekinitev, ki v kateri se izvaja regulacija
 **************************************************************/
#pragma CODE_SECTION(PER_int, "ramfuncs");
void interrupt PER_int(void)
{
    /* lokalne spremenljivke */
    long interrupt_cycles;

    // najprej povem da sem se odzzval na prekinitev
    // Spustimo INT zastavico casovnika ePWM1
    EPwm1Regs.ETCLR.bit.INT = 1;
    // Spustimo INT zastavico v PIE enoti
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

    // pozenem stoprico
    interrupt_cycles = TIC_time;
    TIC_start();

    // izracunam obremenjenost procesorja
    cpu_load = (float)interrupt_cycles / (CPU_FREQ/SWITCH_FREQ);

    //brcnemo psa
    PCB_kick_dog_int();

    // stevec prekinitev, ki se resetira vsako sekundo
    interrupt_cnt = interrupt_cnt + 1;
    if (interrupt_cnt > SWITCH_FREQ)
    {
        interrupt_cnt = 0;
    }

    // vsakih 10ms poklicem SD_card timer handler
    sd_card_cnt = sd_card_cnt + 1;
    if (sd_card_cnt >= SAMP_FREQ/100)
    {
        SD_tick_timer();;
    	sd_card_cnt = 0;
    }

    // generiram želeno vrednost
    REF_gen();

    // vkljapljam in izklapljam breme
    if (ref_gen.kot < ref_gen_load)
    {
        PCB_load_relay_on();
    }
    else
    {
        PCB_load_relay_off();
    }

    // vzorèim in poraèunam vse izpeljane velièine
    get_electrical();       // 32% cputime

    // sinhroniziram na omrežje
    sync();

    // preverim ali sem znotraj meja
    check_limits();         // 4% cputime

    // regulacija napetosti enosmernega tokokroga
    input_bridge_control(); // 25% cputime

    // regulacija izhodne napetosti
    output_bb_control();    // 12% cputime

    // osvežim data loger
    // Initialization = 0, Startup, Standby, Ramp_up, Work, Ramp_down, Fault, Fault_sensed
    if (   (state == Initialization)
        || (state == Startup)
        || (state == Standby)
        || (state == Ramp_up)
        || (state == Work)
        || (state == Ramp_down))
    {
        DLOG_GEN_update();  // 5% cputime
    }

    // ustavim štoparico
    TIC_stop();

    /* preverim, èe me sluèajno èaka nova prekinitev.
       èe je temu tako, potem je nekaj hudo narobe
       saj je èas izvajanja prekinitve predolg
       vse skupaj se mora zgoditi najmanj 10krat,
       da reèemo da je to res problem
    */
    if (EPwm1Regs.ETFLG.bit.INT == TRUE)
    {
        // povecam stevec, ki steje take dogodke
        interrupt_overflow_counter = interrupt_overflow_counter + 1;
        
        // in ce se je vse skupaj zgodilo 10 krat se ustavim
        // v kolikor uC krmili kakšen resen HW, potem moèno
        // proporoèam lepše "hendlanje" takega dogodka
        // beri:ugasni moènostno stopnjo, ...
        if (interrupt_overflow_counter >= 10)
        {
            fault_flags.cpu_overrun = TRUE;
        }
    }

}   // end of PWM_int

#pragma CODE_SECTION(sync, "ramfuncs");
void sync(void)
{
    sync_reg.Ref = 0;
    sync_reg.Fdb = nap_grid_dft.SumA/nap_grid_rms;
    PID_FLOAT_CALC(sync_reg);

    sync_switch_freq = sync_base_freq + sync_reg.Out;

    sync_grid_freq = ((sync_switch_freq/SAMPLING_RATIO)/SAMPLE_POINTS);

    if (sync_use == TRUE)
    {
        BB_frequency(sync_switch_freq);
        FB_frequency(sync_switch_freq);
    }
    else
    {
        BB_frequency(sync_base_freq);
        FB_frequency(sync_base_freq);
    }
}

#pragma CODE_SECTION(input_bridge_control, "ramfuncs");
void input_bridge_control(void)
{
    // regulacija deluje samo v teh primerih
    if (   (state == Standby)
        || (state == Ramp_up)
        || (state == Work)
        || (state == Ramp_down))
    {
        // najprej zapeljem zeleno vrednost po rampi
        SLEW_FLOAT_CALC(nap_dc_slew)

        // izvedem napetostni regulator
      	nap_dc_reg.Ref = nap_dc_slew.Out;
        nap_dc_reg.Fdb = napetost_dc_filtered;
        //nap_dc_reg.Ff = SQRT2 * power_out / nap_grid_rms;
        if (tok_out_source == Meas_dc)
        {
            nap_dc_reg.Ff = tok_out_f.Mean * napetost_dc_filtered * SQRT2 / nap_grid_rms;
        }
        if (tok_dc_source == ABF_dc)

        {
            nap_dc_reg.Ff = tok_dc_abf * napetost_dc_filtered * SQRT2 / nap_grid_rms;
        }
        if (tok_dc_source == None_dc)
        {
            nap_out_reg.Ff = 0.0;
        }
        PID_FLOAT_CALC(nap_dc_reg);

        // izvedem tokovni regulator
        tok_grid_reg.Ref = -nap_dc_reg.Out * nap_grid_form;
        tok_grid_reg.Fdb = tok_grid;
        tok_grid_reg.Ff = nap_grid/nap_dc; // nap_grid_dft.Out;
        PID_FLOAT_CALC(tok_grid_reg);

        // posljem vse skupaj na mostic
        FB_update(tok_grid_reg.Out);
    }
    // sicer pa nicim integralna stanja
    else
    {
        nap_dc_reg.Ui = 0.0;
        tok_grid_reg.Ui = 0.0;
        FB_update(0.0);
    }
}

#pragma CODE_SECTION(output_bb_control, "ramfuncs");
void output_bb_control(void)
{
	if (mode == Control)
	{
		// regulacija deluje samo v teh primerih
		if (state == Ramp_up)
		{
			// zeljeno vrednost napetosti doloèa generator rampe
			nap_out_slew.Slope_up = 10.0 * SAMPLE_TIME;  // 10 V/s
			nap_out_slew.Slope_down = nap_out_slew.Slope_up;
			if (ref_select == Pots)
			{
				nap_out_slew.In = nap_out;
			}
			else
			{
				nap_out_slew.In = ref_gen.out;
			}
			SLEW_FLOAT_CALC(nap_out_slew);

			// ponastavim meje
			nap_out_reg.OutMax = pot_i_coarse * I_MAX + pot_i_fine * I_MAX * 0.05;
			nap_out_reg.OutMin = -nap_out_reg.OutMax;

			// napetostni regulator
			nap_out_reg.Ref = nap_out_slew.Out;
			nap_out_reg.Fdb = nap_cap;
			if (tok_out_source == Meas_out)
			{
				nap_out_reg.Ff = tok_out;
			}
			if (tok_out_source == ABF_out)
			{
				nap_out_reg.Ff = tok_out_abf;
			}
			if (tok_out_source == KF_out)
			{
				nap_out_reg.Ff = 0.0;
			}
			if (tok_out_source == None_out)
			{
				nap_out_reg.Ff = 0.0;
			}
			PID_FLOAT_CALC(nap_out_reg)

			// sedaj pa se tokovna regulatorja/ ali pa samo eden
			#if BB_LEGS == 2
			tok_bb1_reg.Ref = nap_out_reg.Out * 0.5;
			tok_bb1_reg.Fdb = tok_bb1;
			tok_bb1_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb1_reg);

			tok_bb2_reg.Ref = nap_out_reg.Out * 0.5;
			tok_bb2_reg.Fdb = tok_bb2;
			tok_bb2_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb2_reg);

			BB_update(tok_bb1_reg.Out, tok_bb2_reg.Out);
			#endif
			#if BB_LEGS == 1
			tok_bb1_reg.Ref = nap_out_reg.Out;
			tok_bb1_reg.Fdb = tok_bb1;
			tok_bb1_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb1_reg);

			BB_update(tok_bb1_reg.Out, 0.0);
			#endif
		}
		else if (state == Work)
		{
			// zeljeno vrednost napetosti doloèa potenciometer
			nap_out_slew.Slope_up = 1000.0 * SAMPLE_TIME;  // 1000 V/s
			nap_out_slew.Slope_down = nap_out_slew.Slope_up;
			if (ref_select == Pots)
			{
				nap_out_slew.In = pot_u_coarse * U_MAX + pot_u_fine * U_MAX * 0.05;
			}
			else
			{
				nap_out_slew.In = ref_gen.out;
			}
			SLEW_FLOAT_CALC(nap_out_slew);

			// ponastavim meje
			nap_out_reg.OutMax = pot_i_coarse * I_MAX + pot_i_fine * I_MAX * 0.05;
			nap_out_reg.OutMin = -nap_out_reg.OutMax;

			/* omejitev moci */
			if (   (power_out > +P_MAX)
				|| (power_out < -P_MAX))
			{
				nap_out_reg.OutMax = P_MAX/nap_out;
				nap_out_reg.OutMin = -nap_out_reg.OutMax;
			}

			// napetostni regulator
			nap_out_reg.Ref = nap_out_slew.Out;
			nap_out_reg.Fdb = nap_cap;
			if (tok_out_source == Meas_out)
			{
				nap_out_reg.Ff = tok_out;
			}
			if (tok_out_source == ABF_out)
			{
				nap_out_reg.Ff = tok_out_abf;
			}
			if (tok_out_source == KF_out)
			{
				nap_out_reg.Ff = 0.0;
			}
			if (tok_out_source == None_out)
			{
				nap_out_reg.Ff = 0.0;
			}
			PID_FLOAT_CALC(nap_out_reg)

			// sedaj pa se tokovna regulatorja
			#if BB_LEGS == 2
			tok_bb1_reg.Ref = nap_out_reg.Out * 0.5;
			tok_bb1_reg.Fdb = tok_bb1;
			tok_bb1_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb1_reg);

			tok_bb2_reg.Ref = nap_out_reg.Out * 0.5;
			tok_bb2_reg.Fdb = tok_bb2;
			tok_bb2_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb2_reg);

			BB_update(tok_bb1_reg.Out, tok_bb2_reg.Out);
			#endif
			#if BB_LEGS == 1
			tok_bb1_reg.Ref = nap_out_reg.Out;
			tok_bb1_reg.Fdb = tok_bb1;
			tok_bb1_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb1_reg);

			BB_update(tok_bb1_reg.Out, 0.0);
			#endif
		}
		else if (state == Ramp_down)
		{
			// napetostni regulator je izklopljen
			// tokovno referenco doloèa rampa
			SLEW_FLOAT_CALC(tok_bb_slew);

			// sedaj pa se tokovna regulatorja
			#if BB_LEGS == 2
			tok_bb1_reg.Ref = tok_bb_slew.Out * 0.5;
			tok_bb1_reg.Fdb = tok_bb1;
			tok_bb1_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb1_reg);

			tok_bb2_reg.Ref = tok_bb_slew.Out * 0.5;
			tok_bb2_reg.Fdb = tok_bb2;
			tok_bb2_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb2_reg);

			BB_update(tok_bb1_reg.Out, tok_bb2_reg.Out);
			#endif
			#if BB_LEGS == 1
			tok_bb1_reg.Ref = tok_bb_slew.Out;
			tok_bb1_reg.Fdb = tok_bb1;
			tok_bb1_reg.Ff = nap_out/nap_dc;
			PID_FLOAT_CALC(tok_bb1_reg);

			BB_update(tok_bb1_reg.Out, 0.0);

			#endif

		}
		// sicer pa nicim integralna stanja
		else
		{

		}
	}
	else // mode = Open_loop
	{
		nap_out_slew.Slope_up = 10.0 * SAMPLE_TIME;  // 10 V/s
		nap_out_slew.Slope_down = nap_out_slew.Slope_up;
		nap_out_slew.In = pot_u_coarse + pot_u_fine * 0.05;
		SLEW_FLOAT_CALC(nap_out_slew);
		BB_update(nap_out_slew.Out, nap_out_slew.Out);
		tok_bb_slew.Out = 0;
	}
}

/**************************************************************
 * Funckija, ki pripravi vse potrebno za izvajanje
 * prekinitvene rutine
 **************************************************************/
void PER_int_setup(void)
{
    // inicializiram data logger
    dlog.mode = Single;
    dlog.auto_time = 1;
    dlog.holdoff_time = 1;

    dlog.prescalar = 10;

    dlog.slope = Positive;
    dlog.trig = &ref_gen.kot;
    dlog.trig_value = 0.98;

    dlog.iptr1 = &nap_grid;
    dlog.iptr2 = &tok_grid;
    dlog.iptr3 = &nap_dc;
    dlog.iptr4 = &i_cap_dc.i_cap_estimated;
    dlog.iptr5 = &tok_dc_abf;
    dlog.iptr6 = &nap_out;
    dlog.iptr7 = &tok_out;
    dlog.iptr8 = &tok_out_abf;

    // inicializitam generator referenènega signala
    ref_gen.amp = 2;
    ref_gen.offset = U_DC_REF;
    ref_gen.type = Step;
    ref_gen.duty = 0.1;
    ref_gen.frequency = 0.2;
    ref_gen.sampling_period = SAMPLE_TIME;

    // inicializiram DC filter
    DC_FLOAT_MACRO_INIT(napetost_dc_f);

    // inicilaliziram DFT
    DFT_FLOAT_MACRO_INIT(nap_grid_dft);

    // inicializiram nap_dc_slew
    nap_dc_slew.In = U_DC_REF;
    nap_dc_slew.Slope_up = 10.0 * SAMPLE_TIME;  // 10 V/s
    nap_dc_slew.Slope_down = nap_dc_slew.Slope_up;

    // inicializiram regulator DC_link napetosti
    nap_dc_reg.Kp = 2.0; //4.0;
    nap_dc_reg.Ki = 0.0001; //0.0002;
    nap_dc_reg.Kff = 0.9;
    nap_dc_reg.OutMax = +20; //+15; //+10.0; // +33.0
    nap_dc_reg.OutMin = -20; //-15; //-10.0; // -33.0

    // inicializiram regulator omreznega toka
    tok_grid_reg.Kp = 0.2; 			//0.2;
    tok_grid_reg.Ki = 0.008; 		//0.008;
    tok_grid_reg.Kff = 1.2; 		//0.8;
    tok_grid_reg.OutMax = +0.99;    // zaradi bootstrap driverjev ne gre do 1.0
    tok_grid_reg.OutMin = -0.99;    // zaradi bootstrap driverjev ne gre do 1.0

    // inicializiram rampo izhodne napetosti
    nap_out_slew.In = 0;    // kasneje jo doloèa potenciometer
    nap_out_slew.Slope_up = 10.0 * SAMPLE_TIME;  // 10 V/s
    nap_out_slew.Slope_down = nap_out_slew.Slope_up;

    // inicializiram regulator izhodne napetosti
    nap_out_reg.Kp = 10.0;
    nap_out_reg.Ki = 0.1; // 0.1
    nap_out_reg.Kff = 0.8;
    nap_out_reg.OutMax = +0.0;   // kasneje to doloèa potenciometer
    nap_out_reg.OutMin = -0.0;   // kasneje to doloèa potenciometer

    // inicializiram ramp bb toka
    tok_bb_slew.In = 0;
    tok_bb_slew.Slope_up = 10.0 * SAMPLE_TIME;  // 10 A/s
    tok_bb_slew.Slope_down = tok_bb_slew.Slope_up;

    // inicializiram tokovna regulatorja
    tok_bb1_reg.Kp = 0.1;
    tok_bb1_reg.Ki = 0.001;
    tok_bb1_reg.Kff = 0.8;
    tok_bb1_reg.OutMax = +0.99; // zaradi bootstrap driverjev ne gre do 1.0
    tok_bb1_reg.OutMin = -0.99; // zaradi bootstrap driverjev ne gre do 1.0

    tok_bb2_reg.Kp = tok_bb1_reg.Kp;
    tok_bb2_reg.Ki = tok_bb1_reg.Ki;
    tok_bb2_reg.Kff = tok_bb1_reg.Kff;
    tok_bb2_reg.OutMax = tok_bb1_reg.OutMax;
    tok_bb2_reg.OutMin = tok_bb1_reg.OutMin;

    // regultaro frekvence
    sync_reg.Kp = 1000;
    sync_reg.Ki = 0.01;
    sync_reg.OutMax = +SWITCH_FREQ/10;
    sync_reg.OutMin = -SWITCH_FREQ/10;

    // inicializiram statistiko
    STAT_FLOAT_MACRO_INIT(statistika);

    // inicializiram ABF
                                    // 2000 Hz;     1000 Hz;     500 Hz,      100 Hz          50 Hz           10 Hz
    i_cap_abf.Alpha = 0.394940272;  // 0.6911845;   0.394940272 ; 0.209807141; 0.043935349;    0.022091045;    0.004437948
    i_cap_abf.Beta = 0.098696044;   // 0.394784176; 0.098696044; 0.024674011; 0.00098696;     0.00024674;     0.0000098696
    i_cap_abf.Capacitance = 5 * 0.0022;   // 2200 uF

    i_cap_dc.Alpha = 0.394940272;  // 0.6911845;   0.394940272 ; 0.209807141; 0.043935349;    0.022091045;    0.004437948
    i_cap_dc.Beta = 0.098696044;   // 0.394784176; 0.098696044; 0.024674011; 0.00098696;     0.00024674;     0.0000098696
    i_cap_dc.Capacitance = 5 * 0.0022;   // 2200 uF

    // inicializiram delay_linijo
    DELAY_FLOAT_INIT(i_grid_delay)
    i_grid_delay.delay = 10;

    // inicializiram filter za oceno toka
    DC_FLOAT_MACRO_INIT(i_dc_f);

    // inicializiram filter za meritev toka
    DC_FLOAT_MACRO_INIT(tok_out_f);

    // inicializiram štoparico
    TIC_init();

    // Proženje prekinitve
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;   //sproži prekinitev pri prehodu TBCOUNTER skozi 0
    EPwm1Regs.ETPS.bit.INTPRD = SAMPLING_RATIO;         //ob vsakem prvem dogodku
    EPwm1Regs.ETCLR.bit.INT = 1;                //clear possible flag
    EPwm1Regs.ETSEL.bit.INTEN = 1;              //enable interrupt

    // registriram prekinitveno rutino
    EALLOW;
    PieVectTable.EPWM1_INT = &PER_int;
    EDIS;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    IER |= M_INT3;
    // da mi prekinitev teèe  tudi v real time naèinu
    // (za razhoršèevanje main zanke in BACK_loop zanke)
    SetDBGIER(M_INT3);
}

#pragma CODE_SECTION(get_electrical, "ramfuncs");
void get_electrical(void)
{
    static float   tok_grid_offset_calib = 0;
    static float   tok_bb1_offset_calib = 0.0;
    static float   tok_bb2_offset_calib = 0.0;
    static float   tok_out_offset_calib = 0.0;
    static float   nap_grid_offset_calib = 0.0;

    // pocakam da ADC konca s pretvorbo
    ADC_wait();
    // poberem vrednosti iz AD pretvornika
    // kalibracija preostalega toka
    if (   (start_calibration == TRUE)
    	&& (calibration_done == FALSE))
    {
        // akumuliram offset
        tok_grid_offset_calib = tok_grid_offset_calib + TOK_GRID;
        tok_bb1_offset_calib = tok_bb1_offset_calib + TOK_BB1;
        tok_bb2_offset_calib = tok_bb2_offset_calib + TOK_BB2;
        tok_out_offset_calib = tok_out_offset_calib + TOK_OUT;
        nap_grid_offset_calib = nap_grid_offset_calib + NAP_GRID;

        // ko potece dovolj casa, sporocim da lahko grem naprej
        // in izracunam povprecni offset
        current_offset_counter = current_offset_counter + 1;
        if (current_offset_counter == (SAMP_FREQ * 1L))
        {
            calibration_done = TRUE;
            tok_grid_offset = tok_grid_offset_calib / (SAMP_FREQ*1L);
            tok_bb1_offset = tok_bb1_offset_calib / (SAMP_FREQ*1L);
            tok_bb2_offset = tok_bb2_offset_calib / (SAMP_FREQ*1L);
            tok_out_offset = tok_out_offset_calib / (SAMP_FREQ*1L);
            nap_grid_offset = nap_grid_offset_calib / (SAMP_FREQ*1L);
        }

        tok_grid = 0.0;
        tok_bb1 = 0.0;
        tok_bb2 = 0.0;
        tok_out = 0.0;
        nap_grid = 0.0;
    }
    else
    {
        tok_grid = tok_grid_gain * (TOK_GRID - tok_grid_offset);
        tok_bb1 = tok_bb1_gain * (TOK_BB1 - tok_bb1_offset);
        tok_bb2 = tok_bb2_gain * (TOK_BB2 - tok_bb2_offset);
        tok_out = tok_out_gain * (TOK_OUT - tok_out_offset);
        nap_grid = nap_grid_gain * (NAP_GRID - nap_grid_offset);
    }

    // se napetosti
    nap_dc = nap_dc_gain * (NAP_DC - nap_dc_offset);
    nap_cap = nap_cap_gain * (NAP_CAP - nap_cap_offset);
    nap_out = nap_out_gain * (NAP_OUT - nap_out_offset);

    // temperatura hladilnika
    temperatura = 0.06944444 * TEMPERATURA - 109.72222;

    // se potenciometri
    pot_i_coarse = POT_I_COARSE * (1/4096.0);
    pot_i_coarse = pot_i_coarse * 1.0 / (1.0 - (POT_DEAD_BAND_WIDTH + POT_SATURATION_WIDTH));
    if (pot_i_coarse < POT_DEAD_BAND_WIDTH)
    {
        pot_i_coarse = 0.0;
    }
    else
    {
        pot_i_coarse = pot_i_coarse - POT_DEAD_BAND_WIDTH;
    }
    if (pot_i_coarse > 1.0)
    {
        pot_i_coarse = 1.0;
    }

    // dodam se histerezo 0.005 - da imam manj suma
    // najprej zaokrozim na +-0.005
    pot_i_coarse = (long)(pot_i_coarse * 200);

    // dodam histerezo
    if (fabs(pot_i_coarse_old - pot_i_coarse) < 2)
    {
    	pot_i_coarse = pot_i_coarse_old;
    }
    // si zapomnim za naslednjiè
    pot_i_coarse_old = 2*(long)(pot_i_coarse / 2);
    // in spravim dol na 0-1
    pot_i_coarse = pot_i_coarse / 200;

    pot_i_fine = POT_I_FINE * (1/4096.0);
    pot_i_fine = pot_i_fine * 1.0 / (1.0 - (POT_DEAD_BAND_WIDTH + POT_SATURATION_WIDTH));
    if (pot_i_fine < POT_DEAD_BAND_WIDTH)
    {
        pot_i_fine = 0.0;
    }
    else
    {
        pot_i_fine = pot_i_fine - POT_DEAD_BAND_WIDTH;
    }
    if (pot_i_fine > 1.0)
    {
        pot_i_fine = 1.0;
    }

    // dodam se histerezo 0.005 - da imam manj suma
    // najprej zaokrozim na +-0.005
    pot_i_fine = (long)(pot_i_fine * 200);

    // dodam histerezo
    if (fabs(pot_i_fine_old - pot_i_fine) < 2)
    {
    	pot_i_fine = pot_i_fine_old;
    }
    // si zapomnim za naslednjiè
    pot_i_fine_old = 2*(long)(pot_i_fine / 2);
    // in spravim dol na 0-1
    pot_i_fine = pot_i_fine / 200;

    pot_u_coarse = POT_U_COARSE * (1/4096.0);
    pot_u_coarse = pot_u_coarse * 1.0 / (1.0 - (POT_DEAD_BAND_WIDTH + POT_SATURATION_WIDTH));
    if (pot_u_coarse < POT_DEAD_BAND_WIDTH)
    {
        pot_u_coarse = 0.0;
    }
    else
    {
        pot_u_coarse = pot_u_coarse - POT_DEAD_BAND_WIDTH;
    }
    if (pot_u_coarse > 1.0)
    {
        pot_u_coarse = 1.0;
    }

    // dodam se histerezo 0.005 - da imam manj suma
    // najprej zaokrozim na +-0.005
    pot_u_coarse = (long)(pot_u_coarse * 200);

    // dodam histerezo
    if (fabs(pot_u_coarse_old - pot_u_coarse) < 2)
    {
    	pot_u_coarse = pot_u_coarse_old;
    }
    // si zapomnim za naslednjiè
    pot_u_coarse_old = 2*(long)(pot_u_coarse / 2);
    // in spravim dol na 0-1
    pot_u_coarse = pot_u_coarse / 200;

    pot_u_fine = POT_U_FINE * (1/4096.0);
    pot_u_fine = pot_u_fine * 1.0 / (1.0 - (POT_DEAD_BAND_WIDTH + POT_SATURATION_WIDTH));
    if (pot_u_fine < POT_DEAD_BAND_WIDTH)
    {
        pot_u_fine = 0.0;
    }
    else
    {
        pot_u_fine = pot_u_fine - POT_DEAD_BAND_WIDTH;
    }
    if (pot_u_fine > 1.0)
    {
        pot_u_fine = 1.0;
    }

    // dodam se histerezo 0.005 - da imam manj suma
    // najprej zaokrozim na +-0.005
    pot_u_fine = (long)(pot_u_fine * 200);

    // dodam histerezo
    if (fabs(pot_u_fine_old - pot_u_fine) < 2)
    {
    	pot_u_fine = pot_u_fine_old;
    }
    // si zapomnim za naslednjiè
    pot_u_fine_old = 2*(long)(pot_u_fine / 2);
    // in spravim dol na 0-1
    pot_u_fine = pot_u_fine / 200;

    // porcunam DFT omrežne napetosti
    nap_grid_dft.In = nap_grid;
    DFT_FLOAT_MACRO(nap_grid_dft);

    // naraèunam amplitudo omrežne napetosti
    nap_grid_rms = ZSQRT2 * sqrt(nap_grid_dft.SumA * nap_grid_dft.SumA + nap_grid_dft.SumB *nap_grid_dft.SumB);

    // normiram, da dobim obliko
    nap_grid_form = nap_grid_dft.Out / (nap_grid_rms * SQRT2);

    // filtriram DC link napetost
    napetost_dc_f.In = nap_dc;
    DC_FLOAT_MACRO(napetost_dc_f);
    napetost_dc_filtered = napetost_dc_f.Mean;

    // izracunam kaksna moc je na izhodu
    power_out = nap_out * (tok_bb1 + tok_bb2);

    // ocena izhodnega toka z ABF
    i_cap_abf.u_cap_measured = nap_cap;
    ABF_float_calc(&i_cap_abf);
    tok_out_abf = -i_cap_abf.i_cap_estimated + (tok_bb1 + tok_bb2);

    // zakasnim tok_grid
    i_grid_delay.in = tok_grid * tok_grid_reg.Out;
    DELAY_FLOAT_CALC(i_grid_delay);

    // ocena dc toka z ABF
    i_cap_dc.u_cap_measured = nap_dc;
    ABF_float_calc(&i_cap_dc);

    // se filtriram
    i_dc_f.In = -i_cap_dc.i_cap_estimated - i_grid_delay.out;
    DC_FLOAT_MACRO(i_dc_f);
    tok_dc_abf = i_dc_f.Mean;

    // filtriram tudi meritev toka
    tok_out_f.In = tok_out;
    DC_FLOAT_MACRO(tok_out_f);

    // statistika
    statistika.In = nap_dc;
    STAT_FLOAT_MACRO(statistika);
}

#pragma CODE_SECTION(check_limits, "ramfuncs");
void check_limits(void)
{
	// samo èe je kalibracija konènana
	if (calibration_done == TRUE)
	{
		if (nap_grid_rms > NAP_GRID_RMS_MAX)
		{
			fault_flags.overvoltage_grid = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
		if (   (nap_grid_rms < NAP_GRID_RMS_MIN)
				&& (state != Initialization)
				&& (state != Startup))
		{
			fault_flags.undervoltage_grid = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
		if (nap_dc > U_DC_MAX)
		{
			fault_flags.overvoltage_dc = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
		if (   (nap_dc < U_DC_MIN)
				&& (state != Initialization)
				&& (state != Startup))
		{
			fault_flags.undervoltage_dc = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
		if ((tok_grid > +CURRENT_GRID_LIM) || (tok_grid < -CURRENT_GRID_LIM))
		{
			fault_flags.overcurrent_grid = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
		if ((tok_bb1 > +CURRENT_BB_LIM) || (tok_bb1 < -CURRENT_BB_LIM))
		{
			fault_flags.overcurrent_bb = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
		if ((tok_bb2 > +CURRENT_BB_LIM) || (tok_bb2 < -CURRENT_BB_LIM))
		{
			fault_flags.overcurrent_bb = TRUE;
			state = Fault_sensed;
			// izklopim mostic
			FB_disable();
			BB_disable();

			// izklopim vse kontaktorjev
			PCB_in_relay_off();
			PCB_res_relay_off();
			PCB_out_relay_off();
		}
	}
}

