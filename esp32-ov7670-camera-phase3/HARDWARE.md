# HARDWARE.md

## Scope

This file contains wiring notes for the `esp32-ov7670-camera-phase3` sketch.

Active sketch:

- `esp32-ov7670-camera-phase3.ino`

## Goal

This phase-3 sketch captures a single `160x120` grayscale frame from the OV7670
and prints it as an ASCII PGM block over serial.

It assumes phase 1 and phase 2 are already working.

## Camera Board Labels

- `SDA` = SCCB data
- `SCL` = SCCB clock
- `XLK` = external clock input
- `VS` = vertical sync
- `HS` = horizontal sync / href
- `PLK` = pixel clock

## Wiring

- `OV7670 VCC -> ESP32 3.3V`
- `OV7670 GND -> ESP32 GND`
- `OV7670 SDA -> ESP32 GPIO 18`
- `OV7670 SCL -> ESP32 GPIO 19`
- `OV7670 XLK -> ESP32 GPIO 21`
- `OV7670 VS -> ESP32 GPIO 27`
- `OV7670 HS -> ESP32 GPIO 14`
- `OV7670 PLK -> ESP32 GPIO 13`
- `OV7670 D0 -> ESP32 VP`
- `OV7670 D1 -> ESP32 VN`
- `OV7670 D2 -> ESP32 GPIO 34`
- `OV7670 D3 -> ESP32 GPIO 35`
- `OV7670 D4 -> ESP32 GPIO 32`
- `OV7670 D5 -> ESP32 GPIO 33`
- `OV7670 D6 -> ESP32 GPIO 25`
- `OV7670 D7 -> ESP32 GPIO 26`
- `OV7670 RESET -> 3.3V`
- `OV7670 PWDN -> GND`

## Pull-Ups

If your board does not already have SCCB pull-ups:

- `SDA` pull-up to `3.3V`
- `SCL` pull-up to `3.3V`

## Notes

1. `VP` is ESP32 GPIO 36.
2. `VN` is ESP32 GPIO 39.
3. The sketch emits an ASCII PGM block between `BEGIN_PGM` and `END_PGM`.
4. This is a still-image milestone, not video streaming.
