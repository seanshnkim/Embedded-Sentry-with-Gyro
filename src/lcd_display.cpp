#include "lcd_display.h"

LCDDisplay::LCDDisplay() {}

void LCDDisplay::init() {
    lcd.Init();
}

void LCDDisplay::displayMessage(const char* message) {
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetFont(&Font16);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)message, CENTER_MODE);
}