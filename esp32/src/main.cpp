#include <FastLED.h>
#include "led_config.h"
#include "animations.h"
#include <esp_sleep.h>
#include "esp_pm.h"
#include <esp_wifi.h>

void preventSleepState()
{
  #ifdef ESP32C3
  // Disable all sleep wakeup sources to prevent the ESP32 from entering sleep mode
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  // Explicitly configure power managent and disable light sleep mode
  esp_pm_config_esp32c3_t pm_config;
  esp_err_t result = esp_pm_get_configuration(&pm_config); 
  if (result == ESP_OK) {
    Serial.print("Current light_sleep_enable: ");
    Serial.println(pm_config.light_sleep_enable);
    pm_config.light_sleep_enable = false;
    esp_pm_configure(&pm_config);
    Serial.println("Power management configure - light sleep disabled");
  }
  else {
    Serial.printf("Failed to get PM configuration: %s\n", esp_err_to_name(result));
  }

  // Keep RTC peripheral domain powered on during sleep (prevents RTC IO, sensors, ULP from powering down)
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  // Keep RTC slow memory powered on during sleep (preserves data in RTC slow memory)
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
  // Keep RTC fast memory powered on during sleep (preserves data in RTC fast memory)
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
  // Disable WiFi power saving (uncomment if you use WiFi)
  esp_wifi_set_ps(WIFI_PS_NONE);
  #endif
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");
  // preventSleepState();
  pinMode(ONBOARD_LED, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
  random16_add_entropy(analogRead(A0));
  steadyColor(CRGB::Purple, 200); // sanity validation
  Serial.println("Setup complete");
}

void loop()
{
  Serial.println("Loop");
  setOnboardLed(true);
  slowColorDrift(29, random(100, 1001));
  setOnboardLed(false);
}
