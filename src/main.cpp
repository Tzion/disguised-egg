#include <Arduino.h>
#include <FastLED.h>

#define LED_BUILTIN 15

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  Serial.println("Pulsar");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
