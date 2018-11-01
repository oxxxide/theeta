/*
 * Gen.c
 *
 *  Created on: 2018/05/18
 *      Author: devox
 */

#include "Gen.h"

float adcValue1 = 0;
float adcValue2 = 0;
float adcValue3 = 0;
float adcValue4 = 0;

void Gen_trig(Gen *gen){
	AHR_trig(&gen->eg_noise);
	Decay_trig(&gen->decay_filter);
	AHR_trig(&gen->eg_mod);
	AHR_trig(&gen->eg_bend);
	AHR_trig(&gen->eg_amp);
}

void Gen_init(Gen *gen) {

	Filter_Init(&gen->filter);

	gen->carr_level = 0.5f;
	gen->noise_level = 0;
	gen->mod_depth = 0;
	gen->cf_ringmod_dev = 1.0f;
	gen->cv = adcValue1;
	gen->modtype = MODTYPE_FM;

	AHR_Init(&gen->eg_amp);
	AHR_Init(&gen->eg_mod);
	AHR_Init(&gen->eg_bend);
	AHR_Init(&gen->eg_noise);

	Decay_Init(&gen->decay_filter);

	Osc_Init(&gen->carr);
	Osc_Init(&gen->modu);

	LFO_Init(&gen->lfo);

}

float FORCE_INLINE Gen_process_fm(Gen *gen, float cv) {
	float v_eg_amp = AHR_proc(&gen->eg_amp);
	float bend = AHR_proc(&gen->eg_bend);
	float lfov1 = LFO_proc(&gen->lfo);
	float fmv = Osc_proc(&gen->modu);
	fmv = fmv * AHR_proc(&gen->eg_mod) * gen->mod_depth;
	float v_osc_carr = Osc_proc_bend_fm_lfo(&gen->carr, cv, bend, fmv, lfov1);
	float v_noise = Noise_Generate() * AHR_proc(&gen->eg_noise) * gen->noise_level;
	float ret = (v_osc_carr * v_eg_amp * gen->carr_level) + (v_noise);
	float dk = Decay_proc(&gen->decay_filter);
	//Filter
	switch (gen->filter.filter_type) {
	case BYPASS:
		break;
	default:
		//ret = Filter_process_no_envelope(&gen->filter, ret);
		ret = Filter_process_no_envelope_w_lfo(&gen->filter, ret, (int32_t) (dk*gen->decay_filter.i_amount + lfov1) );
		break;
	}
	return ret;
}

float FORCE_INLINE Gen_process_ringmod(Gen *gen, float cv) {
	float v_eg_amp = AHR_proc(&gen->eg_amp);
	float bend = AHR_proc(&gen->eg_bend);
	float lfov1 = LFO_proc(&gen->lfo);
	float amv = Osc_proc(&gen->modu);
	amv = amv * AHR_proc(&gen->eg_mod) * gen->mod_depth;
	float v_osc_carr = Osc_proc_bend_fm_lfo(&gen->carr, cv, bend, 0, lfov1);

	v_osc_carr = (v_osc_carr * (1+amv)) * gen->cf_ringmod_dev;

	float v_noise = Noise_Generate() * AHR_proc(&gen->eg_noise) * gen->noise_level;
	float ret = (v_osc_carr * v_eg_amp * gen->carr_level) + (v_noise);
	float dk = Decay_proc(&gen->decay_filter);
	//Filter
	switch (gen->filter.filter_type) {
	case BYPASS:
		break;
	default:
		//ret = Filter_process_no_envelope(&gen->filter, ret);
		ret = Filter_process_no_envelope_w_lfo(&gen->filter, ret, (int32_t) (dk*gen->decay_filter.i_amount + lfov1) );
		break;
	}
	return ret;
}

//wave

void Gen_set_carr_wave(Gen *gen, Waveform wf) {
	Osc_set_waveform(&gen->carr,wf);
}

Waveform Gen_get_carr_wave(Gen *gen) {
	return gen->carr.waveform;
}

void Gen_set_modtype(Gen *gen, MOD_TYPE type){
	gen->modtype = type;
}

