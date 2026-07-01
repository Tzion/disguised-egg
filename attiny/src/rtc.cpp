// Software-I2C driver for the DS3231 RTC on the ATtiny85.
//
// Bit-banging instead of the USI hardware I2C peripheral because USI's SDA is
// fixed at PB0 — which the red LED occupies. Software lets us pick any pins;
// using PB2 (SCL) and PB3 (SDA) keeps the LED pins free.
//
// Wiring (DS3231 mini board has built-in 4.7K pull-ups, no external R needed):
//   ATtiny PB3 (physical pin 2)  →  DS3231 SDA
//   ATtiny PB2 (physical pin 7)  →  DS3231 SCL
//   ATtiny VCC, GND              →  DS3231 VCC, GND

#include "rtc.h"

static const uint8_t SDA_PIN = 3;   // PB3
static const uint8_t SCL_PIN = 2;   // PB2
static const uint8_t DS3231_ADDR = 0x68;

// Open-drain pattern: keep PORTB bits low; toggle DDR to either drive the line
// low (DDR set) or release it to high-Z (DDR cleared → external pull-up wins).
static inline void sdaHigh() { DDRB &= ~_BV(SDA_PIN); }
static inline void sdaLow()  { DDRB |=  _BV(SDA_PIN); }
static inline void sclHigh() { DDRB &= ~_BV(SCL_PIN); }
static inline void sclLow()  { DDRB |=  _BV(SCL_PIN); }
static inline bool sdaRead() { return (PINB >> SDA_PIN) & 1; }

// ~5 µs half-period → ~100 kHz bus. DS3231 supports up to 400 kHz; staying
// slow makes the bit-bang robust on a breadboard.
static inline void halfBit() { delayMicroseconds(5); }

static void i2cStart() {
    sdaHigh();
    sclHigh();
    halfBit();
    sdaLow();
    halfBit();
    sclLow();
}

static void i2cStop() {
    sdaLow();
    halfBit();
    sclHigh();
    halfBit();
    sdaHigh();
    halfBit();
}

static bool i2cWriteByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80) sdaHigh(); else sdaLow();
        halfBit();
        sclHigh();
        halfBit();
        sclLow();
        byte <<= 1;
    }
    // Read the slave's ACK bit
    sdaHigh();
    halfBit();
    sclHigh();
    halfBit();
    bool ack = !sdaRead();   // 0 from slave = ACK
    sclLow();
    return ack;
}

static uint8_t i2cReadByte(bool sendAck) {
    uint8_t byte = 0;
    sdaHigh();   // release SDA so the slave can drive it
    for (uint8_t i = 0; i < 8; i++) {
        halfBit();
        sclHigh();
        halfBit();
        byte = (byte << 1) | sdaRead();
        sclLow();
    }
    // Master sends ACK (low) or NACK (high) — NACK on the last byte tells the
    // slave to stop sending.
    if (sendAck) sdaLow(); else sdaHigh();
    halfBit();
    sclHigh();
    halfBit();
    sclLow();
    sdaHigh();
    return byte;
}

static inline uint8_t bcd2dec(uint8_t b) {
    return (b >> 4) * 10 + (b & 0x0F);
}

void rtcInit() {
    // PORTB bits for SDA/SCL stay low so DDR-set = drive low.
    PORTB &= ~(_BV(SDA_PIN) | _BV(SCL_PIN));
    // Start released (high-Z); external pull-ups bring the lines high.
    DDRB  &= ~(_BV(SDA_PIN) | _BV(SCL_PIN));
}

Time readRtc() {
    Time t = {};   // zero-initialise; valid = false until we succeed

    i2cStart();
    if (!i2cWriteByte((DS3231_ADDR << 1) | 0)) { i2cStop(); return t; }
    if (!i2cWriteByte(0x00))                   { i2cStop(); return t; }
    i2cStart();
    if (!i2cWriteByte((DS3231_ADDR << 1) | 1)) { i2cStop(); return t; }

    t.s    = bcd2dec(i2cReadByte(true)  & 0x7F);  // mask CH bit
    t.m    = bcd2dec(i2cReadByte(true));
    t.h    = bcd2dec(i2cReadByte(true)  & 0x3F);  // mask 12/24 + AM/PM bits
    t.dow  =         i2cReadByte(true);
    t.day  = bcd2dec(i2cReadByte(true));
    t.mon  = bcd2dec(i2cReadByte(true)  & 0x1F);  // mask century bit
    t.year = 2000 + bcd2dec(i2cReadByte(false));  // NACK = last byte

    i2cStop();
    t.valid = true;
    return t;
}
