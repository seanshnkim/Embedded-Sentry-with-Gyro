#include "lcd_display.h"
#include "gyroscope.h"

// volatile bool isRecording = false;
volatile bool isEnteringKey = false;
volatile uint32_t lastButtonPress = 0;
volatile uint32_t buttonPressStartTime = 0;

InterruptIn userButton(BUTTON1);
DigitalOut led(LED1);
LCDDisplay display;

void startEnterKey();
void stopEnterKey();

void startEnterKey() {
    isEnteringKey = true;
    display.displayMessage("Enter Key");
}

void stopEnterKey() {
    isEnteringKey = false;
    display.displayMessage("Stop Enter Key");
    // lcd.Clear(LCD_COLOR_BLACK);
}

// Button interrupt handler
void buttonCallback() {
    uint32_t currentTime = us_ticker_read() / 1000; // Convert to milliseconds

    if (currentTime - lastButtonPress > 200) {
        lastButtonPress = currentTime;
        if (!isRecording) {
            // Inside this startRecording() function, `isRecording` is set true
            startRecording();
            isRecording = true;
            display.displayMessage("Recording...");
        } else {
            // Inside stopRecording() function, `isRecording` is set false
            stopRecording();
            isRecording = false;
            display.displayMessage("Recording Complete");
        }
    }
}

int main() {
    display.init();
    init_gyroscope();

    // Attach interrupt handler for falling edges
    userButton.fall(buttonCallback);
    
    display.displayMessage("Press button to start");

    while (1) {
        ThisThread::sleep_for(100ms);
    }
}