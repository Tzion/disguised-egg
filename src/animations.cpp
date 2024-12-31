#include <FastLED.h>
#include "animations.h"
#include "led_config.h"

void steadyColor(CRGB color, int duration_millis)
{
  fill_solid(leds, TOTAL_LEDS, color);
  FastLED.show();
  delay(duration_millis);
}

/**
 * Slowly transitions between colors over time
 * @param duration_seconds Total animation duration in seconds
 * @param hueSmooth Hue transition smoothness (0-255)
 * @param satSmooth Saturation transition smoothness (0-255)
 * @param brightSmooth Brightness transition smoothness (0-255)
 */
void slowColorDrift(int duration_seconds, 
                   uint8_t hueSmooth,
                   uint8_t satSmooth,
                   uint8_t brightSmooth) {
    Serial.printf("SlowColorDrift duration: %d, hue: %d, sat: %d, bright: %d\n", 
                 duration_seconds, hueSmooth, satSmooth, brightSmooth);
                 
    const unsigned long total_duration = duration_seconds * 1000UL;
    const unsigned long start_time = millis();
    
    // Change to float to handle fractional steps
    float current_hue = random8();
    float current_sat = random8();
    float current_bright = random8();
    
    const float hue_step = 1.0f + (255 - hueSmooth) / 1024.0f;
    const float sat_step = 0.0f;
    const float bright_step = 0.0f;
    
    while (millis() - start_time < total_duration) {
        // Add steps while keeping as float
        current_hue += hue_step;
        current_sat += sat_step;
        current_bright += bright_step;
        
        // Wrap around at 256
        if (current_hue >= 256) current_hue -= 256;
        if (current_sat >= 256) current_sat -= 256;
        if (current_bright >= 256) current_bright -= 256;
        
        // Convert to uint8_t only when creating color
        CRGB color = CHSV((uint8_t)current_hue, 
                         (uint8_t)current_sat, 
                         (uint8_t)current_bright);
        
        fill_solid(leds, TOTAL_LEDS, color);
        FastLED.show();
        delay(20);
    }
}