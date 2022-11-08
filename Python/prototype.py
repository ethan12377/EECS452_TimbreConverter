from random import sample
from scipy.io import wavfile
from pylab import subplots, show, asarray, int16
from sounddevice import play as playsound
import matplotlib.pyplot as plt
import numpy as np
from scipy import fftpack, signal
from scipy.io.wavfile import write
import time

_,A1 = wavfile.read('A1_109.wav')
_,A2 = wavfile.read('A2_219.wav')
_,A3 = wavfile.read('A3_439.wav')
_,A4 = wavfile.read('A4_879.wav')
RATE,A5 = wavfile.read('A5_1759.wav')
print("Sampling rate:", RATE)

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


def play(w,fs=RATE):
    "Play audio from float array"
    w16 = asarray( w/abs(w).max() * (1<<15), int16)
    playsound(w16,samplerate=fs)


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

### Teensy Prototype ###

frequencies = fftpack.fftfreq(window_size, 1 / RATE)
freq_per_sam = frequencies[1]

# simulate input audio
input_len = 16*8
input_main_freq = [
    523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25, 523.25,
    587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33, 587.33,
    659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26, 659.26,
    698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46,
    784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99, 784.99,
    880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00, 880.00,
    987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77, 987.77,
    1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5, 1046.5
]

output_sound = np.zeros(input_len*window_size)
prev_freq = 0
counter = 0

# generate spectrum from the models
for t in range(len(input_main_freq)):
    output_freq = input_main_freq[t]
    if output_freq < A1_freq:
        peaks = A1_peaks
    elif A1_freq <= output_freq and output_freq < A2_freq:
        ratio1 = (A2_freq - output_freq) / (A2_freq - A1_freq)
        ratio2 = (output_freq - A1_freq) / (A2_freq - A1_freq)
        peaks = A1_peaks*ratio1 + A2_peaks*ratio2
    elif A2_freq <= output_freq and output_freq < A3_freq:
        ratio1 = (A3_freq - output_freq) / (A3_freq - A2_freq)
        ratio2 = (output_freq - A2_freq) / (A3_freq - A2_freq)
        peaks = A2_peaks*ratio1 + A3_peaks*ratio2
    elif A3_freq <= output_freq and output_freq < A4_freq:
        ratio1 = (A4_freq - output_freq) / (A4_freq - A3_freq)
        ratio2 = (output_freq - A3_freq) / (A4_freq - A3_freq)
        peaks = A3_peaks*ratio1 + A4_peaks*ratio2
    elif A4_freq <= output_freq and output_freq < A5_freq:
        ratio1 = (A5_freq - output_freq) / (A5_freq - A4_freq)
        ratio2 = (output_freq - A4_freq) / (A5_freq - A4_freq)
        peaks = A4_peaks*ratio1 + A5_peaks*ratio2
    else:
        peaks = A5_peaks

    if prev_freq==output_freq and counter < 15:
        counter += 1
    else:
        counter = 0
    prev_freq = output_freq

    output_spectrum = np.zeros(window_size, dtype=np.complex_)
    main_freq_index = int(output_freq / freq_per_sam)
    for i in range(harmonic_num):
        output_spectrum[(main_freq_index-half_width)*(i+1):(main_freq_index+half_width)*(i+1)] = peaks[counter][i][:half_width*2*(i+1)]
    sound_reconstructed = np.real(np.fft.ifft(output_spectrum))
    output_sound[t*window_size:(t+1)*window_size] = sound_reconstructed


plt.specgram(output_sound, Fs = RATE)
plt.show()
# write("simple_ifft.wav", RATE, asarray( sound_reconstructed/abs(sound_reconstructed).max() * (1<<15), int16))

play(output_sound)
time.sleep(20)

