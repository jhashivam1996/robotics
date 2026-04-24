# SETUP.md

## Scope

This file contains environment and usage notes for the `esp32-ov7670-camera-phase3`
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
./esp32-ov7670-camera-phase3/upload-sketch.sh --mode compile
```

From inside the project directory:

```bash
cd esp32-ov7670-camera-phase3
./upload-sketch.sh --mode compile
```

## Upload

From the repository root:

```bash
./esp32-ov7670-camera-phase3/upload-sketch.sh --mode upload
```

From inside the project directory:

```bash
cd esp32-ov7670-camera-phase3
./upload-sketch.sh --mode upload
```

## One Command Capture

From the repository root:

```bash
./esp32-ov7670-camera-phase3/capture-frame.sh
```

From inside the project directory:

```bash
cd esp32-ov7670-camera-phase3
./capture-frame.sh
```

This helper will:

1. upload the sketch
2. monitor serial until one frame is dumped
3. extract `frame.pgm`
4. convert to `frame.png` if ImageMagick is installed
5. open the final image if `xdg-open` is available

## Serial Monitor

From the repository root:

```bash
./esp32-ov7670-camera-phase3/monitor.sh
```

From inside the project directory:

```bash
cd esp32-ov7670-camera-phase3
./monitor.sh
```

## Save The Image

1. Start the monitor and save its output:

```bash
cd esp32-ov7670-camera-phase3
./monitor.sh | tee frame.log
```

2. Stop the monitor after `END_PGM` appears.

3. Extract the image:

```bash
./extract-pgm.sh frame.log frame.pgm
```

The output file `frame.pgm` can be viewed directly by many image tools.

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
