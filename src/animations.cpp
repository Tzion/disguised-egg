#include <FastLED.h>
#include "animations.h"
#include "led_config.h"

void steadyColor(CRGB color, int duration_millis)
{
  fill_solid(leds, TOTAL_LEDS, color);
  FastLED.show();
  delay(duration_millis);
}

void slowColorDrift(int duration_seconds, int smoothness)
{
  Serial.printf("SlowColorDrift duration_seconds: %d, smoothness: %d\n", duration_seconds, smoothness);
  const unsigned long total_duration = duration_seconds * 1000UL;
  const unsigned long start_time = millis();
  unsigned long last_print = 0;  // Track last print time


  // Random initial values
  uint8_t current_hue = random8();
  uint8_t saturation = random8(130, 256);
  uint8_t brightness = random8(100, 240);
  Serial.printf("Initial HSB: %d, %d, %d\n", current_hue, saturation, brightness);

  const int min_delay = 7; // minimum delay to maintain LED refresh
  const int delay_time = smoothness + min_delay;

  while (millis() - start_time < total_duration)
  {
    float current_progress = (float)(millis() - start_time);
    current_hue = (uint8_t)(current_hue + 1);
    
     if (millis() - last_print >= 1000) {
      Serial.printf("Current hue: %d\n", current_hue);
      last_print = millis();
    }

    CRGB color = CHSV(current_hue, saturation, brightness);
    fill_solid(leds, TOTAL_LEDS, color);
    FastLED.show();
    delay(delay_time);
  }
}