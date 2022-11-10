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

#ifndef filter_h_
#define filter_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "arm_const_structs.h"

 // Indicates that the code should just pass through the audio
 // without any filtering (as opposed to doing nothing at all)
#define FIR_PASSTHRU ((const short *) 1)
#define FIR_MAX_COEFFS_T 400  // don't forget to test for this limit!!!!!!
#define FFT_SIZE 1024		// FFT size
#define M  (244+13)       	// adding zeros to taps to maket L a multiple of 128 samples
#define L 768
#define N_BLOCKS 6          // number of 128 sample blocks used for one iterations


class Filter : public AudioStream
{
public:
	Filter(void): AudioStream(1,inputQueueArray), coeff_p(NULL) {
	}
	void begin(const short *cp, int n_coeffs) {  // begin .. initializes things
		coeff_p = cp;           // points to q15 filter coefficient values
		coeff_n = n_coeffs;     // number of coefficient values
		block_ctr = 0;          // counts input blocks

		// check for valid filter information
		if ((coeff_p != NULL) && (coeff_p != FIR_PASSTHRU) && (n_coeffs <= FIR_MAX_COEFFS_T)) {
			// probably have valid filter information -- copy coeffs into complex float array
			ptr_short = (short *)cp; ptr_float = &FFT_filter[0];
			// coefficients are q15 int16_t...have to make into float equivalent values
			for (ctr = 0; ctr < FFT_SIZE; ctr++) {
				if (ctr < coeff_n) { *ptr_float++ = (float)(*ptr_short++) / 32767.0; *ptr_float++ = 0; }
				else { *ptr_float++ = 0; *ptr_float++ = 0; }
			}

			// form the FFT of filter coefficients
			arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_filter, 0, 1);
			for (ctr = 0; ctr < M - 1; ctr++) SAVE_array[ctr] = 0; // initialize SAVE_array
		} // end of if filter likely valid test
		else {
			if (coeff_p != FIR_PASSTHRU) { coeff_p = NULL; }  // a brutal ending on error...no one is home
		}
	} // end of begin function

	void end(void) {
		coeff_p = NULL; // this signals termination of execution
	}
	virtual void update(void);
private:
	audio_block_t *inputQueueArray[1];

	// pointer to current coefficients or NULL or FIR_PASSTHRU
	const short *coeff_p; 
	short  coeff_n;

	float FFT_array[2 * FFT_SIZE];        // the FFT data array
	float FFT_filter[2 * FFT_SIZE];       // the filter H array
	float SAVE_array[FIR_MAX_COEFFS_T]; // the save array 

	short *ptr_short;	                // points to a short array
	float *ptr_float, *ptr_FFT;         // point to a floats 
	int16_t block_ctr;
	int16_t ctr;
};

#endif
