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

struct Color {
    uint8_t pin;
    uint8_t current = 0;
    uint8_t target = 0;
    int8_t  direction = 1;
    int     stepDelay = 1000;
    int     stepWaited = 0;
    Color(uint8_t p) : pin(p) {}
};

Color red{0};
Color green{1};
Color blue{4};

void colorRandomWalk(Color& c) {
    if (c.current == c.target) {
        c.target     = random(256);
        c.stepDelay  = random(100, 1500);
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

    // Target loop frequency, in Hz. Tune to roughly what the real ATtiny loop()
    // achieves — probably a few thousand Hz given the code's per-call cost.
    constexpr int SIM_HZ = 100;

    // How often to print, in Hz. 0.2 = 1 print every 5 seconds.
    // Use 1.0 for once per second, 10.0 for fast scrolling, etc.
    constexpr double PRINT_FREQ = 10;

    // Derived: iterations between prints, and per-iteration sleep duration.
    constexpr long PRINT_EVERY_N = (long)(SIM_HZ / PRINT_FREQ) < 1
                                       ? 1
                                       : (long)(SIM_HZ / PRINT_FREQ);
    constexpr long PERIOD_US = 1000000L / SIM_HZ;

    std::cout << "  iter |   R |   G |   B  |  tgt R/G/B  |  delay R/G/B\n";
    std::cout << "-------+-----+-----+------+-------------+----------------\n";

    for (unsigned long iter = 0; ; iter++) {
        colorRandomWalk(red);
        colorRandomWalk(green);
        colorRandomWalk(blue);

        if (iter % PRINT_EVERY_N == 0) {
            if (iter > 0) {
                std::cout << "       ⋮ (" << (PRINT_EVERY_N - 1) << " cycles skipped)\n";
            }
            std::cout << std::setw(6) << iter
                      << " | "  << std::setw(3) << (int)red.current
                      << " | "  << std::setw(3) << (int)green.current
                      << " | "  << std::setw(3) << (int)blue.current
                      << "  | "
                      << std::setw(3) << (int)red.target   << "/"
                      << std::setw(3) << (int)green.target << "/"
                      << std::setw(3) << (int)blue.target
                      << " | "
                      << std::setw(4) << red.stepDelay   << "/"
                      << std::setw(4) << green.stepDelay << "/"
                      << std::setw(4) << blue.stepDelay
                      << "\n";
        }
        if (PERIOD_US > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(PERIOD_US));
        }
    }
}
