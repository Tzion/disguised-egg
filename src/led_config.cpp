#include "led_config.h";

CRGB leds[TOTAL_LEDS];
CRGBSet stripLeft(leds, LEDS_PER_STRIP);
CRGBSet stripFront(leds + LEDS_PER_STRIP, LEDS_PER_STRIP);
CRGBSet stripRight(leds + (2 * LEDS_PER_STRIP), LEDS_PER_STRIP);
CRGBSet stripBack(leds + (3 * LEDS_PER_STRIP), LEDS_PER_STRIP);
CRGBSet* strips[NUM_STRIPS] = {&stripLeft, &stripFront, &stripRight, &stripBack};

