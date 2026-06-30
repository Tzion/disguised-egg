#include <Arduino.h>

const uint8_t RED_PIN = 0;   // PB0, physical pin 5 (also MOSI — flickers during upload)
const uint8_t GREEN_PIN = 1; // PB1, physical pin 6 (also MISO — flickers during upload)
const uint8_t BLUE_PIN = 4;  // PB4, physical pin 3 — PWM only, no ISP conflict

// loop() iterations between ±1 steps. NOT milliseconds.
// Need `long` because values above 32767 overflow AVR's 16-bit `int`.
const long MIN_STEP_DELAY = 10000;
const long MAX_STEP_DELAY = 70000;

// HSV outer bounds. Each WalkBounds picks a sub-range inside these.
const uint8_t MIN_SAT = 180;   // below this colors lose identity (look gray)
const uint8_t MIN_VAL = 199; 

// How many walks reuse the same WalkBounds before a new one is sampled.
const int MIN_WALKS_PER_BOUNDS = 3;
const int MAX_WALKS_PER_BOUNDS = 24;

// Burst gesture: short, dramatic flashing. Each call randomises within
// these bounds. Worst case = MAX_STEPS × (MAX_VIVID + MAX_BLACK) ≈ 3.15s.
const int     MIN_BURST_STEPS    = 15;
const int     MAX_BURST_STEPS    = 35;
const int     MIN_BURST_VIVID_MS = 20;
const int     MAX_BURST_VIVID_MS = 60;
const int     MIN_BURST_BLACK_MS = 10;
const int     MAX_BURST_BLACK_MS = 30;
const uint8_t BURST_MIN_SAT      = 200;

// Heartbeat gesture: smooth red-pink lub-dub with a never-black baseline.
const int     MIN_HEARTBEAT_BPM     = 50;
const int     MAX_HEARTBEAT_BPM     = 90;
const int     MIN_HEARTBEAT_MS      = 5000;
const int     MAX_HEARTBEAT_MS      = 30000;
const uint8_t HEARTBEAT_HUE_MIN     = 220;   // 220–255: magenta-red → pink-red
const uint8_t HEARTBEAT_SAT_MIN     = 150;   // lower sat = pinker
const uint8_t HEARTBEAT_SAT_MAX     = 240;
const uint8_t HEARTBEAT_BASELINE_V  = 40;    // floor brightness — never goes to 0

struct Hue {
    uint8_t current = 0;
    uint8_t target = 0;
    int8_t  direction = 1;
    long    stepDelay = 1000;
    long    stepWaited = 0;
    uint8_t saturation = 255;
    uint8_t value = 255;
};

Hue hue;

// Per-walk param bounds. Refreshed every walksLeft walks via pickNewWalkBounds().
struct WalkBounds {
    int     walksLeft = 0;            // 0 → pick new bounds at next walk start
    long    minDelay = MIN_STEP_DELAY;
    long    maxDelay = MAX_STEP_DELAY;
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

    long delaySample1 = random(MIN_STEP_DELAY, MAX_STEP_DELAY);
    long delaySample2 = random(MIN_STEP_DELAY, MAX_STEP_DELAY);
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
        // Start a new walk: pick its target/direction/phase params
        hue.target     = random(256);
        hue.stepDelay  = random(bounds.minDelay, bounds.maxDelay);
        hue.stepWaited = 0;
        hue.direction  = random(2) ? -1 : 1;
        hue.saturation = random(bounds.minSat, bounds.maxSat);
        hue.value      = random(bounds.minVal, bounds.maxVal);

        // Drift current → target one step at a time
        while (hue.current != hue.target)
        {
            hue.stepWaited++;
            if (hue.stepWaited >= hue.stepDelay)
            {
                hue.current += hue.direction;  // uint8_t wraps around the colour wheel
                hue.stepWaited = 0;

                uint8_t r, g, b;
                hsvToRgb(hue.current, hue.saturation, hue.value, r, g, b);
                analogWrite(RED_PIN, r);
                analogWrite(GREEN_PIN, g);
                analogWrite(BLUE_PIN, b);
            }
        }