MOD_TYPE Gen_get_modtype(Gen *gen){
	return gen->modtype;
}

void Gen_set_carr_coarse(Gen *gen, int note) {

	note = LIMIT(note,127,0);

	Osc_set_pitch(&gen->carr, note);

	Osc_set_pitch(&gen->modu, note + gen->i_fm_harmonics);
}

int Gen_get_carr_coarse(Gen *gen) {
	return gen->carr.pitch;
}

void Gen_set_carr_fine(Gen *gen, int fine) {

	fine = LIMIT(fine,63,-63);

	Osc_set_fine(&gen->carr, fine);
	Osc_set_fine(&gen->modu, fine);
}

int Gen_get_carr_fine(Gen *gen) {
	return gen->carr.fine;
}

void Gen_set_carr_moddepth(Gen *gen, int depth) {
	depth = LIMIT(depth,127,0);
	gen->mod_depth = (depth / 127.0f);
	gen->cf_ringmod_dev = 1.0f / (1.0f + gen->mod_depth);
}

int Gen_get_carr_moddepth(Gen *gen) {
	return (int) (gen->mod_depth * 127);
}

// amplifier

void Gen_set_carr_level(Gen *gen, int level) {


	level = LIMIT(level,127,0);

	gen->carr_level = level / 127.0f;
}

int Gen_get_carr_level(Gen *gen) {
	return (int) (gen->carr_level * 127);
}

void Gen_set_carr_attack(Gen *gen, int v) {


	v = LIMIT(v,127,0);

	AHR_set_attack(&gen->eg_amp, v);
}

int Gen_get_carr_attack(Gen *gen) {
	return gen->eg_amp.i_attack;
}

void Gen_set_carr_hold(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_hold(&gen->eg_amp, v);
}

int Gen_get_carr_hold(Gen *gen) {
	return gen->eg_amp.i_hold;
}

void Gen_set_carr_slope(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_slope(&gen->eg_amp, v);
}

int Gen_get_carr_slope(Gen *gen) {
	return gen->eg_amp.i_slope;
}

void Gen_set_carr_release(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_release(&gen->eg_amp, v);
}

int Gen_get_carr_release(Gen *gen) {
	return gen->eg_amp.i_release;
}

///fm
void Gen_set_fm_amount(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	gen->mod_depth = v/127.0f;
}

int Gen_get_fm_amount(Gen *gen) {
	return (int)(gen->mod_depth * 127.0f);
}

void Gen_set_fm_harmonics(Gen *gen, int v){

	v = LIMIT(v,63,-63);

	gen->i_fm_harmonics = v;
	Osc_set_pitch(&gen->modu,gen->carr.pitch + v);
}

int Gen_get_fm_harmonics(Gen *gen){
	return gen->i_fm_harmonics;
}

void Gen_set_fm_attack(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_attack(&gen->eg_mod, v);
}

int Gen_get_fm_attack(Gen *gen) {
	return gen->eg_mod.i_attack;
}

void Gen_set_fm_hold(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_hold(&gen->eg_mod, v);
}

int Gen_get_fm_hold(Gen *gen) {
	return gen->eg_mod.i_hold;
}

void Gen_set_fm_slope(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_slope(&gen->eg_mod, v);
}

int Gen_get_fm_slope(Gen *gen) {
	return gen->eg_mod.i_slope;
}

void Gen_set_fm_release(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_release(&gen->eg_mod, v);
}

int Gen_get_fm_release(Gen *gen) {
	return gen->eg_mod.i_release;
}

///bend

void Gen_set_bend_amount(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	Osc_set_modgain(&gen->carr, v);
}

int Gen_get_bend_amount(Gen *gen) {
	return gen->carr.egAmount;
}

void Gen_set_bend_attack(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_attack(&gen->eg_bend, v);
}

int Gen_get_bend_attack(Gen *gen) {

	return gen->eg_bend.i_attack;
}

void Gen_set_bend_hold(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_hold(&gen->eg_bend, v);
}

