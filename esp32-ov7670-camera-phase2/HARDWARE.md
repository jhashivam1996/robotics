# HARDWARE.md

## Scope

This file contains wiring notes for the `esp32-ov7670-camera-phase2` sketch.

Active sketch:

- `esp32-ov7670-camera-phase2.ino`

## Goal

This phase-2 sketch assumes phase 1 already works.

It still uses SCCB over `SDA/SCL` and generates `XLK`, but now it also wires:

- `VS`
- `HS`
- `PLK`
- `D0-D7`

The sketch samples sync and data activity and prints timing/data summaries over
serial. It is a full-bus validation step, not full frame extraction.

## Board Label Mapping

- `SDA` on camera = SCCB data
- `SCL` on camera = SCCB clock
- `XLK` on camera = external clock input
- `VS` on camera = vertical sync
- `HS` on camera = horizontal sync / href
- `PLK` on camera = pixel clock

## Wiring

- `OV7670 VCC -> ESP32 3.3V`
- `OV7670 GND -> ESP32 GND`
- `OV7670 SDA -> ESP32 GPIO 18`
- `OV7670 SCL -> ESP32 GPIO 19`
- `OV7670 XLK -> ESP32 GPIO 21`
- `OV7670 VS -> ESP32 GPIO 27`
- `OV7670 HS -> ESP32 GPIO 14`
- `OV7670 PLK -> ESP32 GPIO 13`
- `OV7670 D0 -> ESP32 GPIO 36`
- `OV7670 D1 -> ESP32 GPIO 39`
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

`1k` to `4.7k` is typical.

## Notes

1. This sketch uses many more pins than phase 1, so it will not stay one-sided on a breadboard.
2. The XCLK is intentionally slower than phase 1 to make software polling more realistic.
3. If SCCB works but `VS/HS/PLK` show no transitions, focus on those three wires first.
