#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;

const int LCD_SDA_PIN = 21;
const int LCD_SCL_PIN = 22;

const uint8_t LCD_I2C_ADDRESS = 0x27;

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

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
  writeLine(1, "ESP32 I2C LCD");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting ESP32 I2C 1602A LCD sketch");
  Serial.printf(
    "Using SDA=%d SCL=%d address=0x%02X\n",
    LCD_SDA_PIN,
    LCD_SCL_PIN,
    LCD_I2C_ADDRESS
  );

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.init();
  lcd.backlight();
  showHelloWorld();
}

void loop() {}
