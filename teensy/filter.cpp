/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Pete (El Supremo)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include "filter.h"
#include "arm_math.h"

const int readPin = A0; // ADC0
const int ledPin = 0;   // indicator LED


void Filter::update(void)
{
	audio_block_t *block, *b_new;
	void process(float *, float *, float *);  // does one overlap-add iteration

	block = receiveReadOnly();  // try to read a block of samples
	if (!block) return;         // exit and try again if one is not available

	// If there's no coefficient table, give up.
	if (coeff_p == NULL) {
		release(block);
		return;
	}

	// do passthru
	if (coeff_p == FIR_PASSTHRU)
	{
		// Just pass samples through
		transmit(block);
		release(block);
		return;
	}

	// do a FFT overlap-and-add FIR filter iteration

	b_new = allocate(); // reading values from previous iteration
	ptr_FFT = &FFT_array[0] + 2 * block_ctr * AUDIO_BLOCK_SAMPLES; // values are stored complex
	for (ctr = 0; ctr < AUDIO_BLOCK_SAMPLES; ctr++) {
		*((int16_t  *)b_new->data + ctr) = (int16_t)* ptr_FFT++;  ptr_FFT++;
	}
	transmit(b_new); // this frees up current 128 complex values to hold new input samples
	release(b_new);

	ptr_FFT = &FFT_array[0] + 2 * block_ctr * AUDIO_BLOCK_SAMPLES;
	for (ctr = 0; ctr < AUDIO_BLOCK_SAMPLES; ctr++) {   // samples are real valued
		*ptr_FFT++ = (float) * ((int16_t *)block->data + ctr);
		*ptr_FFT++ = 0;  // imaginary part is zero
	}
	release(block);

	if (++block_ctr < N_BLOCKS) return; // get next input block

	// need to add 2 blocks worth of zeros to input FFT buffer
	// samples filled 6 blocks .. FFT size is 8 blocks by design
	// code is specific for selected filter used in lab 4

	for (ctr = 0; ctr < 2 * AUDIO_BLOCK_SAMPLES; ctr++) {
		*ptr_FFT++ = 0; // in effect adding M-1 complex zeros
		*ptr_FFT++ = 0; // to the FFT array
	}

	digitalWrite(ledPin, HIGH); // checking processing time
	process(FFT_array, FFT_filter, SAVE_array);
	digitalWrite(ledPin, LOW);

	// FFT_array contains filtered samples in real part

	block_ctr = 0;  // reset block counter
	return;         // and iterate again
}



void process(float *FFT_array, float *FFT_filter, float *SAVE_array)
{
	int16_t ctr;
	float a1, a2, b1, b2;


	//input is complex valued array of L values with M-1 complex zeros appended
	//values are selected so that L will be a multiple of 128
	//zeros were added to the filter coefficients to get this to happen

	//form_FFT:
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_array, 0, 1);  // ARM's DSP library complex fp input FFT

	//multiply_by_H: -- FFT of the filter coefficients
	for (ctr = 0; ctr < FFT_SIZE; ctr++) // this many complex values
	{
		//TODO: assign the appropriate values to a1, b1, a2, b2 (recall prelab question regarding the real and imagingary sample order from FFT)
		
		a1 = FFT_array[2 * ctr]; // real part of the FFT of input
		b1 = FFT_array[2 * ctr + 1]; // imaginary part of the FFT of input
		a2 = FFT_filter[2 * ctr]; // real part of the FFT of filter coefficients
		b2 = FFT_filter[2 * ctr + 1]; // imaginary part the FFT of filter coefficients
		

		//TODO: Real part multiplication result, fill in line below
		FFT_array[2 * ctr] = a1*a2 - b1*b2; //compute the real part using a1, b1, a2, b2

		//TODO: Imaginary part multiplication result, fill in the line below
		FFT_array[2 * ctr + 1] = a1*b2 + a2*b1; //compute the imaginary part, use a1, b1, a2, b2
	}

	//back_to_time:
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_array, 1, 1);  // ARM's DSP library complex fp input inverse FFT

	//add_in_M_minus_1_values_from_save:
	for (ctr = 0; ctr < M - 1; ctr++) {
		FFT_array[2 * ctr] = FFT_array[2 * ctr] + SAVE_array[ctr + 1]; 
		FFT_array[2 * ctr + 1] = 0;
	}

	//save_M_minus_one_values;
	for (ctr = 0; ctr < M - 1; ctr++) {
		SAVE_array[ctr] = FFT_array[2 * (ctr + (FFT_SIZE - (M - 1)))]; 
	}
}

