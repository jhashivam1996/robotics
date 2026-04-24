# SETUP.md

## Scope

This file contains environment and usage notes for the `arduino-uno-car` sketch.

Target board:

- `arduino:avr:uno`

## Board Core Setup

Install the Arduino AVR core if needed:

```bash
arduino-cli core install arduino:avr
```

## Compile

From the repository root:

```bash
./arduino-uno-car/upload-sketch.sh --mode compile
```

From inside the project directory:

```bash
cd arduino-uno-car
./upload-sketch.sh --mode compile
```

## Upload

The wrapper defaults to `/dev/ttyUSB0`.

From the repository root:

```bash
./arduino-uno-car/upload-sketch.sh --mode upload
```

From inside the project directory:

```bash
cd arduino-uno-car
./upload-sketch.sh --mode upload
```

## Port Note

The Arduino Uno car wrapper currently hardcodes:

- board: `arduino:avr:uno`
- port: `/dev/ttyUSB0`

If your board appears on a different port, update `arduino-uno-car/upload-sketch.sh`.
