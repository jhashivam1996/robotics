// Full sketch: L298N (two motor groups) + HC-SR04 ultrasonic obstacle detection
// Motor wiring (same as your setup):
// IN1 -> pin 11 (PWM)  Motor A, pinA
// IN2 -> pin 10 (PWM)  Motor A, pinB
// IN3 -> pin 9  (PWM)  Motor B, pinA
// IN4 -> pin 6  (PWM)  Motor B, pinB
//
// Ultrasonic wiring (HC-SR04):
// VCC -> 5V, GND -> GND, TRIG -> pin 7, ECHO -> pin 8
//
// NOTE: Always share GND between Arduino and motor battery pack.

const int IN1 = 11; // Motor A pin A
const int IN2 = 10; // Motor A pin B
const int IN3 = 9;  // Motor B pin A
const int IN4 = 6;  // Motor B pin B

// Ultrasonic pins (change if desired)
const int US_TRIG_PIN = 7;
const int US_ECHO_PIN = 8;

// Ultrasonic measurement timeout (microseconds)
const unsigned long US_TIMEOUT = 30000UL; // ~max 500 cm

// Obstacle handling/tuning
const int OBSTACLE_THRESHOLD_CM = 40;   // stop sooner to account for momentum
const int OBSTACLE_CLEARANCE_CM = 50;   // hysteresis: resume only after safer gap
const int REQUIRED_HITS = 2;            // require consecutive near readings
const int BACKOFF_SPEED = 170;          // reverse speed after detection
const int BACKOFF_MS = 900;             // reverse duration after detection
const int TURN_SPEED = 150;             // turning speed for obstacle avoidance
const int TURN_MS = 320;                // turn duration per try
const int TURN_SETTLE_MS = 120;         // pause before re-checking sensor
const int TURN_TRIES_PER_SIDE = 5;      // tries per side before switching
const int ESCAPE_BOOST_SPEED = 235;     // short high-power reverse burst
const int ESCAPE_BOOST_MS = 220;        // burst duration
const int ESCAPE_WIGGLE_MS = 180;       // one-side reverse wiggle duration
const int ESCAPE_TRIES = 3;             // repeated unstuck tries

void setup() {
  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Make sure motors are off initially
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Ultrasonic pins
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  digitalWrite(US_TRIG_PIN, LOW); // ensure trig is low

  Serial.begin(115200);
  Serial.println("L298N + HC-SR04 starting");
}

// Run one motor with given direction/speed.
// pinA, pinB: the two control pins for that motor channel.
// speed: 0..255 (0 = stop). forward=true => PWM on pinA, pinB LOW.
// forward=false => PWM on pinB, pinA LOW.
void runMotor(int pinA, int pinB, int speed, bool forward) {
  speed = constrain(speed, 0, 255);

  if (speed == 0) {
    // Brake/stop: both low
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    return;
  }

  if (forward) {
    analogWrite(pinA, speed);
    digitalWrite(pinB, LOW);
  } else {
    analogWrite(pinB, speed);
    digitalWrite(pinA, LOW);
  }
}

// Move both motor groups together
void moveBoth(int speed, bool forward) {
  runMotor(IN1, IN2, speed, forward); // left side (Motor A)
  runMotor(IN3, IN4, speed, forward); // right side (Motor B)
}

// Stop both motors
void stopMotors() {
  runMotor(IN1, IN2, 0, true);
  runMotor(IN3, IN4, 0, true);
}

// Measure distance in centimeters using HC-SR04.
// Returns: distance in cm (integer) or -1 if no echo within timeout.
long measureDistanceCM() {
  // Trigger a 10µs pulse
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);

  unsigned long duration = pulseIn(US_ECHO_PIN, HIGH, US_TIMEOUT);

  if (duration == 0UL) {
    return -1; // timeout/no echo
  }

  // Convert to cm: sound round-trip ~58 µs per cm
  long cm = duration / 58UL;
  return cm;
}

// Read several distance samples and return a stable value.
// Returns -1 if all samples timed out.
long measureDistanceFilteredCM(int samples = 3) {
  long sum = 0;
  int valid = 0;
  for (int i = 0; i < samples; i++) {
    long d = measureDistanceCM();
    if (d > 0) {
      sum += d;
      valid++;
    }
    delay(8);
  }
  if (valid == 0) {
    return -1;
  }
  return sum / valid;
}

bool isObstacleNow() {
  static int nearHits = 0;
  long d = measureDistanceFilteredCM(3);

  if (d > 0) {
    Serial.print("US(filtered): ");
    Serial.print(d);
    Serial.println(" cm");
  }

  if (d > 0 && d <= OBSTACLE_THRESHOLD_CM) {
    nearHits++;
  } else if (d > OBSTACLE_CLEARANCE_CM || d == -1) {
    nearHits = 0;
  }

  if (nearHits >= REQUIRED_HITS) {
    nearHits = 0;
    return true;
  }
  return false;
}

