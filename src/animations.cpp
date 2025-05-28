#include <FastLED.h>
#include "animations.h"
#include "led_config.h"

void steadyColor(CRGB color, int duration_millis)
{
  fill_solid(leds, TOTAL_LEDS, color);
  FastLED.show();
  delay(duration_millis);
}

void transitionToHSV(uint8_t targetHue, uint8_t targetSat, uint8_t targetBright, int transition_millis)
{
  // Get current HSV values
  CHSV hsv;
  hsv = rgb2hsv_approximate(leds[0]);
  float startHue = hsv.hue;
  float startSat = hsv.sat;
  float startBright = hsv.val;

  unsigned long startTime = millis();

  while (millis() - startTime < transition_millis)
  {
    float progress = (float)(millis() - startTime) / transition_millis;

    // Calculate current values
    uint8_t currentHue = startHue + ((targetHue - startHue) * progress);
    uint8_t currentSat = startSat + ((targetSat - startSat) * progress);
    uint8_t currentBright = startBright + ((targetBright - startBright) * progress);

    // Apply new color
    fill_solid(leds, TOTAL_LEDS, CHSV(currentHue, currentSat, currentBright));
    FastLED.show();
    delay(20);
  }

  // Ensure we end up exactly at target color
  fill_solid(leds, TOTAL_LEDS, CHSV(targetHue, targetSat, targetBright));
  FastLED.show();
}

void slowColorDrift(int duration_seconds, int smoothness, int disorderChance)
{
  Serial.printf("SlowColorDrift duration_seconds: %d, smoothness: %d, disorderChance: %d\n", duration_seconds, smoothness, disorderChance);
  const unsigned long total_duration = duration_seconds * 1000UL;
  const unsigned long start_time = millis();
  unsigned long last_print = 0;

  // Disorder tracking
  bool is_disordered = false;
  bool strip_independent[NUM_STRIPS] = {false}; // Track which strips went rogue

  // Per-strip values for when they go independent
  struct StripState
  {
    uint8_t hue;
    uint8_t sat;
    uint8_t bright;
    int8_t hue_step; // Can be negative for reverse direction
  } strip_states[NUM_STRIPS];

  // Initialize all strips with same values
  uint8_t current_hue = random8();
  uint8_t saturation = random8(130, 255);
  uint8_t brightness = random8(100, 240);
  Serial.printf("Initial HSB: %d, %d, %d\n", current_hue, saturation, brightness);
  transitionToHSV(current_hue, saturation, brightness, smoothness);

  const int delay_time = smoothness + 7;
  unsigned long last_disorder_check = 0;

  while (millis() - start_time < total_duration)
  {
    // Check for disorder every second
    if (millis() - last_disorder_check >= 1000)
    {
      if (random8() < disorderChance * 2.55)
      { // Convert percentage to 0-255 range
        is_disordered = true;
        // Randomly select a strip to go rogue
        int strip_idx = random8(NUM_STRIPS);
        if (!strip_independent[strip_idx])
        {
          strip_independent[strip_idx] = true;
          strip_states[strip_idx] = {
              random8(),            // random hue
              random8(130, 255),    // random sat
              random8(100, 240),    // random bright
              (int8_t)random8(1, 4) // random speed
          };
          Serial.printf("Strip %d went rogue!\n", strip_idx);
        }
      }
      last_disorder_check = millis();
    }

    // Update colors
    current_hue++; // Main hue progression
    if (millis() - last_print >= 1000) {
      Serial.printf("Current hue: %d\n", current_hue);
      last_print = millis();
    }

    // Update all strips
    for (int i = 0; i < NUM_STRIPS; i++)
    {
      if (strip_independent[i])
      {
        // Update rogue strip
        strip_states[i].hue += strip_states[i].hue_step;
        strips[i]->fill_solid(CHSV(strip_states[i].hue,
                                   strip_states[i].sat,
                                   strip_states[i].bright));
      }
      else
      {
        // Normal strip
        strips[i]->fill_solid(CHSV(current_hue, saturation, brightness));
      }
    }

    FastLED.show();
    delay(delay_time);
  }
  // cleanup
  fill_solid(leds, TOTAL_LEDS, CRGB::Black);
  FastLED.show();
}
