#pragma once
#include <Arduino.h>

struct Time {
    uint8_t  s;    // 0–59
    uint8_t  m;    // 0–59
    uint8_t  h;    // 0–23 (24h mode)
    uint8_t  dow;  // 1–7 (raw from DS3231)
    uint8_t  day;  // 1–31
    uint8_t  mon;  // 1–12
    uint16_t year; // 2000–2099
};

// Configure the I2C pins. Call once at startup.
void rtcInit();

// Read all 7 time/date bytes from the DS3231 at addr 0x68.
// Returns BCD-decoded values. Blocking, ~1.5 ms on a healthy bus at 8 MHz.
Time readRtc();
