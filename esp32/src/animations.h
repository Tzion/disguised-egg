#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <FastLED.h>

extern CRGB leds[]; // Declare the color array from main.cpp

void steadyColor(CRGB color, int duration_millis);

void slowColorDrift(int duration_seconds, 
                   int hueSmooth,
                   int disorderChance = 4);

void safeDelay(unsigned long ms);

#endif // ANIMATIONS_H
