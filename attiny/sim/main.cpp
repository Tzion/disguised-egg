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
// `redLoop()` body back into ../src/blink.cpp.

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
// you want. Keep the signatures (uint8_t, no `Serial.print`, etc.) identical
// so the copy-paste is one-step.

const int RED_PIN = 0;

uint8_t desiredRed;
uint8_t curRed;
int waitingPeriodRed;
int stepWaitedRed;
int directionRed;

void redLoop() {
    if (curRed == desiredRed)
    {
        desiredRed = random(256);
        waitingPeriodRed = random(100, 1500);
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

    std::cout << "  iter | curRed | desiredRed | stepDelay | waited\n";
    std::cout << "-------+--------+--------+-----------+-------\n";

    for (unsigned long iter = 0; ; iter++) {
        redLoop();

        if (iter % PRINT_EVERY_N == 0) {
            if (iter > 0) {
                std::cout << "       ⋮ (" << (PRINT_EVERY_N - 1) << " cycles skipped)\n";
            }
            std::cout << std::setw(6) << iter
                      << " | "  << std::setw(6) << (int)curRed
                      << " | "  << std::setw(6) << (int)desiredRed
                      << " | "  << std::setw(9) << (int)waitingPeriodRed
                      << " | "  << std::setw(6) << (int)stepWaitedRed
                      << "\n";
        }
        if (PERIOD_US > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(PERIOD_US));
        }
    }
}
