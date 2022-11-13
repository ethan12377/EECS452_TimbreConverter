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

#include "filter.h"


void Filter::update(void){
	audio_block_t *block, *b_new;
	void process(float *); 

	block = receiveReadOnly();  // try to read a block of samples
	if (!block) return;         // exit and try again if one is not available

	// do passthru
	if (PASS_THRU){
		// Just pass samples through
		transmit(block);
		release(block);
		return;
	}

	b_new = allocate(); // reading values from previous iteration
	ptr_FFT = &FFT_array[0] + 2 * block_ctr * AUDIO_BLOCK_SAMPLES; // values are stored complex
	for (int16_t ctr = 0; ctr < AUDIO_BLOCK_SAMPLES; ctr++) {
		*((int16_t  *)b_new->data + ctr) = (int16_t)* ptr_FFT++;  ptr_FFT++;
	}
	transmit(b_new); // this frees up current 128 complex values to hold new input samples
	release(b_new);

	ptr_FFT = &FFT_array[0] + 2 * block_ctr * AUDIO_BLOCK_SAMPLES;
	for (int16_t ctr = 0; ctr < AUDIO_BLOCK_SAMPLES; ctr++) {   // samples are real valued
		*ptr_FFT++ = (float) * ((int16_t *)block->data + ctr);
		*ptr_FFT++ = 0;  // imaginary part is zero
	}
	release(block);

	if (++block_ctr < N_BLOCKS) return; // get next input block

	process(FFT_array);

	block_ctr = 0;  // reset block counter
	return;         // and iterate again
}


void process(float *FFT_array){
	// do FFT on 4096 samples
	arm_cfft_f32(&arm_cfft_sR_f32_len4096, FFT_array, 0, 1);

	timbre_convert(FFT_array);

	// iFFT back to time
	arm_cfft_f32(&arm_cfft_sR_f32_len4096, FFT_array, 1, 1); 
}

