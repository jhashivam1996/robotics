#include <Wire.h>

const uint8_t OV7670_SCCB_ADDRESSES[] = {0x21, 0x42};

// One-sided bring-up wiring for ESP32 DevKit V1.
const int CAMERA_SIOD_PIN = 18;
const int CAMERA_SIOC_PIN = 19;
const int CAMERA_XCLK_PIN = 21;

const int CAMERA_VSYNC_PIN = -1;
const int CAMERA_PCLK_PIN = -1;
const int CAMERA_HREF_PIN = -1;

const int CAMERA_D0_PIN = -1;
const int CAMERA_D1_PIN = -1;
const int CAMERA_D2_PIN = -1;
const int CAMERA_D3_PIN = -1;
const int CAMERA_D4_PIN = -1;
const int CAMERA_D5_PIN = -1;
const int CAMERA_D6_PIN = -1;
const int CAMERA_D7_PIN = -1;

const int CAMERA_RESET_PIN = -1;
const int CAMERA_PWDN_PIN = -1;

const uint32_t CAMERA_XCLK_FREQUENCY = 10000000UL;
const uint8_t CAMERA_XCLK_RESOLUTION_BITS = 1;

const uint8_t REG_PID = 0x0A;
const uint8_t REG_VER = 0x0B;
const uint8_t REG_MIDH = 0x1C;
const uint8_t REG_MIDL = 0x1D;
uint8_t activeCameraAddress = 0;

bool writeCameraRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readCameraRegister(uint8_t address, uint8_t reg, uint8_t &value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  // OV7670 SCCB is often happier with a STOP between register select and read.
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

void startCameraClock() {
  ledcAttach(CAMERA_XCLK_PIN, CAMERA_XCLK_FREQUENCY, CAMERA_XCLK_RESOLUTION_BITS);
  ledcWrite(CAMERA_XCLK_PIN, 1);
}

void printPinSummary() {
  Serial.printf("SIOD=%d SIOC=%d XCLK=%d\n", CAMERA_SIOD_PIN, CAMERA_SIOC_PIN, CAMERA_XCLK_PIN);
  Serial.println("This bring-up sketch does not use VSYNC/PCLK/HREF/D0-D7.");
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
  bool sawAck = false;

  for (uint8_t i = 0; i < sizeof(OV7670_SCCB_ADDRESSES); i++) {
    uint8_t address = OV7670_SCCB_ADDRESSES[i];
    Serial.printf("Probing SCCB address 0x%02X...\n", address);

    if (!cameraAddressAck(address)) {
      Serial.printf("Address 0x%02X did not ACK.\n", address);
      continue;
    }

    sawAck = true;
    Serial.printf("Address 0x%02X ACKed.\n", address);

    if (probeCameraIdentityAtAddress(address)) {
      return true;
    }
  }

  if (!sawAck) {
    Serial.println("No SCCB address ACKed. Check SDA/SCL wiring, pull-ups, power, and XCLK.");
  }

  return false;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting ESP32 OV7670 camera bring-up");
  printPinSummary();

  configureOptionalControlPins();
  Wire.begin(CAMERA_SIOD_PIN, CAMERA_SIOC_PIN);
  startCameraClock();
  delay(50);

  if (!probeCameraIdentity()) {
    Serial.println("Camera identity probe failed.");
  }

  // Harmless write/read sanity check for SCCB comms.
  if (activeCameraAddress == 0) {
    Serial.println("Skipping reset command because no camera address was confirmed.");
  } else if (!writeCameraRegister(activeCameraAddress, 0x12, 0x80)) {
    Serial.println("Camera reset command did not ACK.");
  } else {
    Serial.printf("Camera reset command ACKed at address 0x%02X.\n", activeCameraAddress);
  }
}

void loop() {
  delay(1000);
}
