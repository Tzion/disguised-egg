#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <FastLED.h>

extern CRGB leds[]; // Declare the color array from main.cpp

void steadyColor(CRGB color, int duration_seconds);

/**
 * Slowly transitions between colors over time
 * @param duration_seconds Total animation duration in seconds
 * @param smoothness Transition smoothness (0-255), higher = smoother = slower transitions
 */
void slowColorDrift(int duration_seconds, uint8_t smoothness = 128);

#endif // ANIMATIONS_H
