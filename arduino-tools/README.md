# Arduino tools (Nano sketches)

Sketches that get flashed directly onto the **Arduino Nano** over its USB cable.
Each `[env:...]` in `platformio.ini` is a different role the Nano can play —
flash whichever one you need at the moment.

| Env | Purpose | Flash to Nano when… |
|---|---|---|
| `isp_firmware` | ArduinoISP — turns the Nano into an in-circuit programmer for AVR chips (e.g. ATtiny85) | …you want to upload code to an ATtiny |
| `rtc_reader`   | Reads time from a DS3231 over I²C and prints it on Serial | …you want to verify the RTC is keeping time |

```bash
pio run -e <env> -t upload          # flash that role onto the Nano
pio device monitor -b 9600          # for rtc_reader only — view the time stream
```

## Wiring (Nano as ISP) — to ATtiny85

This is the wiring used when the Nano has `isp_firmware` loaded and is being
used to flash code onto an ATtiny85.

```
   ARDUINO NANO                       ATtiny85 (top view, notch on left)
   ┌────────────┐                   ┌─────────┐
   │         5V ├──────────────────►│8  VCC   │
   │        GND ├──────────────────►│4  GND   │
   │            │                   │         │
   │       D10  ├──────────────────►│1  RESET │
   │  D11  MOSI ├──────────────────►│5  PB0   │
   │  D12  MISO |◄-─────────────────┤6  PB1   │
   │  D13  SCK  ├──────────────────►│7  PB2   │
   │            │                   │         │
   │ RESET ──┤(+)10µF(-)├── GND     │  pin 2,3 leave open
   └────────────┘                   └─────────┘
```

| Nano | ATtiny85 pin | Function |
|---|---|---|
| 5V | 8 | VCC |
| GND | 4 | GND |
| D10 | 1 | RESET |
| D11 | 5 | MOSI |
| D12 | 6 | MISO |
| D13 | 7 | SCK |

### ⚠️ Things that kill the chip — verify before powering

1. **VCC/GND reversal** is the #1 ATtiny killer. Pin 8 is VCC, pin 4 is GND —
   diagonally opposite. The notch on the chip marks the pin-1 end. Count pins
   from the right end and you'll fry it in under a second.
2. **5 V only** — never feed Vin or anything >5.5 V into pin 8.
3. **No load > 20 mA per pin** — drive LEDs through a resistor (≥220 Ω).

### The 10 µF capacitor between Nano RESET and GND

Required when the Nano is being used as ISP; **must be removed** when you flash
a new sketch onto the Nano itself.

| You're doing… | Cap state | Why |
|---|---|---|
| `pio run -e isp_firmware -t upload` (flashing the Nano) | **REMOVED** | The cap blocks the auto-reset the bootloader needs |
| `pio run -e attiny85 -t upload` (flashing the ATtiny, from `../attiny/`) | **INSTALLED** | The cap stops the Nano from resetting itself when avrdude opens the port |
| `pio run -e rtc_reader -t upload` (flashing the Nano) | **REMOVED** | Same reason as the first row |

Long leg (+) of the cap goes to RESET, short leg (–) to GND.

## Wiring (DS3231 mini) — to Nano

For `rtc_reader`:

| Nano | DS3231 mini | |
|---|---|---|
| 5V  | VCC | |
| GND | GND | |
| A4  | SDA | I²C data |
| A5  | SCL | I²C clock |

A coin cell on the DS3231 module keeps time when the Nano is unpowered.
