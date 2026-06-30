// Native host simulator for the ATtiny RGB drift logic.
//
// Build & run:
//     g++ -std=c++17 -O2 main.cpp -o sim && ./sim
//
// The Arduino functions (pinMode/analogWrite/random/delay) are stubbed below so
// the same algorithm code that runs on the ATtiny can run unmodified here. The
// only difference: analogWrite() just goes to stdout via main() instead of
// hardware PWM.
//
// Workflow: tune the algorithm here, watch the values evolve, then copy the
// algorithm block back into ../src/blink.cpp.

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

// ─── Arduino API stubs ────────────────────────────────────────────────────
constexpr int OUTPUT = 1;

void pinMode(int /*pin*/, int /*mode*/) {}
void analogWrite(int /*pin*/, int /*val*/) {} // logging happens in main()

long random(long max)            { return std::rand() % max; }
long random(long min, long max)  { return min + std::rand() % (max - min); }

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
// ─── end stubs ────────────────────────────────────────────────────────────


// ─── Algorithm under test ─────────────────────────────────────────────────
// Copy this block into ../src/blink.cpp when you've got it behaving the way
// you want.

// loop iterations between ±1 steps. NOT milliseconds.
const int MIN_STEP_DELAY = 100;
const int MAX_STEP_DELAY = 15000;

// HSV outer bounds. Each WalkBounds picks a sub-range inside these.
const uint8_t MIN_SAT = 200;
const uint8_t MIN_VAL = 180;

// How many walks reuse the same WalkBounds before a new one is sampled.
const int MIN_WALKS_PER_BOUNDS = 3;
const int MAX_WALKS_PER_BOUNDS = 10;

// Burst gesture: short, dramatic flashing.
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

struct WalkBounds {
    int     walksLeft = 0;
    int     minDelay = MIN_STEP_DELAY;
    int     maxDelay = MAX_STEP_DELAY;
    uint8_t minSat   = MIN_SAT;
    uint8_t maxSat   = 255;
    uint8_t minVal   = MIN_VAL;
    uint8_t maxVal   = 255;
};

WalkBounds bounds;

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

// Pin constants — only the sim references them through the analogWrite stub
// so they're for documentation. On the ATtiny side these come from color_drift.cpp.
const int RED_PIN = 0, GREEN_PIN = 1, BLUE_PIN = 4;

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

void hueRandomWalk(Hue& hue)
{
    if (hue.current == hue.target) {
        if (bounds.walksLeft <= 0) pickNewWalkBounds();
        bounds.walksLeft--;

        hue.target     = random(256);
        hue.stepDelay  = random(bounds.minDelay, bounds.maxDelay);
        hue.stepWaited = 0;
        hue.direction  = random(2) ? -1 : 1;
        hue.saturation = random(bounds.minSat, bounds.maxSat);
        hue.value      = random(bounds.minVal, bounds.maxVal);
    }
    hue.stepWaited++;
    if (hue.stepWaited >= hue.stepDelay) {
        hue.current += hue.direction;
        hue.stepWaited = 0;

        uint8_t r, g, b;
        hsvToRgb(hue.current, hue.saturation, hue.value, r, g, b);
        analogWrite(RED_PIN, r);
        analogWrite(GREEN_PIN, g);
        analogWrite(BLUE_PIN, b);
    }
}

// One-shot dramatic gesture: rapid, non-smooth, vivid colour flashes with
// black gaps between. Blocking (uses delay()). ~1.8s total at default values.
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
// Not invoked from main() — keep here for comparison / quick switch-over.
// To use: replace hueRandomWalk(hue) in main()'s loop with three rgbRandomWalk
// calls on red/green/blue.

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
// ─── end algorithm ────────────────────────────────────────────────────────


// ─── Driver: runs the loop, prints state, controls simulation rate ────────
int main() {
    std::cout << std::unitbuf;   // flush every << so prints aren't lost on Ctrl-C
    std::srand(42);   // reproducible — change seed for variety

    constexpr int    SIM_HZ     = 100;
    constexpr double PRINT_FREQ = 10;

    constexpr long PRINT_EVERY_N = (long)(SIM_HZ / PRINT_FREQ) < 1
                                       ? 1
                                       : (long)(SIM_HZ / PRINT_FREQ);
    constexpr long PERIOD_US = 1000000L / SIM_HZ;

    std::cout << "  iter |   H |   S |   V |   R |   G |   B  |  tgt | delay\n";
    std::cout << "-------+-----+-----+-----+-----+-----+------+------+-------\n";

    int prevWalksLeft = -1;
    for (unsigned long iter = 0; ; iter++) {
        hueRandomWalk(hue);

        if (bounds.walksLeft > prevWalksLeft) {
            std::cout << "   ─── new WalkBounds: walks=" << (bounds.walksLeft + 1)
                      << "  delay=[" << bounds.minDelay << "," << bounds.maxDelay << "]"
                      << "  sat=[" << (int)bounds.minSat << "," << (int)bounds.maxSat << "]"
                      << "  val=[" << (int)bounds.minVal << "," << (int)bounds.maxVal << "]\n";
        }
        prevWalksLeft = bounds.walksLeft;

        if (iter % PRINT_EVERY_N == 0) {
            if (iter > 0) {
                std::cout << "       ⋮ (" << (PRINT_EVERY_N - 1) << " cycles skipped)\n";
            }
            // Compute the RGB the algorithm would have written, for display.
            uint8_t r, g, b;
            hsvToRgb(hue.current, hue.saturation, hue.value, r, g, b);
            std::cout << std::setw(6) << iter
                      << " | "  << std::setw(3) << (int)hue.current
                      << " | "  << std::setw(3) << (int)hue.saturation
                      << " | "  << std::setw(3) << (int)hue.value
                      << " | "  << std::setw(3) << (int)r
                      << " | "  << std::setw(3) << (int)g
                      << " | "  << std::setw(3) << (int)b
                      << "  | " << std::setw(4) << (int)hue.target
                      << " | " << std::setw(5) << hue.stepDelay
                      << "\n";
        }
        if (PERIOD_US > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(PERIOD_US));
        }
    }
}
