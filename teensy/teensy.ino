#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <stdio.h>
#include "filter.h"

const int myInput = AUDIO_INPUT_LINEIN;

AudioInputI2S          audioInput;         // audio shield: mic or line-in
Filter                 filter;
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out

AudioConnection patchCord1(audioInput, 0, filter, 0);
AudioConnection patchCord2(audioInput, 1, filter, 1);

AudioConnection patchCord3(filter, 0, audioOutput, 0);
AudioConnection patchCord4(filter, 1, audioOutput, 1);

AudioControlSGTL5000 audioShield;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  printf("setup started\n"); fflush(stdout);

  AudioMemory(12);
  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);

  // Configure the window algorithm to use
  myFFT.windowFunction(AudioWindowHanning1024);
  myIFFT.windowFunction(AudioWindowHanning1024);
  //myFFT.windowFunction(NULL);

  printf("setup done\n"); fflush(stdout);
}

void loop()
{
  
}
