#include <FastLED.h>
#include "led_config.h"
#include "animations.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(ONBOARD_LED, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
  random16_add_entropy(analogRead(A0));
  steadyColor(CRGB::Purple, 200); // sanity validation
}

void loop()
{
  Serial.println("Loop");
  setOnboardLed(true);
  slowColorDrift(29, random(100, 1001));
  setOnboardLed(false);
  delay(1000);
}
