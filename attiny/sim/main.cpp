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

uint8_t currentRed    = 0;
uint8_t targetRed     = 0;
uint8_t stepDelayRed  = 5;
uint8_t stepWaitedRed = 0;

void redLoop() {
    if (currentRed == targetRed) {
        targetRed     = random(256);
        stepDelayRed  = random(1, 100);
        stepWaitedRed = 0;
    }

    stepWaitedRed++;
    if (stepWaitedRed >= stepDelayRed) {
        if (currentRed < targetRed) currentRed++;
        else                        currentRed--;
        analogWrite(RED_PIN, currentRed);
        stepWaitedRed = 0;
    }
}
// ─── end algorithm ────────────────────────────────────────────────────────


// ─── Driver: runs the loop, prints state, controls simulation rate ────────
int main() {
    std::srand(42);   // reproducible — change seed for variety

    constexpr int SIM_HZ = 100;   // how fast to step the simulation
    constexpr int PERIOD_MS = 1000 / SIM_HZ;

    std::cout << "  iter | curRed | target | stepDelay | waited\n";
    std::cout << "-------+--------+--------+-----------+-------\n";

    for (unsigned long iter = 0; ; iter++) {
        redLoop();
        std::cout << std::setw(6) << iter
                  << " | "  << std::setw(6) << (int)currentRed
                  << " | "  << std::setw(6) << (int)targetRed
                  << " | "  << std::setw(9) << (int)stepDelayRed
                  << " | "  << std::setw(6) << (int)stepWaitedRed
                  << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(PERIOD_MS));
    }
}
