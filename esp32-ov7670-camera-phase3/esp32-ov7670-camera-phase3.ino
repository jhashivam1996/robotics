#include <Wire.h>

const uint8_t OV7670_SCCB_ADDRESSES[] = {0x21, 0x42};

const int CAMERA_SIOD_PIN = 18;
const int CAMERA_SIOC_PIN = 19;
const int CAMERA_XCLK_PIN = 21;

const int CAMERA_VSYNC_PIN = 27;
const int CAMERA_HREF_PIN = 14;
const int CAMERA_PCLK_PIN = 13;

const int CAMERA_D0_PIN = 36;  // VP
const int CAMERA_D1_PIN = 39;  // VN
const int CAMERA_D2_PIN = 34;
const int CAMERA_D3_PIN = 35;
const int CAMERA_D4_PIN = 32;
const int CAMERA_D5_PIN = 33;
const int CAMERA_D6_PIN = 25;
const int CAMERA_D7_PIN = 26;

const int CAMERA_RESET_PIN = -1;
const int CAMERA_PWDN_PIN = -1;

const uint32_t CAMERA_XCLK_PROBE_FREQUENCY = 10000000UL;
const uint32_t CAMERA_XCLK_CAPTURE_FREQUENCY = 2000000UL;
const uint8_t CAMERA_XCLK_RESOLUTION_BITS = 1;

const uint16_t FRAME_WIDTH = 160;
const uint16_t FRAME_HEIGHT = 120;
const uint16_t COMPARISON_WIDTH = FRAME_WIDTH * 2;
uint8_t risingFrameBuffer[FRAME_WIDTH * FRAME_HEIGHT];
uint8_t fallingFrameBuffer[FRAME_WIDTH * FRAME_HEIGHT];
uint8_t lineBuffer[FRAME_WIDTH * 2];

const uint8_t REG_CLKRC = 0x11;
const uint8_t REG_COM7 = 0x12;
const uint8_t REG_COM8 = 0x13;
const uint8_t REG_COM9 = 0x14;
const uint8_t REG_COM3 = 0x0C;
const uint8_t REG_COM14 = 0x3E;
const uint8_t REG_COM15 = 0x40;
const uint8_t REG_TSLB = 0x3A;
const uint8_t REG_SCALING_XSC = 0x70;
const uint8_t REG_SCALING_YSC = 0x71;
const uint8_t REG_SCALING_DCWCTR = 0x72;
const uint8_t REG_SCALING_PCLK_DIV = 0x73;
const uint8_t REG_SCALING_PCLK_DELAY = 0xA2;
const uint8_t REG_PID = 0x0A;
const uint8_t REG_VER = 0x0B;
const uint8_t REG_MIDH = 0x1C;
const uint8_t REG_MIDL = 0x1D;

uint8_t activeCameraAddress = 0;
bool frameDumped = false;
bool cameraReady = false;

enum CaptureEdgeMode {
  CAPTURE_ON_RISING_EDGE,
  CAPTURE_ON_FALLING_EDGE,
};

struct RegisterWrite {
  uint8_t reg;
  uint8_t value;
};

// Derived from the OV7670 implementation guide QQVGA YUV mode table plus
// TSLB/COM15 choices for YUYV and full-range output.
const RegisterWrite QQVGA_YUV_REGISTERS[] = {
  {REG_CLKRC, 0x01},
  {REG_COM7, 0x00},
  {REG_COM8, 0x8F},   // keep auto gain/exposure/white-balance enabled
  {REG_COM9, 0x1A},   // lower AGC ceiling to 4x instead of the default high ceiling
  {REG_COM3, 0x04},
  {REG_COM14, 0x1A},
  {REG_SCALING_XSC, 0x3A},
  {REG_SCALING_YSC, 0x35},
  {REG_SCALING_DCWCTR, 0x22},
  {REG_SCALING_PCLK_DIV, 0xF2},
  {REG_SCALING_PCLK_DELAY, 0x02},
  {REG_TSLB, 0x05},   // Y U Y V, keep bit2 high, enable auto output window
  {REG_COM15, 0xC0},  // full-range [00..FF]
};

bool writeCameraRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readCameraRegister(uint8_t address, uint8_t reg, uint8_t &value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(true) != 0) {
    return false;
  }

  delayMicroseconds(100);

  if (Wire.requestFrom((int)address, 1) != 1) {
    return false;
  }

  value = Wire.read();
  return true;
}

