#include "mbed.h"
#include "lcd_display.h"
#include "gyroscope.h"

volatile bool isEnteringKey = false;
volatile bool shouldStartRecording = false;
volatile bool shouldStopRecording = false;
volatile uint32_t lastButtonPress = 0;

InterruptIn userButton(BUTTON1);
DigitalOut led(LED1);
LCDDisplay display;

// Button interrupt handler
void buttonCallback() {
    uint32_t currentTime = us_ticker_read() / 1000;
    if (currentTime - lastButtonPress > 200) {
        lastButtonPress = currentTime;
        if (!isRecording) {
            shouldStartRecording = true;
        } else {
            shouldStopRecording = true;
        }
    }
}

int main() {
    display.init();
    init_gyroscope();
    userButton.fall(buttonCallback);

    display.displayMessage("Press button to start");

    while (1) {
        if (shouldStartRecording) {
            isRecording = true;
            display.displayMessage("Recording...");
            shouldStartRecording = false;
            sample_gyro_data();
        }
        if (shouldStopRecording) {
            isRecording = false;
            display.displayMessage("Recording Complete");
            shouldStopRecording = false;
        }
        if (isRecording && gestureIndex % 10 == 0) {
            // Print the latest data
            printf("Latest data: gInd=%d, gx=%.5f, gy=%.5f, gz=%.5f\n",
                               gestureIndex,
                               recordedGesture[gestureIndex-1].x,
                               recordedGesture[gestureIndex-1].y,
                               recordedGesture[gestureIndex-1].z);
            }
        ThisThread::sleep_for(100ms);
    }
}