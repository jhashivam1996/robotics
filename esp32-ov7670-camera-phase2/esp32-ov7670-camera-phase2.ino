#include <Wire.h>

const uint8_t OV7670_SCCB_ADDRESSES[] = {0x21, 0x42};

const int CAMERA_SIOD_PIN = 18;
const int CAMERA_SIOC_PIN = 19;
const int CAMERA_XCLK_PIN = 21;

const int CAMERA_VSYNC_PIN = 27;
const int CAMERA_HREF_PIN = 14;
const int CAMERA_PCLK_PIN = 13;

const int CAMERA_D0_PIN = 36;
const int CAMERA_D1_PIN = 39;
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

const uint8_t REG_PID = 0x0A;
const uint8_t REG_VER = 0x0B;
const uint8_t REG_MIDH = 0x1C;
const uint8_t REG_MIDL = 0x1D;

uint8_t activeCameraAddress = 0;
unsigned long lastReportAt = 0;

struct BusCaptureSummary {
  unsigned long elapsedMicros;
  int vsyncTransitions;
  int hrefTransitions;
  int pclkTransitions;
  int sampledBytes;
  uint8_t bytes[32];
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

void printPinSummary() {
  Serial.printf("SDA=%d SCL=%d XLK=%d\n", CAMERA_SIOD_PIN, CAMERA_SIOC_PIN, CAMERA_XCLK_PIN);
  Serial.printf("VS=%d HS=%d PLK=%d\n", CAMERA_VSYNC_PIN, CAMERA_HREF_PIN, CAMERA_PCLK_PIN);
  Serial.printf(
    "D0=%d D1=%d D2=%d D3=%d D4=%d D5=%d D6=%d D7=%d\n",
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

BusCaptureSummary captureBusActivity(unsigned long captureMicros) {
  BusCaptureSummary summary = {};
  summary.elapsedMicros = captureMicros;

  int lastVsync = digitalRead(CAMERA_VSYNC_PIN);
  int lastHref = digitalRead(CAMERA_HREF_PIN);
  int lastPclk = digitalRead(CAMERA_PCLK_PIN);

  unsigned long startMicros = micros();

  while (micros() - startMicros < captureMicros) {
    int vsync = digitalRead(CAMERA_VSYNC_PIN);
    int href = digitalRead(CAMERA_HREF_PIN);
    int pclk = digitalRead(CAMERA_PCLK_PIN);

    if (vsync != lastVsync) {
      summary.vsyncTransitions++;
      lastVsync = vsync;
    }

    if (href != lastHref) {
      summary.hrefTransitions++;
      lastHref = href;
    }

    if (pclk != lastPclk) {
      summary.pclkTransitions++;

      if (lastPclk == LOW && pclk == HIGH && href == HIGH && summary.sampledBytes < 32) {
        summary.bytes[summary.sampledBytes] = readDataBusByte();
        summary.sampledBytes++;
      }

      lastPclk = pclk;
    }
  }

  return summary;
}

void printCaptureSummary(const BusCaptureSummary &summary) {
  Serial.printf(
    "Bus activity over %lu us: VS=%d HS=%d PLK=%d sampled=%d\n",
    summary.elapsedMicros,
    summary.vsyncTransitions,
    summary.hrefTransitions,
    summary.pclkTransitions,
    summary.sampledBytes
  );

  if (summary.sampledBytes == 0) {
    Serial.println("No bytes sampled while HS was high.");
    return;
  }

  Serial.print("First sampled bytes:");
  for (int i = 0; i < summary.sampledBytes; i++) {
    Serial.printf(" %02X", summary.bytes[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting ESP32 OV7670 phase-2 bus validation");
  printPinSummary();

  configureOptionalControlPins();
  Wire.begin(CAMERA_SIOD_PIN, CAMERA_SIOC_PIN);
  startCameraClock(CAMERA_XCLK_PROBE_FREQUENCY);
  delay(50);

  if (!probeCameraIdentity()) {
    return;
  }

  if (!writeCameraRegister(activeCameraAddress, 0x12, 0x80)) {
    Serial.println("Camera reset command did not ACK. Continuing with existing camera state.");
  } else {
    Serial.printf("Camera reset command ACKed at address 0x%02X.\n", activeCameraAddress);
    delay(100);
  }

  configureParallelPins();
  startCameraClock(CAMERA_XCLK_CAPTURE_FREQUENCY);
  delay(50);
  Serial.printf("Capture clock switched to %lu Hz.\n", CAMERA_XCLK_CAPTURE_FREQUENCY);
}

void loop() {
  if (activeCameraAddress == 0) {
    delay(1000);
    return;
  }

  if (millis() - lastReportAt < 2000UL) {
    delay(10);
    return;
  }

  lastReportAt = millis();
  BusCaptureSummary summary = captureBusActivity(250000UL);
  printCaptureSummary(summary);
}
