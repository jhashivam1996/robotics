#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;

const int LCD_SDA_PIN = 21;
const int LCD_SCL_PIN = 22;

const uint8_t LCD_I2C_ADDRESS_1 = 0x27;
const uint8_t LCD_I2C_ADDRESS_2 = 0x3F;

LiquidCrystal_I2C lcd27(LCD_I2C_ADDRESS_1, LCD_COLUMNS, LCD_ROWS);
LiquidCrystal_I2C lcd3f(LCD_I2C_ADDRESS_2, LCD_COLUMNS, LCD_ROWS);
LiquidCrystal_I2C* lcd = nullptr;

const char kHeaderMessage[] = "HARYANA ROADWAYS";
const char kMarqueeMessage[] = "YAMUNANAGAR to DELHI via PANIPAT KARNAL";
const char kMarqueeGap[] = "   ";
const unsigned long kScrollIntervalMs = 400;

unsigned long lastScrollMs = 0;
size_t scrollIndex = 0;

bool devicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool selectLcd() {
  if (devicePresent(LCD_I2C_ADDRESS_1)) {
    lcd = &lcd27;
    Serial.printf("LCD detected at address 0x%02X\n", LCD_I2C_ADDRESS_1);
    return true;
  }

  if (devicePresent(LCD_I2C_ADDRESS_2)) {
    lcd = &lcd3f;
    Serial.printf("LCD detected at address 0x%02X\n", LCD_I2C_ADDRESS_2);
    return true;
  }

  lcd = nullptr;
  return false;
}

void writeLine(uint8_t row, const char* text) {
  if (lcd == nullptr) {
    return;
  }

  lcd->setCursor(0, row);

  int column = 0;
  while (column < LCD_COLUMNS && text[column] != '\0') {
    lcd->print(text[column]);
    column++;
  }

  while (column < LCD_COLUMNS) {
    lcd->print(' ');
    column++;
  }
}

void writeWindow(uint8_t row, const char* message, size_t startIndex) {
  if (lcd == nullptr) {
    return;
  }

  const String scrollText = String(message) + kMarqueeGap;
  const size_t messageLength = scrollText.length();
  if (messageLength == 0) {
    writeLine(row, "");
    return;
  }

  lcd->setCursor(0, row);
  for (int column = 0; column < LCD_COLUMNS; column++) {
    const size_t sourceIndex = (startIndex + column) % messageLength;
    lcd->print(scrollText[sourceIndex]);
  }
}

void updateMarquee() {
  writeLine(0, kHeaderMessage);
  writeWindow(1, kMarqueeMessage, scrollIndex);

  const size_t totalLength = strlen(kMarqueeMessage) + strlen(kMarqueeGap);
  scrollIndex = (scrollIndex + 1) % totalLength;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting ESP32 I2C 1602A LCD sketch");
  Serial.printf(
    "Using SDA=%d SCL=%d trying addresses 0x%02X and 0x%02X\n",
    LCD_SDA_PIN,
    LCD_SCL_PIN,
    LCD_I2C_ADDRESS_1,
    LCD_I2C_ADDRESS_2
  );

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

  if (!selectLcd()) {
    Serial.println("No LCD found at 0x27 or 0x3F");
    return;
  }

  lcd->init();
  lcd->backlight();
  lcd->clear();
  updateMarquee();
}

void loop() {
  if (lcd == nullptr) {
    delay(250);
    return;
  }

  if (millis() - lastScrollMs >= kScrollIntervalMs) {
    lastScrollMs = millis();
    updateMarquee();
  }
}
