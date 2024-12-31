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
}

void loop()
{
  Serial.println("Loop");
  digitalWrite(ONBOARD_LED, HIGH);
  slowColorDrift(18, random8());
  digitalWrite(ONBOARD_LED, LOW);
  delay(2000);
}
