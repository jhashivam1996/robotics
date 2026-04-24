# HARDWARE.md

## Scope

This file contains wiring and setup notes for the `esp32-lcd` sketch.

Active sketch:

- `esp32-lcd.ino`

## Supported LCD Setup

This sketch is written for a bare `1602A` / `HD44780-compatible` 16-pin LCD in
4-bit parallel mode.

## Default Wiring

ESP32 DevKit V1 to LCD:

- `ESP32 GND -> LCD pin 1 (VSS)`
- `ESP32 5V -> LCD pin 2 (VDD)`
- `LCD pin 3 (VO) -> GND` if you do not have a potentiometer
- `ESP32 GPIO 14 -> LCD pin 4 (RS)`
- `ESP32 GND -> LCD pin 5 (RW)`
- `ESP32 GPIO 27 -> LCD pin 6 (E)`
- `LCD pin 7 (D0) -> not connected`
- `LCD pin 8 (D1) -> not connected`
- `LCD pin 9 (D2) -> not connected`
- `LCD pin 10 (D3) -> not connected`
- `ESP32 GPIO 26 -> LCD pin 11 (D4)`
- `ESP32 GPIO 25 -> LCD pin 12 (D5)`
- `ESP32 GPIO 33 -> LCD pin 13 (D6)`
- `ESP32 GPIO 32 -> LCD pin 14 (D7)`
- `5V through resistor -> LCD pin 15 (A)`
- `ESP32 GND -> LCD pin 16 (K)`

Potentiometer option:

- one side -> `GND`
- other side -> `3.3V` or `5V`
- middle pin -> `LCD pin 3 (VO)`

No-pot fallback:

- `LCD pin 3 (VO) -> GND`

This usually gives strong contrast and is the simplest way to get text visible
without a potentiometer. If the characters look too dark or the full row blocks
stay visible, add a small resistor between `VO` and `GND` or switch to a proper
10k potentiometer later.

If you change the data/control wiring, update these constants in `esp32-lcd.ino`:

- `LCD_RS_PIN`
- `LCD_ENABLE_PIN`
- `LCD_D4_PIN`
- `LCD_D5_PIN`
- `LCD_D6_PIN`
- `LCD_D7_PIN`

## Library Requirement

The sketch includes:

- `LiquidCrystal.h`

`LiquidCrystal.h` must be provided by an installed Arduino library.

## Notes

1. `RW` should stay tied to `GND`.
2. Only `D4-D7` are used in 4-bit mode.
3. If you have no potentiometer, tie `VO` to `GND` first.
4. If the backlight turns on but text is still unreadable, the next fix is a proper contrast potentiometer.
5. Many bare 1602A modules are happiest at `5V`, while ESP32 GPIO is `3.3V`. If it behaves unreliably, level shifting may be needed.