void configureOptionalControlPins() {
  if (CAMERA_PWDN_PIN >= 0) {
    pinMode(CAMERA_PWDN_PIN, OUTPUT);
    digitalWrite(CAMERA_PWDN_PIN, LOW);
  }

  if (CAMERA_RESET_PIN >= 0) {
    pinMode(CAMERA_RESET_PIN, OUTPUT);
    digitalWrite(CAMERA_RESET_PIN, HIGH);
    delay(5);
    digitalWrite(CAMERA_RESET_PIN, LOW);
    delay(5);
    digitalWrite(CAMERA_RESET_PIN, HIGH);
    delay(10);
  }
}

void startCameraClock(uint32_t frequency) {
  ledcDetach(CAMERA_XCLK_PIN);
  ledcAttach(CAMERA_XCLK_PIN, frequency, CAMERA_XCLK_RESOLUTION_BITS);
  ledcWrite(CAMERA_XCLK_PIN, 1);
}

void configureParallelPins() {
  pinMode(CAMERA_VSYNC_PIN, INPUT);
  pinMode(CAMERA_HREF_PIN, INPUT);
  pinMode(CAMERA_PCLK_PIN, INPUT);

  pinMode(CAMERA_D0_PIN, INPUT);
  pinMode(CAMERA_D1_PIN, INPUT);
  pinMode(CAMERA_D2_PIN, INPUT);
  pinMode(CAMERA_D3_PIN, INPUT);
  pinMode(CAMERA_D4_PIN, INPUT);
  pinMode(CAMERA_D5_PIN, INPUT);
  pinMode(CAMERA_D6_PIN, INPUT);
  pinMode(CAMERA_D7_PIN, INPUT);
}

uint8_t readDataBusByte() {
  uint8_t value = 0;
  value |= (digitalRead(CAMERA_D0_PIN) ? 1U : 0U) << 0;
  value |= (digitalRead(CAMERA_D1_PIN) ? 1U : 0U) << 1;
  value |= (digitalRead(CAMERA_D2_PIN) ? 1U : 0U) << 2;
  value |= (digitalRead(CAMERA_D3_PIN) ? 1U : 0U) << 3;
  value |= (digitalRead(CAMERA_D4_PIN) ? 1U : 0U) << 4;
  value |= (digitalRead(CAMERA_D5_PIN) ? 1U : 0U) << 5;
  value |= (digitalRead(CAMERA_D6_PIN) ? 1U : 0U) << 6;
  value |= (digitalRead(CAMERA_D7_PIN) ? 1U : 0U) << 7;
  return value;
}

bool cameraAddressAck(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool probeCameraIdentityAtAddress(uint8_t address) {
  uint8_t midh = 0;
  uint8_t midl = 0;
  uint8_t pid = 0;
  uint8_t ver = 0;

  if (!readCameraRegister(address, REG_MIDH, midh)) {
    Serial.printf("Address 0x%02X ACKed but MIDH read failed.\n", address);
    return false;
  }
  if (!readCameraRegister(address, REG_MIDL, midl)) {
    Serial.printf("Address 0x%02X ACKed but MIDL read failed.\n", address);
    return false;
  }
  if (!readCameraRegister(address, REG_PID, pid)) {
    Serial.printf("Address 0x%02X ACKed but PID read failed.\n", address);
    return false;
  }
  if (!readCameraRegister(address, REG_VER, ver)) {
    Serial.printf("Address 0x%02X ACKed but VER read failed.\n", address);
    return false;
  }

  activeCameraAddress = address;
  Serial.printf(
    "Address 0x%02X MIDH=0x%02X MIDL=0x%02X PID=0x%02X VER=0x%02X\n",
    address,
    midh,
    midl,
    pid,
    ver
  );
  return true;
}

bool probeCameraIdentity() {
  for (uint8_t i = 0; i < sizeof(OV7670_SCCB_ADDRESSES); i++) {
    uint8_t address = OV7670_SCCB_ADDRESSES[i];
    Serial.printf("Probing SCCB address 0x%02X...\n", address);

    if (!cameraAddressAck(address)) {
      Serial.printf("Address 0x%02X did not ACK.\n", address);
      continue;
    }

    Serial.printf("Address 0x%02X ACKed.\n", address);

    if (probeCameraIdentityAtAddress(address)) {
      return true;
    }
  }

  Serial.println("Camera identity probe failed.");
  return false;
}

bool applyRegisterTable(const RegisterWrite *table, size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (!writeCameraRegister(activeCameraAddress, table[i].reg, table[i].value)) {
      Serial.printf("Register write failed at 0x%02X.\n", table[i].reg);
      return false;
    }
    delayMicroseconds(200);
  }

  return true;
}

bool waitForPinState(int pin, int expectedState, unsigned long timeoutMicros) {
  unsigned long startMicros = micros();
  while (micros() - startMicros < timeoutMicros) {
    if (digitalRead(pin) == expectedState) {
      return true;
    }
  }
  return false;
}

