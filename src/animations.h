#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <FastLED.h>

extern CRGB leds[]; // Declare the color array from main.cpp

void steadyColor(CRGB color, int duration_millis);

void slowColorDrift(int duration_seconds, 
                   uint8_t hueSmooth);

#endif // ANIMATIONS_H