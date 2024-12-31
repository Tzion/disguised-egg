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
  steadyColor(CRGB::Purple, 40); // sanity validation
}

void loop()
{
  Serial.println("Loop");
  steadyColor(CRGB::Green, 40); // sanity validation
  digitalWrite(ONBOARD_LED, HIGH);
  slowColorDrift(18, random8());
  digitalWrite(ONBOARD_LED, LOW);
  delay(2000);
}
