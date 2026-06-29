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
