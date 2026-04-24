# HARDWARE.md

## Scope

This file contains wiring notes for the `esp32-ov7670-camera` sketch.

Active sketch:

- `esp32-ov7670-camera.ino`

## Assumption

This project assumes your module is an `OV7670` camera without FIFO.

The current sketch is a bring-up sketch, not a full image-capture pipeline.
It does these steps:

1. generates `XCLK`
2. starts SCCB on `SIOD/SIOC`
3. probes the camera ID registers
4. prints results over serial

## Current Wiring

This bring-up sketch is intentionally mapped to one side of an ESP32 DevKit V1
so it is easier to place the board on a breadboard.

Use the labels printed on many OV7670 boards like this:

- `SDA` on the camera board = `SIOD`
- `SCL` on the camera board = `SIOC`
- `XLK` on the camera board = `XCLK`
- `VS` on the camera board = `VSYNC`
- `HS` on the camera board = `HREF` or `HSYNC`
- `PLK` on the camera board = `PCLK`

Active pins used by the current sketch:

- `OV7670 SDA -> ESP32 GPIO 18`
- `OV7670 SCL -> ESP32 GPIO 19`
- `OV7670 XLK -> ESP32 GPIO 21`

The current sketch leaves these external:

- `OV7670 RESET -> 3.3V`
- `OV7670 PWDN -> GND`

These camera pins are not used in the current bring-up sketch:

- `VS`
- `PLK`
- `HS`
- `D0-D7`

Leave them disconnected for now.

## Power

- `OV7670 VCC -> 3.3V`
- `OV7670 GND -> GND`

Do not power a typical bare OV7670 module from `5V` unless your exact module is
documented for it.

## SCCB Pull-Ups

The OV7670 SCCB lines usually need pull-ups:

- `SDA` pull-up to `3.3V`
- `SCL` pull-up to `3.3V`

`1k` to `4.7k` is common. If your module board already includes pull-ups, do not
duplicate them blindly.

## Notes

1. This project is only for initial communication bring-up.
2. The current sketch only needs `VCC`, `GND`, `SIOD`, `SIOC`, `XCLK`, `RESET`, and `PWDN`.
3. Full frame capture on OV7670 without FIFO will need many more pins and will not stay one-sided.
4. If SCCB probing fails, first verify power, ground, `XCLK`, and SCCB pull-ups.
