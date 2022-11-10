from scipy.io import wavfile
import numpy as np
import matplotlib.pyplot as plt
from scipy import fft, signal

_,A1 = wavfile.read('A1_109.wav')
_,A2 = wavfile.read('A2_219.wav')
_,A3 = wavfile.read('A3_439.wav')
_,A4 = wavfile.read('A4_879.wav')
RATE,A5 = wavfile.read('A5_1759.wav')
print("Sampling rate:", RATE)

# Fundamental frequencies for each octave
A1_freq = 109
A2_freq = 219
A3_freq = 439
A4_freq = 879
A5_freq = 1759

### Tuneable parameters ###
window_size = 2**12
num_clips = 16
half_width = 10
harmonic_num = 8

A1 = A1[:,0][0:window_size*num_clips].reshape((num_clips, window_size))
A2 = A2[:,0][0:window_size*num_clips].reshape((num_clips, window_size))
A3 = A3[:,0][0:window_size*num_clips].reshape((num_clips, window_size))
A4 = A4[:,0][0:window_size*num_clips].reshape((num_clips, window_size))
A5 = A5[:,0][0:window_size*num_clips].reshape((num_clips, window_size))

def get_model(sound, main_freq, harmonic_num):

    N = sound.shape[0]
    spectrum = fft.fft(sound)
    frequencies = fft.fftfreq(N, 1 / RATE)
    freq_per_sam = frequencies[1]
    main_freq_index = int(main_freq / freq_per_sam)

    peaks = np.zeros((harmonic_num, 2*half_width*harmonic_num), dtype=np.complex_)

    for i in range(harmonic_num):
        peak = signal.windows.blackman(half_width*2*(i+1)) * spectrum[(main_freq_index-half_width)*(i+1):(main_freq_index+half_width)*(i+1)]
        peaks[i][:half_width*2*(i+1)] = peak
    
    return peaks

A1_peaks = np.zeros((num_clips, harmonic_num, 2*half_width*harmonic_num), dtype=np.complex_)
A2_peaks = np.zeros((num_clips, harmonic_num, 2*half_width*harmonic_num), dtype=np.complex_)
A3_peaks = np.zeros((num_clips, harmonic_num, 2*half_width*harmonic_num), dtype=np.complex_)
A4_peaks = np.zeros((num_clips, harmonic_num, 2*half_width*harmonic_num), dtype=np.complex_)
A5_peaks = np.zeros((num_clips, harmonic_num, 2*half_width*harmonic_num), dtype=np.complex_)

for i in range(num_clips):
    A1_peaks[i] = get_model(A1[i], A1_freq, harmonic_num)
    A2_peaks[i] = get_model(A2[i], A2_freq, harmonic_num)
    A3_peaks[i] = get_model(A3[i], A3_freq, harmonic_num)
    A4_peaks[i] = get_model(A4[i], A4_freq, harmonic_num)
    A5_peaks[i] = get_model(A5[i], A5_freq, harmonic_num)

# Write the initial part of the header file
initial_header_text = \
"#include <complex.h>\n\n" + \
"#define num_clips " + str(num_clips) + "\n" + \
"#define harmonic_num " + str(harmonic_num) + "\n" + \
"#define third_arg " + str(2*half_width*harmonic_num) + "\n\n" + \
"const double complex A1_peaks[num_clips][harmonic_num][third_arg] = {\n"

# Reset the file for appending
file = open("freq.h", "w")
file.write("")
file.close()

# Append new stuff
file = open("freq.h", "a")
file.write(initial_header_text)

arg_three = 2*half_width*harmonic_num

for i in range(num_clips):
    file.write("{")
    for j in range(harmonic_num):
        file.write("{")
        for k in range(arg_three):
            file.write(str(A1_peaks[i][j][k].real) + "+" + str(A1_peaks[i][j][k].imag) + "i")
            if k != arg_three-1: file.write(",")

        if j != harmonic_num-1: file.write("},\n")
        else: file.write("}\n")

    if i != num_clips-1: file.write("},\n")
    else: file.write("}\n")
file.write("};")

file.close()
