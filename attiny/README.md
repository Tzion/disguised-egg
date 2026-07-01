# ATtiny85

Code that runs on the ATtiny85 chip. Programmed via the Arduino Nano running ArduinoISP
firmware — for the wiring between Nano and ATtiny and the cap rule, see
[`../arduino-tools/README.md#wiring-nano-as-isp--to-attiny85`](../arduino-tools/README.md#wiring-nano-as-isp--to-attiny85).

## Pinout

Top view, notch / dot at the **pin-1 end** (left side):

```
                +----U----+
   RESET    1 --|         |-- 8    VCC
   PB3 (3)  2 --|         |-- 7    PB2 (2)   SCK
   PB4 (4)  3 --|         |-- 6    PB1 (1)   MISO
   GND      4 --|         |-- 5    PB0 (0)   MOSI
                +---------+
```

Numbers on the **outside** are physical pin numbers (1–8 on the chip).
Numbers in **parentheses** are Arduino code-pin numbers (what you pass to
`pinMode`, `digitalWrite`, `analogWrite`).

## Pin reference

| Physical | Code | PWM | ISP role | Notes |
|---:|:---:|:---:|---|---|
| 1 | 5 | — | **RESET** | Don't use as I/O. Held low during programming. |
| 2 | 3 | — | — | General-purpose I/O. No hardware PWM. |
| 3 | 4 | ✓ | — | General-purpose I/O. Hardware PWM (OC1B). |
| 4 | — | — | — | **GND** |
| 5 | 0 | ✓ | **MOSI** | Hardware PWM (OC0A). Used by ISP — LEDs here flicker during upload. |
| 6 | 1 | ✓ | **MISO** | Hardware PWM (OC0B). Used by ISP. |
| 7 | 2 | — | **SCK** | No hardware PWM. Used by ISP. |
| 8 | — | — | — | **VCC** (2.7–5.5 V) |

PWM-capable pins (work with `analogWrite`): **0, 1, 4**.

## Wiring — DS3231 RTC (software I²C)

We can't use hardware I²C (USI) because it puts SDA on PB0, which is the red LED.
Bit-banged on PB2/PB3 keeps the LED pins free. See `src/rtc.cpp`.

| ATtiny pin | DS3231 pin | |
|---|---|---|
| PB3 — physical pin 2 | SDA | I²C data |
| PB2 — physical pin 7 | SCL | I²C clock |
| VCC | VCC | 3.3 V or 5 V |
| GND | GND | |

DS3231 mini boards have built-in 4.7 kΩ pull-ups on SDA/SCL — no external
resistors needed. Coin cell keeps time when the ATtiny is unpowered.

### ⚠️ SCL conflicts with ISP SCK during upload

**PB2 is both** the I²C `SCL` line AND the ISP programming `SCK` line. The
DS3231's 4.7 kΩ pull-up on SCL loads the SCK signal during ISP, slows the
edges, and causes intermittent `invalid device signature` errors.

**Before every ATtiny upload, disconnect the DS3231's SCL wire** (or just kill
the DS3231's VCC, which disables its pull-ups). Reconnect after upload.

## Notes

- **Don't ever touch `RSTDISBL` or `SPIEN` in `hfuse`** — clearing the wrong bit
  bricks the chip for ISP forever. Current fuses (`lfuse=0xE2 hfuse=0xDF efuse=0xFF`)
  are safe. See comment in [`platformio.ini`](platformio.ini).
- Max ~20 mA per pin (safe), ~40 mA per port total. Drive LEDs through ≥220 Ω resistors.
- Pins 0/1/2 work as normal I/O when not programming, but anything wired there will
  toggle briefly during code uploads (since avrdude drives the SPI lines).

## Build & upload

```bash
pio run -e attiny85 -t fuses    # one-time per chip
pio run -e attiny85 -t upload
```
