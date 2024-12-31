#include <FastLED.h>
#include "animations.h"

#define LED_BUILTIN 15
#define LED_PIN 7

#define NUM_STRIPS 4
#define LEDS_PER_STRIP 61
#define TOTAL_LEDS (NUM_STRIPS * LEDS_PER_STRIP)

CRGB leds[TOTAL_LEDS];

void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(LED_BUILTIN, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
}

void loop()
{
  Serial.println("Loop");
  digitalWrite(LED_BUILTIN, HIGH);
  steadyColor(leds, TOTAL_LEDS, CRGB::Red, 5);   // Red for 5 seconds
  steadyColor(leds, TOTAL_LEDS, CRGB::Blue, 5);  // Blue for 5 seconds
  steadyColor(leds, TOTAL_LEDS, CRGB::Green, 5); // Green for 5 seconds
  digitalWrite(LED_BUILTIN, LOW);
}