bool waitForPclkEdge(CaptureEdgeMode edgeMode, unsigned long timeoutMicros) {
  unsigned long startMicros = micros();
  int last = digitalRead(CAMERA_PCLK_PIN);

  while (micros() - startMicros < timeoutMicros) {
    int current = digitalRead(CAMERA_PCLK_PIN);
    bool matched = (edgeMode == CAPTURE_ON_RISING_EDGE && last == LOW && current == HIGH) ||
                   (edgeMode == CAPTURE_ON_FALLING_EDGE && last == HIGH && current == LOW);
    if (matched) {
      return true;
    }
    last = current;
  }

  return false;
}

bool waitForPinTransition(int pin, unsigned long timeoutMicros) {
  unsigned long startMicros = micros();
  int last = digitalRead(pin);

  while (micros() - startMicros < timeoutMicros) {
    int current = digitalRead(pin);
    if (current != last) {
      return true;
    }
  }

  return false;
}

bool waitForHrefHighWithinFrame(unsigned long timeoutMicros) {
  unsigned long startMicros = micros();

  while (micros() - startMicros < timeoutMicros) {
    if (digitalRead(CAMERA_HREF_PIN) == HIGH) {
      return true;
    }

    // If VS toggles during the search, keep looking inside the new frame.
    (void)digitalRead(CAMERA_VSYNC_PIN);
  }

  return false;
}

bool syncToFrameStart() {
  // Do not assume VS polarity here; just wait for one transition and then the
  // next transition so capture starts at a real frame boundary.
  if (!waitForPinTransition(CAMERA_VSYNC_PIN, 1000000UL)) {
    Serial.println("Timed out waiting for VS transition 1.");
    return false;
  }
  if (!waitForPinTransition(CAMERA_VSYNC_PIN, 1000000UL)) {
    Serial.println("Timed out waiting for VS transition 2.");
    return false;
  }
  return true;
}

bool captureSingleGrayscaleFrame(CaptureEdgeMode edgeMode, uint8_t *destinationFrame) {
  if (!syncToFrameStart()) {
    return false;
  }

  if (!waitForHrefHighWithinFrame(1000000UL)) {
    Serial.println("Timed out waiting for first HS high after frame sync.");
    return false;
  }

  for (uint16_t row = 0; row < FRAME_HEIGHT; row++) {
    if (row > 0 && !waitForPinState(CAMERA_HREF_PIN, HIGH, 200000UL)) {
      Serial.printf("Timed out waiting for HS high on row %u.\n", row);
      return false;
    }

    for (uint16_t i = 0; i < FRAME_WIDTH * 2; i++) {
      if (!waitForPclkEdge(edgeMode, 100000UL)) {
        Serial.printf("Timed out waiting for byte on row %u index %u.\n", row, i);
        return false;
      }
      lineBuffer[i] = readDataBusByte();
    }

    if (!waitForPinState(CAMERA_HREF_PIN, LOW, 200000UL)) {
      Serial.printf("Timed out waiting for HS low on row %u.\n", row);
      return false;
    }

    unsigned long evenScore = 0;
    unsigned long oddScore = 0;

    for (uint16_t col = 0; col < FRAME_WIDTH; col++) {
      evenScore += abs((int)lineBuffer[col * 2] - 128);
      oddScore += abs((int)lineBuffer[col * 2 + 1] - 128);
    }

    int yOffset = (oddScore > evenScore) ? 1 : 0;

    for (uint16_t col = 0; col < FRAME_WIDTH; col++) {
      destinationFrame[row * FRAME_WIDTH + col] = lineBuffer[col * 2 + yOffset];
    }
  }

  return true;
}

bool discardSingleGrayscaleFrame(CaptureEdgeMode edgeMode) {
  if (!syncToFrameStart()) {
    return false;
  }

  if (!waitForHrefHighWithinFrame(1000000UL)) {
    return false;
  }

  for (uint16_t row = 0; row < FRAME_HEIGHT; row++) {
    if (row > 0 && !waitForPinState(CAMERA_HREF_PIN, HIGH, 200000UL)) {
      return false;
    }

    for (uint16_t i = 0; i < FRAME_WIDTH * 2; i++) {
      if (!waitForPclkEdge(edgeMode, 100000UL)) {
        return false;
      }
      (void)readDataBusByte();
    }

    if (!waitForPinState(CAMERA_HREF_PIN, LOW, 200000UL)) {
      return false;
    }
  }

  return true;
}

