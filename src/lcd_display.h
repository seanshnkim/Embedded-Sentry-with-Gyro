#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "LCD_DISCO_F429ZI.h"

class LCDDisplay {
public:
    LCDDisplay();
    void init();
    void displayMessage(const char* message);

private:
    LCD_DISCO_F429ZI lcd;
};

#endif // LCD_DISPLAY_H