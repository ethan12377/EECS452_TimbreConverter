from scipy.io import wavfile
import numpy as np
import matplotlib.pyplot as plt
from scipy import fftpack, signal

_,A1 = wavfile.read('A1_109.wav')
_,A2 = wavfile.read('A2_219.wav')
_,A3 = wavfile.read('A3_439.wav')
_,A4 = wavfile.read('A4_879.wav')
RATE,A5 = wavfile.read('A5_1759.wav')
print("Sampling rate:", RATE)

# Define the octave frequencies
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
    spectrum = fftpack.fft(sound)
    frequencies = fftpack.fftfreq(N, 1 / RATE)
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

for i in A1_peaks:
    # Print the peak to a file
    # Do the same for the other peaks
