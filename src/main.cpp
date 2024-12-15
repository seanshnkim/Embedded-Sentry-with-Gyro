#include "mbed.h"
#include "lcd_display.h"
#include "gyroscope.h"

volatile bool startSettingKey = false;
volatile bool stopSettingKey = false;
volatile bool startEntering = false;
volatile bool stopEntering = false;

volatile bool isComparing = false;
volatile bool isUnlocked = false;

volatile uint32_t lastButtonPress = 0;
volatile uint32_t buttonPressStartTime = 0;

InterruptIn userButton(BUTTON1);
DigitalOut led(LED1);
LCDDisplay display;
Thread samplingThread;

int lenKeyGest = 0;
int lenEnteredGest = 0;

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
                if (!isEntering) {
                    startEntering = true;
                } else {
                    stopEntering = true;
                }
            }
        }
    }
}

void samplingThreadFunc() {
    while (1) {
        if (isEntering) {
            sample_gyro_data(enteredGesture);
            lenEnteredGest = gestureIndex;
        }
        if (isRecording) {
            sample_gyro_data(keyGesture);
            lenKeyGest = gestureIndex;
        }
        if (isComparing) {
            isUnlocked = compareGest(keyGesture, enteredGesture, lenKeyGest, lenEnteredGest);
            isComparing = false;
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
            display.displayMessage("Recording the key...");
            startSettingKey = false;
        }
        if (stopSettingKey) {
            isRecording = false;
            display.displayMessage("Recording complete");
            stopSettingKey = false;
        }
        if (startEntering) {
            isEntering = true;
            display.displayMessage("Enter the key!");
            startEntering = false;
        }
        if (stopEntering) {
            isEntering = false;
            display.displayMessage("Entered complete");
            stopEntering = false;
            isComparing = true;
        }
        if (isUnlocked) {
            display.displayMessage("Unlocked");
            isUnlocked = false;
        }
        else {
            display.displayMessage("Wrong key!");
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
        if (isEntering) {
            // Entered gesture data
            printf("Entered Gesture Data: gInd=%d, gx=%.5f, gy=%.5f, gz=%.5f\n",
                               gestureIndex,
                               enteredGesture[gestureIndex-1].x,
                               enteredGesture[gestureIndex-1].y,
                               enteredGesture[gestureIndex-1].z);
        }
        if (isComparing) {
            
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