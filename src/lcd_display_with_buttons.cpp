#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"

volatile bool isRecording = false;
volatile bool isEnteringKey = false;
volatile uint32_t lastButtonPress = 0;
volatile uint32_t buttonPressStartTime = 0;

InterruptIn userButton(BUTTON1);
LCD_DISCO_F429ZI lcd;
DigitalOut led(LED1);

void displayMessage(const char* message);
void startRecording();
void stopRecording();
void startEnterKey();
void stopEnterKey();

void displayMessage(const char* message) {
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetFont(&Font16);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)message, CENTER_MODE);
}

void startRecording() {
    isRecording = true;
    displayMessage("Recording...");
}

void stopRecording() {
    isRecording = false;
    displayMessage("Recording Complete");
    // In the middle of interrupt, ThisThrea::sleep_for() is not allowed. 
    lcd.Clear(LCD_COLOR_BLACK);
}

void startEnterKey() {
    isEnteringKey = true;
    displayMessage("Enter Key");
}

void stopEnterKey() {
    isEnteringKey = false;
    displayMessage("Stop Enter Key");
    lcd.Clear(LCD_COLOR_BLACK);
}

// Button interrupt handler
void buttonCallback() {
    uint32_t currentTime = us_ticker_read() / 1000; // Convert to milliseconds

    if (currentTime - lastButtonPress > 200) {
        lastButtonPress = currentTime;
        if (!isRecording) {
            // Inside this startRecording() function, `isRecording` is set true
            startRecording();
        } else {
            // Inside stopRecording() function, `isRecording` is set false
            stopRecording();
        }
    }
}

int main() {
    lcd.Init();

    // Attach interrupt handler for falling edges
    userButton.fall(buttonCallback);
    
    displayMessage("Press button to start");

    while (1) {
        ThisThread::sleep_for(100ms);
    }
}