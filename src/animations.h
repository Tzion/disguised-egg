#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <FastLED.h>

extern CRGB leds[]; // Declare the color array from main.cpp

void steadyColor(CRGB color, int duration_seconds);

void slowColorDrift(int duration_seconds, 
                   uint8_t hueSmooth,
                   uint8_t satSmooth,
                   uint8_t brightSmooth);

#endif // ANIMATIONS_H