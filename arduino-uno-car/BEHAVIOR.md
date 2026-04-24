# BEHAVIOR.md

## Scope

This file contains robot-specific behavior, hardware, and tuning notes for the Arduino Uno car project.

Active sketch:

- `arduino-uno-car.ino`

## Hardware

### Motor Driver

- `IN1 -> 11`
- `IN2 -> 10`
- `IN3 -> 9`
- `IN4 -> 6`

### Ultrasonic Sensor

- `TRIG -> 7`
- `ECHO -> 8`

Role:

- medium-range obstacle detection
- main path-clear decision source

### Front IR Obstacle Sensor

- `OUT -> 5`

Role:

- close-range front obstacle confirmation
- stuck detection support

Notes:

- This is not a cliff sensor.
- `IR_ACTIVE_LOW` may need to be flipped if the module logic is inverted.
- The IR sensor is mounted just above the ultrasonic sensor and faces forward.

## Motion Model

The robot is differential drive. It cannot move sideways directly.

Available motion patterns:

- both sides forward
- both sides backward
- one side stopped and the other moving for pivot/arc turning
- one-side reverse wiggle for unsticking

## Current Avoidance Behavior

1. Move forward while checking the front path.
2. Use filtered ultrasonic readings as the main distance signal.
3. Use the IR sensor as close-range confirmation.
4. Confirm an obstacle before starting reverse.
5. Reverse to create space.
6. Try turning on one side up to 5 times.
7. If still blocked, try the other side up to 5 times.

## Tuning Guidance

If the car is:

- too conservative:
  - reduce ultrasonic threshold
  - improve confirmation logic
  - check whether the IR module is over-triggering
- hitting walls:
  - increase ultrasonic threshold
  - reduce speed
  - reduce decision delay
- backing off for no reason:
  - increase debounce
  - strengthen confirmation
  - inspect IR sensitivity physically
- getting stuck against walls:
  - increase escape duration or force
  - preserve close-range checks after reverse

Prefer changing constants before changing structure.

## Physical Setup Notes

1. Keep Arduino ground and motor battery ground shared.
2. Keep the IR sensor aimed roughly parallel to the ultrasonic sensor.
3. Do not tilt the front IR sensor so far downward that it sees the floor during normal driving.
4. Tune the IR module potentiometer so it reacts only at short range.

## Limitations

1. The front IR sensor does not provide cliff detection.
2. This setup is not safe for table-edge protection.
3. For cliff detection, use downward-facing cliff sensors or downward ToF sensors.
