# HARDWARE.md

## Scope

This file contains wiring notes for the `esp32-lcd-i2c` sketch.

Active sketch:

- `esp32-lcd-i2c.ino`

## Supported LCD Setup

This sketch is written for a `1602A` LCD with an `I2C backpack`.

Expected backpack pins:

- `GND`
- `VCC`
- `SDA`
- `SCL`

## Default Wiring

ESP32 DevKit V1 to LCD backpack:

- `ESP32 GND -> LCD GND`
- `ESP32 5V -> LCD VCC`
- `ESP32 GPIO 21 -> LCD SDA`
- `ESP32 GPIO 22 -> LCD SCL`

If your ESP32 board uses different I2C pins, update these constants in
`esp32-lcd-i2c.ino`:

- `LCD_SDA_PIN`
- `LCD_SCL_PIN`

## I2C Address

The sketch is hardcoded to:

- `0x27`

If your backpack uses a different address, update:

- `LCD_I2C_ADDRESS`

Common alternatives include `0x3F`.

## Library Requirement

The sketch includes:

- `Wire.h`
- `LiquidCrystal_I2C.h`

`Wire.h` comes from the ESP32 board core.
`LiquidCrystal_I2C.h` must be provided by an installed Arduino library.

## Notes

1. This is the preferred setup for ESP32 because it only needs 4 wires.
2. If the backlight turns on but no text appears, first check the backpack contrast trimmer.
3. If the display stays blank, verify the I2C address and SDA/SCL wiring.
