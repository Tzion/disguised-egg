#include <Arduino.h>

const uint8_t RED_PIN = 0;   // PB0, physical pin 5 (also MOSI — flickers during upload)
const uint8_t GREEN_PIN = 1; // PB1, physical pin 6 (also MISO — flickers during upload)
const uint8_t BLUE_PIN = 4;  // PB4, physical pin 3 — PWM only, no ISP conflict

// loop() iterations between ±1 steps. NOT milliseconds.
const int MIN_STEP_DELAY = 10000;
const int MAX_STEP_DELAY = 70000;

// HSV outer bounds. Each WalkBounds picks a sub-range inside these.
const uint8_t MIN_SAT = 180;   // below this colors lose identity (look gray)
const uint8_t MIN_VAL = 199; 

// How many walks reuse the same WalkBounds before a new one is sampled.
const int MIN_WALKS_PER_BOUNDS = 3;
const int MAX_WALKS_PER_BOUNDS = 24;

// Burst gesture: short, dramatic flashing of vivid colors.
const int     BURST_STEPS    = 30;
const int     BURST_VIVID_MS = 40;
const int     BURST_BLACK_MS = 20;
const uint8_t BURST_MIN_SAT  = 200;

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

// Per-walk param bounds. Refreshed every walksLeft walks via pickNewWalkBounds().
struct WalkBounds {
    int     walksLeft = 0;            // 0 → pick new bounds at next walk start
    int     minDelay = MIN_STEP_DELAY;
    int     maxDelay = MAX_STEP_DELAY;
    uint8_t minSat   = MIN_SAT;
    uint8_t maxSat   = 255;
    uint8_t minVal   = MIN_VAL;
    uint8_t maxVal   = 255;
};

WalkBounds bounds;

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

// Pick a fresh sub-range of bounds. Called when current bounds expire.
void pickNewWalkBounds()
{
    bounds.walksLeft = random(MIN_WALKS_PER_BOUNDS, MAX_WALKS_PER_BOUNDS);

    int delaySample1 = random(MIN_STEP_DELAY, MAX_STEP_DELAY);
    int delaySample2 = random(MIN_STEP_DELAY, MAX_STEP_DELAY);
    bounds.minDelay = (delaySample1 < delaySample2) ? delaySample1 : delaySample2;
    bounds.maxDelay = (delaySample1 < delaySample2) ? delaySample2 : delaySample1;

    int satSample1 = random(MIN_SAT, 256);
    int satSample2 = random(MIN_SAT, 256);
    bounds.minSat = (satSample1 < satSample2) ? satSample1 : satSample2;
    bounds.maxSat = (satSample1 < satSample2) ? satSample2 : satSample1;

    int valSample1 = random(MIN_VAL, 256);
    int valSample2 = random(MIN_VAL, 256);
    bounds.minVal = (valSample1 < valSample2) ? valSample1 : valSample2;
    bounds.maxVal = (valSample1 < valSample2) ? valSample2 : valSample1;
}

void hueRandomWalk(Hue &hue)
{
    while (bounds.walksLeft > 0)
    {
        // walk ended, pick a new target / direction / phase params (within current bounds)
        if (hue.current == hue.target)
        {
            bounds.walksLeft--;

            hue.target = random(256);
            hue.stepDelay = random(bounds.minDelay, bounds.maxDelay);
            hue.stepWaited = 0;
            hue.direction = random(2) ? -1 : 1;
            hue.saturation = random(bounds.minSat, bounds.maxSat);
            hue.value = random(bounds.minVal, bounds.maxVal);
        }
        hue.stepWaited++;
        if (hue.stepWaited >= hue.stepDelay)
        {
            hue.current += hue.direction; // uint8_t wraps around the colour wheel
            hue.stepWaited = 0;

            uint8_t r, g, b;
            hsvToRgb(hue.current, hue.saturation, hue.value, r, g, b);
            analogWrite(RED_PIN, r);
            analogWrite(GREEN_PIN, g);
            analogWrite(BLUE_PIN, b);
        }
    }
    pickNewWalkBounds();
}

// One-shot dramatic gesture: rapid, non-smooth, vivid colour flashes with
// black gaps between. Blocking (uses delay()). ~1.8s total at default values.
// Intended to be triggered from a higher level (e.g. an RTC event).
void burst()
{
    for (int i = 0; i < BURST_STEPS; i++) {
        uint8_t h = random(256);
        uint8_t s = random(BURST_MIN_SAT, 256);
        uint8_t r, g, b;
        hsvToRgb(h, s, 255, r, g, b);
        analogWrite(RED_PIN, r);
        analogWrite(GREEN_PIN, g);
        analogWrite(BLUE_PIN, b);
        delay(BURST_VIVID_MS);

        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
        delay(BURST_BLACK_MS);
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

// Simple functions for sanity testing

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
    burst();
}