int Gen_get_bend_hold(Gen *gen) {
	return gen->eg_bend.i_hold;
}

void Gen_set_bend_slope(Gen *gen, int v) {


	v = LIMIT(v,127,0);

	AHR_set_slope(&gen->eg_bend, v);
}

int Gen_get_bend_slope(Gen *gen) {
	return gen->eg_bend.i_slope;
}

void Gen_set_bend_release(Gen *gen, int v) {

	v = LIMIT(v,127,0);

	AHR_set_release(&gen->eg_bend, v);
}

int Gen_get_bend_release(Gen *gen) {
	return gen->eg_bend.i_release;
}

//Noise
void Gen_set_noise_level(Gen* gen, int level) {

	level = LIMIT(level,127,0);

	gen->noise_level = level / 127.0f;
}

//Noise
int Gen_get_noise_level(Gen* gen) {
	return (int)(gen->noise_level * 127);
}

void Gen_set_noise_attack(Gen* gen, int v) {
	AHR_set_attack(&gen->eg_noise, v);
}

int Gen_get_noise_attack(Gen* gen) {
	return gen->eg_noise.i_attack;
}

void Gen_set_noise_hold(Gen* gen, int v) {
	AHR_set_hold(&gen->eg_noise, v);
}

void Gen_set_noise_slope(Gen* gen, int v) {
	AHR_set_slope(&gen->eg_noise, v);
}

int Gen_get_noise_hold(Gen* gen) {
	return gen->eg_noise.i_hold;
}

int Gen_get_noise_slope(Gen* gen) {
	return gen->eg_noise.i_slope;
}

void Gen_set_noise_release(Gen* gen, int v) {
	AHR_set_release(&gen->eg_noise, v);
}

int Gen_get_noise_release(Gen* gen) {
	return gen->eg_noise.i_release;
}

//Filter
//
void Gen_set_filter_cutoff(Gen* gen, int v) {
	gen->i_cutoff = LIMIT(v,127,0);
	Filter_setCutoff(&gen->filter, v, note_to_freq(v));
}

void Gen_set_filter_cutoff_w_lfo(Gen* gen, int v,float lfov1) {
	gen->i_cutoff = LIMIT(v,127,0);
	Filter_setCutoff(&gen->filter, v, note_to_freq(v));
}

uint8_t Gen_get_filter_cutoff(Gen* gen) {
	return gen->i_cutoff;
}
//

uint8_t Gen_get_filter_amount(Gen* gen) {
	return (uint8_t)gen->decay_filter.i_amount;
}

void Gen_set_filter_resonance(Gen* gen, int v) {

	v = LIMIT(v,6,0);

	gen->filter.resonance = v;
}

uint32_t Gen_get_filter_resonance(Gen* gen) {
	return gen->filter.resonance;
}

void Gen_set_filter_type(Gen* gen, int v) {

	v = LIMIT(v,NUM_OF_FILTER_TYPES,0);

	Filter_setFilterType(&gen->filter, v, 1);
}

int Gen_get_filter_type(Gen* gen) {
	return gen->filter.filter_type;
}

//Filter Envelope
// -64 <= v >= +64
void Gen_set_filter_amount(Gen* gen, int v) {
	if (v < 0) {
		gen->filter_amount = (1.0f / ((-v) / 12.7f)) + 1.0f;
	} else {
		gen->filter_amount = (v / 12.7f) + 1.0f;
	}
}

void Gen_set_filter_decay(Gen* gen, int v){

	v = LIMIT(v,127,0);

	Decay_set_Decay(&gen->decay_filter,v);
}

int Gen_get_filter_decay(Gen* gen){
	return gen->decay_filter.i_decay;
}


void Gen_set_lfo_speed(Gen* gen, uint8_t v) {

	v = LIMIT(v,127,0);

	LFO_setSpeed(&gen->lfo, v);
}

void Gen_set_lfo_depth(Gen* gen, uint8_t v) {

	v = LIMIT(v,127,0);

	LFO_setDepth(&gen->lfo, v);
}