// Move only one side forward while the other side is stopped.
// This creates a pivot/arc turn to "sidestep" around obstacles.
// moveLeftSide=true  => left side moves, right side locked.
// moveLeftSide=false => right side moves, left side locked.
void sideStepTurn(bool moveLeftSide) {
  if (moveLeftSide) {
    runMotor(IN3, IN4, 0, true);              // lock right side
    runMotor(IN1, IN2, TURN_SPEED, true);     // move left side
  } else {
    runMotor(IN1, IN2, 0, true);              // lock left side
    runMotor(IN3, IN4, TURN_SPEED, true);     // move right side
  }
}

bool pathLooksClear() {
  long d = measureDistanceFilteredCM(3);
  if (d > 0) {
    Serial.print("US(clear-check): ");
    Serial.print(d);
    Serial.println(" cm");
  } else {
    Serial.println("US(clear-check): no echo");
  }
  return (d == -1) || (d > OBSTACLE_CLEARANCE_CM);
}

bool isFrontStillTooClose() {
  long d = measureDistanceFilteredCM(3);
  if (d > 0) {
    Serial.print("US(stuck-check): ");
    Serial.print(d);
    Serial.println(" cm");
    return d <= OBSTACLE_THRESHOLD_CM;
  }
  // If no echo, do not treat as stuck.
  Serial.println("US(stuck-check): no echo");
  return false;
}

void reverseWithBoostAndWiggle() {
  // Strong kick to break static friction.
  moveBoth(ESCAPE_BOOST_SPEED, false);
  delay(ESCAPE_BOOST_MS);

  // Continue normal reverse to gain distance.
  moveBoth(BACKOFF_SPEED, false);
  delay(BACKOFF_MS);
  stopMotors();
  delay(120);

  // If still close, do asymmetric reverse wiggles to peel away.
  if (isFrontStillTooClose()) {
    runMotor(IN1, IN2, BACKOFF_SPEED, false); // left reverse only
    runMotor(IN3, IN4, 0, true);
    delay(ESCAPE_WIGGLE_MS);
    stopMotors();
    delay(80);

    runMotor(IN1, IN2, 0, true);
    runMotor(IN3, IN4, BACKOFF_SPEED, false); // right reverse only
    delay(ESCAPE_WIGGLE_MS);
    stopMotors();
    delay(80);
  }
}

void handleObstacleAvoidance() {
  Serial.println("Obstacle detected! Stop, back off, and avoid.");
  stopMotors();
  delay(120); // allow chassis to settle before reversing

  for (int t = 1; t <= ESCAPE_TRIES; t++) {
    Serial.print("Escape try ");
    Serial.print(t);
    Serial.print("/");
    Serial.println(ESCAPE_TRIES);
    reverseWithBoostAndWiggle();

    if (!isFrontStillTooClose()) {
      break;
    }
  }
  delay(200);

  // Rule: try one side 5 times; if still blocked, try the other side 5 times.
  // Order below starts with right-side move (left locked), then left-side move.
  bool sideOrder[2] = {false, true};
  for (int side = 0; side < 2; side++) {
    Serial.print("Avoid side group: ");
    Serial.println(side == 0 ? "RIGHT-SIDE-MOVE" : "LEFT-SIDE-MOVE");

    for (int i = 1; i <= TURN_TRIES_PER_SIDE; i++) {
      Serial.print("Turn try ");
      Serial.print(i);
      Serial.print("/");
      Serial.println(TURN_TRIES_PER_SIDE);

      sideStepTurn(sideOrder[side]);
      delay(TURN_MS);
      stopMotors();
      delay(TURN_SETTLE_MS);

      if (pathLooksClear()) {
        Serial.println("Path clear after turning.");
        return;
      }
    }
  }

  Serial.println("Path still blocked after both-side attempts.");
}

void loop() {
  // Tunable parameters
  const int minSpeed = 110;    // starting ramp speed (0..255)
  const int maxSpeed = 200;    // top ramp speed (reduced to shorten stopping distance)
  const int step = 8;          // ramp step
  const int rampDelay = 30;    // ms between ramp steps
  const int holdMs = 1000;     // hold after ramp
  const int holdCheckEveryMs = 60;

  Serial.println("Ramping FORWARD");
  // Ramp forward from minSpeed -> maxSpeed
  for (int s = minSpeed; s <= maxSpeed; s += step) {
    moveBoth(s, true); // true = forward

    if (isObstacleNow()) {
      handleObstacleAvoidance();
      goto START_BACKWARD;
    }

    delay(rampDelay);
  }

  // Hold forward briefly only if path is clear.
  // Keep checking during hold to avoid crashing at max speed.
  Serial.println("Holding forward (guarded)");
  unsigned long holdStart = millis();
  while (millis() - holdStart < (unsigned long)holdMs) {
    if (isObstacleNow()) {
      Serial.println("Obstacle during hold.");
      handleObstacleAvoidance();
      goto START_BACKWARD;
    }
    moveBoth(maxSpeed, true);
    delay(holdCheckEveryMs);
  }

START_BACKWARD:
  Serial.println("Ramping BACKWARD");
  // Ramp backward from maxSpeed -> minSpeed
  for (int s = maxSpeed; s >= minSpeed; s -= step) {
    moveBoth(s, false); // false = backward
    delay(rampDelay);
  }

  Serial.println("Holding backward");
  delay(holdMs);

  // Stop briefly before next cycle
  Serial.println("Cycle complete - stopping briefly");
  stopMotors();
  delay(700);
}
