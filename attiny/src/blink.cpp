#include <Arduino.h>

const uint8_t RED_PIN = 0;   // PB0, physical pin 5 (also MOSI — flickers during upload)
const uint8_t GREEN_PIN = 1; // PB1, physical pin 6 (also MISO — flickers during upload)
const uint8_t BLUE_PIN = 4;  // PB4, physical pin 3 — PWM only, no ISP conflict

// loop() iterations between ±1 steps. NOT milliseconds.
const int MIN_STEP_DELAY = 100;
const int MAX_STEP_DELAY = 3000;

// HSV random bounds. Lower = pastel/dim, higher = vivid/bright.
const uint8_t MIN_SAT = 180;   // below this colors lose identity (look gray)
const uint8_t MIN_VAL = 199;   // below this the LED reads as off

struct Hue {
    uint8_t current = 0;
    uint8_t target = 0;
    int8_t  direction = 1;
    int     stepDelay = 1000;
    int     stepWaited = 0;
    uint8_t saturation = 255;
    uint8_t value = 255;
};

Hue hue;

uint8_t counter;

void setup()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
}

// 8-bit HSV→RGB. h/s/v all 0–255. Standard integer algorithm.
void hsvToRgb(uint8_t h, uint8_t s, uint8_t v,
              uint8_t& r, uint8_t& g, uint8_t& b)
{
    uint8_t region    = h / 43;
    uint8_t remainder = (h - region * 43) * 6;

    uint8_t p = ((uint16_t)v * (255 - s)) >> 8;
    uint8_t q = ((uint16_t)v * (255 - (((uint16_t)s * remainder) >> 8))) >> 8;
    uint8_t t = ((uint16_t)v * (255 - (((uint16_t)s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
}

void hueRandomWalk(Hue& h)
{
    // walk ended, pick a new target and step delay, and direction
    if (h.current == h.target) {
        h.target     = random(256);
        h.stepDelay  = random(MIN_STEP_DELAY, MAX_STEP_DELAY);
        h.stepWaited = 0;
        h.direction  = random(2) ? -1 : 1;
        h.saturation = random(MIN_SAT, 256);
        h.value      = random(MIN_VAL, 256);
    }
    h.stepWaited++;
    if (h.stepWaited >= h.stepDelay) {
        h.current += h.direction;     // uint8_t wraps around the colour wheel
        h.stepWaited = 0;

        uint8_t r, g, b;
        hsvToRgb(h.current, h.saturation, h.value, r, g, b);
        analogWrite(RED_PIN, r);
        analogWrite(GREEN_PIN, g);
        analogWrite(BLUE_PIN, b);
    }
}

// ─── Alternative algorithm: independent random walks in raw RGB space ─────
// Not invoked from loop() — keep here for comparison / quick switch-over.
// To use: replace hueRandomWalk(hue) in loop() with three rgbRandomWalk calls.

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

void rgbRandomWalk(Color& c)
{
    if (c.current == c.target) {
        c.target     = random(256);
        c.stepDelay  = random(MIN_STEP_DELAY, MAX_STEP_DELAY);
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
    hueRandomWalk(hue);
}
