#include <FastLED.h>
#include "animations.h"

extern CRGB leds[]; // Declare the color array from main.cpp

void steadyColor(CRGB* leds, int totalLeds, CRGB color, int duration_seconds)
{
  fill_solid(leds, totalLeds, color);
  FastLED.show();
  delay(duration_seconds * 1000);
}
