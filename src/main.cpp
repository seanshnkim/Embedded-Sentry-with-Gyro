#include "mbed.h"
#include "lcd_display.h"
#include "gyroscope.h"

volatile bool startSettingKey = false;
volatile bool stopSettingKey = false;
volatile bool startUnlocking = false;
volatile bool stopUnlocking = false;

volatile uint32_t lastButtonPress = 0;
volatile uint32_t buttonPressStartTime = 0;

InterruptIn userButton(BUTTON1);
DigitalOut led(LED1);
LCDDisplay display;
Thread samplingThread;

// Button interrupt handler
void buttonCallback() {
    uint32_t currentTime = us_ticker_read() / 1000;
    if (userButton.read() == 1) {
        // If button is pressed
        buttonPressStartTime = currentTime;
    } else {
        // If button is released
        uint32_t pressDuration = currentTime - buttonPressStartTime;
        if (currentTime - lastButtonPress > 200) {
            lastButtonPress = currentTime;
            if (pressDuration > 1000) {
                // Long press - Record a key (reference)
                if (!isRecording) {
                    startSettingKey = true;
                } else {
                    stopSettingKey = true;
                }
            } else {
                // Short press - Enter a gesture to unlock the device
                if (!isUnlocking) {
                    startUnlocking = true;
                } else {
                    stopUnlocking = true;
                }
            }
        }
    }
}

void samplingThreadFunc() {
    while (1) {
        if (isUnlocking) {
            sample_gyro_data(enteredGesture);
        }
        if (isRecording) {
            sample_gyro_data(keyGesture);
        }
    }
}

int main() {
    display.init();
    init_gyroscope();
    
    // rise is also necessary to detect the button release (else-statement)
    userButton.rise(buttonCallback);
    userButton.fall(buttonCallback);

    samplingThread.start(samplingThreadFunc);

    display.displayMessage("Press button to start");

    while (1) {
        if (startSettingKey) {
            isRecording = true;
            gestureIndex = 0;
            display.displayMessage("Recording the key...");
            startSettingKey = false;
        }
        if (stopSettingKey) {
            isRecording = false;
            display.displayMessage("Recording complete");
            stopSettingKey = false;
        }
        if (startUnlocking) {
            isUnlocking = true;
            gestureIndex = 0;
            // gestureIndex = 0;
            display.displayMessage("Unlocking...");
            startUnlocking = false;
        }
        if (stopUnlocking) {
            isUnlocking = false;
            display.displayMessage("Unlocked complete");
            stopUnlocking = false;
        }
        // For debugging, print out gyroscope data
        if (isRecording) {
            // Key gesture data
            printf("Key Gesture Data: gInd=%d, gx=%.5f, gy=%.5f, gz=%.5f\n",
                               gestureIndex,
                               keyGesture[gestureIndex-1].x,
                               keyGesture[gestureIndex-1].y,
                               keyGesture[gestureIndex-1].z);
        }
        if (isUnlocking) {
            // Entered gesture data
            printf("Entered Gesture Data: gInd=%d, gx=%.5f, gy=%.5f, gz=%.5f\n",
                               gestureIndex,
                               enteredGesture[gestureIndex-1].x,
                               enteredGesture[gestureIndex-1].y,
                               enteredGesture[gestureIndex-1].z);
            }
        ThisThread::sleep_for(100ms);
        
        // If time passed 10 seconds, stop samplingThread
        // Lack of documentation, I guess us_ticker_read() increments the counter by 1 every 1us
        // if (us_ticker_read() > 1'000'000'0) {
        //     display.displayMessage("Timeout");
        //     samplingThread.terminate();
        //     break;
        // }
    }
    return 0;
}