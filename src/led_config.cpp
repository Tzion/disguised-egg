#include "led_config.h";

CRGB leds[TOTAL_LEDS];
CRGBSet strip = CRGBSet(leds, LEDS_PER_STRIP);
CRGBSet *strips[NUM_STRIPS] = {&strip};

// Initialize strips array
void initStrips()
{
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    strips[i] = new CRGBSet(leds + (i * LEDS_PER_STRIP), LEDS_PER_STRIP);
  }
}