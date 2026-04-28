# ESP32-CAM OV3660 Streamer

This project assumes a common `ESP32-CAM` board with:
- onboard camera connector
- PSRAM
- OV3660 sensor attached to the camera connector

Even if your PCB does not say `AI Thinker`, the local board profile we are using is:
- `esp32:esp32:esp32cam`

That board definition is named `AI Thinker ESP32-CAM` in Arduino, and it matches many generic clones.

## Programming Wiring

If you have an `ESP32-CAM-MB` USB base:
- plug the board into the base and use micro-USB

If you have a bare `ESP32-CAM`:

```text
USB-TTL Adapter             ESP32-CAM
---------------            ---------
5V                      -> 5V
GND                     -> GND
TX                      -> U0R
RX                      -> U0T
GND                     -> IO0   (only during upload)
```

## Upload Mode

For a bare board:
1. Connect `IO0` to `GND`
2. Power the board
3. Upload the sketch
4. Remove `IO0 -> GND`
5. Press `RST` once to boot normally

## Runtime

After normal boot:
- the board joins Wi‑Fi
- starts the camera web server
- prints the local IP address on serial

Then open:

```text
http://<board-ip>/
```

## If Camera Init Fails

This project starts with the common `AI Thinker` pin map.

If you see camera init failures, the next likely cause is a different ESP32-CAM pin layout. At that point:
- keep this project
- capture the exact serial error
- we can switch to another supported pin map
