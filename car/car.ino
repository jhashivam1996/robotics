// Full sketch: L298N (two motor groups) + HC-SR04 ultrasonic obstacle detection
// Motor wiring:
// IN1 -> pin 11 (PWM)  Motor A, pinA
// IN2 -> pin 10 (PWM)  Motor A, pinB
// IN3 -> pin 9  (PWM)  Motor B, pinA
// IN4 -> pin 6  (PWM)  Motor B, pinB
//
// Ultrasonic wiring (HC-SR04):
// VCC -> 5V, GND -> GND, TRIG -> pin 7, ECHO -> pin 8
// IR obstacle sensor wiring:
// Front sensor: VCC -> 5V, GND -> GND, OUT -> pin 5
// Rear sensor:  VCC -> 5V, GND -> GND, OUT -> pin 4
//
// NOTE: Always share GND between Arduino and motor battery pack.

const int IN1 = 11; // Motor A pin A
const int IN2 = 10; // Motor A pin B
const int IN3 = 9;  // Motor B pin A
const int IN4 = 6;  // Motor B pin B

const int US_TRIG_PIN = 7;
const int US_ECHO_PIN = 8;
const int FRONT_IR_OBSTACLE_PIN = 5;
const int REAR_IR_OBSTACLE_PIN = 4;

const unsigned long US_TIMEOUT = 30000UL; // ~max 500 cm

// Simplified obstacle controller tuning.
const bool IR_ACTIVE_LOW = true;         // most obstacle IR modules pull LOW on detect
const int FRONT_STOP_CM = 40;            // stop distance for front obstacles
const int FRONT_CLEAR_CM = 50;           // clearance hysteresis after obstacle
const int IR_ASSIST_US_CM = 45;          // only trust brief front IR hits when ultrasonic also sees something nearby
const int US_REQUIRED_HITS = 2;          // consecutive ultrasonic hits before stop
const int FRONT_IR_REQUIRED_HITS = 3;    // debounce front IR
const int FRONT_IR_ONLY_REQUIRED_HITS = 12; // sustained IR-only block before stop
const int REAR_IR_REQUIRED_HITS = 3;     // debounce rear IR
const int FORWARD_SPEED = 120;           // steady forward speed
const int REVERSE_SPEED = 110;           // gentle reverse speed
const int REVERSE_MS = 260;              // short reverse to create space
const int TURN_SPEED = 110;              // in-place turn speed
const int TURN_MS = 180;                 // in-place turn duration
const int TURN_SETTLE_MS = 120;          // pause after turning
const int TURN_TRIES_PER_SIDE = 2;       // tries per side before switching
const int FRONT_BLOCK_HOLD_MS = 90;      // require a stable front block before committing to avoidance

enum RobotState {
  DRIVE_FORWARD,
  REVERSE_AWAY,
  TURN_LEFT,
  TURN_RIGHT,
  SETTLE,
  BLOCKED
};

RobotState currentState = DRIVE_FORWARD;
unsigned long stateStartedAt = 0;
bool nextPreferredTurnLeft = true;
bool currentTurnLeft = true;
bool triedOtherSide = false;
int turnsOnCurrentSide = 0;
int frontIrHits = 0;
int rearIrHits = 0;
int usNearHits = 0;
long frontDistanceCm = -1;
bool frontIrBlocked = false;
bool rearIrBlocked = false;
unsigned long frontBlockedStartedAt = 0;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  digitalWrite(US_TRIG_PIN, LOW);

  pinMode(FRONT_IR_OBSTACLE_PIN, INPUT);
  pinMode(REAR_IR_OBSTACLE_PIN, INPUT);

  Serial.begin(115200);
  Serial.println("L298N + HC-SR04 + front/rear IR starting");
  Serial.println("Front IR pin: D5, Rear IR pin: D4");
  Serial.println("State -> DRIVE_FORWARD");
}

void runMotor(int pinA, int pinB, int speed, bool forward) {
  speed = constrain(speed, 0, 255);

  if (speed == 0) {
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

void moveBoth(int speed, bool forward) {
  runMotor(IN1, IN2, speed, forward);
  runMotor(IN3, IN4, speed, forward);
}

void stopMotors() {
  runMotor(IN1, IN2, 0, true);
  runMotor(IN3, IN4, 0, true);
}

long measureDistanceCM() {
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);

  unsigned long duration = pulseIn(US_ECHO_PIN, HIGH, US_TIMEOUT);
  if (duration == 0UL) {
    return -1;
  }
  return duration / 58UL;
}

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

bool readFrontIrBlocked() {
  int raw = digitalRead(FRONT_IR_OBSTACLE_PIN);
  bool blocked = IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);

  if (blocked) {
    frontIrHits++;
  } else {
    frontIrHits = 0;
  }

  return frontIrHits >= FRONT_IR_REQUIRED_HITS;
}

bool readRearIrBlocked() {
  int raw = digitalRead(REAR_IR_OBSTACLE_PIN);
  bool blocked = IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);

  if (blocked) {
    rearIrHits++;
  } else {
    rearIrHits = 0;
  }

  return rearIrHits >= REAR_IR_REQUIRED_HITS;
}

