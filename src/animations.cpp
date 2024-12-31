#include <FastLED.h>
#include "animations.h"
#include "led_config.h"

void steadyColor(CRGB color, int duration_millis)
{
  fill_solid(leds, TOTAL_LEDS, color);
  FastLED.show();
  delay(duration_millis);
}

void slowColorDrift(int duration_seconds, uint8_t smoothness)
{
  Serial.printf("SlowColorDrift duration_seconds: %d, smoothness: %d\n", duration_seconds, smoothness);
  const unsigned long total_duration = duration_seconds * 1000UL;
  const unsigned long start_time = millis();
  unsigned long last_print = 0;  // Track last print time


  // Random initial values
  uint8_t current_hue = random8();
  uint8_t saturation = random8();
  uint8_t brightness = random8();

  const float hue_step = 1.0f / smoothness;  // Smaller steps for higher smoothness

  while (millis() - start_time < total_duration)
  {
    float current_progress = (float)(millis() - start_time);
    current_hue = (uint8_t)(current_hue + hue_step);
    
     if (millis() - last_print >= 1000) {
      Serial.printf("Current hue: %d\n", current_hue);
      last_print = millis();
    }

    CRGB color = CHSV(current_hue, saturation, brightness);
    fill_solid(leds, TOTAL_LEDS, color);
    FastLED.show();
    delay(15);
  }
}