void letAutoExposureSettle(CaptureEdgeMode edgeMode) {
  const char *edgeLabel = edgeMode == CAPTURE_ON_RISING_EDGE ? "rising" : "falling";
  Serial.printf("Settling auto exposure for %s-edge capture...\n", edgeLabel);
  delay(500);

  for (int i = 0; i < 3; i++) {
    if (!discardSingleGrayscaleFrame(edgeMode)) {
      Serial.printf("%s-edge warm-up frame %d discard failed.\n", edgeLabel, i + 1);
      return;
    }
    Serial.printf("Discarded %s-edge warm-up frame %d.\n", edgeLabel, i + 1);
  }
}

void dumpFrameAsAsciiPgm() {
  Serial.println("BEGIN_PGM");
  Serial.println("P2");
  Serial.printf("%u %u\n", COMPARISON_WIDTH, FRAME_HEIGHT);
  Serial.println("255");

  for (uint16_t row = 0; row < FRAME_HEIGHT; row++) {
    for (uint16_t col = 0; col < FRAME_WIDTH; col++) {
      Serial.print(risingFrameBuffer[row * FRAME_WIDTH + col]);
      Serial.print(' ');
    }
    for (uint16_t col = 0; col < FRAME_WIDTH; col++) {
      Serial.print(fallingFrameBuffer[row * FRAME_WIDTH + col]);
      if (col + 1 < FRAME_WIDTH) {
        Serial.print(' ');
      }
    }
    Serial.println();
  }

  Serial.println("END_PGM");
}

void printPinSummary() {
  Serial.printf("SDA=%d SCL=%d XLK=%d\n", CAMERA_SIOD_PIN, CAMERA_SIOC_PIN, CAMERA_XCLK_PIN);
  Serial.printf("VS=%d HS=%d PLK=%d\n", CAMERA_VSYNC_PIN, CAMERA_HREF_PIN, CAMERA_PCLK_PIN);
  Serial.printf(
    "D0=%d(VP) D1=%d(VN) D2=%d D3=%d D4=%d D5=%d D6=%d D7=%d\n",
    CAMERA_D0_PIN,
    CAMERA_D1_PIN,
    CAMERA_D2_PIN,
    CAMERA_D3_PIN,
    CAMERA_D4_PIN,
    CAMERA_D5_PIN,
    CAMERA_D6_PIN,
    CAMERA_D7_PIN
  );
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Starting ESP32 OV7670 phase-3 single-frame capture");
  printPinSummary();

  configureOptionalControlPins();
  Wire.begin(CAMERA_SIOD_PIN, CAMERA_SIOC_PIN);
  startCameraClock(CAMERA_XCLK_PROBE_FREQUENCY);
  delay(50);

  if (!probeCameraIdentity()) {
    return;
  }

  if (!writeCameraRegister(activeCameraAddress, REG_COM7, 0x80)) {
    Serial.println("Camera reset command did not ACK. Continuing without reset.");
  } else {
    Serial.printf("Camera reset command ACKed at address 0x%02X.\n", activeCameraAddress);
    delay(200);
  }

  if (!applyRegisterTable(QQVGA_YUV_REGISTERS, sizeof(QQVGA_YUV_REGISTERS) / sizeof(QQVGA_YUV_REGISTERS[0]))) {
    Serial.println("Failed to apply QQVGA YUV register table.");
    return;
  }

  configureParallelPins();
  startCameraClock(CAMERA_XCLK_CAPTURE_FREQUENCY);
  delay(300);

  Serial.printf("Capture clock switched to %lu Hz.\n", CAMERA_XCLK_CAPTURE_FREQUENCY);
  cameraReady = true;
  Serial.println("Ready. Send 'c' over serial to capture one frame.");
}

void loop() {
  if (!cameraReady) {
    delay(500);
    return;
  }

  if (Serial.available() <= 0) {
    delay(50);
    return;
  }

  int incoming = Serial.read();
  if (incoming != 'c' && incoming != 'C') {
    return;
  }

  frameDumped = false;
  letAutoExposureSettle(CAPTURE_ON_RISING_EDGE);
  Serial.println("Capturing rising-edge 160x120 grayscale frame...");

  if (!captureSingleGrayscaleFrame(CAPTURE_ON_RISING_EDGE, risingFrameBuffer)) {
    Serial.println("Rising-edge frame capture failed.");
    return;
  }

  letAutoExposureSettle(CAPTURE_ON_FALLING_EDGE);
  Serial.println("Capturing falling-edge 160x120 grayscale frame...");

  if (!captureSingleGrayscaleFrame(CAPTURE_ON_FALLING_EDGE, fallingFrameBuffer)) {
    Serial.println("Falling-edge frame capture failed.");
    return;
  }

  Serial.println("Dual-edge frame capture succeeded. Left=rising, right=falling.");
  dumpFrameAsAsciiPgm();
  frameDumped = true;
  Serial.println("Ready. Send 'c' over serial to capture another dual-edge frame.");
}