void updateSensors() {
  frontDistanceCm = measureDistanceFilteredCM(3);
  frontIrBlocked = readFrontIrBlocked();
  rearIrBlocked = readRearIrBlocked();

  if (frontDistanceCm > 0 && frontDistanceCm <= FRONT_STOP_CM) {
    usNearHits++;
  } else {
    usNearHits = 0;
  }
}

bool frontBlockedNow() {
  bool usBlocked = usNearHits >= US_REQUIRED_HITS;
  bool irAssistedByUs = frontIrBlocked &&
                        frontDistanceCm > 0 &&
                        frontDistanceCm <= IR_ASSIST_US_CM;
  bool irEmergencyOnly = frontIrHits >= FRONT_IR_ONLY_REQUIRED_HITS;

  return usBlocked || irAssistedByUs || irEmergencyOnly;
}

bool frontClearNow() {
  return (frontDistanceCm == -1) || (frontDistanceCm > FRONT_CLEAR_CM);
}

bool frontBlockedStableNow() {
  if (!frontBlockedNow()) {
    frontBlockedStartedAt = 0;
    return false;
  }

  if (frontBlockedStartedAt == 0) {
    frontBlockedStartedAt = millis();
    return false;
  }

  return millis() - frontBlockedStartedAt >= (unsigned long)FRONT_BLOCK_HOLD_MS;
}

const char* stateName(RobotState state) {
  switch (state) {
    case DRIVE_FORWARD: return "DRIVE_FORWARD";
    case REVERSE_AWAY: return "REVERSE_AWAY";
    case TURN_LEFT: return "TURN_LEFT";
    case TURN_RIGHT: return "TURN_RIGHT";
    case SETTLE: return "SETTLE";
    case BLOCKED: return "BLOCKED";
    default: return "UNKNOWN";
  }
}

void setState(RobotState newState) {
  currentState = newState;
  stateStartedAt = millis();
  frontBlockedStartedAt = 0;
  Serial.print("State -> ");
  Serial.println(stateName(newState));
}

void startAvoidance() {
  stopMotors();
  currentTurnLeft = nextPreferredTurnLeft;
  triedOtherSide = false;
  turnsOnCurrentSide = 0;

  if (!rearIrBlocked) {
    setState(REVERSE_AWAY);
  } else {
    setState(currentTurnLeft ? TURN_LEFT : TURN_RIGHT);
  }
}

void rotateLeftInPlace() {
  runMotor(IN1, IN2, TURN_SPEED, false);
  runMotor(IN3, IN4, TURN_SPEED, true);
}

void rotateRightInPlace() {
  runMotor(IN1, IN2, TURN_SPEED, true);
  runMotor(IN3, IN4, TURN_SPEED, false);
}

void handleStateDriveForward() {
  moveBoth(FORWARD_SPEED, true);

  if (frontBlockedStableNow()) {
    startAvoidance();
  }
}

void handleStateReverseAway() {
  if (rearIrBlocked) {
    stopMotors();
    setState(currentTurnLeft ? TURN_LEFT : TURN_RIGHT);
    return;
  }

  moveBoth(REVERSE_SPEED, false);
  if (millis() - stateStartedAt >= (unsigned long)REVERSE_MS) {
    stopMotors();
    setState(currentTurnLeft ? TURN_LEFT : TURN_RIGHT);
  }
}

void handleStateTurn(bool turnLeft) {
  if (turnLeft) {
    rotateLeftInPlace();
  } else {
    rotateRightInPlace();
  }

  if (millis() - stateStartedAt >= (unsigned long)TURN_MS) {
    stopMotors();
    turnsOnCurrentSide++;
    setState(SETTLE);
  }
}

void handleStateSettle() {
  stopMotors();

  if (millis() - stateStartedAt < (unsigned long)TURN_SETTLE_MS) {
    return;
  }

  if (frontClearNow()) {
    nextPreferredTurnLeft = !nextPreferredTurnLeft;
    setState(DRIVE_FORWARD);
    return;
  }

  if (turnsOnCurrentSide < TURN_TRIES_PER_SIDE) {
    setState(currentTurnLeft ? TURN_LEFT : TURN_RIGHT);
    return;
  }

  if (!triedOtherSide) {
    triedOtherSide = true;
    currentTurnLeft = !currentTurnLeft;
    turnsOnCurrentSide = 0;

    if (!rearIrBlocked) {
      setState(REVERSE_AWAY);
    } else {
      setState(currentTurnLeft ? TURN_LEFT : TURN_RIGHT);
    }
    return;
  }

  setState(BLOCKED);
}

void handleStateBlocked() {
  stopMotors();

  if (frontClearNow()) {
    nextPreferredTurnLeft = !nextPreferredTurnLeft;
    setState(DRIVE_FORWARD);
  }
}

void loop() {
  updateSensors();

  switch (currentState) {
    case DRIVE_FORWARD:
      handleStateDriveForward();
      break;
    case REVERSE_AWAY:
      handleStateReverseAway();
      break;
    case TURN_LEFT:
      handleStateTurn(true);
      break;
    case TURN_RIGHT:
      handleStateTurn(false);
      break;
    case SETTLE:
      handleStateSettle();
      break;
    case BLOCKED:
      handleStateBlocked();
      break;
  }
}
