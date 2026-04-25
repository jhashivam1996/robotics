# ESP32 HW-484 Setup

This project targets:
- board: `esp32:esp32:esp32doit-devkit-v1`
- serial port default: `/dev/ttyUSB0`

## Requirements

Install the ESP32 core if needed:

```bash
arduino-cli core install esp32:esp32 \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

If ESP32 tools complain about missing Python serial support:

```bash
sudo apt install python3-serial
```

## Compile

From repo root:

```bash
./esp32-hw484-sound-detector/upload-sketch.sh --mode compile
```

From inside the project:

```bash
cd esp32-hw484-sound-detector
./upload-sketch.sh --mode compile
```

## Upload

From repo root:

```bash
./esp32-hw484-sound-detector/upload-sketch.sh --mode upload
```

From inside the project:

```bash
cd esp32-hw484-sound-detector
./upload-sketch.sh --mode upload
```

## Monitor

From repo root:

```bash
./esp32-hw484-sound-detector/monitor.sh
```

From inside the project:

```bash
cd esp32-hw484-sound-detector
./monitor.sh
```

The monitor defaults to:
- port: `/dev/ttyUSB0`
- baudrate: `115200`

Override if needed:

```bash
./monitor.sh --port /dev/ttyUSB1 --baudrate 115200
```

## Behavior

The sketch prints:
- current analog sound value
- digital trigger state
- current analog threshold
- recent peak value
- speaker beeps for sound intensity bands

Use the onboard potentiometer on the `HW-484` to tune the `DO` trigger sensitivity.

Use serial commands `1`, `2`, and `3` to test the three beep patterns without making noise near the sensor.
