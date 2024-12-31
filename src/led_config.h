#ifndef LED_CONFIG_H
#define LED_CONFIG_H

#include <FastLED.h>

#define LED_PIN 7
#define ONBOARD_LED 15

#define NUM_STRIPS 4
#define LEDS_PER_STRIP 61
#define TOTAL_LEDS (NUM_STRIPS * LEDS_PER_STRIP)

extern CRGB leds[TOTAL_LEDS];
extern CRGBSet stripLeft;
extern CRGBSet stripFront;
extern CRGBSet stripRight;
extern CRGBSet stripBack;
extern CRGBSet *strips[NUM_STRIPS];

#endif