# SETUP.md

## Scope

This file contains environment setup commands for the `esp32-lcd-i2c` sketch.

Target board:

- `esp32:esp32:esp32doit-devkit-v1`

## Board Core Setup

This project uses the Espressif Arduino core.

Install it with:

```bash
arduino-cli core update-index \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

arduino-cli core install esp32:esp32 \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

## LCD Library Setup

Install the I2C LCD library required by `esp32-lcd-i2c.ino`:

```bash
arduino-cli lib install "LiquidCrystal I2C"
```

## Python Tooling Requirement

Some ESP32 toolchains require Python `pyserial` during compile and upload.

If the wrapper reports `Missing Python module: serial`, install `pyserial`.

Option 1: use the system package manager.

```bash
sudo apt-get install python3-serial
```

Option 2: use `pip` if needed.

```bash
python3 -m pip install --user pyserial
```

## Port Discovery

Before upload, find the serial port:

```bash
arduino-cli board list
```

Then upload with:

From the repository root:

```bash
./esp32-lcd-i2c/upload-sketch.sh --mode upload
```

From inside the project directory:

```bash
cd esp32-lcd-i2c
./upload-sketch.sh --mode upload
```

The wrapper defaults to `/dev/ttyUSB0`.
If your ESP32 appears on a different port, override it:

From the repository root:

```bash
./esp32-lcd-i2c/upload-sketch.sh --mode upload --port /dev/ttyUSB1
```

From inside the project directory:

```bash
cd esp32-lcd-i2c
./upload-sketch.sh --mode upload --port /dev/ttyUSB1
```

## Serial Monitor

From the repository root:

```bash
./esp32-lcd-i2c/monitor.sh
```

From inside the project directory:

```bash
cd esp32-lcd-i2c
./monitor.sh
```