        bounds.walksLeft--;
    }
    pickNewWalkBounds();
}

// One-shot dramatic gesture: rapid, non-smooth, vivid colour flashes with
// black gaps between. Blocking (uses delay()). ~1.8s total at default values.
// Intended to be triggered from a higher level (e.g. an RTC event).
void burst()
{
    int steps   = random(MIN_BURST_STEPS,    MAX_BURST_STEPS);
    int vividMs = random(MIN_BURST_VIVID_MS, MAX_BURST_VIVID_MS);
    int blackMs = random(MIN_BURST_BLACK_MS, MAX_BURST_BLACK_MS);

    for (int i = 0; i < steps; i++) {
        uint8_t h = random(256);
        uint8_t s = random(BURST_MIN_SAT, 256);
        uint8_t r, g, b;
        hsvToRgb(h, s, 255, r, g, b);
        analogWrite(RED_PIN, r);
        analogWrite(GREEN_PIN, g);
        analogWrite(BLUE_PIN, b);
        delay(vividMs);

        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
        delay(blackMs);
    }
}

// Smoothly fade brightness (V) from fromV to toV over durationMs, holding
// hue and saturation constant. Used by heartbeat() for gentle pulses.
static void fadeV(uint8_t hue, uint8_t sat, uint8_t fromV, uint8_t toV, int durationMs)
{
    const int steps = 20;
    int stepMs = durationMs / steps;
    if (stepMs < 1) stepMs = 1;

    uint8_t r, g, b;
    for (int i = 0; i <= steps; i++) {
        int v = (int)fromV + ((int)toV - (int)fromV) * i / steps;
        hsvToRgb(hue, sat, (uint8_t)v, r, g, b);
        analogWrite(RED_PIN, r);
        analogWrite(GREEN_PIN, g);
        analogWrite(BLUE_PIN, b);
        delay(stepMs);
    }
}

// Heartbeat gesture: gentle lub-dub in red-pink tones with a faint baseline
// glow that never goes fully dark. Randomised tempo, duration, hue, sat.
void heartbeat()
{
    const int     RISE_MS  = 80;
    const int     FALL_MS  = 120;
    const int     GAP_MS   = 70;
    const uint8_t LUB_PEAK = 255;
    const uint8_t DUB_PEAK = 180;
    const int     FIXED_MS = (RISE_MS + FALL_MS) * 2 + GAP_MS;

    int bpm     = random(MIN_HEARTBEAT_BPM, MAX_HEARTBEAT_BPM);
    int totalMs = random(MIN_HEARTBEAT_MS,  MAX_HEARTBEAT_MS);
    uint8_t hue = random(HEARTBEAT_HUE_MIN, 256);
    uint8_t sat = random(HEARTBEAT_SAT_MIN, HEARTBEAT_SAT_MAX);

    int beatPeriodMs = 60000 / bpm;
    int restMs       = beatPeriodMs - FIXED_MS;
    int numBeats     = totalMs / beatPeriodMs;
    if (numBeats < 1) numBeats = 1;

    // Settle at baseline before the first beat
    uint8_t r, g, b;
    hsvToRgb(hue, sat, HEARTBEAT_BASELINE_V, r, g, b);
    analogWrite(RED_PIN, r);
    analogWrite(GREEN_PIN, g);
    analogWrite(BLUE_PIN, b);

    for (int i = 0; i < numBeats; i++) {
        // LUB
        fadeV(hue, sat, HEARTBEAT_BASELINE_V, LUB_PEAK, RISE_MS);
        fadeV(hue, sat, LUB_PEAK, HEARTBEAT_BASELINE_V, FALL_MS);

        delay(GAP_MS);

        // DUB
        fadeV(hue, sat, HEARTBEAT_BASELINE_V, DUB_PEAK, RISE_MS);
        fadeV(hue, sat, DUB_PEAK, HEARTBEAT_BASELINE_V, FALL_MS);

        if (restMs > 0) delay(restMs);
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
    long    stepDelay = 1000;
    long    stepWaited = 0;
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
    if (random(100) < 2)  burst();
    if (random(100) < 2) heartbeat();
}
