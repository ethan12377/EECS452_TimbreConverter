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
#include "conversion.h"

#define FFT_SIZE 4096		// FFT size
#define N_BLOCKS 32         // 32 x 128 = 4096
#define PASS_THRU 0

class Filter : public AudioStream
{
public:
	Filter(void): AudioStream(1,inputQueueArray), block_ctr(0) {}
	virtual void update(void);
	void update_main_freq(uint16_t);
private:
	audio_block_t *inputQueueArray[1];
	float FFT_array[2 * FFT_SIZE];        // the FFT data array
	float *ptr_FFT;         // point to a floats 
	int16_t block_ctr;		// counts input block
};

#endif
