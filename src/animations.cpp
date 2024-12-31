#include <FastLED.h>
#include "animations.h"
#include "led_config.h"

void steadyColor(CRGB color, int duration_seconds)
{
  fill_solid(leds, TOTAL_LEDS, color);
  FastLED.show();
  delay(duration_seconds * 1000);
}

void slowColorDrift(int duration_seconds, uint8_t smoothness)
{
  Serial.printf("SlowColorDrift duration_seconds: %d, smoothness: %d\n", duration_seconds, smoothness);
  const unsigned long total_duration = duration_seconds * 1000UL;
  const unsigned long start_time = millis();
  const float hue_change_per_ms = 255.0f / total_duration;

  // Random initial values
  uint8_t current_hue = random8();
  uint8_t saturation = random8();
  uint8_t brightness = random8();

  const int min_delay = 7; // minimum delay to maintain LED refresh
  const int delay_time = (smoothness * 1000 / 255) + min_delay;
  // Adjust hue change based on smoothness
  const float hue_step = 1.0f / smoothness;  // Smaller steps for higher smoothness

  // const float hue_step = 1.0f / smoothness;  // Smaller steps for higher smoothness
  // const float hue_step = 1.0f + (255 - smoothness) / 32.0f; // Map smoothness to step size (inverse relationship)

  while (millis() - start_time < total_duration)
  {
    float current_progress = (float)(millis() - start_time);
    current_hue = (uint8_t)(current_progress * hue_change_per_ms);
    
    CRGB color = CHSV(current_hue, saturation, brightness);
    fill_solid(leds, TOTAL_LEDS, color);
    FastLED.show();
    delay(delay_time); // Longer delay for even smoother transition
  }
}