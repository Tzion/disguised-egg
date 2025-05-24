#ifndef LED_CONFIG_H
#define LED_CONFIG_H

#include <FastLED.h>

#ifdef ESP32S2
  #define LED_PIN 9
  #define ONBOARD_LED 15
  #pragma message "Using ESP32-S2 board configuration"
#elif defined(ESP32C3)
  #define LED_PIN 3
  #define ONBOARD_LED 8
  #pragma message "Using ESP32-C3 board configuration"
#endif

#define NUM_STRIPS 1
#define LEDS_PER_STRIP 6
#define TOTAL_LEDS (NUM_STRIPS * LEDS_PER_STRIP)

extern CRGB leds[TOTAL_LEDS];
extern CRGBSet stripLeft;
extern CRGBSet stripFront;
extern CRGBSet stripRight;
extern CRGBSet stripBack;
extern CRGBSet *strips[NUM_STRIPS];

#endif


inline void setOnboardLed(bool on){
  #ifdef ESP32S2
    digitalWrite(ONBOARD_LED, on);
  #elif defined(ESP32C3)
    digitalWrite(ONBOARD_LED, !on); // C3 onboard led is active low
  #endif
}