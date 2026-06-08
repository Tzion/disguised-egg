#include <Arduino.h>

// Arduino code pin 3 == ATtiny85 physical pin 2
const int LED_PIN = 3;

void setup() {
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
}
