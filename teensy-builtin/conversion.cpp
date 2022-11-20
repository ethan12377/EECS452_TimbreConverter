#include "conversion.h"
#include <math.h>

uint16_t clip_counter = 0;
uint16_t prev_main_frequency = 0;
uint16_t curr_main_frequency = 0;
uint8_t freq_lock = 0;

void timbre_convert(float FFT_array[]) {
	// this should be dynamic memory so we can pass a ptr to get_peaks instead of
	// an entire 3d array
    float peak[peak_window];

    // pick the next clip if the main frequency is unchanged, other wise start
    // from 1st clip
    while(freq_lock){}
    freq_lock = 1;
    clip_counter = curr_main_frequency == prev_main_frequency
                       ? (clip_counter + 1) % num_clips
                       : 0;

    // determins the 16 clips of peaks that gonna be used based on main
    // freqeuncy, stored in 3D array peak
    get_peaks(peak, curr_main_frequency, clip_counter);

    // shift the harmonics with regard to main frequency
    float freq_per_sam = (float)sample_rate / (process_size/2);
    uint16_t main_index = (uint16_t) (curr_main_frequency / freq_per_sam);
    reconstruct(peak, FFT_array, main_index);
    freq_lock = 0;
}

void update_freq(uint16_t new_main_freq){
    if(!freq_lock){
        freq_lock = 1;
        prev_main_frequency = curr_main_frequency;
        curr_main_frequency = new_main_freq;
        if(curr_main_frequency <= 20){
            curr_main_frequency = 20;
        }
        if(curr_main_frequency >= 10000){
            curr_main_frequency = 10000;
        }
    }
}

void get_peaks(float peak[], uint16_t curr_main_frequency, uint16_t clip_counter) {
    float ratio1 = 1;
    float ratio2 = 0;
    const float (*left)[peak_window];
    const float (*right)[peak_window];

    if (curr_main_frequency < A1_freq)
        left = right = A1_peaks;
    else if (curr_main_frequency >= A1_freq && curr_main_frequency < A2_freq) {
        ratio1 = (float)(A2_freq - curr_main_frequency) / (A2_freq - A1_freq);
        ratio2 = (float)(curr_main_frequency - A1_freq) / (A2_freq - A1_freq);
        left = A1_peaks;
        right = A2_peaks;
    } else if (curr_main_frequency >= A2_freq && curr_main_frequency < A3_freq) {
        ratio1 = (float)(A3_freq - curr_main_frequency) / (A3_freq - A2_freq);
        ratio2 = (float)(curr_main_frequency - A2_freq) / (A3_freq - A2_freq);
        left = A2_peaks;
        right = A3_peaks;
    } else if (curr_main_frequency >= A3_freq && curr_main_frequency < A4_freq) {
        ratio1 = (float)(A4_freq - curr_main_frequency) / (A4_freq - A3_freq);
        ratio2 = (float)(curr_main_frequency - A3_freq) / (A4_freq - A3_freq);
        left = A3_peaks;
        right = A4_peaks;
    } else if (curr_main_frequency >= A4_freq && curr_main_frequency < A5_freq) {
        ratio1 = (float)(A5_freq - curr_main_frequency) / (A5_freq - A4_freq);
        ratio2 = (float)(curr_main_frequency - A4_freq) / (A5_freq - A4_freq);
        left = A4_peaks;
        right = A5_peaks;
    } else
        left = right = A5_peaks;

    uint16_t index = 0;
    for(int i=0; i<harmonic_num; i++){
        for(int j=0; j<window_size*2*(i+1); j++){
            if(left == right){
                peak[index] = pgm_read_float(*(left+clip_counter)+index);
                // peak[index] = left[clip_counter][index];
            }else{
                // peak[index] = ratio1 * left[clip_counter][index] + ratio2 * right[clip_counter][index]; 
                peak[index] = ratio1 * pgm_read_float(*(left+clip_counter)+index) + ratio2 * pgm_read_float(*(right+clip_counter)+index);            }
            index++;
        }
    }
    
}

void reconstruct(float peak[], float FFT_array[], uint16_t main_index){
    // clear the array for output
    for(uint16_t i=0; i<process_size; i++)
        FFT_array[i] = 0;

    if (main_index < half_window_size)
        main_index = half_window_size + 1;
    
    uint16_t index = 0;
    uint16_t freq_idx = 0;
    for(uint16_t i=0; i<harmonic_num; i++){ // harmonic number
        for(uint16_t j=0; j<window_size*(i+1); j++){ // window size
            freq_idx = (main_index - half_window_size)*(i+1) + j;
            FFT_array[2*(freq_idx-1)] = peak[index++];  // real part
            FFT_array[2*freq_idx-1] = peak[index++];    // imag part
        }
    }
}
