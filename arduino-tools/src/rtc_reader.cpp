#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

// Wiring (Nano <-> DS3231 mini):
//   Nano 5V  -> DS3231 VCC
//   Nano GND -> DS3231 GND
//   Nano A4  -> DS3231 SDA
//   Nano A5  -> DS3231 SCL

RTC_DS3231 rtc;

static void printTwoDigits(uint8_t v) {
    if (v < 10) Serial.print('0');
    Serial.print(v);
}

void setup() {
    Serial.begin(9600);
    while (!Serial) { ; }

    if (!rtc.begin()) {
        Serial.println(F("DS3231 not found — check wiring"));
        while (true) { delay(1000); }
    }

    if (rtc.lostPower()) {
        Serial.println(F("RTC lost power — setting from compile timestamp"));
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    } else {
        Serial.println(F("RTC already running — reading existing time"));
    }
}

void loop() {
    DateTime now = rtc.now();
    Serial.print(now.year()); Serial.print('-');
    printTwoDigits(now.month()); Serial.print('-');
    printTwoDigits(now.day());   Serial.print(' ');
    printTwoDigits(now.hour());  Serial.print(':');
    printTwoDigits(now.minute());Serial.print(':');
    printTwoDigits(now.second());Serial.println();
    delay(1000);
}
