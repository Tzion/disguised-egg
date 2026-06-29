#include <Arduino.h>

const int RED_PIN = 3; // Arduino code pin 3 == ATtiny85 physical pin 2
const int GREEN_PIN = 2;
const int BLUE_PIN = 4;

void setup() {
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
}

void loop() {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    delay(5000);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
    delay(1000);
}