void Gen_set_lfo_dest(Gen* gen, uint8_t v) {

	v = LIMIT(v,127,0);

	LFO_setDest(&gen->lfo, v);
}


void preset_kikck(Gen* gen){
	Gen_set_carr_coarse(gen,29);
	Gen_set_carr_moddepth(gen,0);
	Gen_set_carr_hold(gen,10);
	Gen_set_carr_release(gen,40);
	Gen_set_bend_amount(gen,70);
	Gen_set_bend_hold(gen,0);
	Gen_set_bend_release(gen,25);
}

void preset_hihat(Gen* gen) {
	Gen_set_carr_level(gen,63);
	Gen_set_carr_coarse(gen, 120);
	Gen_set_carr_moddepth(gen, 127);
	Gen_set_carr_hold(gen, 7);
	Gen_set_carr_release(gen, 25);

	Gen_set_fm_harmonics(gen, 123);
	Gen_set_fm_hold(gen, 1);
	Gen_set_fm_release(gen, 20);

	Gen_set_bend_amount(gen, 0);
	Gen_set_bend_hold(gen, 0);
	Gen_set_bend_release(gen, 0);

	Gen_set_noise_level(gen, 65);
	Gen_set_noise_hold(gen, 5);
	Gen_set_noise_release(gen, 15);

	Gen_set_filter_type(gen, HIGHPASS);
	Gen_set_filter_cutoff(gen, 110);
	Gen_set_filter_resonance(gen, 70);
}


void preset_click(Gen* gen) {
	Gen_set_carr_level(gen,70);
	Gen_set_carr_coarse(gen, 120);
	Gen_set_carr_moddepth(gen, 100);
	Gen_set_carr_hold(gen, 10);
	Gen_set_carr_release(gen, 15);

	Gen_set_fm_attack(gen, 32);
	Gen_set_fm_harmonics(gen, 90);
	Gen_set_fm_hold(gen, 1);
	Gen_set_fm_release(gen, 10);

	Gen_set_bend_amount(gen, 0);
	Gen_set_bend_hold(gen, 0);
	Gen_set_bend_release(gen, 0);

	Gen_set_noise_level(gen, 30);
	Gen_set_noise_hold(gen, 10);
	Gen_set_noise_release(gen, 5);

	Gen_set_filter_type(gen, HIGHPASS);
	Gen_set_filter_cutoff(gen, 60);
	Gen_set_filter_resonance(gen, 60);
}


void preset_hh_rnd(Gen* gen){
	Gen_set_carr_release(gen, (int)(20 + Noise_Generate() * 20));
	Gen_set_fm_harmonics(gen, (int)(90 + Noise_Generate()*40));
}

void preset_click_rnd(Gen* gen) {

	Gen_set_carr_level(gen, 40 );
	//Gen_set_carr_coarse(gen, (int)(110 + Noise_Generate() * 10) );
	Gen_set_carr_moddepth(gen, (int) 100 + Noise_Generate() * 27);
	Gen_set_carr_hold(gen, 10);
	Gen_set_carr_release(gen, 30);

	Gen_set_fm_attack(gen, (int) (32 + Noise_Generate() * 32));
	Gen_set_fm_harmonics(gen, (int) (90 + Noise_Generate() * 60));
	Gen_set_fm_hold(gen, (int) (20 + Noise_Generate() * 20));
	Gen_set_fm_release(gen, 5);

	Gen_set_bend_amount(gen, 0);
	Gen_set_bend_hold(gen, 0);
	Gen_set_bend_release(gen, 0);

	Gen_set_noise_level(gen, 80);
	Gen_set_noise_attack(gen, (int)(30 + Noise_Generate() * 30));
	Gen_set_noise_hold(gen, (int)(15 + Noise_Generate() * 15));
	Gen_set_noise_release(gen, 10);

	Gen_set_filter_type(gen, HIGHPASS);
	Gen_set_filter_cutoff(gen, (int) (70 + Noise_Generate() * 40));
	Gen_set_filter_resonance(gen, (int) (70 + Noise_Generate() * 40));
}
