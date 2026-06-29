#include <Arduino.h>

const uint8_t RED_PIN = 0;   // PB0, physical pin 5 (also MOSI — flickers during upload)
const uint8_t GREEN_PIN = 1; // PB1, physical pin 6 (also MISO — flickers during upload)
const uint8_t BLUE_PIN = 4;  // PB4, physical pin 3 — PWM only, no ISP conflict

struct Color {
    uint8_t pin;
    uint8_t current = 0;
    uint8_t target = 0;
    int8_t  direction = 1;
    int     stepDelay = 1000;
    int     stepWaited = 0;
    Color(uint8_t p) : pin(p) {}
};

Color red{RED_PIN};
Color green{GREEN_PIN};
Color blue{BLUE_PIN};

uint8_t counter;

void setup()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
}

void colorRandomWalk(Color& c)
{
    if (c.current == c.target) {
        c.target     = random(256);
        c.stepDelay  = random(1000, 30000);
        c.stepWaited = 0;
        c.direction  = random(2) ? -1 : 1;
    }
    c.stepWaited++;
    if (c.stepWaited >= c.stepDelay) {
        c.current += c.direction;
        analogWrite(c.pin, c.current);
        c.stepWaited = 0;
    }
}

void drift(uint8_t pin)
{
    counter = counter % 255;
    analogWrite(pin, counter);
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

void loop()
{
    colorRandomWalk(red);
    colorRandomWalk(green);
    colorRandomWalk(blue);
}
