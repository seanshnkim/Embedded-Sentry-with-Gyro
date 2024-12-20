#include "lcd_display.h"

LCDDisplay::LCDDisplay() {}

void LCDDisplay::init() {
    lcd.Init();
    lcd.Clear(LCD_COLOR_BLACK);
}

void LCDDisplay::displayMessage(const char* message) {
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetFont(&Font16);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)message, CENTER_MODE);
}

void LCDDisplay::clear() {
    lcd.Clear(LCD_COLOR_BLACK);
}

void LCDDisplay::displayMultilineMessage(const char* message1, const char* message2) {
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetFont(&Font16);
    lcd.DisplayStringAt(0, LINE(4), (uint8_t *)message1, CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(6), (uint8_t *)message2, CENTER_MODE);
}

void LCDDisplay::displayBlinkMessage(const char* message, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        displayMessage(message);
        ThisThread::sleep_for(chrono::milliseconds(delayMs));
        clear();
        ThisThread::sleep_for(chrono::milliseconds(delayMs));
    }
}

void LCDDisplay::displayWithSmile(const char* message) {
    // Clear the screen and set the background color
    lcd.Clear(LCD_COLOR_BLACK);

    // Draw the face (yellow circle)
    lcd.SetTextColor(LCD_COLOR_YELLOW);
    lcd.FillCircle(120, 120, 80);

    // Draw the eyes (black circles)
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillCircle(90, 90, 10);  // Left eye
    lcd.FillCircle(150, 90, 10); // Right eye

    // Draw the smile (red arc)
    lcd.SetTextColor(LCD_COLOR_RED);
    Point smile[] = {{80, 140}, {120, 160}, {160, 140}};
    lcd.FillPolygon(smile, 3);

    // Display target ("Unlocked successfully!") message
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetFont(&Font24);
    lcd.DisplayStringAt(0, 220, (uint8_t *)message, CENTER_MODE);
}

void LCDDisplay::displayWithSadFace(const char* message) {
    // Clear the screen and set the background color
    lcd.Clear(LCD_COLOR_BLACK);

    // Draw the face (yellow circle)
    lcd.SetTextColor(LCD_COLOR_YELLOW);
    lcd.FillCircle(120, 120, 80);

    // Draw angry eyes (black triangles)
    lcd.SetTextColor(LCD_COLOR_BLACK);
    
    // Left eye (angry triangle)
    Point leftEye[] = {{80, 100}, {100, 80}, {100, 100}};
    lcd.FillPolygon(leftEye, 3);
    
    // Right eye (angry triangle)
    Point rightEye[] = {{140, 80}, {160, 100}, {140, 100}};
    lcd.FillPolygon(rightEye, 3);

    // Draw the frown (red arc)
    lcd.SetTextColor(LCD_COLOR_RED);
    Point frown[] = {{80, 160}, {120, 140}, {160, 160}};
    lcd.FillPolygon(frown, 3);

    // Display target ("Try Again.") message
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetFont(&Font24);
    lcd.DisplayStringAt(0, 220, (uint8_t *)message, CENTER_MODE);
}