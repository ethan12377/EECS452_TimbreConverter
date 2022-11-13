#ifndef conversion_h
#define conversion_h

#include <stdint.h>
#include "freq.h"

/*
#define num_clips 16
#define harmonic_num 8
#define third_arg 160
*/

#define sample_rate 44100
#define process_size 4096
#define half_window_size 20

#define A1_freq 109
#define A2_freq 219
#define A3_freq 439
#define A4_freq 879
#define A5_freq 1759

void timbre_convert(float*);

uint16_t find_main_freq(float *);
void get_peaks(float [][max_window_size*2], uint16_t, uint16_t);
void reconstruct(float [][max_window_size*2], float*, uint16_t);

#endif
