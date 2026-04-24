# SETUP.md

## Scope

This file contains environment and usage notes for the `esp32-ov7670-camera`
sketch.

Target board:

- `esp32:esp32:esp32doit-devkit-v1`

## Board Core Setup

Install the Espressif Arduino core if needed:

```bash
arduino-cli core update-index \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

arduino-cli core install esp32:esp32 \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

## Compile

From the repository root:

```bash
./esp32-ov7670-camera/upload-sketch.sh --mode compile
```

From inside the project directory:

```bash
cd esp32-ov7670-camera
./upload-sketch.sh --mode compile
```

## Upload

The wrapper defaults to `/dev/ttyUSB0`.

From the repository root:

```bash
./esp32-ov7670-camera/upload-sketch.sh --mode upload
```

From inside the project directory:

```bash
cd esp32-ov7670-camera
./upload-sketch.sh --mode upload
```

If your ESP32 appears on a different port:

From the repository root:

```bash
./esp32-ov7670-camera/upload-sketch.sh --mode upload --port /dev/ttyUSB1
```

From inside the project directory:

```bash
cd esp32-ov7670-camera
./upload-sketch.sh --mode upload --port /dev/ttyUSB1
```

## Serial Monitor

From the repository root:

```bash
./esp32-ov7670-camera/monitor.sh
```

From inside the project directory:

```bash
cd esp32-ov7670-camera
./monitor.sh
```

## Python Tooling Requirement

Some ESP32 toolchains require Python `pyserial` during compile and upload.

Option 1:

```bash
sudo apt-get install python3-serial
```

Option 2:

```bash
python3 -m pip install --user pyserial
```
