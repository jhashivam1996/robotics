# SETUP.md

## Scope

This file contains the environment setup commands for the `esp32-lcd` sketch when targeting:

- `esp32:esp32:esp32doit-devkit-v1`

## Board Core Setup

This project uses the Espressif Arduino core, not the Arduino-maintained `arduino:esp32` core.

Add the Espressif package index and install the required core with:

```bash
arduino-cli core update-index \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

arduino-cli core install esp32:esp32 \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

After installation, the hardcoded wrapper should match this board:

From the repository root:

```bash
./esp32-lcd/upload-sketch.sh --mode compile
```

From inside the project directory:

```bash
cd esp32-lcd
./upload-sketch.sh --mode compile
```

## LCD Library Setup

Install the LCD library required by `esp32-lcd.ino`:

```bash
arduino-cli lib install "LiquidCrystal"
```

## Python Tooling Requirement

Some ESP32 toolchains require Python `pyserial` during compile and upload.

If the wrapper reports `Missing Python module: serial`, install `pyserial`.

Option 1: use the system package manager.
On this Debian machine, this command worked:

```bash
sudo apt-get install python3-serial
```

Option 2: use `pip` if your system does not provide that package and already has `pip`.

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
./esp32-lcd/upload-sketch.sh --mode upload
```

From inside the project directory:

```bash
cd esp32-lcd
./upload-sketch.sh --mode upload
```

The wrapper defaults to `/dev/ttyUSB0`.
If your ESP32 appears on a different port, override it:

From the repository root:

```bash
./esp32-lcd/upload-sketch.sh --mode upload --port /dev/ttyUSB1
```

From inside the project directory:

```bash
cd esp32-lcd
./upload-sketch.sh --mode upload --port /dev/ttyUSB1
```

## References

These setup commands are based on Espressif's Arduino-ESP32 installation documentation and package index:

- https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
- https://espressif.github.io/arduino-esp32/package_esp32_index.json
