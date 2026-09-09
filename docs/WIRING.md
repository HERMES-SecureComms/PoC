# HERMES PoC v0.1 Wiring

This guide describes the default GPIO assignments in `firmware/include/config.h`.
Disconnect USB power while changing wires, and verify the pin labels for the
specific ESP32-S3 and PN532 boards before applying power.

## PN532 SPI connection

First configure the PN532 module for **SPI mode**. The switch, solder bridge, or
jumper combination differs between PN532 module vendors; check that module's
silkscreen and datasheet. Labels such as `SS`, `SSEL`, and `CS` commonly refer to
the same chip-select function.

| ESP32-S3 | PN532 | Function |
|---|---|---|
| 3.3V | VCC | Reader power (only if the module supports 3.3 V input) |
| GND | GND | Common ground |
| GPIO12 | SCK | SPI clock |
| GPIO13 | MISO | SPI reader-to-ESP32 data |
| GPIO11 | MOSI | SPI ESP32-to-reader data |
| GPIO10 | SS / SSEL / CS | SPI chip select |

Some PN532 carrier boards include regulators or level shifters and advertise a
different supply range. Do not infer this from the PN532 chip alone; follow the
carrier-board documentation and ensure its logic is compatible with 3.3 V.

## PTT button

The firmware uses `INPUT_PULLUP`, so an open button reads HIGH and a pressed
button connected to ground reads LOW.

| ESP32-S3 | Button | Function |
|---|---|---|
| GPIO4 | Terminal 1 | Active-low PTT input |
| GND | Terminal 2 | Ground when pressed |

Use opposite sides of a four-pin tactile switch rather than two internally
common legs on the same side. Firmware applies a 30 ms debounce interval.

## Status LEDs

Each LED needs its own 220–330 ohm series resistor. For a conventional LED, the
long leg is the anode and the flat-side/short leg is the cathode.

| ESP32-S3 | Series path | Function |
|---|---|---|
| GPIO6 | 220–330 ohm resistor → green LED anode; cathode → GND | TX allowed/enabled |
| GPIO7 | 220–330 ohm resistor → red LED anode; cathode → GND | PTT denied or latched PN532 fault |

The green LED is on only while simulated TX is enabled. The red LED turns on for
an unauthorized PTT press and remains on after a boot-time PN532 failure.

## TX gate test point

| ESP32-S3 | Destination | Function |
|---|---|---|
| GPIO5 | Logic analyzer, high-impedance multimeter, or unconnected test point | Simulated `TX_GATE` |
| GND | Instrument ground | Measurement reference |

Expected levels are HIGH only for an authorized, pressed PTT and LOW otherwise.
Software sets the GPIO output latch LOW before initializing the PN532. Because an
ESP32 pin can be high-impedance during reset, a future safety-related interface
must also provide an external hardware default-off path; firmware alone is not a
production interlock.

> **ESP32 GPIO를 실제 무전기의 PTT 라인에 직접 연결하지 말 것.**
>
> **Do not connect the ESP32 GPIO directly to a real radio PTT line.**

Real integration requires voltage, current, polarity, grounding, and keying-mode
measurements followed by a suitable optocoupler, transistor, MOSFET, or analog
switch design. That work is intentionally outside PoC v0.1.

## Pre-power checklist

1. PN532 is explicitly set to SPI mode.
2. PN532 supply and logic voltage match the exact module documentation.
3. ESP32-S3 and PN532 share ground.
4. Each LED has a separate series resistor and correct polarity.
5. The button connects GPIO4 to ground only when pressed.
6. GPIO5 is not connected to a radio or other load.
7. No configured GPIO is already used by an onboard peripheral on the selected
   ESP32-S3 development board.

