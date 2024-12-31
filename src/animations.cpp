#include <FastLED.h>
#include "animations.h"
#include "led_config.h"

void steadyColor(CRGB color, int duration_seconds)
{
  fill_solid(leds, TOTAL_LEDS, color);
  FastLED.show();
  delay(duration_seconds * 1000);
}
