#include "conversion.h"

void timbre_convert(float* FFT_array) {
    uint16_t clip_counter = 0;
    uint16_t prev_main_frequency = 0;
    uint16_t curr_main_frequency = 0;
    double complex peak[num_clips][harmonic_num][third_arg];

    // find main frequency
    curr_main_frequency = find_main_freq(FFT_array);

    // determins the 16 clips of peaks that gonna be used based on main
    // freqeuncy, stored in 3D array peak
    get_peaks(peak, curr_main_frequency);

    // pick the next clip if the main frequency is unchanged, other wise start
    // from 1st clip
    clip_counter = curr_main_frequency == prev_main_frequency
                       ? (clip_counter + 1) % num_clips
                       : 0;
    prev_main_frequency = curr_main_frequency;

    // shift the harmonics with regard to main frequency
    uint16_t freq_per_sam = sample_rate / process_size;
    uint16_t main_index = (uint16_t) (curr_main_frequency / freq_per_sam);
    reconstruct(peak, FFT_array, clip_counter, main_index);
}

uint16_t find_main_freq(float * FFT_array){

}

void get_peaks(double complex peak[][harmonic_num][third_arg], uint16_t curr_main_frequency) {
    double ratio1 = 1;
    double ratio2 = 0;
    double complex (*left)[harmonic_num][third_arg];
    double complex (*right)[harmonic_num][third_arg];

    if (curr_main_frequency < A1_freq)
        left = right = A1_peaks;
    else if (curr_main_frequency >= A1_freq && curr_main_frequency < A2_freq) {
        ratio1 = (A2_freq - curr_main_frequency) / (A2_freq - A1_freq);
        ratio2 = (curr_main_frequency - A1_freq) / (A2_freq - A1_freq);
        left = A1_peaks;
        right = A2_peaks;
    } else if (curr_main_frequency >= A2_freq && curr_main_frequency < A3_freq) {
        ratio1 = (A3_freq - curr_main_frequency) / (A3_freq - A2_freq);
        ratio2 = (curr_main_frequency - A2_freq) / (A3_freq - A2_freq);
        left = A2_peaks;
        right = A3_peaks;
    } else if (curr_main_frequency >= A3_freq && curr_main_frequency < A4_freq) {
        ratio1 = (A4_freq - curr_main_frequency) / (A4_freq - A3_freq);
        ratio2 = (curr_main_frequency - A3_freq) / (A4_freq - A3_freq);
        left = A3_peaks;
        right = A4_peaks;
    } else if (curr_main_frequency >= A4_freq && curr_main_frequency < A5_freq) {
        ratio1 = (A5_freq - curr_main_frequency) / (A5_freq - A4_freq);
        ratio2 = (curr_main_frequency - A4_freq) / (A5_freq - A4_freq);
        left = A4_peaks;
        right = A5_peaks;
    } else
        left = right = A5_peaks;

    for(int i=0; i<num_clips; i++){
        for(int j=0; j<harmonic_num; j++){
            for(int k=0; k<third_arg; k++){
                if(left == right){
                    peak[i][j][k] = left[i][j][k];
                }else{
                    peak[i][j][k] = ratio1 * left[i][j][k] + ratio2 * right[i][j][k];
                }
            }
        }
    }
}

void reconstruct(double complex peak[][harmonic_num][third_arg], float* FFT_array, uint16_t clip_counter, uint16_t main_index){
    // clear the array for output
    for(uint16_t i=0; i<process_size; i++)
        FFT_array[i] = 0;
    
    for(uint16_t i=0; i<harmonic_num; i++){ // harmonic number
        for(uint16_t j=0; j<half_window_size*2*i; j++){ // window size
            FFT_array[(main_index - half_window_size)*(i+1) + j] = creal(peak[clip_counter][i][j]);
            FFT_array[(main_index - half_window_size)*(i+1) + j + 1] = cimag(peak[clip_counter][i][j]);
        }
    }
}