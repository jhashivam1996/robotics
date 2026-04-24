#include <LiquidCrystal.h>

const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;

// Bare 1602A LCD in 4-bit mode.
const int LCD_RS_PIN = 14;
const int LCD_ENABLE_PIN = 27;
const int LCD_D4_PIN = 26;
const int LCD_D5_PIN = 25;
const int LCD_D6_PIN = 33;
const int LCD_D7_PIN = 32;

LiquidCrystal lcd(
  LCD_RS_PIN,
  LCD_ENABLE_PIN,
  LCD_D4_PIN,
  LCD_D5_PIN,
  LCD_D6_PIN,
  LCD_D7_PIN
);

void writeLine(uint8_t row, const char* text) {
  lcd.setCursor(0, row);

  int column = 0;
  while (column < LCD_COLUMNS && text[column] != '\0') {
    lcd.print(text[column]);
    column++;
  }

  while (column < LCD_COLUMNS) {
    lcd.print(' ');
    column++;
  }
}

void showHelloWorld() {
  lcd.clear();
  writeLine(0, "Hello World");
  writeLine(1, "ESP32 + 1602A");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting ESP32 bare 1602A LCD sketch");
  Serial.printf(
    "Using RS=%d E=%d D4=%d D5=%d D6=%d D7=%d\n",
    LCD_RS_PIN,
    LCD_ENABLE_PIN,
    LCD_D4_PIN,
    LCD_D5_PIN,
    LCD_D6_PIN,
    LCD_D7_PIN
  );

  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  showHelloWorld();
}

void loop() {}
