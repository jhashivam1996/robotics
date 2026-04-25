# ESP32 HW-484 Sound Detector

This project uses the `HW-484` sound sensor and a small speaker for simple beeps.

It does:
- read the analog output `AO`
- read the digital comparator output `DO`
- print sound activity over serial
- generate different beep patterns for low, medium, and high sound levels

It does not do:
- audio recording
- stored audio playback
- voice storage

## Wiring

```text
ESP32 DevKit V1              HW-484
-----------------           -------
3.3V                     -> VCC
GND                      -> GND
GPIO 34                  -> AO
GPIO 27                  -> DO
```

## Minimal Speaker Wiring

Use this only for a quiet test beep. A raw `16 ohm` speaker is not a safe direct GPIO load by itself.

```text
ESP32 DevKit V1              Speaker Path
-----------------           ---------------------------
GPIO 25                  -> 100uF capacitor (+ on GPIO side)
capacitor other side     -> 220 ohm resistor
220 ohm resistor         -> speaker terminal 1
speaker terminal 2       -> GND
```

Preferred:
- use a transistor or small amplifier stage if you have one

Minimal fallback:
- the `100uF + 220 ohm` path limits current and keeps the beep quiet
- do not connect the `16 ohm` speaker straight to the ESP32 pin without current limiting

## Notes

- Use `3.3V`, not `5V`, so the outputs stay ESP32-safe.
- `GPIO 34` is input-only and works well for analog sensing.
- `DO` depends on the module's onboard trimmer setting.
- Adjust the small potentiometer on the `HW-484` if the digital output is too sensitive or never triggers.
- `GPIO 25` is used for beep output.

## Serial Output

After upload, open the serial monitor at `115200`.

You will see:
- `AO=<value>` for the analog reading
- `DO=<value>` for the digital comparator output
- threshold and peak information
- speaker state

## Serial Commands

- `h`: help
- `a`: toggle digital trigger reporting
- `b`: toggle speaker beeps
- `t`: raise analog threshold
- `g`: lower analog threshold
- `i`: print current status once
- `1`: test low beep
- `2`: test medium beep
- `3`: test high beep

## What This Module Is Good For

- clap detection
- noise level trigger
- sound/no-sound event detection
- rough relative sound measurements
- simple audible feedback beeps

## What This Module Is Not Good For

- clean microphone recording
- speech capture
- full audio playback
- storing useful waveform data
