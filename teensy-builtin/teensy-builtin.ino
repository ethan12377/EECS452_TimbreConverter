#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <stdio.h>
#include "filter.h"

// const int myInput = AUDIO_INPUT_LINEIN;
const int myInput = AUDIO_INPUT_MIC;


AudioInputI2S          audioInput;         // audio shield: mic or line-in
Filter                 filter;
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out
AudioAnalyzeNoteFrequency notefreq;

AudioConnection patchCord1(audioInput, 0, filter, 0);
// AudioConnection patchCord2(audioInput, 1, filter, 1);

AudioConnection patchCord2(audioInput, 0, notefreq, 0);

AudioConnection patchCord3(filter, 0, audioOutput, 0);
// AudioConnection patchCord4(filter, 1, audioOutput, 1);

AudioControlSGTL5000 audioShield;

void setup() {
  AudioMemory(32);
  // Enable the audio shield and set the output volume.
  notefreq.begin(.9);
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);
}

void loop(){
  if (notefreq.available()) {
    float note = notefreq.read();
    filter.update_main_freq((uint16_t)note);
    Serial.printf("Note: %3.2f", note);
    Serial.println();
  } 
}
