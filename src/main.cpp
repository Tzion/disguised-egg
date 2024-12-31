#include <FastLED.h>
#include "led_config.h"
#include "animations.h"


void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(ONBOARD_LED, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
}

void loop()
{
  Serial.println("Loop");
  digitalWrite(ONBOARD_LED, HIGH);
  stripFront.fill_solid(CRGB::Yellow);
  stripBack.fill_solid(CRGB::Purple);
  FastLED.show();
  delay(4000);
  steadyColor(CRGB::Red, 5);   // Red for 5 seconds
  steadyColor(CRGB::Blue, 5);  // Blue for 5 seconds
  steadyColor(CRGB::Green, 5); // Green for 5 seconds
  digitalWrite(ONBOARD_LED, LOW);
  delay(2000);
}
