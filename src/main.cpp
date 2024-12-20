#include "mbed.h"
#include "lcd_display.h"
#include "gyroscope.h"
#include "state.h" 
#include <string>

State currentState = IDLE;

volatile bool buttonPressed = false;
volatile uint32_t buttonPressStartTime = 0;
volatile uint32_t lastButtonPressTime = 0;

InterruptIn userButton(BUTTON1);
DigitalOut led(LED1);
LCDDisplay display;
Thread samplingThread;

EventQueue eventQueue;
Mutex stateMutex;  // mutex for protecting state transitions

int lenKeyGest = 0;
int lenEnteredGest = 0;

volatile bool isComparing = false;
volatile bool isUnlocked = false;

// Event handlers for button actions
void startRecording() {
    stateMutex.lock();
    currentState = RECORDING;
    stateMutex.unlock();
    display.displayMessage("Recording the key...");
}

void stopRecording() {
    stateMutex.lock();
    currentState = IDLE;
    stateMutex.unlock();
    display.displayMessage("Recording stopped.");
}

void startEntering() {
    stateMutex.lock();
    currentState = ENTERING;
    stateMutex.unlock();
    display.displayMessage("Entering the key...");
}

void stopEntering() {
    stateMutex.lock();
    currentState = COMPARING;
    stateMutex.unlock();
    display.displayMessage("Comparing...");
}

void buttonPressedCallback() {
    buttonPressStartTime = us_ticker_read() / 1000; // Button press start time
}

void buttonReleasedCallback() {
    uint32_t currentTime = us_ticker_read() / 1000;
    uint32_t pressDuration = currentTime - buttonPressStartTime;

    if (currentTime - lastButtonPressTime < 200) {
        return;  // Debounce
    }
    lastButtonPressTime = currentTime;

    if (pressDuration > 1000) {  // Long press
        if (currentState == IDLE) eventQueue.call(startRecording);
        else if (currentState == RECORDING) eventQueue.call(stopRecording);
    } else {  // Short press
        if (currentState == IDLE) eventQueue.call(startEntering);
        else if (currentState == ENTERING) eventQueue.call(stopEntering);
    }
}

void samplingThreadFunc() {
    while (1) {
        stateMutex.lock();
        State localState = currentState;
        stateMutex.unlock();

        switch (localState) {
            case RECORDING:
                printf("Recording gesture...\n");
                sample_gyro_data(keyGesture);
                lenKeyGest = gestureIndex;  // Update gesture length
                printf("Recorded length: %d\n", lenKeyGest);
                break;

            case ENTERING:
                printf("Entering gesture...\n");
                sample_gyro_data(enteredGesture);
                lenEnteredGest = gestureIndex;  // Update gesture length
                printf("Entered length: %d\n", lenEnteredGest);
                break;

            case COMPARING: {
                if (lenKeyGest == 0 || lenEnteredGest == 0) {
                    display.displayMessage("Invalid gesture length");
                    stateMutex.lock();
                    currentState = IDLE;
                    stateMutex.unlock();
                    break;
                }
                printf("Before DTW: KeyLen=%d, EnteredLen=%d\n", lenKeyGest, lenEnteredGest);
                float dtwDist = dtw(keyGesture, enteredGesture, lenKeyGest, lenEnteredGest);
                printf("After DTW: DTW Distance=%.2f\n", dtwDist);
                if (dtwDist < 50.0f) {
                    display.displayWithSmile("Unlocked!");
                    led = 1;
                    ThisThread::sleep_for(1s);
                    led = 0;
                } else {
                    display.displayWithSadFace("Try Again.");
                    ThisThread::sleep_for(1s);  // Short delay for readability
                }

                stateMutex.lock();
                currentState = IDLE;
                stateMutex.unlock();
                break;
            }
            default:
                break;
        }
        // printf("Current gesture length: %d\n", gestureIndex);
        ThisThread::sleep_for(100ms);
    }
}

int main() {
    display.init();
    display.displayMessage("Press to start");
    
    init_gyroscope();
    
    userButton.rise(buttonPressedCallback);
    userButton.fall(buttonReleasedCallback);
    
    samplingThread.start(samplingThreadFunc);
    eventQueue.dispatch_forever();

    while (1) {
        ThisThread::sleep_for(500ms);
    }
}