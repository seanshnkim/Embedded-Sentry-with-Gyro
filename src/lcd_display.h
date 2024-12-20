#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H
#include "LCD_DISCO_F429ZI.h"

class LCDDisplay {
public:
    LCDDisplay();
    void init();
    void displayMessage(const char* message);
    void displayMultilineMessage(const char* message1, const char* message2);
    void displayBlinkMessage(const char* message, int times, int delayMs);
    void displayWithSmile(const char* message);
    void displayWithSadFace(const char* message);
    void clear();

private:
    LCD_DISCO_F429ZI lcd;
};

#endif // LCD_DISPLAY_H
