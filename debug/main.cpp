#include "conversion.h"
#include <stdint.h>

int main(){
    float FFT_array[4096*2];
    for (int i=0; i<4096*2; i++){
        FFT_array[i] = 200;
    }
    timbre_convert(FFT_array);
    // for (int i=0; i<4096*2; i++){
    //     cout << FFT_array[i] << endl;
    // }
    return 0;
}