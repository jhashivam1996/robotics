#include <Arduino.h>

namespace {
constexpr int kMicAnalogPin = 34;
constexpr int kMicDigitalPin = 27;
constexpr int kSpeakerPin = 25;
constexpr uint8_t kSpeakerPwmResolutionBits = 8;
constexpr float kThresholdStep = 0.01f;
constexpr float kMediumLevelMargin = 5.0f;
constexpr float kHighLevelMargin = 10.0f;

constexpr unsigned long kStatusIntervalMs = 200;
constexpr unsigned long kPeakHoldMs = 1500;
constexpr unsigned long kTriggerCooldownMs = 700;

struct ToneStep {
  uint16_t frequencyHz;
  uint16_t durationMs;
};

constexpr ToneStep kLowPattern[] = {
  {523, 80},
  {0, 80},
};

constexpr ToneStep kMediumPattern[] = {
  {659, 90},
  {0, 60},
  {784, 110},
  {0, 80},
};

constexpr ToneStep kHighPattern[] = {
  {880, 90},
  {0, 45},
  {1175, 90},
  {0, 45},
  {1568, 140},
  {0, 100},
};

bool digitalTriggerEnabled = true;
bool speakerEnabled = true;
float analogThreshold = 0.5f;
int peakValue = 0;
unsigned long peakCapturedMs = 0;
unsigned long lastStatusMs = 0;
unsigned long lastTriggerMs = 0;

const ToneStep *activePattern = nullptr;
size_t activePatternLength = 0;
size_t activePatternIndex = 0;
unsigned long activeStepStartedMs = 0;

void stopSpeaker() {
  ledcWriteTone(kSpeakerPin, 0);
  activePattern = nullptr;
  activePatternLength = 0;
  activePatternIndex = 0;
}

void startPattern(const ToneStep *pattern, size_t patternLength, const __FlashStringHelper *label) {
  if (!speakerEnabled || pattern == nullptr || patternLength == 0) {
    return;
  }

  activePattern = pattern;
  activePatternLength = patternLength;
  activePatternIndex = 0;
  activeStepStartedMs = millis();
  ledcWriteTone(kSpeakerPin, activePattern[0].frequencyHz);
  Serial.print(F("Speaker pattern: "));
  Serial.println(label);
}

void updateSpeaker() {
  if (activePattern == nullptr || activePatternIndex >= activePatternLength) {
    return;
  }

  if (millis() - activeStepStartedMs < activePattern[activePatternIndex].durationMs) {
    return;
  }

  activePatternIndex++;
  activeStepStartedMs = millis();

  if (activePatternIndex >= activePatternLength) {
    stopSpeaker();
    return;
  }

  ledcWriteTone(kSpeakerPin, activePattern[activePatternIndex].frequencyHz);
}

void triggerSpeakerForLevel(int analogValue) {
  if (!speakerEnabled || activePattern != nullptr || millis() - lastTriggerMs < kTriggerCooldownMs) {
    return;
  }

  if (analogValue >= analogThreshold + kHighLevelMargin) {
    startPattern(kHighPattern, sizeof(kHighPattern) / sizeof(kHighPattern[0]), F("high"));
    lastTriggerMs = millis();
    return;
  }

  if (analogValue >= analogThreshold + kMediumLevelMargin) {
    startPattern(kMediumPattern, sizeof(kMediumPattern) / sizeof(kMediumPattern[0]), F("medium"));
    lastTriggerMs = millis();
    return;
  }

  if (analogValue >= analogThreshold) {
    startPattern(kLowPattern, sizeof(kLowPattern) / sizeof(kLowPattern[0]), F("low"));
    lastTriggerMs = millis();
  }
}

void printHelp() {
  Serial.println();
  Serial.println(F("ESP32 + HW-484 sound detector + beeper controls"));
  Serial.println(F("  h: show help"));
  Serial.println(F("  a: toggle digital trigger reporting"));
  Serial.println(F("  b: toggle speaker beeps"));
  Serial.println(F("  t: raise analog threshold"));
  Serial.println(F("  g: lower analog threshold"));
  Serial.println(F("  i: print current status once"));
  Serial.println(F("  1: test low beep"));
  Serial.println(F("  2: test medium beep"));
  Serial.println(F("  3: test high beep"));
  Serial.println();
}

void printStatus(int analogValue, int digitalValue) {
  Serial.print(F("AO="));
  Serial.print(analogValue);
  Serial.print(F(" DO="));
  Serial.print(digitalValue);
  Serial.print(F(" threshold="));
  Serial.print(analogThreshold, 1);
  Serial.print(F(" peak="));
  Serial.print(peakValue);
  Serial.print(F(" trigger="));
  Serial.print(digitalTriggerEnabled ? F("on") : F("off"));
  Serial.print(F(" speaker="));
  Serial.println(speakerEnabled ? F("on") : F("off"));
}

void handleCommand(char command, int analogValue, int digitalValue) {
  switch (command) {
    case 'h':
    case '?':
      printHelp();
      break;
    case 'a':
      digitalTriggerEnabled = !digitalTriggerEnabled;
      Serial.print(F("Digital trigger reporting "));
      Serial.println(digitalTriggerEnabled ? F("enabled") : F("disabled"));
      break;
    case 'b':
      speakerEnabled = !speakerEnabled;
      if (!speakerEnabled) {
        stopSpeaker();
      }
      Serial.print(F("Speaker beeps "));
      Serial.println(speakerEnabled ? F("enabled") : F("disabled"));
      break;
    case 't':
      analogThreshold += kThresholdStep;
      if (analogThreshold > 4095.0f) {
        analogThreshold = 4095.0f;
      }
      Serial.print(F("Threshold="));
      Serial.println(analogThreshold, 1);
      break;
    case 'g':
      analogThreshold -= kThresholdStep;
      if (analogThreshold < 0.0f) {
        analogThreshold = 0.0f;
      }
      Serial.print(F("Threshold="));
      Serial.println(analogThreshold, 1);
      break;
    case 'i':
      printStatus(analogValue, digitalValue);
      break;
    case '1':
      startPattern(kLowPattern, sizeof(kLowPattern) / sizeof(kLowPattern[0]), F("low"));
      break;
    case '2':
      startPattern(kMediumPattern, sizeof(kMediumPattern) / sizeof(kMediumPattern[0]), F("medium"));
      break;
    case '3':
      startPattern(kHighPattern, sizeof(kHighPattern) / sizeof(kHighPattern[0]), F("high"));
      break;
    default:
      break;
  }
}

void readCommands(int analogValue, int digitalValue) {
  while (Serial.available() > 0) {
    const char command = static_cast<char>(Serial.read());
    if (command == '\n' || command == '\r') {
      continue;
    }

    handleCommand(command, analogValue, digitalValue);
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(250);

  analogReadResolution(12);
  pinMode(kMicAnalogPin, INPUT);
  pinMode(kMicDigitalPin, INPUT);
  ledcAttach(kSpeakerPin, 1000, kSpeakerPwmResolutionBits);
  stopSpeaker();

  Serial.println();
  Serial.println(F("Starting ESP32 HW-484 sound detector with simple speaker beeps"));
  Serial.println(F("AO reports rough analog sound level."));
  Serial.println(F("DO reports threshold-based digital detection from the onboard comparator."));
  Serial.println(F("This is a sound detector, not a real audio recorder."));
  Serial.println(F("Beep pattern changes with sound intensity above the analog threshold."));
  printHelp();
}

void loop() {
  const int analogValue = analogRead(kMicAnalogPin);
  const int digitalValue = digitalRead(kMicDigitalPin);

  updateSpeaker();
  readCommands(analogValue, digitalValue);

  if (analogValue > peakValue) {
    peakValue = analogValue;
    peakCapturedMs = millis();
  }

  if (peakValue > 0 && millis() - peakCapturedMs >= kPeakHoldMs) {
    peakValue = analogValue;
    peakCapturedMs = millis();
  }

  if (digitalTriggerEnabled && digitalValue == LOW) {
    Serial.print(F("Sound trigger detected: AO="));
    Serial.println(analogValue);
    delay(40);
  }

  if (analogValue >= analogThreshold) {
    Serial.print(F("Analog threshold crossed: AO="));
    Serial.println(analogValue);
    triggerSpeakerForLevel(analogValue);
    delay(40);
  }

  if (millis() - lastStatusMs >= kStatusIntervalMs) {
    lastStatusMs = millis();
    printStatus(analogValue, digitalValue);
  }
}
