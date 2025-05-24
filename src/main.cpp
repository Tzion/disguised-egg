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

void loop_old()
{
  Serial.println("Loop");
  setOnboardLed(true);
  slowColorDrift(29, random(100, 1001));
  setOnboardLed(false);
  delay(2000);
}

void Fire2012WithPalette(uint8_t *heat)
{
  for (int i = 0; i < TOTAL_LEDS; i++)
  {
    heat[i] = qsub8(heat[i], random8(0, 3));
    int16_t tmpHeat = heat[i] + random8(0, 5);
    heat[i] = qadd8(heat[i], tmpHeat > 255 ? 255 : tmpHeat);
    leds[i] = ColorFromPalette(currentPalette, heat[i]);
  }
}

void loop()
  {
    Serial.println("Rainbow");
    static uint8_t startIndex = 0;
    fill_rainbow(leds, TOTAL_LEDS, startIndex, 5);
    FastLED.show();
    startIndex++;
    delay(10000);

  Serial.println("Fire");
    static uint8_t heat[TOTAL_LEDS];
    Fire2012WithPalette(heat);
    FastLED.show();
    delay(10000);

    Serial.println("Noise");
    uint16_t time = millis() / 5;
    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      uint8_t noise = inoise8(i * 20, time);
      leds[i] = ColorFromPalette(currentPalette, noise);
    }
    FastLED.show();
    delay(10000);
  }
