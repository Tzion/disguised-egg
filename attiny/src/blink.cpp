#include <Arduino.h>

const int RED_PIN = 0;   // PB0, physical pin 5 (also MOSI — flickers during upload)
const int GREEN_PIN = 1; // PB1, physical pin 6 (also MISO — flickers during upload)
const int BLUE_PIN = 4;  // PB4, physical pin 3 — PWM only, no ISP conflict

void setup()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
}

uint8_t counter;

void drift()
{
    counter = counter % 255;
    analogWrite(BLUE_PIN, counter);
    delay(10);
    counter++;
}

void blink()
{
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    delay(5000);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
    delay(1000);
}

uint8_t desiredRed;
uint8_t curRed;
int waitingPeriodRed;
int stepWaitedRed;
int directionRed;

void redLoop() {
    if (curRed == desiredRed)
    {
        desiredRed = random(256);
        waitingPeriodRed = random(1000, 30000);
        stepWaitedRed = 0;
        directionRed = random(2) ? -1 : 1;
    }
    stepWaitedRed++;
    if (stepWaitedRed >= waitingPeriodRed)
    {
        curRed += directionRed;
        analogWrite(RED_PIN, curRed);
        stepWaitedRed= 0;
    }
}
void loop()
{
    redLoop();
}
